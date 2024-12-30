use std::fs;
use std::path::Path;
use std::collections::HashMap;
use weighted_rand::builder::{NewBuilder, WalkerTableBuilder};

static LOOKUP:&str = "lookup.dat";
static PARTICLES:&str = "particles";
static SOURCES:&str = "sources";

struct MarkovFileReference {
    hash:String,
    occurences:u32
}

// MARKOV FILE FORMAT
/* <hash>.dat
<word>
<future hash>:occurences:<comma seperated metadata hashes>
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
                occurences:  parts.next().unwrap().parse::<u32>().unwrap()
            });
        }

        MarkovFile {
            hash: hash.to_string(),
            word,
            futures
        }
    }

    fn pick_next(&self, allow_end:bool) -> Option<String> {
        let mut hashes:Vec<&str> = Vec::new();
        let mut weights:Vec<u32> = Vec::new();

        for future in self.futures.iter() {
            if (future.hash == "0") && !allow_end {
            } else {
                hashes.push(future.hash.as_str());
                weights.push(future.occurences);
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