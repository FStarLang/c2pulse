use num_bigint::BigInt;
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
#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub struct Location {
    pub file_name: Rc<str>,
    pub range: Range,
}

#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub struct Ast<T> {
    pub val: T,
    pub loc: Rc<Location>,
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
}

pub type RValue = Ast<RValueT>;
#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub enum RValueT {
    IntLit { val: Rc<BigInt>, ty: Type },
    LValue(Rc<LValue>),
    Ref(Rc<LValue>),
    Cast { val: Rc<RValue>, ty: Type },
    Error,
}

pub type Ident = str;

pub type LValue = Ast<LValueT>;
#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub enum LValueT {
    Var(Rc<Ident>),
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
}

#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub struct StructDefn {
    pub name: Ast<Rc<Ident>>,
    pub fields: Vec<(Ast<Rc<Ident>>, Rc<Type>)>,
}

#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub struct FnDecl {
    pub name: Ast<Rc<Ident>>,
    pub ret_type: Rc<Type>,
    pub args: Vec<(Ast<Rc<Ident>>, Rc<Type>)>,
}

#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub struct FnDefn {
    pub decl: FnDecl,
    pub body: Stmts,
}

#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub struct CodeToken {
    pub before: &'static str,
    pub text: Ast<Rc<String>>,
}

#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub struct Code {
    pub tokens: Vec<CodeToken>,
}

#[derive(Debug, PartialEq, Eq, Hash, Clone)]
pub struct IncludeDecl {
    pub code: Ast<Code>,
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
