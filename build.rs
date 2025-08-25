use std::process::Command;

fn llvm_config_flags(arg: &str) -> Vec<String> {
    Command::new("llvm-config")
        .arg(arg)
        .output()
        .expect("Cannot find llvm-config")
        .stdout
        .split(|c| c.is_ascii_whitespace())
        .filter(|flag| flag.len() > 0)
        .map(|flag| str::from_utf8(flag).unwrap().into())
        .collect()
}

fn split_off<'a>(prefix: &str, s: &'a str) -> Option<&'a str> {
    if s.starts_with(prefix) {
        Some(&s[prefix.len()..])
    } else {
        None
    }
}

fn main() {
    let mut build = cxx_build::bridge("src/clang.rs");
    build.file("src/clang.cpp").std("c++17");
    for flag in llvm_config_flags("--cxxflags") {
        build.flag(flag);
    }
    build.compile("c2pulse-clang");

    for flag in llvm_config_flags("--libs") {
        if let Some(lib) = split_off("-l", &flag) {
            println!("cargo::rustc-link-lib={lib}");
        } else if let Some(lib_path) = split_off("-L", &flag) {
            println!("cargo::rustc-link-search={lib_path}");
        }
    }
    println!("cargo::rustc-link-lib=clang-cpp");

    println!("cargo:rerun-if-changed=src/clang.cpp");
    println!("cargo:rerun-if-changed=src/clang_bridge.h");
}
