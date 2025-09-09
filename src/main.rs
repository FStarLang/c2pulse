mod clang;
mod diag;
mod ir;
mod prune;

fn main() {
    let argv: Vec<_> = std::env::args().collect();
    let file_name = std::fs::canonicalize(&argv[1]).unwrap();
    let (mut tu, diags) = clang::parse_file(file_name.to_str().unwrap());
    prune::prune(&mut tu);
    println!("{:#?}", (tu, diags));
}
