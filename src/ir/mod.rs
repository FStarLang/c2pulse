use num_bigint::BigInt;
use std::fmt::{Debug, Display};
use std::rc::Rc;
mod pretty;

#[derive(Debug, PartialEq, Eq, Hash, Clone, Copy, PartialOrd, Ord)]
pub struct Position {
    pub line: u32,
    pub character: u32,
}

#[derive(PartialEq, Eq, Hash, Clone, Copy)]
pub struct Range {
    pub start: Position,
    pub end: Position,
}

impl Range {
    pub fn union(&self, other: &Range) -> Range {
        Range {
            start: self.start.min(other.start),
            end: self.end.max(other.end),
        }
    }
}

impl Display for Range {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            "{}:{}-{}:{}",
            self.start.line, self.start.character, self.end.line, self.end.character
        )
    }
}

impl Debug for Range {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        Display::fmt(&self, f)
    }
}

#[derive(PartialEq, Eq, Hash, Clone)]
pub struct Location {
    pub file_name: Rc<str>,
    pub range: Range,
}

impl Display for Location {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "{}:{:#?}", self.file_name, self.range)
    }
}

impl Debug for Location {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        Display::fmt(&self, f)
    }
}

#[derive(PartialEq, Eq, Hash, Clone)]
pub enum SourceInfo {
    Original(Location),
    Fallback(Location),
}

impl SourceInfo {
    pub fn location(&self) -> &Location {
        match self {
            SourceInfo::Original(location) => location,
            SourceInfo::Fallback(location) => location,
        }
    }
}

impl Debug for SourceInfo {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        match self {
            Self::Original(loc) => write!(f, "Original({:#?})", loc),
            Self::Fallback(loc) => write!(f, "Fallback({:#?})", loc),
        }
    }
}

#[derive(PartialEq, Eq, Hash, Clone)]
pub struct Ast<T> {
    pub val: T,
    pub loc: Rc<SourceInfo>,
}

impl<T> Ast<T> {
    pub fn reuse_loc<S>(&self, val: S) -> Ast<S> {
        Ast {
            loc: self.loc.clone(),
            val,
        }
    }
}

pub trait WithLoc: Sized {
    fn with_loc(self, loc: Rc<SourceInfo>) -> Rc<Ast<Self>>;
}

impl<T> WithLoc for T {
    fn with_loc(self, loc: Rc<SourceInfo>) -> Rc<Ast<Self>> {
        Rc::new(Ast { val: self, loc })
    }
}

impl<T> Debug for Ast<T>
where
    T: Debug,
{
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "{:#?} @ {:#?}", self.val, self.loc)
    }
}

impl<T> Display for Ast<T>
where
    T: Display,
{
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        self.val.fmt(f)
    }
}

#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub enum PointerKind {
    Unknown,
    Ref,
    Array,
}

pub type Type = Ast<TypeT>;
#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub enum TypeT {
    Void,
    Bool,
    Int { signed: bool, width: u32 },
    SizeT,
    Pointer(Rc<Type>, PointerKind),

    SpecInt,
    SLProp,

    Requires(Rc<Type>, Rc<RValue>),
    Ensures(Rc<Type>, Rc<RValue>),
    Consumes(Rc<Type>),
    Plain(Rc<Type>),

    Error,
}

#[derive(Debug, PartialEq, Eq, Hash, Clone, Copy)]
pub enum BinOp {
    Eq,
    LEq,
    LogAnd,
    Mul,
    Div,
    Mod,
    Add,
    Sub,
}

impl BinOp {
    pub fn to_str(self) -> &'static str {
        match self {
            BinOp::Eq => "==",
            BinOp::LEq => "<=",
            BinOp::LogAnd => "&&",
            BinOp::Mul => "*",
            BinOp::Div => "/",
            BinOp::Mod => "%",
            BinOp::Add => "+",
            BinOp::Sub => "-",
        }
    }
}

pub type RValue = Ast<RValueT>;
pub type RValues = Vec<Rc<RValue>>;
#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub enum RValueT {
    BoolLit(bool),
    IntLit(Rc<BigInt>, Rc<Type>),
    LValue(Rc<LValue>),
    Ref(Rc<LValue>),
    BinOp(BinOp, Rc<RValue>, Rc<RValue>),
    FnCall(Rc<Ident>, RValues),
    Cast(Rc<RValue>, Rc<Type>),
    InlinePulse(Rc<InlineCode>, Rc<Type>),
    Live(Rc<LValue>),
    Old(Rc<RValue>),
    Error(Rc<Type>),
}

pub type IdentT = str;
pub type Ident = Ast<Rc<IdentT>>;

pub type LValue = Ast<LValueT>;
#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub enum LValueT {
    Var(Rc<Ident>),
    Deref(Rc<RValue>),
    Error(Rc<Type>),
}

pub type Stmt = Ast<StmtT>;
pub type Stmts = Vec<Rc<Stmt>>;
#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub enum StmtT {
    Call(Rc<RValue>),
    Decl(Rc<Ident>, Rc<Type>),
    Assign(Rc<LValue>, Rc<RValue>),
    If(Rc<RValue>, Rc<Stmts>, Rc<Stmts>),
    While(Rc<RValue>, Rc<RValues>, Rc<Stmts>),
    Return(Rc<RValue>),
    Error,
}

#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub struct StructDefn {
    pub name: Rc<Ident>,
    pub fields: Vec<(Ident, Rc<Type>)>,
}

pub type FnArg = (Option<Rc<Ident>>, Rc<Type>);

#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub struct FnDecl {
    pub name: Rc<Ident>,
    pub ret_type: Rc<Type>,
    pub args: Vec<FnArg>,
    pub requires: RValues,
    pub ensures: RValues,
}

#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub struct FnDefn {
    pub decl: FnDecl,
    pub body: Stmts,
}

#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub struct CodeToken {
    pub before: &'static str,
    pub text: Ast<Rc<str>>,
}

#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub struct InlineCode {
    pub tokens: Vec<CodeToken>,
}

impl InlineCode {
    pub fn to_string(&self) -> String {
        let mut result = String::new();
        for tok in &self.tokens {
            result.push_str(tok.before);
            result.push_str(&*tok.text.val);
        }
        result
    }
}

#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub struct IncludeDecl {
    pub code: InlineCode,
}

pub type Decl = Ast<DeclT>;
#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub enum DeclT {
    FnDefn(FnDefn),
    FnDecl(FnDecl),
    StructDefn(StructDefn),
    IncludeDecl(IncludeDecl),
}

#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub struct TranslationUnit {
    pub main_file_name: Rc<str>,
    pub decls: Vec<Decl>,
}
