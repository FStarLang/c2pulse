use std::{path::Path, rc::Rc};

use crate::{
    diag::{Diagnostic, Diagnostics},
    ir::{Location, Position, Range},
    vfs::{OverlayFS, RealFS, VFS},
};
use clap::Parser;

mod clang;
mod diag;
mod env;
mod hauntedc;
mod ir;
mod mayberc;
mod pass;
mod source_range_info;
mod vfs;

#[derive(Parser)]
struct Cli {
    #[arg(long = "tmpdir")]
    tmpdir: Option<String>,

    #[arg(long = "print-ir")]
    print_ir: bool,

    file: String,
}

fn derive_module_name(file_name: &str) -> String {
    let file_stem = Path::new(file_name).file_stem().unwrap().to_string_lossy();
    (file_stem.chars().take(1))
        .flat_map(char::to_uppercase)
        .chain(file_stem.chars().skip(1))
        .collect()
}

fn serialize_diags(for_file: &str, diags: &Diagnostics) -> String {
    let for_file = Rc::from(for_file);
    serde_json::to_string_pretty(
        &diags
            .diags
            .iter()
            .map(Diagnostic::clone)
            .map(|mut diag| {
                if diag.loc.file_name == for_file {
                    diag
                } else {
                    let pos0 = Position {
                        line: 0,
                        character: 0,
                    };
                    diag.loc = Location {
                        file_name: for_file.clone(),
                        range: Range {
                            start: pos0,
                            end: pos0,
                        },
                    };
                    diag
                }
            })
            .map(|diag| Diagnostic::to_lsp(&diag))
            .collect::<Vec<_>>(),
    )
    .unwrap()
}

fn main() {
    let cli = Cli::parse();

    let file_name = std::path::absolute(cli.file.clone())
        .unwrap()
        .to_string_lossy()
        .into_owned();
    let mut vfs: Box<dyn VFS>;
    match &cli.tmpdir {
        Some(tmpdir) => {
            let mut overlayfs = OverlayFS::new(RealFS::new());
            let tmpdir = Path::new(tmpdir);
            let contents = String::from_utf8(
                std::fs::read(tmpdir.join(Path::new(&file_name).file_name().unwrap())).unwrap(),
            )
            .unwrap();
            overlayfs.add_overlay(file_name.clone(), contents);
            vfs = Box::new(overlayfs);
        }
        None => {
            vfs = Box::new(RealFS::new());
        }
    }

    if let Err(error) = vfs.read_vfs_file(&file_name) {
        eprintln!("Cannot open {}: {}", file_name, error);
        std::process::exit(1);
    }

    let module_name = derive_module_name(&file_name);

    let (mut tu, mut diags) = clang::parse_file(&file_name, &mut *vfs);
    pass::check::check(&mut diags, &mut tu, "clang", false);
    pass::prune::prune(&mut tu);
    pass::check::check(&mut diags, &mut tu, "prune", false);
    pass::restructure_goto::restructure_goto(&mut tu);
    pass::elab::elab(&mut diags, &mut tu);
    pass::check::check(&mut diags, &mut tu, "elab", true);

    if cli.print_ir {
        println!("{}", tu);
        return;
    }

    let (pulse_code, range_map) = pass::emit::emit(&mut diags, &module_name, &tu);

    let outdir = match &cli.tmpdir {
        Some(tmpdir) => Path::new(tmpdir),
        None => Path::new(&file_name).parent().unwrap(),
    };

    std::fs::write(outdir.join(module_name.clone() + ".fst"), &pulse_code).unwrap();
    std::fs::write(
        outdir.join(module_name.clone() + "_source_range_info.json"),
        source_range_info::serialize(&range_map),
    )
    .unwrap();
    std::fs::write(
        outdir.join(module_name.clone() + "_diagnostics.json"),
        &serialize_diags(&file_name, &diags),
    )
    .unwrap();

    diags.print_to_stderr(&mut *vfs);
    if diags.has_errors() {
        std::process::exit(0)
    }
}
