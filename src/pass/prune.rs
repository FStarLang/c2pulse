use std::{
    collections::{HashMap, HashSet},
    hash::Hash,
    rc::Rc,
};

use crate::ir::*;

struct Deps<T> {
    roots: HashSet<T>,
    deps: HashMap<T, HashSet<T>>,
}

impl<T: Eq + Hash> Deps<T> {
    fn new() -> Deps<T> {
        Deps {
            roots: HashSet::new(),
            deps: HashMap::new(),
        }
    }

    fn add_root(&mut self, root: T) {
        self.roots.insert(root);
    }

    fn deps_for(&mut self, from: T) -> &mut HashSet<T> {
        self.deps.entry(from).or_default()
    }

    fn propagate(&mut self) -> PropagatedDeps<'_, T> {
        let mut todo = vec![];
        for r in &self.roots {
            if let Some(t) = self.deps.remove(r) {
                todo.push(t);
            }
        }
        while let Some(next) = todo.pop() {
            for n in next {
                if let Some(t) = self.deps.remove(&n) {
                    todo.push(t);
                }
                self.roots.insert(n);
            }
        }
        PropagatedDeps(self)
    }
}

struct PropagatedDeps<'a, T>(&'a Deps<T>);
impl<'a, T: Eq + Hash> PropagatedDeps<'a, T> {
    #[inline]
    fn contains(&self, t: &T) -> bool {
        self.0.roots.contains(t)
    }
}

#[derive(Debug, Hash, PartialEq, Eq, Clone)]
enum DeclName {
    Fn(Rc<str>),
    Struct(Rc<str>),
    Typedef(Rc<str>),
    Include(Rc<SourceInfo>),
}

fn in_main_file(mf: &Rc<str>, loc: &SourceInfo) -> bool {
    loc.location().file_name == *mf
}

fn scan_type(deps: &mut HashSet<DeclName>, ty: &Type) {
    match &ty.val {
        TypeT::Int {
            signed: _,
            width: _,
        } => {}
        TypeT::SizeT => {}
        TypeT::Pointer(to, kind) => {
            scan_type(deps, &*to);
            match kind {
                PointerKind::Unknown => {}
                PointerKind::Ref => {}
                PointerKind::Array => {}
            }
        }
        TypeT::Error => {}
        TypeT::Void => {}
        TypeT::SLProp => {}
        TypeT::Bool => {}
        TypeT::TypeRef(n) => {
            deps.insert(match n {
                TypeRefKind::Typedef(n) => DeclName::Typedef(n.val.clone()),
                TypeRefKind::Struct(n) => DeclName::Struct(n.val.clone()),
            });
        }
        TypeT::Requires(ty, p) | TypeT::Ensures(ty, p) => {
            scan_type(deps, ty);
            scan_rvalue(deps, p);
        }
        TypeT::Consumes(ty) | TypeT::Plain(ty) => scan_type(deps, ty),
        TypeT::SpecInt => {}
    }
}

fn scan_lvalue(deps: &mut HashSet<DeclName>, lv: &LValue) {
    match &lv.val {
        LValueT::Var(_) => {}
        LValueT::Deref(v) => scan_rvalue(deps, v),
        LValueT::Member(x, _a) => scan_lvalue(deps, x),
        LValueT::Error(ty) => scan_type(deps, ty),
    }
}

fn scan_rvalues(deps: &mut HashSet<DeclName>, rvs: &RValues) {
    for rv in rvs {
        scan_rvalue(deps, rv);
    }
}

