use std::collections::HashMap;
use weighted_rand::builder::{NewBuilder, WalkerTableBuilder};
use sqlite::Connection;

static SQL_GETWORD:&str = "select * from words where id = ?";
static SQL_INSERTWORD:&str = "insert into words values (NULL, ?, \"\")";
static SQL_CURRENTWORD:&str = "select * from words order by id desc limit 1";
static SQL_UPDATEFUTURES:&str = "update words set futures = ? where id = ?";
static SQL_GETLOOKUP:&str = "select * from words";
static SQL_INIT:&str = "create table words (id integer primary key, word text, futures text)";

struct MarkovWordReference {
    id:i64,
    occurrences:i64
}

struct MarkovWord {
    id:i64,
    word:String,
    futures:Vec<MarkovWordReference>
}

impl MarkovWord {
    fn from_read(connection:&Connection, id:i64) -> Self {
        let mut statement = connection.prepare(SQL_GETWORD).unwrap();
        statement.bind((1, id)).unwrap();
        statement.next().unwrap();

        let word = statement.read::<String, _>("word").unwrap();

        let mut futures:Vec<MarkovWordReference> = Vec::new();
        let futures_raw = statement.read::<String, _>("futures").unwrap();
        for future_raw in futures_raw.split(",").filter(|&s| !s.is_empty()) {
            let mut parts = future_raw.split(":");
            futures.push(MarkovWordReference {
                id: parts.next().unwrap().parse::<i64>().unwrap(),
                occurrences: parts.next().unwrap().parse::<i64>().unwrap()
            });
        }

        MarkovWord {
            id,
            word,
            futures
        }
    }

    // returns id
    fn create_self(connection:&Connection, word:&str) -> i64 {
        let mut statement = connection.prepare(SQL_INSERTWORD).unwrap();
        statement.bind((1, word)).unwrap();
        statement.next().unwrap();

        // now get the id of the new word
        let mut statement = connection.prepare(SQL_CURRENTWORD).unwrap();
        statement.next().unwrap();

        statement.read::<i64, _>("id").unwrap()
    }

    fn update_self(&self, connection:&Connection) {
        let mut futures = String::new();
        for future in self.futures.iter() {
            futures.push_str(format!("{}:{},", future.id, future.occurrences).as_str());
        }

        let mut statement = connection.prepare(SQL_UPDATEFUTURES).unwrap();
        statement.bind((1, futures.as_str())).unwrap();
        statement.bind((2, self.id)).unwrap();
        statement.next().unwrap();
    }

    fn add_occurrence(&mut self, id:i64) {
        // todo: not stupid search
        let mut found = false;
        for future in self.futures.iter_mut() {
            if future.id == id {
                found = true;
                future.occurrences += 1;
            }
        }

        if !found {
            self.futures.push(MarkovWordReference {
                id,
                occurrences: 1
            });
        }
    }

    fn pick_next(&self, allow_end:bool) -> Option<i64> {
        let mut ids:Vec<i64> = Vec::new();
        let mut weights:Vec<u32> = Vec::new();

        for future in self.futures.iter() {
            if (future.id == -1) && !allow_end {
            } else {
                ids.push(future.id);
                weights.push(future.occurrences as u32);
            }
        }

        if ids.len() == 0 {
            None
        } else {
            let table = WalkerTableBuilder::new(weights.as_slice()).build();
            let id = ids[table.next()];

            if id == -1 {
                None
            } else {
                Some(id)
            }
        }
    }
}

pub struct MarkovOptions {
    allow_early_end:bool
}

pub struct Markov {
    connection:Connection,
    lookup:HashMap<String, i64>
}

impl Markov {
    fn load_lookup(markov:&mut Markov) {
        let mut statement = markov.connection.prepare(SQL_GETLOOKUP).unwrap();
        while let Ok(sqlite::State::Row) = statement.next() {
            markov.lookup.insert(
                statement.read::<String, _>("word").unwrap(),
                statement.read::<i64, _>("id").unwrap()
            );
        };
    }
    pub fn new_from_existing(path:&str) -> Self {
        let connection = sqlite::open(path).unwrap();

        let mut markov = Markov {
            connection,
            lookup: HashMap::new()
        };

        Markov::load_lookup(&mut markov);
        markov
    }

    // if None is passed, will instantiate inside memory
    pub fn new_from_scratch(path:Option<&str>) -> Self {
        let path = match path {
            Some(s) => s,
            None => ":memory:"
        };
        let connection = sqlite::open(path).unwrap();

        let mut markov = Markov {
            connection,
            lookup: HashMap::new()
        };

        // init
        {
            let mut statement = markov.connection.prepare(SQL_INIT).unwrap();
            statement.next().unwrap();
        }

        Markov::load_lookup(&mut markov);
        markov
    }

    fn id_from_word(&self, word:&str) -> Option<&i64> {
        self.lookup.get(word)
    }

    pub fn markov(&self, start:&str, maxlen:u32, options:Option<MarkovOptions>) -> Option<String> {
        let options = options.unwrap_or(MarkovOptions {
            allow_early_end: true
        });

        let mut output = String::new();

        let exists = self.id_from_word(start);
        if exists == None {
            return None;
        }

        let mut id = *exists.unwrap();

        let mut i = 0;
        loop {
            i += 1;

            let word = MarkovWord::from_read(&self.connection, id);
            output.push_str(format!("{} ", word.word).as_str());

            let next = word.pick_next(options.allow_early_end);
            if next == None {
                return Some(output);
            }

            id = next.unwrap();

            if i > maxlen {
                break;
            }
        }

        Some(output)
    }

    fn train_pair(&mut self, word:&str, future:&str) {
        let wordid = *self.lookup.entry(word.to_string()).or_insert_with(|| MarkovWord::create_self(&self.connection, word));
        let futureid = *self.lookup.entry(future.to_string()).or_insert_with(|| MarkovWord::create_self(&self.connection, future));

        let mut markov = MarkovWord::from_read(&self.connection, wordid);
        markov.add_occurrence(futureid);
        markov.update_self(&self.connection);
    }

    pub fn train(&mut self, content:&str) {
        let mut words = content.split_whitespace();
        let mut previous = words.next().unwrap();
        for word in words {
            self.train_pair(previous, word);

            previous = word;
        }

        // add end trait to final element in string
        let previousid = *self.id_from_word(previous).unwrap();
        let mut word = MarkovWord::from_read(&self.connection, previousid);
        word.add_occurrence(-1);

        // make sure lookup is up to date by replacing it
        let lookup:HashMap<String, i64> = HashMap::new();
        self.lookup = lookup; // drop old table
        Markov::load_lookup(self);
    }
}