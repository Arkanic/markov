use std::{collections::hash_map::Entry, fs};
use std::path::Path;
use std::collections::HashMap;
use weighted_rand::builder::{NewBuilder, WalkerTableBuilder};
use sha256::digest;

static LOOKUP:&str = "lookup.dat";
static PARTICLES:&str = "particles";
static SOURCES:&str = "sources";

struct MarkovFileReference {
    hash:String,
    occurrences:u32
}

// MARKOV FILE FORMAT
/* <hash>.dat
<word>
<future hash>:occurrences:<comma seperated metadata hashes>
1...n
*/

struct MarkovFile {
    hash:String,
    word:String,
    futures:Vec<MarkovFileReference>
}

impl MarkovFile {
    fn from_read(datadir:&str, hash:&str) -> Self {
        let file = fs::read_to_string(Path::new(datadir).join(PARTICLES).join(format!("{}.dat", hash))).unwrap();
        let mut lines = file.lines();
        
        let word = lines.next().unwrap().to_string();
        let mut futures:Vec<MarkovFileReference> = Vec::new();
        for line in lines {
            let mut parts = line.split(":");
            futures.push(MarkovFileReference {
                hash: parts.next().unwrap().to_string(),
                occurrences:  parts.next().unwrap().parse::<u32>().unwrap()
            });
        }

        MarkovFile {
            hash: hash.to_string(),
            word,
            futures
        }
    }

    // returns hash
    fn create_self(datadir:&str, word:&str) -> String {
        let hash = digest(digest(word));
        let file = MarkovFile {
            hash: hash.clone(),
            word: word.to_string(),
            futures: vec![]
        };
        file.write_self(datadir);

        hash
    }

    fn write_self(&self, datadir:&str) {
        let mut data = String::new();
        data.push_str(format!("{}\n", self.word).as_str());
        for future in self.futures.iter() {
            data.push_str(format!("{}:{}\n", future.hash, future.occurrences).as_str());
        }

        fs::write(Path::new(datadir).join(PARTICLES).join(format!("{}.dat", self.hash)), data).unwrap();
    }

    fn add_occurrence(&mut self, occurrence_hash:&str) {
        // todo: not stupid search
        let mut found = false;
        for future in self.futures.iter_mut() {
            if future.hash == occurrence_hash {
                found = true;
                future.occurrences += 1;
            }
        }

        if !found {
            self.futures.push(MarkovFileReference {
                hash: occurrence_hash.to_string(),
                occurrences: 1
            });
        }
    }

    fn pick_next(&self, allow_end:bool) -> Option<String> {
        let mut hashes:Vec<&str> = Vec::new();
        let mut weights:Vec<u32> = Vec::new();

        for future in self.futures.iter() {
            if (future.hash == "0") && !allow_end {
            } else {
                hashes.push(future.hash.as_str());
                weights.push(future.occurrences);
            }
        }

        if hashes.len() == 0 {
            None
        } else {
            let table = WalkerTableBuilder::new(weights.as_slice()).build();
            let hash = hashes[table.next()].to_string();

            if hash == "0" {
                None
            } else {
                Some(hash)
            }
        }
    }
}


pub struct MarkovOptions {
    allow_early_end:bool
}

pub struct Markov {
    datadir:String,
    lookup:HashMap<String, String>
}

impl Markov {
    pub fn new_from_existing(dir:&str) -> Self {
        // read lookup table to memory
        let mut lookup:HashMap<String, String> = HashMap::new();
        for line in fs::read_to_string(Path::new(dir).join(LOOKUP)).unwrap().lines() {
            let mut parts = line.split("␟");
            lookup.insert(
                parts.next().unwrap().to_string(),
                parts.next().unwrap().to_string()
            );
        }

        let data = Markov {
            datadir: dir.to_owned(),
            lookup
        };

        data
    }

    pub fn new_from_scratch(dir:&str) -> Self {
        fs::create_dir(Path::new(dir)).unwrap();
        fs::create_dir(Path::new(dir).join(PARTICLES)).unwrap();
        fs::create_dir(Path::new(dir).join(SOURCES)).unwrap();
        fs::File::create(Path::new(dir).join(LOOKUP)).unwrap();

        Markov::new_from_existing(dir)
    }

    fn hash_from_word(&self, word:&str) -> Option<&String> {
        self.lookup.get(word)
    }

    pub fn markov(&self, start:&str, maxlen:u32, options:Option<MarkovOptions>) -> Option<String> {
        let options = options.unwrap_or(MarkovOptions {
            allow_early_end: true
        });

        let mut output = String::new();

        let exists = self.hash_from_word(start);
        if exists == None {
            return None;
        }

        let mut hash = exists.unwrap().to_string();

        let mut i = 0;
        loop {
            i += 1;

            let file = MarkovFile::from_read(self.datadir.as_str(), hash.as_str());
            output.push_str(format!("{} ", file.word).as_str());

            let next = file.pick_next(options.allow_early_end);
            if next == None {
                return Some(output);
            }

            hash = next.unwrap();

            if i > maxlen {
                break;
            }
        }

        Some(output)
    }

    fn train_pair(&mut self, word:&str, future:&str) {
        let wordhash = match self.lookup.entry(word.to_string()) {
            Entry::Occupied(o) => o.into_mut(),
            Entry::Vacant(v) => v.insert(MarkovFile::create_self(self.datadir.as_str(), word))
        }.to_owned();
        let futurehash = match self.lookup.entry(future.to_string()) {
            Entry::Occupied(o) => o.into_mut(),
            Entry::Vacant(v) => v.insert(MarkovFile::create_self(self.datadir.as_str(), future))
        }.to_owned();

        let mut file = MarkovFile::from_read(self.datadir.as_str(), wordhash.as_str());
        file.add_occurrence(futurehash.as_str());
        file.write_self(self.datadir.as_str());
    }

    pub fn train_from_string(&mut self, content:&str) {
        let mut words = content.split_whitespace();
        let mut previous = words.next().unwrap();
        for word in words {
            self.train_pair(previous, word);

            previous = word;
        }

        // add end trait to final element in string
        let previoushash = self.hash_from_word(previous).unwrap();
        let mut file = MarkovFile::from_read(self.datadir.as_str(), previoushash);
        file.add_occurrence("0");

        // save the adjusted lookup table
        self.save_lookup();
    }

    fn save_lookup(&self) {
        let mut data = String::new();
        for (key, value) in self.lookup.clone().into_iter() {
            data.push_str(format!("{}␟{}\n", key, value).as_str());
        }

        fs::write(Path::new(self.datadir.as_str()).join(LOOKUP), data).unwrap();
    }
}

impl Drop for Markov {
    fn drop(&mut self) {
        // we want to save updated lookup table on shutdown
        self.save_lookup();
    }
}