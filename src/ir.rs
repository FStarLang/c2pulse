use num_bigint::BigInt;
use std::fmt::Debug;
use std::rc::Rc;

#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub struct Position {
    pub line: u32,
    pub character: u32,
}

#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub struct Range {
    pub start: Position,
    pub end: Position,
}
#[derive(PartialEq, Eq, Hash, Clone)]
pub struct Location {
    pub file_name: Rc<str>,
    pub range: Range,
}

impl Debug for Location {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            "{}:{}:{}-{}:{}",
            self.file_name,
            self.range.start.line,
            self.range.start.character,
            self.range.end.line,
            self.range.end.character
        )
    }
}

#[derive(PartialEq, Eq, Hash, Clone)]
pub enum SourceInfo {
    None,
    Original(Location),
}

impl Debug for SourceInfo {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        match self {
            Self::None => write!(f, "None"),
            Self::Original(loc) => write!(f, "Original({:#?})", loc),
        }
    }
}

#[derive(PartialEq, Eq, Hash, Clone)]
pub struct Ast<T> {
    pub val: T,
    pub loc: Rc<SourceInfo>,
}

impl<T> Debug for Ast<T>
where
    T: Debug,
{
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "{:#?} @ {:#?}", self.val, self.loc)
    }
}

#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub enum PointerKind {
    Unknown,
    Ref,
    Array { length: Rc<RValue> },
}

pub type Type = Ast<TypeT>;
#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub enum TypeT {
    Int { signed: bool, width: u32 },
    SizeT,
    Pointer { to: Rc<Type>, kind: PointerKind },
    Error,
}

pub type RValue = Ast<RValueT>;
#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub enum RValueT {
    IntLit { val: Rc<BigInt>, ty: Rc<Type> },
    LValue(Rc<LValue>),
    Ref(Rc<LValue>),
    Cast { val: Rc<RValue>, ty: Rc<Type> },
    Error,
}

pub type IdentT = str;
pub type Ident = Ast<Rc<IdentT>>;

pub type LValue = Ast<LValueT>;
#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub enum LValueT {
    Var(Rc<Ident>),
    Deref(Rc<RValue>),
    Error,
}

pub type Stmt = Ast<StmtT>;
pub type Stmts = Vec<Rc<Stmt>>;
#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub enum StmtT {
    Call(Rc<RValue>),
    Decl(Rc<Ident>, Rc<Type>),
    Assign(Rc<LValue>, Rc<RValue>),
    If(Rc<RValue>, Rc<Stmts>, Rc<Stmts>),
    Return(Rc<RValue>),
    Error,
}

#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub struct StructDefn {
    pub name: Ident,
    pub fields: Vec<(Ident, Rc<Type>)>,
}

#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub struct FnDecl {
    pub name: Ident,
    pub ret_type: Rc<Type>,
    pub args: Vec<(Option<Ident>, Rc<Type>)>,
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

#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub struct IncludeDecl {
    pub code: Ast<InlineCode>,
}

#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub enum Decl {
    FnDefn(FnDefn),
    FnDecl(FnDecl),
    StructDefn(StructDefn),
    IncludeDecl(IncludeDecl),
}

#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub struct TranslationUnit {
    pub decls: Vec<Decl>,
}