fn scan_rvalue(deps: &mut HashSet<DeclName>, rv: &RValue) {
    match &rv.val {
        RValueT::IntLit(_, ty) => scan_type(deps, ty),
        RValueT::LValue(v) => scan_lvalue(deps, v),
        RValueT::Ref(v) => scan_lvalue(deps, v),
        RValueT::Cast(val, ty) => {
            scan_rvalue(deps, val);
            scan_type(deps, ty);
        }
        RValueT::Error(ty) => scan_type(deps, ty),
        RValueT::InlinePulse(_, ty) => scan_type(deps, ty),
        RValueT::BinOp(_, lhs, rhs) => {
            scan_rvalue(deps, lhs);
            scan_rvalue(deps, rhs);
        }
        RValueT::FnCall(f, args) => {
            deps.insert(DeclName::Fn(f.val.clone()));
            scan_rvalues(deps, args);
        }
        RValueT::BoolLit(_) => {}
        RValueT::Live(v) => scan_lvalue(deps, v),
        RValueT::Old(v) => scan_rvalue(deps, v),
    }
}

fn scan_stmt(deps: &mut HashSet<DeclName>, stmt: &Stmt) {
    match &stmt.val {
        StmtT::Call(v) => scan_rvalue(deps, v),
        StmtT::Decl(_name, ty) => scan_type(deps, ty),
        StmtT::Assign(_name, v) => scan_rvalue(deps, v),
        StmtT::If(c, b1, b2) => {
            scan_rvalue(deps, c);
            scan_stmts(deps, b1);
            scan_stmts(deps, b2)
        }
        StmtT::While(cond, invs, body) => {
            scan_rvalue(deps, cond);
            scan_rvalues(deps, invs);
            scan_stmts(deps, body);
        }
        StmtT::Return(v) => scan_rvalue(deps, v),
        StmtT::Error => {}
    }
}

fn scan_stmts(deps: &mut HashSet<DeclName>, stmts: &Vec<Rc<Stmt>>) {
    for stmt in stmts {
        scan_stmt(deps, stmt);
    }
}

fn decl_name(decl: &Decl) -> DeclName {
    match &decl.val {
        DeclT::FnDefn(fn_defn) => DeclName::Fn(fn_defn.decl.name.val.clone()),
        DeclT::FnDecl(fn_decl) => DeclName::Fn(fn_decl.name.val.clone()),
        DeclT::Typedef(type_defn) => DeclName::Typedef(type_defn.name.val.clone()),
        DeclT::StructDefn(struct_defn) => DeclName::Struct(struct_defn.name.val.clone()),
        DeclT::IncludeDecl(_) => DeclName::Include(decl.loc.clone()),
    }
}

fn scan_translation_unit(deps: &mut Deps<DeclName>, tu: &TranslationUnit) {
    fn scan_fn_decl(
        ds: &mut HashSet<DeclName>,
        FnDecl {
            name: _,
            ret_type,
            args,
            requires,
            ensures,
        }: &FnDecl,
    ) {
        for (_name, ty) in args {
            scan_type(ds, ty)
        }
        scan_type(ds, &ret_type);

        for r in requires {
            scan_rvalue(ds, r);
        }
        for e in ensures {
            scan_rvalue(ds, e);
        }
    }

    for decl in &tu.decls {
        let n = decl_name(decl);
        match &decl.val {
            DeclT::FnDefn(FnDefn { decl, body }) => {
                let ds = deps.deps_for(n);
                scan_fn_decl(ds, &decl);
                scan_stmts(ds, &body);
            }
            DeclT::FnDecl(fn_decl) => {
                scan_fn_decl(deps.deps_for(n), fn_decl);
            }
            DeclT::Typedef(TypeDefn { name: _, body }) => scan_type(deps.deps_for(n), body),
            DeclT::StructDefn(StructDefn { name: _, fields }) => {
                let ds = deps.deps_for(n);
                for (_name, ty) in fields {
                    scan_type(ds, ty)
                }
            }
            DeclT::IncludeDecl(_) => {}
        }
    }
}

pub fn prune(tu: &mut TranslationUnit) {
    let mut deps = Deps::new();
    scan_translation_unit(&mut deps, tu);
    for decl in &tu.decls {
        if in_main_file(&tu.main_file_name, &decl.loc) {
            deps.add_root(decl_name(&decl))
        }
    }
    let deps = deps.propagate();
    tu.decls.retain(|decl| deps.contains(&decl_name(decl)));
}
