mod clang;
mod diag;
mod elab;
mod emit;
mod env;
mod hauntedc;
mod ir;
mod prune;

fn main() {
    let argv: Vec<_> = std::env::args().collect();
    let file_name = std::fs::canonicalize(&argv[1]).unwrap();
    let (mut tu, mut diags) = clang::parse_file(file_name.to_str().unwrap());
    prune::prune(&mut tu);
    elab::elab(&mut diags, &mut tu);
    let (pulse_code, range_map) = emit::emit(&tu);
    println!("{:#?}", range_map);
    println!("{:#?}", tu);
    println!("{}", pulse_code);
    diags.print_to_stderr();
    if diags.has_errors() {
        std::process::exit(1)
    }
}
