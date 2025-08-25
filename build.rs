fn main() {
    cxx_build::bridge("src/clang.rs")
        .file("src/clang.cpp")
        .std("c++17")
        .compile("c2pulse-clang");

    println!("cargo:rerun-if-changed=src/clang.cpp");
    println!("cargo:rerun-if-changed=src/clang_bridge.h");
}