fn main() {
    println!("cargo::rerun-if-changed=src/c/ll.c");
    println!("cargo::rerun-if-changed=src/c/hashmap.c");
    println!("cargo::rerun-if-changed=src/c/markov.c");

    cc::Build::new()
        .file("src/c/ll.c")
        .file("src/c/hashmap.c")
        .file("src/c/markov.c")
        .compile("libmarkov");
}