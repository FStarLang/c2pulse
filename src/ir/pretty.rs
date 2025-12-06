use crate::ir::*;
use ::pretty::RcDoc;

pub trait PrettyIR {
    fn to_doc(&self) -> RcDoc<'_, ()>;
}

impl<A> PrettyIR for Ast<A>
where
    A: PrettyIR,
{
    fn to_doc(&self) -> RcDoc<'_, ()> {
        self.val.to_doc()
    }
}

impl Display for Type {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "{}", self.to_doc().pretty(80))
    }
}

impl PrettyIR for TypeT {
    fn to_doc(&self) -> RcDoc<'_, ()> {
        match self {
            TypeT::Void => RcDoc::text("void"),
            TypeT::Bool => RcDoc::text("_Bool"),
            TypeT::Int { signed, width } => RcDoc::text(if *signed {
                format!("int{}_t", width)
            } else {
                format!("uint{}_t", width)
            }),
            TypeT::SizeT => RcDoc::text("size_t"),
            TypeT::Pointer(ty, PointerKind::Ref) => ty.to_doc().append("*"),
            TypeT::Pointer(ty, PointerKind::Array) => ty.to_doc().append(RcDoc::text("[]")),
            TypeT::Pointer(ty, PointerKind::Unknown) => ty.to_doc().append(RcDoc::text("[?]")),
            TypeT::SpecInt => RcDoc::text("_specint"),
            TypeT::SLProp => RcDoc::text("_slprop"),
            TypeT::Requires(ty, p) => RcDoc::text("_requires(")
                .append(p.to_doc())
                .append(")")
                .group()
                .nest(2)
                .append(RcDoc::line())
                .append(ty.to_doc()),
            TypeT::Ensures(ty, p) => RcDoc::text("_ensures(")
                .append(p.to_doc())
                .append(")")
                .group()
                .nest(2)
                .append(RcDoc::line())
                .append(ty.to_doc()),
            TypeT::Consumes(ty) => RcDoc::text("_consumes")
                .append(RcDoc::line())
                .append(ty.to_doc()),
            TypeT::Plain(ty) => RcDoc::text("_plain")
                .append(RcDoc::line())
                .append(ty.to_doc()),
            TypeT::Error => RcDoc::text("???"),
        }
    }
}

impl Display for RValue {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "{}", self.to_doc().pretty(80))
    }
}

impl PrettyIR for RValueT {
    fn to_doc(&self) -> RcDoc<'_, ()> {
        match self {
            RValueT::BoolLit(b) => RcDoc::text(if *b { "true" } else { "false" }),
            RValueT::IntLit(n, _ty) => RcDoc::text(n.to_string()),
            RValueT::LValue(lval) => lval.to_doc(),
            RValueT::Ref(lval) => RcDoc::text("&").append(lval.to_doc()),
            RValueT::BinOp(bin_op, lhs, rhs) => RcDoc::text("(")
                .append(lhs.to_doc())
                .append(RcDoc::space())
                .append(bin_op.to_str())
                .group()
                .append(RcDoc::line())
                .append(rhs.to_doc())
                .append(")")
                .nest(2)
                .group(),
            RValueT::FnCall(f, args) => RcDoc::text(&*f.val)
                .append("(")
                .append(RcDoc::intersperse(
                    args.iter().map(|arg| arg.to_doc()),
                    RcDoc::text(",").append(RcDoc::line()),
                ))
                .append(")"),
            RValueT::Cast(rval, ty) => (RcDoc::text("(")
                .append(ty.to_doc())
                .append(")")
                .nest(2)
                .group())
            .append(RcDoc::line())
            .append(rval.to_doc()),
            RValueT::InlinePulse(inline_code, ty) => (RcDoc::text("(")
                .append(ty.to_doc())
                .append(")")
                .nest(2)
                .group())
            .append(RcDoc::line())
            .append(RcDoc::text("_inline_pulse(").append(inline_code.to_string()))
            .append(RcDoc::text(")")),
            RValueT::Live(v) => RcDoc::text("_live(")
                .append(v.to_doc())
                .append(")")
                .nest(2)
                .group(),
            RValueT::Old(v) => RcDoc::text("_old(")
                .append(v.to_doc())
                .append(")")
                .nest(2)
                .group(),
            RValueT::Error(_) => RcDoc::text("???"),
        }
    }
}

impl Display for LValue {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "{}", self.to_doc().pretty(80))
    }
}

impl PrettyIR for LValueT {
    fn to_doc(&self) -> RcDoc<'_, ()> {
        match self {
            LValueT::Var(x) => RcDoc::text(&*x.val),
            LValueT::Deref(rval) => RcDoc::text("*").append(rval.to_doc()),
            LValueT::Error(_) => RcDoc::text("???"),
        }
    }
}

fn pretty_block(stmts: &Stmts) -> RcDoc<'_, ()> {
    if stmts.is_empty() {
        RcDoc::text("{}")
    } else {
        RcDoc::text("{")
            .append(RcDoc::hardline())
            .append(stmts.to_doc())
            .nest(2)
            .append(RcDoc::hardline())
            .group()
    }
}

impl PrettyIR for StmtT {
    fn to_doc(&self) -> RcDoc<'_, ()> {
        match self {
            StmtT::Call(f) => f.to_doc(),
            StmtT::Decl(x, ty) => (ty.to_doc())
                .append(" ")
                .append(&*x.val)
                .append(";")
                .nest(2)
                .group(),
            StmtT::Assign(x, v) => (x.to_doc())
                .append(" =")
                .append(RcDoc::line())
                .append(v.to_doc())
                .append(";")
                .nest(2)
                .group(),
            StmtT::If(c, b1, b2) => RcDoc::text("if (")
                .append(c.to_doc().nest(4))
                .append(")")
                .append(RcDoc::line())
                .append(pretty_block(b1))
                .append(" else ")
                .append(pretty_block(b2))
                .group(),
            StmtT::While(cond, invs, body) => RcDoc::text("while (")
                .append(cond.to_doc().nest(4))
                .append(")")
                .append(
                    RcDoc::concat(invs.iter().map(|inv| {
                        RcDoc::text("invariant")
                            .append(RcDoc::line())
                            .append(inv.to_doc())
                            .nest(2)
                            .group()
                    }))
                    .nest(2),
                )
                .append(RcDoc::hardline())
                .append(pretty_block(body))
                .group(),
            StmtT::Return(v) => RcDoc::text("return")
                .append(RcDoc::line())
                .append(v.to_doc())
                .nest(2)
                .group(),
            StmtT::Error => todo!(),
        }
    }
}

impl PrettyIR for Stmts {
    fn to_doc(&self) -> RcDoc<'_, ()> {
        RcDoc::intersperse(self.iter().map(|stmt| stmt.to_doc()), RcDoc::hardline()).group()
    }
}
