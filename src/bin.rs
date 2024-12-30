extern crate markov;

fn main() {
    println!("hello");

    let chain = markov::Markov::new_from_existing("testdata");
    let output = chain.markov("hello", 10, None).unwrap();
    println!("{}", output);
}