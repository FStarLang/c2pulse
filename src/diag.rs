use crate::ir::Location;

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
