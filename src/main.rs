mod clang;
mod diag;
mod ir;

fn main() {
    let argv: Vec<_> = std::env::args().collect();
    let tu = clang::parse_file(&argv[1]);
    println!("{:#?}", tu);
}
