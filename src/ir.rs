use std::rc::Rc;
use num_bigint::BigInt;

#[derive(Debug, PartialEq, Eq, Hash)]
pub struct Position {
    pub line: u32,
    pub character: u32,
}

#[derive(Debug, PartialEq, Eq, Hash)]
pub struct Range {
    pub start: Position,
    pub end: Position,
}
#[derive(Debug, PartialEq, Eq, Hash)]
pub struct Location {
    pub file_name: Rc<str>,
    pub range: Range,
}

#[derive(Debug, PartialEq, Eq, Hash)]
pub struct Ast<T> {
    pub val: T,
    pub loc: Rc<Location>,
}

#[derive(Debug, PartialEq, Eq, Hash)]
pub enum PointerKind {
    Unknown,
    Ref,
    Array { length: Rc<RValue> },
}

pub type Type = Ast<TypeT>;
#[derive(Debug, PartialEq, Eq, Hash)]
pub enum TypeT {
    Int { signed: bool, width: u32 },
    SizeT,
    Pointer { to: Rc<Type>, kind: PointerKind },
}

pub type RValue = Ast<RValueT>;
#[derive(Debug, PartialEq, Eq, Hash)]
pub enum RValueT {
    IntLit { val: Rc<BigInt>, ty: Type },
    LValue(Rc<LValue>),
    Ref(Rc<LValue>),
    Cast { val: Rc<RValue>, ty: Type },
    Error,
}

pub type Ident = str;
    
pub type LValue = Ast<LValueT>;
#[derive(Debug, PartialEq, Eq, Hash)]
pub enum LValueT {
    Var(Rc<Ident>),
    Error,
}

pub type Stmt = Ast<StmtT>;
pub type Stmts = Vec<Rc<Stmt>>;
#[derive(Debug, PartialEq, Eq, Hash)]
pub enum StmtT {
    Call(Rc<RValue>),
    Decl(Rc<Ident>, Rc<Type>),
    Assign(Rc<LValue>, Rc<RValue>),
    If(Rc<RValue>, Rc<Stmts>, Rc<Stmts>),
}

#[derive(Debug, PartialEq, Eq, Hash)]
pub struct Decl {
    ret_type: Rc<Type>,
    args: Vec<(Rc<Ident>, Rc<Type>)>,
    body: Stmts,
}