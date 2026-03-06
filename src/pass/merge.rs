use std::collections::HashMap;
use std::rc::Rc;

use crate::{
    diag::{Diagnostic, DiagnosticLevel, Diagnostics},
    env::Env,
    ir::*,
};

fn report(diags: &mut Diagnostics, msg: String, loc: &SourceInfo) {
    diags.report(Diagnostic {
        loc: loc.location().clone(),
        level: DiagnosticLevel::Error,
        msg,
    });
}

fn types_match(env: &Env, decl: &FnDecl, defn: &FnDecl) -> bool {
    if decl.args.len() != defn.args.len() {
        return false;
    }
    for (arg_a, arg_b) in decl.args.iter().zip(defn.args.iter()) {
        if !env.vtype_eq(arg_a.ty.clone().into(), arg_b.ty.clone().into()) {
            return false;
        }
    }
    env.vtype_eq(decl.ret_type.clone().into(), defn.ret_type.clone().into())
}

pub fn merge(diags: &mut Diagnostics, tu: &mut TranslationUnit) {
    // Build index: fn name → position of its FnDefn in tu.decls
    let mut defn_indices: HashMap<Rc<str>, usize> = HashMap::new();
    for (i, decl) in tu.decls.iter().enumerate() {
        if let DeclT::FnDefn(fn_defn) = &decl.val {
            defn_indices.insert(fn_defn.decl.name.val.clone(), i);
        }
    }

    // Track which FnDecl indices to remove (merged into their FnDefn)
    let mut to_remove: Vec<usize> = Vec::new();

    // Build an Env for type comparison (need typedef resolution)
    let mut env = Env::new();
    for decl in tu.decls.iter() {
        env.push_decl(decl);
    }

    // Match each FnDecl to its FnDefn
    for (i, decl) in tu.decls.iter().enumerate() {
        if let DeclT::FnDecl(fn_decl) = &decl.val {
            if let Some(&defn_idx) = defn_indices.get(&fn_decl.name.val) {
                let defn = match &tu.decls[defn_idx].val {
                    DeclT::FnDefn(fd) => fd,
                    _ => unreachable!(),
                };

                // Validate types match
                if !types_match(&env, fn_decl, &defn.decl) {
                    report(
                        diags,
                        format!(
                            "declaration of {} has different types than its definition",
                            fn_decl.name.val
                        ),
                        &fn_decl.name.loc,
                    );
                    continue;
                }

                let decl_has_specs = !fn_decl.requires.is_empty() || !fn_decl.ensures.is_empty();
                let defn_has_specs =
                    !defn.decl.requires.is_empty() || !defn.decl.ensures.is_empty();

                if defn_has_specs && !decl_has_specs {
                    // Specs on the definition but not on the declaration — error
                    report(
                        diags,
                        format!(
                            "definition of {} has specifications, but its declaration does not; \
                             specifications should be on the declaration",
                            fn_decl.name.val
                        ),
                        &defn.decl.name.loc,
                    );
                    continue;
                }

                if decl_has_specs && defn_has_specs {
                    // Both have specs — they must match
                    if fn_decl.requires != defn.decl.requires
                        || fn_decl.ensures != defn.decl.ensures
                    {
                        report(
                            diags,
                            format!(
                                "declaration and definition of {} have differing specifications",
                                fn_decl.name.val
                            ),
                            &fn_decl.name.loc,
                        );
                        continue;
                    }
                }

                // Mark FnDecl for removal — it will be merged into the FnDefn
                to_remove.push(i);
            }
        }
    }

    // Copy specs from declarations into definitions before removing them
    // (need a separate pass to avoid borrow conflicts)
    for &i in &to_remove {
        let fn_decl = match &tu.decls[i].val {
            DeclT::FnDecl(fd) => fd.clone(),
            _ => unreachable!(),
        };
        if let Some(&defn_idx) = defn_indices.get(&fn_decl.name.val) {
            if let DeclT::FnDefn(ref mut defn) = tu.decls[defn_idx].val {
                if defn.decl.requires.is_empty() && defn.decl.ensures.is_empty() {
                    defn.decl.requires = fn_decl.requires;
                    defn.decl.ensures = fn_decl.ensures;
                }
            }
        }
    }

    // Remove merged FnDecls (iterate in reverse to preserve indices)
    to_remove.sort_unstable();
    for &i in to_remove.iter().rev() {
        tu.decls.remove(i);
    }
}
