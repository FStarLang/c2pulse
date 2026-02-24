use std::rc::Rc;

use crate::ir::*;

fn stmt_contains_goto(stmt: &Stmt, label: &str) -> bool {
    match &stmt.val {
        StmtT::Goto(l) => *l.val == *label,
        StmtT::If(_, b1, b2) => stmts_contain_goto(b1, label) || stmts_contain_goto(b2, label),
        StmtT::While { body, .. } => stmts_contain_goto(body, label),
        StmtT::GotoBlock { body, .. } => stmts_contain_goto(body, label),
        _ => false,
    }
}

fn stmts_contain_goto(stmts: &[Rc<Stmt>], label: &str) -> bool {
    stmts.iter().any(|s| stmt_contains_goto(s, label))
}

fn restructure_stmts(stmts: &mut Vec<Rc<Stmt>>) {
    // First, recursively restructure nested statement lists
    for stmt in stmts.iter_mut() {
        restructure_stmt(Rc::make_mut(stmt));
    }

    // Find and restructure label statements
    // Process from the end so indices remain valid
    let label_positions: Vec<(usize, Rc<Ident>, Rc<Exprs>)> = stmts
        .iter()
        .enumerate()
        .filter_map(|(i, s)| match &s.val {
            StmtT::Label { name, ensures } => Some((i, name.clone(), ensures.clone())),
            _ => None,
        })
        .collect();

    for (label_idx, label_name, label_ensures) in label_positions.into_iter().rev() {
        // Find the earliest statement containing a goto to this label
        let first_goto = stmts[..label_idx]
            .iter()
            .position(|s| stmt_contains_goto(s, &label_name.val));

        if let Some(start) = first_goto {
            // Extract statements [start..label_idx) into the block body
            let body: Vec<Rc<Stmt>> = stmts[start..label_idx].to_vec();
            let loc = stmts[label_idx].loc.clone();

            let goto_block = StmtT::GotoBlock {
                body: Rc::new(body),
                label: label_name,
                ensures: label_ensures,
            }
            .with_loc(loc);

            // Replace [start..=label_idx] with the single GotoBlock
            stmts.splice(start..=label_idx, std::iter::once(goto_block));
        }
    }
}

fn restructure_stmt(stmt: &mut Stmt) {
    match &mut stmt.val {
        StmtT::If(_, b1, b2) => {
            restructure_stmts(Rc::make_mut(b1));
            restructure_stmts(Rc::make_mut(b2));
        }
        StmtT::While { body, .. } => {
            restructure_stmts(Rc::make_mut(body));
        }
        StmtT::GotoBlock { body, .. } => {
            restructure_stmts(Rc::make_mut(body));
        }
        _ => {}
    }
}

fn restructure_decl(decl: &mut Decl) {
    match &mut decl.val {
        DeclT::FnDefn(FnDefn { body, .. }) => {
            restructure_stmts(body);
        }
        _ => {}
    }
}

pub fn restructure_goto(tu: &mut TranslationUnit) {
    for decl in &mut tu.decls {
        restructure_decl(decl);
    }
}
