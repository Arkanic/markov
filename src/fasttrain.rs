use std::collections::HashMap;

pub struct MTWordRef {
    pub word:String,
    pub occurrences:u32
}

impl MTWordRef {
    fn new(word:&str) -> Self {
        Self {
            word: word.to_string(),
            occurrences: 0
        }
    }

    fn increment(&mut self) {
        self.occurrences += 1;
    }
}

pub struct MTWord {
    pub word:String,
    pub futures:HashMap<String, MTWordRef>
}

impl MTWord {
    fn new(word:&str) -> Self {
        Self {
            word: word.to_string(),
            futures: HashMap::new()
        }
    }

    fn occurence(&mut self, word:&str) {
        let getr = self.futures.get_mut(word);
        if getr.is_some() {
            getr.unwrap().increment();
        } else {
            self.futures.insert(word.to_string(), MTWordRef::new(word));
        }
    }
}

pub struct MemTrain {
    pub words:HashMap<String, MTWord>
}

impl MemTrain {
    pub fn new() -> Self {
        MemTrain {
            words: HashMap::new()
        }
    }

    // if second is None, presume blank
    fn train_pair(&mut self, first:&str, second:Option<&str>) {
        let second = second.unwrap_or("");
        {
            self.words.entry(second.to_string()).or_insert_with(|| MTWord::new(second));
        }

        let first = self.words.entry(first.to_string()).or_insert_with(|| MTWord::new(first));
        first.occurence(second);
    }

    pub fn train(&mut self, content:&str) {
        let mut words = content.split_whitespace();
        let mut previous = words.next().unwrap();
        for word in words {
            self.train_pair(word, Some(previous));

            previous = word;
        }
        self.train_pair(previous, None); // last
    }
}