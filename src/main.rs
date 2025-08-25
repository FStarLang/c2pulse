mod ir;
mod clang;

fn main() {
    clang::parse_file("foo.c");
}
