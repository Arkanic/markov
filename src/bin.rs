extern crate markov;

use std::{env, fs};

fn main() {
    let mut args = env::args();
    args.next(); // skip first one

    match args.next().unwrap().as_str() {
        "memtest" => {
            let file = args.next().unwrap();
            let content = fs::read_to_string(file).unwrap();

            println!("creating in memory");
            let mut markov = markov::Markov::new();
            println!("training");
            markov.train(&content);
            println!("done.");
        },
        _ => {
            println!("markov new <name> - create new\nmarkov train <name> <file> - train name with file content\nmarkov run <name> <starting word> - run model name with starting word\nnote: no args error handling so don't mess up");
        }
    }
}