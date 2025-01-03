extern crate markov;

use std::env;
use std::fs;

fn main() {
    let mut args = env::args();
    args.next(); // skip first
    match args.next().unwrap().as_str() {
        "new" => {
            let name = args.next().unwrap();
            markov::Markov::new_from_scratch(Some(&name));
        }
        "train" => {
            let name = args.next().unwrap();
            let mut markov = markov::Markov::new_from_existing(&name);

            let file = args.next().unwrap();
            let content = fs::read_to_string(file).unwrap();
            let mut ft = markov::fasttrain::MemTrain::new();
            println!("training in memory...");
            ft.train(&content);
            println!("saving to database...");
            markov.apply_memtrain(&ft);
            println!("done!");
        }
        "run" => {
            let name = args.next().unwrap();
            let markov = markov::Markov::new_from_existing(&name);
            let starting_word = args.next().unwrap();
            
            let output = markov.markov(&starting_word, 1000, None).unwrap();
            println!("{}", output);
        }
        "memtest" => {
            let file = args.next().unwrap();
            let content = fs::read_to_string(file).unwrap();

            println!("creating in memory");
            let mut ft = markov::fasttrain::MemTrain::new();
            println!("training in memory");
            ft.train(&content);
            println!("done.");
        }
        _ => {
            println!("markov new <name> - create new\nmarkov train <name> <file> - train name with file content\nmarkov run <name> <starting word> - run model name with starting word\nnote: no args error handling so don't mess up");
        }
    }
}