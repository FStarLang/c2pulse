use crate::ir::*;
use ::pretty::RcDoc;

macro_rules! impl_display_using_prettyir {
    ($t:ty) => {
        impl Display for $t {
            fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
                write!(f, "{}", self.to_doc().pretty(80))
            }
        }
    };
}

pub trait PrettyIR {
    fn to_doc(&self) -> RcDoc<'_, ()>;
}

impl<A: PrettyIR> PrettyIR for Ast<A> {
    fn to_doc(&self) -> RcDoc<'_, ()> {
        self.val.to_doc()
    }
}

impl<A: PrettyIR + ?Sized> PrettyIR for Rc<A> {
    fn to_doc(&self) -> RcDoc<'_, ()> {
        A::to_doc(self)
    }
}

impl<'a, A: PrettyIR> PrettyIR for &'a A {
    fn to_doc(&self) -> RcDoc<'_, ()> {
        A::to_doc(self)
    }
}

impl PrettyIR for str {
    fn to_doc(&self) -> RcDoc<'_, ()> {
        RcDoc::text(self)
    }
}

impl_display_using_prettyir!(TypeRefKind);

impl PrettyIR for TypeRefKind {
    fn to_doc(&self) -> RcDoc<'_, ()> {
        match self {
            TypeRefKind::Typedef(n) => n.to_doc(),
            TypeRefKind::Struct(n) => RcDoc::text("struct")
                .append(RcDoc::line())
                .append(n.to_doc())
                .group()
                .nest(2),
        }
    }
}

impl_display_using_prettyir!(TypeT);

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
            TypeT::TypeRef(n) => n.to_doc(),
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

impl_display_using_prettyir!(RValueT);

impl PrettyIR for RValueT {
    fn to_doc(&self) -> RcDoc<'_, ()> {
        match self {
            RValueT::BoolLit(b) => RcDoc::text(if *b { "true" } else { "false" }),
            RValueT::IntLit(n, _ty) => RcDoc::text(n.to_string()),
            RValueT::LValue(lval) => lval.to_doc(),
            RValueT::Ref(lval) => RcDoc::text("&").append(lval.to_doc()),
            RValueT::UnOp(un_op, arg) => RcDoc::text(un_op.to_str()).append(arg.to_doc()),
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
            RValueT::FnCall(f, args) => f
                .to_doc()
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
            RValueT::StructInit(name, fields) => RcDoc::text("(")
                .append(name.to_doc())
                .append(") {")
                .append(RcDoc::intersperse(
                    fields.iter().map(|(fld, val)| {
                        RcDoc::text(".")
                            .append(fld.to_doc())
                            .append(" = ")
                            .append(val.to_doc())
                    }),
                    RcDoc::text(",").append(RcDoc::line()),
                ))
                .append("}"),
        }
    }
}

impl_display_using_prettyir!(LValueT);

impl PrettyIR for LValueT {
    fn to_doc(&self) -> RcDoc<'_, ()> {
        match self {
            LValueT::Var(x) => x.to_doc(),
            LValueT::Deref(rval) => RcDoc::text("(*").append(rval.to_doc()).append(")"),
            LValueT::Member(x, n) => x.to_doc().append(RcDoc::text(".")).append(n.to_doc()),
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
            .append("}")
            .group()
    }
}

impl PrettyIR for StmtT {
    fn to_doc(&self) -> RcDoc<'_, ()> {
        match self {
            StmtT::Call(f) => f.to_doc().append(";"),
            StmtT::Decl(x, ty) => (ty.to_doc())
                .append(" ")
                .append(x.to_doc())
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
                .append(") ")
                .append(pretty_block(b1))
                .append(" else ")
                .append(pretty_block(b2))
                .group(),
            StmtT::While(cond, invs, body) => RcDoc::text("while (")
                .append(cond.to_doc().nest(4))
                .append(")")
                .append(
                    RcDoc::concat(invs.iter().map(|inv| {
                        RcDoc::line().append(
                            RcDoc::text("_invariant(")
                                .append(RcDoc::line_())
                                .append(inv.to_doc())
                                .nest(2)
                                .append(")")
                                .group(),
                        )
                    }))
                    .nest(2),
                )
                .append(RcDoc::hardline())
                .append(pretty_block(body))
                .group(),
            StmtT::Return(v) => RcDoc::text("return")
                .append(RcDoc::line())
                .append(v.to_doc())
                .append(";")
                .nest(2)
                .group(),
            StmtT::Assert(v) => RcDoc::text("_assert(")
                .append(v.to_doc())
                .append(");")
                .nest(2)
                .group(),
            StmtT::Error => RcDoc::text("???;"),
        }
    }
}

