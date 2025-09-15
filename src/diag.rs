use std::collections::HashMap;

use crate::ir::{Location, Position};

#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub enum DiagnosticLevel {
    Warning,
    Error,
}

#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub struct Diagnostic {
    pub loc: Location,
    pub level: DiagnosticLevel,
    pub msg: String,
}

pub fn print_to_stderr<'a>(diags: &'a Vec<Diagnostic>) {
    use codespan_reporting::diagnostic::*;
    use codespan_reporting::term::termcolor::StandardStream;
    use codespan_reporting::{files::Files, term};
    use codespan_reporting::{files::SimpleFiles, term::termcolor::ColorChoice};
    let mut files = SimpleFiles::new();
    let mut file_ids: HashMap<&'a str, usize> = HashMap::new();
    let writer = StandardStream::stderr(ColorChoice::Always);
    let config = codespan_reporting::term::Config::default();
    for diag in diags {
        let mut d = if diag.level == DiagnosticLevel::Error {
            Diagnostic::error()
        } else {
            Diagnostic::warning()
        };
        d = d.with_message(&diag.msg);
        let file_name = &*diag.loc.file_name;
        let file_id = *file_ids.entry(file_name).or_insert_with(|| {
            files.add(
                file_name,
                String::from_utf8(std::fs::read(file_name).unwrap_or_else(|_| vec![]))
                    .unwrap_or("".to_string()),
            )
        });
        let pos_to_byte = |pos: Position| {
            files
                .line_range(file_id, (pos.line - 1) as usize)
                .expect("invalid position")
                .start
                + ((pos.character - 1) as usize)
        };
        d = d.with_label(Label::primary(
            file_id,
            pos_to_byte(diag.loc.range.start)..(pos_to_byte(diag.loc.range.end) + 1),
        ));
        term::emit(&mut writer.lock(), &config, &files, &d).expect("printing diag");
    }
}
