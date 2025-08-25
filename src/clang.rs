use crate::ir::*;
use std::{collections::{HashMap, HashSet}, hash::Hash, rc::Rc};

struct Handles<K, T> {
    vals: HashMap<K, T>,
    ctr: usize,
}

impl<K, T> Handles<K, T>
where
    K: Into<usize> + From<usize> + Hash + Eq,
{
    fn new() -> Handles<K, T> {
        Handles {
            vals: HashMap::new(),
            ctr: 0,
        }
    }

    fn insert(&mut self, t: T) -> K {
        let i = self.ctr;
        self.vals.insert(i.into(), t);
        self.ctr += 1;
        i.into()
    }

    fn remove(&mut self, i: K) -> T {
        self.vals.remove(&i).unwrap()
    }
}

#[cxx::bridge]
mod ffi {
    #[derive(PartialEq, Eq, Hash)]
    struct LocationH {
        idx: usize,
    }
    #[derive(PartialEq, Eq, Hash)]
    struct LValueH {
        idx: usize,
    }
    struct RValueH {
        idx: usize,
    }
    struct DeclH {
        idx: usize,
    }
    struct TypeH {
        idx: usize,
    }

    extern "Rust" {
        type Ctx;
        fn mk_location(
            self: &mut Ctx,
            file_name: &str,
            start_line: u32,
            start_char: u32,
            end_line: u32,
            end_char: u32,
        ) -> LocationH;
    }

    unsafe extern "C++" {
        include!("c2pulse/src/clang_bridge.h");
        fn parse_file(ctx: &mut Ctx, file_name: &str) -> DeclH;
    }
}

macro_rules! impl_handle_traits {
    ($x:path) => {
        impl Into<usize> for $x {
            fn into(self) -> usize {
                self.idx
            }
        }
        impl From<usize> for $x {
            fn from(value: usize) -> Self {
                $x { idx: value }
            }
        }
    };
}

impl_handle_traits!(ffi::LocationH);
impl_handle_traits!(ffi::LValueH);
impl_handle_traits!(ffi::RValueH);
impl_handle_traits!(ffi::DeclH);
impl_handle_traits!(ffi::TypeH);

struct Ctx {
    file_names: HashSet<Rc<str>>,
    lvalue_handles: Handles<ffi::LValueH, Rc<LValue>>,
    loc_handles: Handles<ffi::LocationH, Rc<Location>>,
}

impl Ctx {
    fn mk_location(
        &mut self,
        file_name: &str,
        start_line: u32,
        start_char: u32,
        end_line: u32,
        end_char: u32,
    ) -> ffi::LocationH {
        let file_name =
            match self.file_names.get(file_name) {
                Some(file_name) => file_name.clone(),
                None => {
                    let file_name: Rc<str> = Rc::from(file_name);
                    self.file_names.insert(file_name.clone());
                    file_name
                }
            };
        self.loc_handles.insert(Rc::new(Location {
            file_name: file_name,
            range: Range {
                start: Position {
                    line: start_line,
                    character: start_char,
                },
                end: Position {
                    line: end_line,
                    character: end_char,
                },
            },
        }))
    }
}