impl PrettyIR for Stmts {
    fn to_doc(&self) -> RcDoc<'_, ()> {
        RcDoc::intersperse(self.iter().map(|stmt| stmt.to_doc()), RcDoc::hardline()).group()
    }
}

impl PrettyIR for StructDefn {
    fn to_doc(&self) -> RcDoc<'_, ()> {
        RcDoc::text("struct ")
            .append(self.name.to_doc())
            .append(" {")
            .append(RcDoc::line_())
            .append(RcDoc::concat(self.fields.iter().map(|f| {
                f.1.to_doc()
                    .append(RcDoc::line())
                    .append(f.0.to_doc())
                    .append(";")
                    .group()
                    .nest(2)
                    .append(RcDoc::hardline())
            })))
            .group()
            .nest(2)
            .append("};")
    }
}

impl PrettyIR for FnDecl {
    fn to_doc(&self) -> RcDoc<'_, ()> {
        self.ret_type
            .to_doc()
            .append(RcDoc::line())
            .append(self.name.to_doc())
            .append("(")
            .append(
                RcDoc::intersperse(
                    self.args.iter().map(|(n, ty)| {
                        ty.to_doc()
                            .append(match n {
                                Some(n) => RcDoc::line().append(n.to_doc()),
                                None => RcDoc::nil(),
                            })
                            .group()
                            .nest(2)
                    }),
                    RcDoc::text(",").append(RcDoc::line()),
                )
                .group()
                .nest(2),
            )
            .append(")")
            .group()
            .append(RcDoc::concat(self.requires.iter().map(|req| {
                RcDoc::hardline().append(
                    RcDoc::text("_requires(")
                        .append(req.to_doc())
                        .append(")")
                        .group()
                        .nest(2),
                )
            })))
            .append(RcDoc::concat(self.ensures.iter().map(|ens| {
                RcDoc::hardline().append(
                    RcDoc::text("_ensures(")
                        .append(ens.to_doc())
                        .append(")")
                        .group()
                        .nest(2),
                )
            })))
            .nest(2)
            .group()
    }
}

impl PrettyIR for FnDefn {
    fn to_doc(&self) -> RcDoc<'_, ()> {
        self.decl
            .to_doc()
            .append(RcDoc::hardline())
            .append(pretty_block(&self.body))
    }
}

impl PrettyIR for TypeDefn {
    fn to_doc(&self) -> RcDoc<'_, ()> {
        RcDoc::text("typedef")
            .append(RcDoc::line())
            .append(self.body.to_doc())
            .append(RcDoc::line())
            .append(self.name.to_doc())
            .append(";")
            .group()
            .nest(2)
    }
}

impl PrettyIR for IncludeDecl {
    fn to_doc(&self) -> RcDoc<'_, ()> {
        RcDoc::text("_include_pulse(")
            .append(RcDoc::hardline())
            .append(self.code.to_string())
            .group()
            .nest(2)
            .append(RcDoc::hardline())
            .append(")")
    }
}

impl PrettyIR for DeclT {
    fn to_doc(&self) -> RcDoc<'_, ()> {
        match self {
            DeclT::FnDefn(fn_defn) => fn_defn.to_doc(),
            DeclT::FnDecl(fn_decl) => fn_decl.to_doc(),
            DeclT::Typedef(type_defn) => type_defn.to_doc(),
            DeclT::StructDefn(struct_defn) => struct_defn.to_doc(),
            DeclT::IncludeDecl(include_decl) => include_decl.to_doc(),
        }
    }
}

impl_display_using_prettyir!(TranslationUnit);

impl PrettyIR for TranslationUnit {
    fn to_doc(&self) -> RcDoc<'_, ()> {
        let header_comment = RcDoc::text("// ").append(&*self.main_file_name);

        RcDoc::intersperse(
            std::iter::once(header_comment)
                .chain(self.decls.iter().map(|decl| decl.to_doc()))
                .map(|doc| doc.append(RcDoc::hardline())),
            RcDoc::hardline(),
        )
    }
}
