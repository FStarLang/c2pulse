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

impl<T> Deps<T>
where
    T: Eq + Hash,
{
    fn new() -> Deps<T> {
        Deps {
            roots: HashSet::new(),
            deps: HashMap::new(),
        }
    }

    fn add_root(&mut self, root: T) {
        self.roots.insert(root);
    }

    fn get_deps(&mut self, from: T) -> &mut HashSet<T> {
        self.deps.entry(from).or_default()
    }

    fn add_dep(&mut self, from: T, to: T) {
        self.get_deps(from).insert(to);
    }

    fn propagate(&mut self) {
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
    }
}

#[derive(Debug, Hash, PartialEq, Eq, Clone)]
enum DeclName {
    Fn(Rc<str>),
    Struct(Rc<str>),
}

fn in_main_file(mf: &Rc<str>, loc: &SourceInfo) -> bool {
    match loc {
        SourceInfo::None => false,
        SourceInfo::Original(location) => location.file_name == *mf,
    }
}

fn scan_type(deps: &mut HashSet<DeclName>, ty: &Type) {
    match &ty.val {
        TypeT::Int {
            signed: _,
            width: _,
        } => {}
        TypeT::SizeT => {}
        TypeT::Pointer { to, kind: _ } => scan_type(deps, &*to),
        TypeT::Error => {}
    }
}

fn scan_lvalue(deps: &mut HashSet<DeclName>, lv: &LValue) {
    match &lv.val {
        LValueT::Var(_) => {}
        LValueT::Deref(v) => scan_rvalue(deps, v),
        LValueT::Error(ty) => scan_type(deps, ty),
    }
}

fn scan_rvalue(deps: &mut HashSet<DeclName>, rv: &RValue) {
    match &rv.val {
        RValueT::IntLit { val: _, ty } => scan_type(deps, ty),
        RValueT::LValue(v) => scan_lvalue(deps, v),
        RValueT::Ref(v) => scan_lvalue(deps, v),
        RValueT::Cast { val, ty } => {
            scan_rvalue(deps, val);
            scan_type(deps, ty);
        }
        RValueT::Error(ty) => scan_type(deps, ty),
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
        StmtT::Return(v) => scan_rvalue(deps, v),
        StmtT::Error => todo!(),
    }
}

fn scan_stmts(deps: &mut HashSet<DeclName>, stmts: &Vec<Rc<Stmt>>) {
    for stmt in stmts {
        scan_stmt(deps, stmt);
    }
}

fn scan_translation_unit(deps: &mut Deps<DeclName>, tu: &TranslationUnit) {
    for decl in &tu.decls {
        let in_main_file = in_main_file(&tu.main_file_name, &*decl.loc);
        let mut scan_fn_decl = |decl: &FnDecl| -> DeclName {
            let n = DeclName::Fn(decl.name.val.clone());

            if in_main_file {
                deps.add_root(n.clone());
            }
            let ds = deps.get_deps(n.clone());
            for arg in &decl.args {
                scan_type(ds, &arg.1)
            }
            scan_type(ds, &decl.ret_type);

            n
        };
        match &decl.val {
            Decl::FnDefn(fn_defn) => {
                let n = scan_fn_decl(&fn_defn.decl);
                scan_stmts(deps.get_deps(n), &fn_defn.body);
            }
            Decl::FnDecl(fn_decl) => {
                scan_fn_decl(fn_decl);
            }
            Decl::StructDefn(struct_defn) => {
                let n = DeclName::Struct(struct_defn.name.val.clone());
                let ds = deps.get_deps(n);
                for fld in &struct_defn.fields {
                    scan_type(ds, &fld.1)
                }
            }
            Decl::IncludeDecl(_) => {}
        }
    }
}

pub fn prune(tu: &mut TranslationUnit) {
    let mut deps = Deps::new();
    scan_translation_unit(&mut deps, tu);
    deps.propagate();
    tu.decls.retain(|decl| {
        in_main_file(&tu.main_file_name, &*decl.loc)
            || (match &decl.val {
                Decl::FnDefn(fn_defn) => deps
                    .roots
                    .contains(&DeclName::Fn(fn_defn.decl.name.val.clone())),
                Decl::FnDecl(fn_decl) => {
                    deps.roots.contains(&DeclName::Fn(fn_decl.name.val.clone()))
                }
                Decl::StructDefn(struct_defn) => deps
                    .roots
                    .contains(&DeclName::Struct(struct_defn.name.val.clone())),
                Decl::IncludeDecl(_) => true,
            })
    });
}
