use std::{collections::HashMap, fmt::Display, rc::Rc, str::FromStr};

use chumsky::{
    Parser,
    input::{IterInput, ValueInput},
    prelude::*,
};
use num_bigint::BigInt;

use crate::{
    diag::{Diagnostic, DiagnosticLevel, Diagnostics},
    ir::*,
};

#[derive(Debug, PartialEq, Eq, Hash, Clone, Copy)]
enum IntegerSuffix {
    None,
    U,
    UL,
    ULL,
    UZ,
    L,
    LL,
    Z,
}
impl IntegerSuffix {
    fn make_unsigned(self) -> IntegerSuffix {
        match self {
            IntegerSuffix::None => IntegerSuffix::U,
            IntegerSuffix::L => IntegerSuffix::UL,
            IntegerSuffix::LL => IntegerSuffix::ULL,
            IntegerSuffix::Z => IntegerSuffix::UZ,
            _ => self,
        }
    }
    fn to_str(self) -> &'static str {
        match self {
            IntegerSuffix::None => "",
            IntegerSuffix::U => "u",
            IntegerSuffix::UL => "ul",
            IntegerSuffix::ULL => "ull",
            IntegerSuffix::UZ => "uz",
            IntegerSuffix::L => "l",
            IntegerSuffix::LL => "ll",
            IntegerSuffix::Z => "z",
        }
    }
}
impl Display for IntegerSuffix {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.to_str())
    }
}

macro_rules! mk_punct_table {
    { $($n:ident => $s:expr,)* } => {
        #[derive(Debug, PartialEq, Eq, Hash, Clone, Copy)]
        enum Punct {
            $($n,)*
        }

        impl Punct {
            fn to_str(self) -> &'static str {
                match self {
                    $(Punct::$n => $s,)*
                }
            }

            fn lexer<'src>() -> impl Parser<'src, &'src str, Punct> {
                choice([
                    $(just($s).to(Punct::$n),)*
                ])
            }
        }
    }
}

mk_punct_table! {
    StarEq => "*=",
    SlashEq => "/=",
    PercEq => "%=",
    PlusEq => "+=",
    DashEq => "-=",
    LtLtEq => "<<=",
    GtGtEq => ">>=",
    AmpEq => "&=",
    HatEq => "^=",
    PipeEq => "|=",

    LBracket => "[",
    RBracket => "]",
    LParen => "(",
    RParen => ")",
    LBrace => "{",
    RBrace => "}",
    DotDotDot => "...",
    Dot => ".",
    DashGt => "->",

    PlusPlus => "++",
    DashDash => "--",
    AmpAmp => "&&",
    Amp => "&",
    Star => "*",
    Plus => "+",
    Dash => "-",
    Tilde => "~",
    BangEq => "!=",
    Bang => "!",

    Slash => "/",
    Perc => "%",
    LtLt => "<<",
    GtGt => ">>",
    LtEq => "<=",
    GtEq => ">=",
    Lt => "<",
    Gt => ">",
    EqEq => "==",
    EqEqGt => "==>",
    Hat => "^",
    PipePipe => "||",
    Pipe => "|",

    Question => "?",
    ColonColon => "::",
    Colon => ":",
    Semi => ";",

    Eq => "=",

    Comma => ",",
    HashHash => "##",
    Hash => "#",

    Dollar => "$",
    Backtick => "`",
}

#[derive(Debug, PartialEq, Eq, Hash, Clone, Copy)]
enum Token<'src> {
    Whitespace,

    String(&'src str),
    Integer(&'src str, IntegerSuffix),
    Ident(&'src str),

    Punct(Punct),

    Error,
}

impl<'src> Display for Token<'src> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Token::Whitespace => write!(f, " "),
            Token::String(tok) => write!(f, "{}", tok),
            Token::Integer(i, suffix) => write!(f, "{}{}", i, suffix),
            Token::Ident(id) => write!(f, "{}", id),
            Token::Punct(punct) => write!(f, "{}", punct.to_str()),
            Token::Error => write!(f, "(LEXING ERROR)"),
        }
    }
}

fn lex_core_token<'src>() -> impl Parser<'src, &'src str, Token<'src>> {
    let integer_suffix_l = Parser::or(
        one_of("lL").ignore_then(
            one_of("lL")
                .to(IntegerSuffix::LL)
                .or(empty().to(IntegerSuffix::L)),
        ),
        one_of("zZ").to(IntegerSuffix::Z),
    );
    let integer_suffix = choice((
        one_of("uU").ignore_then(
            integer_suffix_l
                .or_not()
                .map(|s| s.unwrap_or(IntegerSuffix::None))
                .map(IntegerSuffix::make_unsigned),
        ),
        integer_suffix_l
            .then(one_of("uU").or_not())
            .map(|(s, u)| if u.is_some() { s.make_unsigned() } else { s }),
        empty().to(IntegerSuffix::None),
    ));

    let decimal_literal = text::int(10); // also happens to accept 0, which should be an octal literal in C

    let integer_literal = decimal_literal
        .then(integer_suffix)
        .map(|(i, s)| Token::Integer(i, s));

    let op = Punct::lexer().map(Token::Punct);

    let ident = text::ident().map(Token::Ident); // as C demands: XID_Start XID_Continue*

    // TODO FIXME
    let string = just('"')
        .not()
        .repeated()
        .to_slice()
        .delimited_by(just('"'), just('"'))
        .map(Token::String);

    let fallback = text::whitespace()
        .not()
        .repeated()
        .at_least(1)
        .to(Token::Error);

    integer_literal.or(op).or(ident).or(string).or(fallback)
}

fn ws<'tokens, 'src: 'tokens, I: ValueInput<'tokens, Token = Token<'src>, Span = Span>, Span>()
-> impl Parser<'tokens, I, (), extra::Err<Rich<'tokens, Token<'src>, Span>>> + Clone {
    select! { Token::Whitespace => () }.repeated()
}

fn punct<'tokens, 'src: 'tokens, I: ValueInput<'tokens, Token = Token<'src>, Span = Span>, Span>(
    op: Punct,
) -> impl Parser<'tokens, I, Token<'src>, extra::Err<Rich<'tokens, Token<'src>, Span>>> + Clone {
    just(Token::Punct(op)).padded_by(ws())
}

#[derive(Debug, Clone, Copy)]
pub struct TargetIntWidths {
    pub char_width: u32,
    pub short_width: u32,
    pub int_width: u32,
    pub long_width: u32,
    pub long_long_width: u32,
}

impl Default for TargetIntWidths {
    fn default() -> Self {
        // LP64 defaults (Linux/macOS amd64)
        TargetIntWidths {
            char_width: 8,
            short_width: 16,
            int_width: 32,
            long_width: 64,
            long_long_width: 64,
        }
    }
}

#[derive(Debug, Default, Clone)]
pub struct SnippetMap {
    pub snippets: HashMap<u32, InlineCode>,
}

trait SourceInfoForTokens {
    fn resolve_source_info(&self, span: &SimpleSpan) -> Rc<SourceInfo>;
    fn resolve_error_location(&self, span: &SimpleSpan) -> Location;
}

// With unified ExprT, the parser Expr is just Rc<crate::ir::Expr>.
// We use a newtype for convenience methods.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
struct Expr(Rc<crate::ir::Expr>);

impl Expr {
    fn to_rvalue(self) -> Rc<crate::ir::Expr> {
        self.0
    }
}
impl From<Rc<crate::ir::Expr>> for Expr {
    fn from(value: Rc<crate::ir::Expr>) -> Self {
        Expr(value)
    }
}

macro_rules! left_recursion {
    ($base:expr, { $($n:ident($acc:ident, $x:ident: $t:ty: $p:expr) $(= $extra:ident)? => $cb:expr,)* }) => {{
        enum Rhs {
            $($n($t),)*
        }
        let rhs = choice((
            $(($p).map(Rhs::$n),)*
        ));
        ($base).try_foldl(rhs.repeated(), |acc, rhs, _extra| match rhs {
            $(Rhs::$n($x) => {
                let $acc = acc;
                $(let $extra = _extra;)?
                $cb
            },)*
        })
    }}
}

macro_rules! left_rec_binop {
    ($base:expr, { $($n:ident($acc:ident, $p:expr, $x:ident) $(= $extra:ident)? => $cb:expr,)* }) => {{
        let base = $base;
        left_recursion!(base, {$($n($acc, $x: Expr: $p.ignore_then(base.clone())) $(= $extra)? => Ok($cb),)*})
    }}
}

macro_rules! and_then {
    ($left:expr, { $($n:ident($x:ident, $y:ident: $t:ty: $p:expr) $(= $extra:ident)? => $cb:expr,)* }) => {{
        enum Rhs {
            $($n($t),)*
        }
        let rhs = choice((
            $(($p).map(Rhs::$n),)*
        ));
        $left.then(rhs).map_with(|(x, y), e| {
            match y {
                $(Rhs::$n($y) => {
                    let $x = x;
                    $(let $extra = e;)?
                    $cb
                },)*
            }
        })
    }}
}

fn mk_binop(binop: BinOp, lhs: Expr, rhs: Expr, loc: Rc<SourceInfo>) -> Expr {
    ExprT::BinOp(binop, lhs.to_rvalue(), rhs.to_rvalue())
        .with_loc(loc)
        .into()
}

type Extra<'tokens, 'src> = extra::Err<Rich<'tokens, Token<'src>, SimpleSpan>>;

fn type_parser<
    'tokens,
    'src: 'tokens,
    I: ValueInput<'tokens, Token = Token<'src>, Span = SimpleSpan>,
    SIFT: SourceInfoForTokens,
>(
    sift: &'src SIFT,
    target_widths: &'src TargetIntWidths,
) -> impl Parser<'tokens, I, Rc<crate::ir::Type>, Extra<'tokens, 'src>> + Clone {
    let signedness = select! {
        Token::Ident("signed") => true,
        Token::Ident("unsigned") => false,
    }
    .padded_by(ws());

    let size_modifier = select! {
        Token::Ident("short") => 0u8,
        Token::Ident("long") => 1u8,
    }
    .padded_by(ws())
    .then(
        select! { Token::Ident("long") => () }
            .padded_by(ws())
            .or_not(),
    )
    .map(|(m, extra_long)| match (m, extra_long) {
        (0, _) => 0u8,
        (1, None) => 1u8,
        (1, Some(())) => 2u8,
        _ => unreachable!(),
    });

    let int_or_char = select! {
        Token::Ident("int") => false,
        Token::Ident("char") => true,
    }
    .padded_by(ws());

    let tw = *target_widths;
    let integer_type = signedness
        .or_not()
        .then(size_modifier.or_not())
        .then(int_or_char.or_not())
        .try_map(move |((sign_opt, size_opt), base_opt), span| {
            if sign_opt.is_none() && size_opt.is_none() && base_opt.is_none() {
                return Err(Rich::custom(span, "expected type specifier"));
            }
            let signed = sign_opt.unwrap_or(true);
            let is_char = base_opt == Some(true);
            let width = if is_char {
                tw.char_width
            } else {
                match size_opt {
                    Some(0) => tw.short_width,
                    Some(1) => tw.long_width,
                    Some(2) => tw.long_long_width,
                    None | Some(_) => tw.int_width,
                }
            };
            Ok(TypeT::Int { signed, width })
        });

    let ident = select! { Token::Ident(ident) => ident }
        .map_with(|ident, e| Rc::<str>::from(ident).with_loc(sift.resolve_source_info(&e.span())))
        .padded_by(ws());

    let struct_type = select! { Token::Ident("struct") => () }
        .padded_by(ws())
        .ignore_then(ident.clone())
        .map(|name| TypeT::TypeRef(TypeRefKind::Struct(name)));

    let base_type = choice((
        select! {
            Token::Ident("_slprop") => TypeT::SLProp,
            Token::Ident("_specint") => TypeT::SpecInt,
            Token::Ident("_specnat") => TypeT::SpecNat,
            Token::Ident("void") => TypeT::Void,
            Token::Ident("size_t") => TypeT::SizeT,
        }
        .padded_by(ws()),
        select! {
            Token::Ident("bool") => TypeT::Bool,
            Token::Ident("_Bool") => TypeT::Bool,
        }
        .padded_by(ws()),
        struct_type,
        integer_type,
        ident
            .clone()
            .map(|name| TypeT::TypeRef(TypeRefKind::Typedef(name))),
    ));

    base_type
        .map_with(|ty, e| ty.with_loc(sift.resolve_source_info(&e.span())))
        .then(punct(Punct::Star).repeated().collect::<Vec<_>>())
        .map_with(|(base_ty, stars), e| {
            let loc = sift.resolve_source_info(&e.span());
            stars.into_iter().fold(base_ty, |inner, _| {
                TypeT::Pointer(inner, PointerKind::Unknown).with_loc(loc.clone())
            })
        })
}

fn expr_parser<
    'tokens,
    'src: 'tokens,
    I: ValueInput<'tokens, Token = Token<'src>, Span = SimpleSpan>,
    SIFT: SourceInfoForTokens,
>(
    snip_map: &'src SnippetMap,
    sift: &'src SIFT,
    target_widths: &'src TargetIntWidths,
) -> impl Parser<'tokens, I, Expr, Extra<'tokens, 'src>> + Clone {
    recursive(|expr| {
        let assignment_expression = expr.clone();

        // let expression = assignment_expression;

        // Parse C integer type specifiers: [signed|unsigned] [short|long|long long] [int|char]
        let signedness = select! {
            Token::Ident("signed") => true,
            Token::Ident("unsigned") => false,
        }
        .padded_by(ws());

        let size_modifier = select! {
            Token::Ident("short") => 0u8,
            Token::Ident("long") => 1u8,
        }
        .padded_by(ws())
        .then(
            select! { Token::Ident("long") => () }
                .padded_by(ws())
                .or_not(),
        )
        .map(|(m, extra_long)| match (m, extra_long) {
            (0, _) => 0u8,        // short
            (1, None) => 1u8,     // long
            (1, Some(())) => 2u8, // long long
            _ => unreachable!(),
        });

        let int_or_char = select! {
            Token::Ident("int") => false,
            Token::Ident("char") => true,
        }
        .padded_by(ws());

        let tw = *target_widths;
        let integer_type = signedness
            .or_not()
            .then(size_modifier.or_not())
            .then(int_or_char.or_not())
            .try_map(move |((sign_opt, size_opt), base_opt), span| {
                if sign_opt.is_none() && size_opt.is_none() && base_opt.is_none() {
                    return Err(Rich::custom(span, "expected type specifier"));
                }
                let signed = sign_opt.unwrap_or(true);
                let is_char = base_opt == Some(true);
                let width = if is_char {
                    tw.char_width
                } else {
                    match size_opt {
                        Some(0) => tw.short_width,
                        Some(1) => tw.long_width,
                        Some(2) => tw.long_long_width,
                        None | Some(_) => tw.int_width,
                    }
                };
                Ok(TypeT::Int { signed, width })
            });

        let ident = select! { Token::Ident(ident) => ident }
            .map_with(|ident, e| {
                Rc::<str>::from(ident).with_loc(sift.resolve_source_info(&e.span()))
            })
            .padded_by(ws());

        let struct_type = select! { Token::Ident("struct") => () }
            .padded_by(ws())
            .ignore_then(ident.clone())
            .map(|name| TypeT::TypeRef(TypeRefKind::Struct(name)));

        let base_type = choice((
            select! {
                Token::Ident("_slprop") => TypeT::SLProp,
                Token::Ident("_specint") => TypeT::SpecInt,
                Token::Ident("_specnat") => TypeT::SpecNat,
                Token::Ident("void") => TypeT::Void,
                Token::Ident("size_t") => TypeT::SizeT,
            }
            .padded_by(ws()),
            select! {
                Token::Ident("bool") => TypeT::Bool,
                Token::Ident("_Bool") => TypeT::Bool,
            }
            .padded_by(ws()),
            struct_type,
            integer_type,
        ));

        let type_name = base_type
            .map_with(|ty, e| ty.with_loc(sift.resolve_source_info(&e.span())))
            .then(punct(Punct::Star).repeated().collect::<Vec<_>>())
            .map_with(|(base_ty, stars), e| {
                let loc = sift.resolve_source_info(&e.span());
                stars.into_iter().fold(base_ty, |inner, _| {
                    TypeT::Pointer(inner, PointerKind::Unknown).with_loc(loc.clone())
                })
            });

        let inline_pulse = select! { Token::Ident("_inline_pulse") => () }
            .ignore_then(
                select! { Token::Integer(i, _) => i }
                    .delimited_by(punct(Punct::LParen), punct(Punct::RParen)),
            )
            .try_map(|i, span| {
                let snip = str::parse::<u32>(i)
                    .ok()
                    .and_then(|i| snip_map.snippets.get(&i));
                match snip {
                    Some(snip) => {
                        let fallback_loc = sift.resolve_source_info(&span);
                        let code =
                            process_inline_pulse(&fallback_loc, snip, snip_map, target_widths);
                        Ok(Rc::new(code))
                    }
                    None => Err(Rich::custom(span, format!("snippet {} not found", i))),
                }
            })
            .boxed();

        let identifier = ident.clone().map(|i| {
            let loc = i.loc.clone();
            ExprT::Var(i).with_loc(loc).into()
        });

        let integer_constant =
            select! { Token::Integer(i, suf) => (i,suf) }.try_map(move |(i, suf), span| {
                match BigInt::from_str(i) {
                    Ok(i) => {
                        let loc = sift.resolve_source_info(&span);
                        let ty_val = match suf {
                            IntegerSuffix::None => TypeT::SpecInt,
                            IntegerSuffix::U => TypeT::Int {
                                signed: false,
                                width: tw.int_width,
                            },
                            IntegerSuffix::L => TypeT::Int {
                                signed: true,
                                width: tw.long_width,
                            },
                            IntegerSuffix::UL => TypeT::Int {
                                signed: false,
                                width: tw.long_width,
                            },
                            IntegerSuffix::LL => TypeT::Int {
                                signed: true,
                                width: tw.long_long_width,
                            },
                            IntegerSuffix::ULL => TypeT::Int {
                                signed: false,
                                width: tw.long_long_width,
                            },
                            IntegerSuffix::Z | IntegerSuffix::UZ => TypeT::SizeT,
                        };
                        let ty = ty_val.with_loc(loc.clone());
                        Ok(ExprT::IntLit(Rc::new(i), ty).with_loc(loc).into())
                    }
                    Err(err) => Err(Rich::custom(span, err)),
                }
            });

        let constant = integer_constant;

        let parenthesized = expr
            .clone()
            .delimited_by(punct(Punct::LParen), punct(Punct::RParen));

        let primary_expression = identifier.or(constant).or(parenthesized);

        let quantifier = select! {
            Token::Ident("_forall") => true,
            Token::Ident("_exists") => false,
        }
        .padded_by(ws())
        .then(
            type_name
                .clone()
                .then(ident.clone())
                .then_ignore(punct(Punct::Comma))
                .then(assignment_expression.clone())
                .delimited_by(punct(Punct::LParen), punct(Punct::RParen)),
        )
        .map_with(|(is_forall, ((ty, var), body)): (bool, _), extra| -> Expr {
            let loc = sift.resolve_source_info(&extra.span());
            let body: Expr = body;
            let body_rv = body.to_rvalue();
            if is_forall {
                ExprT::Forall(var, ty, body_rv).with_loc(loc).into()
            } else {
                ExprT::Exists(var, ty, body_rv).with_loc(loc).into()
            }
        })
        .boxed();

        let postfix_expression_nonrec = choice((
            quantifier,
            ident
                .clone() // TODO: function should be postfix_expression
                .then(
                    assignment_expression
                        .clone()
                        .separated_by(punct(Punct::Comma))
                        .collect::<Vec<_>>()
                        .delimited_by(punct(Punct::LParen), punct(Punct::RParen)),
                )
                .try_map(|(f, args): (_, Vec<Expr>), s| match &*f.val {
                    "_old" => {
                        let &[arg] = &args.as_slice() else {
                            return Err(Rich::custom(s, "_old takes exactly one argument"));
                        };
                        let arg: &Expr = arg;
                        Ok(ExprT::Old(arg.clone().to_rvalue())
                            .with_loc(sift.resolve_source_info(&s))
                            .into())
                    }
                    "_live" => {
                        let &[arg] = &args.as_slice() else {
                            return Err(Rich::custom(s, "_live takes exactly one argument"));
                        };
                        Ok(ExprT::Live(arg.clone().to_rvalue())
                            .with_loc(sift.resolve_source_info(&s))
                            .into())
                    }
                    _ => Ok(ExprT::FnCall(
                        f,
                        args.into_iter().map(|e: Expr| e.to_rvalue()).collect(),
                    )
                    .with_loc(sift.resolve_source_info(&s))
                    .into()),
                })
                .boxed(),
            primary_expression.boxed(),
        ))
        .boxed();
        let postfix_expression = left_recursion!(postfix_expression_nonrec, {
            Index(lhs, idx: Expr: assignment_expression.clone().delimited_by(punct(Punct::LBracket), punct(Punct::RBracket))) = e =>
                Ok(ExprT::Index(lhs.to_rvalue(), idx.to_rvalue()).with_loc(sift.resolve_source_info(&e.span())).into()),
            Dot(lhs, id: Rc<Ident>: punct(Punct::Dot).ignore_then(ident.clone())) = e =>
                Ok(ExprT::Member(lhs.to_rvalue(), id).with_loc(sift.resolve_source_info(&e.span())).into()),
            Arrow(lhs, id: Rc<Ident>: punct(Punct::DashGt).ignore_then(ident.clone())) = e => {
                let loc = sift.resolve_source_info(&e.span());
                Ok(ExprT::Member(ExprT::Deref(lhs.to_rvalue()).with_loc(loc.clone()), id).with_loc(loc).into())
            },
        }).boxed();

        let cast_expression = recursive(|cast_expression| {
            let unary_expression = choice((
                postfix_expression,
                punct(Punct::Star)
                    .ignore_then(cast_expression.clone())
                    .map_with(|e: Expr, extra| {
                        ExprT::Deref(e.to_rvalue())
                            .with_loc(sift.resolve_source_info(&extra.span()))
                            .into()
                    }),
                punct(Punct::Amp)
                    .ignore_then(cast_expression.clone())
                    .map_with(|e: Expr, extra| {
                        ExprT::Ref(e.to_rvalue())
                            .with_loc(sift.resolve_source_info(&extra.span()))
                            .into()
                    }),
                punct(Punct::Bang)
                    .ignore_then(cast_expression.clone())
                    .map_with(|e: Expr, extra| {
                        ExprT::UnOp(UnOp::Not, e.to_rvalue())
                            .with_loc(sift.resolve_source_info(&extra.span()))
                            .into()
                    }),
                punct(Punct::Tilde)
                    .ignore_then(cast_expression.clone())
                    .map_with(|e: Expr, extra| {
                        ExprT::UnOp(UnOp::BitNot, e.to_rvalue())
                            .with_loc(sift.resolve_source_info(&extra.span()))
                            .into()
                    }),
                punct(Punct::Dash)
                    .ignore_then(cast_expression.clone())
                    .map_with(|e: Expr, extra| {
                        ExprT::UnOp(UnOp::Neg, e.to_rvalue())
                            .with_loc(sift.resolve_source_info(&extra.span()))
                            .into()
                    }),
            ));

            and_then!(type_name.delimited_by(punct(Punct::LParen), punct(Punct::RParen)), {
                InlinePulse(ty, code: Rc<InlinePulseCode>: inline_pulse) = e =>
                    ExprT::InlinePulse(code, ty).with_loc(sift.resolve_source_info(&e.span())).into(),
                Plain(ty, x: Expr: cast_expression) = e =>
                    ExprT::Cast(x.to_rvalue(), ty).with_loc(sift.resolve_source_info(&e.span())).into(),
            }).or(unary_expression)
        });

        let multiplicative_expression = left_rec_binop!(cast_expression, {
            Mul(lhs, punct(Punct::Star), rhs) = e =>
                mk_binop(BinOp::Mul, lhs, rhs, sift.resolve_source_info(&e.span())),
            Div(lhs, punct(Punct::Slash), rhs) = e =>
                mk_binop(BinOp::Div, lhs, rhs, sift.resolve_source_info(&e.span())),
            Mod(lhs, punct(Punct::Perc), rhs) = e =>
                mk_binop(BinOp::Mod, lhs, rhs, sift.resolve_source_info(&e.span())),
        })
        .boxed();

        let additive_expression = left_rec_binop!(multiplicative_expression, {
            Add(lhs, punct(Punct::Plus), rhs) = e =>
                mk_binop(BinOp::Add, lhs, rhs, sift.resolve_source_info(&e.span())),
            Sub(lhs, punct(Punct::Dash), rhs) = e =>
                mk_binop(BinOp::Sub, lhs, rhs, sift.resolve_source_info(&e.span())),
        })
        .boxed();

        let shift_expression = left_rec_binop!(additive_expression, {
            Shl(lhs, punct(Punct::LtLt), rhs) = e =>
                mk_binop(BinOp::Shl, lhs, rhs, sift.resolve_source_info(&e.span())),
            Shr(lhs, punct(Punct::GtGt), rhs) = e =>
                mk_binop(BinOp::Shr, lhs, rhs, sift.resolve_source_info(&e.span())),
        })
        .boxed();

        let relational_expression = left_rec_binop!(shift_expression, {
            LEq(lhs, punct(Punct::LtEq), rhs) = e =>
                mk_binop(BinOp::LEq, lhs, rhs, sift.resolve_source_info(&e.span())),
            Lt(lhs, punct(Punct::Lt), rhs) = e =>
                mk_binop(BinOp::Lt, lhs, rhs, sift.resolve_source_info(&e.span())),
            GtEq(lhs, punct(Punct::GtEq), rhs) = e =>
                mk_binop(BinOp::LEq, rhs, lhs, sift.resolve_source_info(&e.span())),
            Gt(lhs, punct(Punct::Gt), rhs) = e =>
                mk_binop(BinOp::Lt, rhs, lhs, sift.resolve_source_info(&e.span())),
        })
        .boxed();

        let equality_expression = left_rec_binop!(relational_expression, {
            Eq(lhs, punct(Punct::EqEq), rhs) = e =>
                mk_binop(BinOp::Eq, lhs, rhs, sift.resolve_source_info(&e.span())),
            NEq(lhs, punct(Punct::BangEq), rhs) = e => {
                let loc = sift.resolve_source_info(&e.span());
                let eq = mk_binop(BinOp::Eq, lhs, rhs, loc.clone());
                ExprT::UnOp(UnOp::Not, eq.to_rvalue()).with_loc(loc).into()
            },
        })
        .boxed();

        let and_expression = left_rec_binop!(equality_expression, {
            BitAnd(lhs, punct(Punct::Amp), rhs) = e =>
                mk_binop(BinOp::BitAnd, lhs, rhs, sift.resolve_source_info(&e.span())),
        })
        .boxed();

        let exclusive_or_expression = left_rec_binop!(and_expression, {
            BitXor(lhs, punct(Punct::Hat), rhs) = e =>
                mk_binop(BinOp::BitXor, lhs, rhs, sift.resolve_source_info(&e.span())),
        })
        .boxed();
        let inclusive_or_expression = left_rec_binop!(exclusive_or_expression, {
            BitOr(lhs, punct(Punct::Pipe), rhs) = e =>
                mk_binop(BinOp::BitOr, lhs, rhs, sift.resolve_source_info(&e.span())),
        })
        .boxed();
        let logical_and_expression = left_rec_binop!(inclusive_or_expression, {
            LogAnd(lhs, punct(Punct::AmpAmp), rhs) = e =>
                mk_binop(BinOp::LogAnd, lhs, rhs, sift.resolve_source_info(&e.span())),
        })
        .boxed();
        let logical_or_expression = left_rec_binop!(logical_and_expression, {
            LogOr(lhs, punct(Punct::PipePipe), rhs) = e =>
                mk_binop(BinOp::LogOr, lhs, rhs, sift.resolve_source_info(&e.span())),
        })
        .boxed();

        let conditional_expression = logical_or_expression
            .clone()
            .then(
                punct(Punct::EqEqGt)
                    .ignore_then(recursive(|implies_rhs| {
                        logical_or_expression
                            .clone()
                            .then(punct(Punct::EqEqGt).ignore_then(implies_rhs).or_not())
                            .map_with(|(lhs, rhs): (Expr, Option<Expr>), extra| -> Expr {
                                match rhs {
                                    Some(rhs) => mk_binop(
                                        BinOp::Implies,
                                        lhs,
                                        rhs,
                                        sift.resolve_source_info(&extra.span()),
                                    ),
                                    None => lhs,
                                }
                            })
                    }))
                    .or_not(),
            )
            .map_with(|(lhs, rhs): (Expr, Option<Expr>), extra| -> Expr {
                match rhs {
                    Some(rhs) => mk_binop(
                        BinOp::Implies,
                        lhs,
                        rhs,
                        sift.resolve_source_info(&extra.span()),
                    ),
                    None => lhs,
                }
            })
            .boxed();

        let constant_expression = conditional_expression;

        let expr = constant_expression;

        expr.padded_by(ws())
    })
}

fn location_of_source_infos(infos: &[Rc<SourceInfo>]) -> Option<Location> {
    let mut span_loc = None;
    for info in infos {
        if let SourceInfo::Original(tok_loc) = &**info {
            match &mut span_loc {
                None => span_loc = Some(tok_loc.clone()),
                Some(span_loc) => {
                    if tok_loc.file_name == span_loc.file_name {
                        span_loc.range = span_loc.range.union(&tok_loc.range)
                    }
                }
            }
        }
    }
    span_loc
}

struct TokenSI {
    source_infos: Vec<Rc<SourceInfo>>,
    fallback: Rc<SourceInfo>,
}

impl SourceInfoForTokens for TokenSI {
    fn resolve_source_info(&self, span: &SimpleSpan) -> Rc<SourceInfo> {
        if span.end >= self.source_infos.len() {
            return self.fallback.clone();
        }
        if span.start == span.end {
            return self.source_infos[span.start].clone();
        }
        let infos = &self.source_infos[span.start..span.end];
        if let [info] = infos {
            return info.clone();
        }
        match location_of_source_infos(infos) {
            Some(span_loc) => Rc::new(SourceInfo::Original(span_loc)),
            None => self.fallback.clone(),
        }
    }

    fn resolve_error_location(&self, span: &SimpleSpan) -> Location {
        if span.end >= self.source_infos.len() {
            return self.fallback.location().clone();
        }
        if span.start == span.end {
            if let SourceInfo::Original(loc) = &*self.source_infos[span.start] {
                return loc.clone();
            }
        }
        let infos = &self.source_infos[span.start..span.end];
        if let [info] = infos {
            if let SourceInfo::Original(loc) = &**info {
                return loc.clone();
            }
        }
        location_of_source_infos(infos).unwrap_or_else(|| self.fallback.location().clone())
    }
}

struct RelexedTokens<'a> {
    tokens: Vec<(Token<'a>, SimpleSpan)>,
    source_infos: Vec<Rc<SourceInfo>>,
}

fn relex_inline_code<'a>(diagnostics: &mut Diagnostics, code: &'a InlineCode) -> RelexedTokens<'a> {
    let mut tokens: Vec<(Token, SimpleSpan)> = vec![];
    let mut source_infos: Vec<Rc<SourceInfo>> = vec![];
    for (
        i,
        CodeToken {
            before,
            text: Ast { loc, val: token },
        },
    ) in code.tokens.iter().enumerate()
    {
        if !before.is_empty() {
            tokens.push((Token::Whitespace, (i..i).into()))
        }
        // Split tokens starting with '$' (clang merges $ident into one token)
        if token.starts_with('$') && token.len() > 1 {
            tokens.push((Token::Punct(Punct::Dollar), (i..i).into()));
            let rest = &token[1..];
            // Handle $`ident (tick antiquotation): split into $ + ` + ident
            if rest.starts_with('`') && rest.len() > 1 {
                tokens.push((Token::Punct(Punct::Backtick), (i..i).into()));
                let ident_part = &rest[1..];
                let result = lex_core_token().parse(ident_part);
                diagnostics
                    .diags
                    .extend(result.errors().map(|err| Diagnostic {
                        loc: loc.location().clone(),
                        level: DiagnosticLevel::Error,
                        msg: format!("{}", err),
                    }));
                tokens.push((
                    *result.output().unwrap_or(&Token::Error),
                    (i..(i + 1)).into(),
                ));
            } else {
                let result = lex_core_token().parse(rest);
                diagnostics
                    .diags
                    .extend(result.errors().map(|err| Diagnostic {
                        loc: loc.location().clone(),
                        level: DiagnosticLevel::Error,
                        msg: format!("{}", err),
                    }));
                tokens.push((
                    *result.output().unwrap_or(&Token::Error),
                    (i..(i + 1)).into(),
                ));
            }
        } else {
            let result = lex_core_token().parse(token);
            diagnostics
                .diags
                .extend(result.errors().map(|err| Diagnostic {
                    loc: loc.location().clone(),
                    level: DiagnosticLevel::Error,
                    msg: format!("{}", err),
                }));
            tokens.push((
                *result.output().unwrap_or(&Token::Error),
                (i..(i + 1)).into(),
            ));
        }
        source_infos.push(loc.clone());
    }
    // Merge adjacent EqEq + Gt tokens (no whitespace between) into EqEqGt.
    // Clang's lexer tokenizes `==>` as `==` followed by `>`, so we merge them here.
    let mut i = 0;
    while i + 1 < tokens.len() {
        if tokens[i].0 == Token::Punct(Punct::EqEq) && tokens[i + 1].0 == Token::Punct(Punct::Gt) {
            tokens[i].0 = Token::Punct(Punct::EqEqGt);
            // Expand span to cover both original tokens
            tokens[i].1 = (tokens[i].1.start..tokens[i + 1].1.end).into();
            tokens.remove(i + 1);
        }
        i += 1;
    }
    RelexedTokens {
        tokens,
        source_infos,
    }
}

pub fn parse_expr(
    diagnostics: &mut Diagnostics,
    fallback_loc: &Rc<SourceInfo>,
    code: &InlineCode,
    snippets: &SnippetMap,
    target_widths: &TargetIntWidths,
) -> Rc<crate::ir::Expr> {
    let RelexedTokens {
        tokens,
        source_infos,
    } = relex_inline_code(diagnostics, code);
    let source_infos = TokenSI {
        source_infos,
        fallback: fallback_loc.clone(),
    };
    let result = expr_parser(snippets, &source_infos, target_widths).parse(IterInput::new(
        tokens.iter().map(Clone::clone),
        (tokens.len()..tokens.len()).into(),
    ));
    let output = match result.output() {
        Some(output) => output.clone().to_rvalue(),
        None => {
            ExprT::Error(TypeT::Error.with_loc(fallback_loc.clone())).with_loc(fallback_loc.clone())
        }
    };
    diagnostics
        .diags
        .extend(result.errors().map(|err| Diagnostic {
            loc: source_infos.resolve_error_location(err.span()),
            level: DiagnosticLevel::Error,
            msg: format!("{}", err),
        }));
    output
}

fn parse_type_inner(
    diagnostics: &mut Diagnostics,
    fallback_loc: &Rc<SourceInfo>,
    code: &InlineCode,
    target_widths: &TargetIntWidths,
) -> Rc<crate::ir::Type> {
    let RelexedTokens {
        tokens,
        source_infos,
    } = relex_inline_code(diagnostics, code);
    let source_infos = TokenSI {
        source_infos,
        fallback: fallback_loc.clone(),
    };
    let result = type_parser(&source_infos, target_widths).parse(IterInput::new(
        tokens.iter().map(Clone::clone),
        (tokens.len()..tokens.len()).into(),
    ));
    match result.output() {
        Some(output) => output.clone(),
        None => {
            diagnostics
                .diags
                .extend(result.errors().map(|err| Diagnostic {
                    loc: source_infos.resolve_error_location(err.span()),
                    level: DiagnosticLevel::Error,
                    msg: format!("{}", err),
                }));
            TypeT::Error.with_loc(fallback_loc.clone())
        }
    }
}

/// Parse a `_let` signature of the form: `_slprop foo(_array bool *r, _specnat n)`
/// Returns (name, return_type, params) or None on parse error.
pub fn parse_let_signature(
    diagnostics: &mut Diagnostics,
    fallback_loc: &Rc<SourceInfo>,
    code: &InlineCode,
    target_widths: &TargetIntWidths,
) -> Option<(Rc<Ident>, Rc<Type>, Vec<FnArg>)> {
    let RelexedTokens {
        tokens,
        source_infos,
    } = relex_inline_code(diagnostics, code);
    let source_infos = TokenSI {
        source_infos,
        fallback: fallback_loc.clone(),
    };

    // Build the parser for the signature
    let sig_parser = let_signature_parser(&source_infos, target_widths);

    let result = sig_parser.parse(IterInput::new(
        tokens.iter().map(Clone::clone),
        (tokens.len()..tokens.len()).into(),
    ));

    match result.output() {
        Some(output) => Some(output.clone()),
        None => {
            diagnostics
                .diags
                .extend(result.errors().map(|err| Diagnostic {
                    loc: source_infos.resolve_error_location(err.span()),
                    level: DiagnosticLevel::Error,
                    msg: format!("in _let signature: {}", err),
                }));
            None
        }
    }
}

fn let_signature_parser<
    'tokens,
    'src: 'tokens,
    I: ValueInput<'tokens, Token = Token<'src>, Span = SimpleSpan>,
    SIFT: SourceInfoForTokens,
>(
    sift: &'src SIFT,
    target_widths: &'src TargetIntWidths,
) -> impl Parser<'tokens, I, (Rc<Ident>, Rc<Type>, Vec<FnArg>), Extra<'tokens, 'src>> {
    let ret_type = type_parser(sift, target_widths);

    let fn_name = select! { Token::Ident(ident) => ident }
        .map_with(|ident, e| {
            Rc::new(Ast {
                val: Rc::<str>::from(ident),
                loc: sift.resolve_source_info(&e.span()),
            })
        })
        .padded_by(ws());

    // Parameter mode annotations
    let param_mode = select! {
        Token::Ident("_plain") => ParamMode::Const,
        Token::Ident("_consumes") => ParamMode::Consumed,
        Token::Ident("_out") => ParamMode::Out,
    }
    .padded_by(ws());

    // Pointer kind annotations
    let pointer_kind = select! {
        Token::Ident("_array") => PointerKind::Array,
        Token::Ident("_arrayptr") => PointerKind::ArrayPtr,
    }
    .padded_by(ws());

    // A single parameter: [mode] [ptr_kind] type [name]
    let param = param_mode
        .or_not()
        .then(pointer_kind.or_not())
        .then(type_parser(sift, target_widths))
        .then(
            select! { Token::Ident(ident) => ident }
                .map_with(|ident, e| {
                    Rc::new(Ast {
                        val: Rc::<str>::from(ident),
                        loc: sift.resolve_source_info(&e.span()),
                    })
                })
                .padded_by(ws())
                .or_not(),
        )
        .map(|(((mode_opt, ptr_kind_opt), ty), name_opt)| {
            let mode = mode_opt.unwrap_or(ParamMode::Regular);
            // If a pointer kind annotation was given, override the pointer kind in the type
            let ty = if let Some(ptr_kind) = ptr_kind_opt {
                match &ty.val {
                    TypeT::Pointer(inner, _) => {
                        TypeT::Pointer(inner.clone(), ptr_kind).with_loc(ty.loc.clone())
                    }
                    _ => ty,
                }
            } else {
                ty
            };
            FnArg {
                name: name_opt,
                ty,
                mode,
            }
        });

    let params = param
        .separated_by(punct(Punct::Comma))
        .allow_trailing()
        .collect::<Vec<_>>()
        .delimited_by(punct(Punct::LParen), punct(Punct::RParen));

    ret_type
        .then(fn_name)
        .then(params)
        .map(|((ret_ty, name), params)| (name, ret_ty, params))
}

pub fn process_inline_pulse(
    fallback_loc: &Rc<SourceInfo>,
    code: &InlineCode,
    snippets: &SnippetMap,
    target_widths: &TargetIntWidths,
) -> InlinePulseCode {
    let mut diags = Diagnostics::empty();
    let RelexedTokens {
        tokens: relexed,
        source_infos: _,
    } = relex_inline_code(&mut diags, code);

    // Intermediate representation: chumsky parser produces token index ranges,
    // then we post-process to parse antiquotation inner expressions.
    #[derive(Debug, Clone)]
    enum RawToken {
        Verbatim(SimpleSpan),
        Antiquot {
            is_lvalue: bool,
            dollar_span: SimpleSpan,
            body_span: SimpleSpan,
        },
        TypeAntiquot {
            dollar_span: SimpleSpan,
            body_span: SimpleSpan,
        },
        FieldAntiquot {
            dollar_span: SimpleSpan,
            body_span: SimpleSpan,
        },
        DeclareAntiquot {
            dollar_span: SimpleSpan,
            body_span: SimpleSpan,
        },
        TickAntiquot {
            dollar_span: SimpleSpan,
            ident_span: SimpleSpan,
        },
    }

    // Balanced parentheses: matches everything between ( and ), handling nesting.
    let balanced_inner = recursive(|inner: Recursive<dyn Parser<_, _, Extra<'_, '_>>>| {
        choice((
            any()
                .filter(|t: &Token| {
                    *t != Token::Punct(Punct::LParen) && *t != Token::Punct(Punct::RParen)
                })
                .ignored(),
            just(Token::Punct(Punct::LParen))
                .ignored()
                .then(inner)
                .then(just(Token::Punct(Punct::RParen)).ignored())
                .ignored(),
        ))
        .repeated()
        .ignored()
        .map_with(|_, extra| extra.span())
    });

    let antiquot = just(Token::Punct(Punct::Dollar))
        .map_with(|_, extra| extra.span())
        .then(
            just(Token::Punct(Punct::Amp))
                .or_not()
                .map(|amp| amp.is_some()),
        )
        .then_ignore(just(Token::Punct(Punct::LParen)))
        .then(balanced_inner.clone())
        .then_ignore(just(Token::Punct(Punct::RParen)))
        .map(|((dollar_span, is_lvalue), body_span)| RawToken::Antiquot {
            is_lvalue,
            dollar_span,
            body_span,
        });

    let dollar_keyword = |kw| {
        just(Token::Punct(Punct::Dollar))
            .map_with(|_, extra| extra.span())
            .then_ignore(just(Token::Ident(kw)).padded_by(ws()))
            .then_ignore(just(Token::Punct(Punct::LParen)))
            .then(balanced_inner.clone())
            .then_ignore(just(Token::Punct(Punct::RParen)))
    };

    let type_antiquot =
        dollar_keyword("type").map(|(dollar_span, body_span)| RawToken::TypeAntiquot {
            dollar_span,
            body_span,
        });

    let field_antiquot =
        dollar_keyword("field").map(|(dollar_span, body_span)| RawToken::FieldAntiquot {
            dollar_span,
            body_span,
        });

    let declare_antiquot =
        dollar_keyword("declare").map(|(dollar_span, body_span)| RawToken::DeclareAntiquot {
            dollar_span,
            body_span,
        });

    // $`ident → emits 'ident (F* implicit/ticked argument)
    let tick_antiquot = just(Token::Punct(Punct::Dollar))
        .map_with(|_, extra| extra.span())
        .then_ignore(just(Token::Punct(Punct::Backtick)))
        .then(select! { Token::Ident(_) => () }.map_with(|_, extra| extra.span()))
        .map(|(dollar_span, ident_span)| RawToken::TickAntiquot {
            dollar_span,
            ident_span,
        });

    let verbatim = any()
        .filter(|t: &Token| *t != Token::Whitespace)
        .map_with(|_, extra| RawToken::Verbatim(extra.span()));

    let inline_pulse_parser = choice((
        type_antiquot,
        field_antiquot,
        declare_antiquot,
        tick_antiquot,
        antiquot,
        verbatim,
    ))
    .padded_by(ws())
    .repeated()
    .collect::<Vec<_>>();

    let parse_result = inline_pulse_parser.parse(IterInput::new(
        relexed.iter().map(Clone::clone),
        (relexed.len()..relexed.len()).into(),
    ));

    let raw_tokens = parse_result.output().cloned().unwrap_or_default();

    // Post-process: convert RawTokens to InlinePulseTokens
    let result = raw_tokens
        .into_iter()
        .map(|raw| match raw {
            RawToken::Verbatim(span) => {
                let mut ct = code.tokens[span.start].clone();
                // If the relexer merged tokens (e.g., EqEq+Gt → EqEqGt),
                // the span covers multiple original tokens; reconstruct text.
                if span.end > span.start + 1 {
                    let merged: String = (span.start..span.end)
                        .map(|i| code.tokens[i].text.val.as_ref())
                        .collect();
                    ct.text = Ast {
                        val: Rc::from(merged.as_str()),
                        loc: ct.text.loc.clone(),
                    };
                }
                InlinePulseToken::Verbatim(ct)
            }
            RawToken::Antiquot {
                is_lvalue,
                dollar_span,
                body_span,
            } => {
                let before = code.tokens[dollar_span.start].before;
                let inner_code = InlineCode {
                    tokens: code.tokens[body_span.start..body_span.end].to_vec(),
                };
                let mut inner_diags = Diagnostics::empty();
                let expr = parse_expr(
                    &mut inner_diags,
                    fallback_loc,
                    &inner_code,
                    snippets,
                    target_widths,
                );
                if is_lvalue {
                    InlinePulseToken::LValueAntiquot { before, expr }
                } else {
                    InlinePulseToken::RValueAntiquot { before, expr }
                }
            }
            RawToken::TypeAntiquot {
                dollar_span,
                body_span,
            } => {
                let before = code.tokens[dollar_span.start].before;
                let inner_code = InlineCode {
                    tokens: code.tokens[body_span.start..body_span.end].to_vec(),
                };
                let mut inner_diags = Diagnostics::empty();
                let ty =
                    parse_type_inner(&mut inner_diags, fallback_loc, &inner_code, target_widths);
                InlinePulseToken::TypeAntiquot { before, ty }
            }
            RawToken::FieldAntiquot {
                dollar_span,
                body_span,
            } => {
                let before = code.tokens[dollar_span.start].before;
                let inner_code = InlineCode {
                    tokens: code.tokens[body_span.start..body_span.end].to_vec(),
                };
                let mut inner_diags = Diagnostics::empty();
                let RelexedTokens {
                    tokens: inner_relexed,
                    source_infos: inner_si,
                } = relex_inline_code(&mut inner_diags, &inner_code);
                let inner_sift = TokenSI {
                    source_infos: inner_si,
                    fallback: fallback_loc.clone(),
                };
                let mk_ident = select! { Token::Ident(ident) => ident }
                    .map_with(|ident, e| {
                        Rc::<str>::from(ident).with_loc(inner_sift.resolve_source_info(&e.span()))
                    })
                    .padded_by(ws());
                let field_parser = type_parser(&inner_sift, target_widths)
                    .then_ignore(just(Token::Punct(Punct::ColonColon)))
                    .then(mk_ident);
                let result = field_parser.parse(IterInput::new(
                    inner_relexed.iter().map(Clone::clone),
                    (inner_relexed.len()..inner_relexed.len()).into(),
                ));
                match result.output() {
                    Some((ty, f)) => InlinePulseToken::FieldAntiquot {
                        before,
                        ty: ty.clone(),
                        field_name: f.clone(),
                    },
                    _ => InlinePulseToken::Verbatim(code.tokens[dollar_span.start].clone()),
                }
            }
            RawToken::DeclareAntiquot {
                dollar_span,
                body_span,
            } => {
                let before = code.tokens[dollar_span.start].before;
                let inner_code = InlineCode {
                    tokens: code.tokens[body_span.start..body_span.end].to_vec(),
                };
                // Parse as type followed by identifier
                let mut inner_diags = Diagnostics::empty();
                let RelexedTokens {
                    tokens: inner_relexed,
                    source_infos: inner_si,
                } = relex_inline_code(&mut inner_diags, &inner_code);
                let inner_sift = TokenSI {
                    source_infos: inner_si,
                    fallback: fallback_loc.clone(),
                };
                let decl_parser = type_parser(&inner_sift, target_widths).then(
                    select! { Token::Ident(ident) => ident }
                        .map_with(|ident, e| {
                            Rc::<str>::from(ident)
                                .with_loc(inner_sift.resolve_source_info(&e.span()))
                        })
                        .padded_by(ws()),
                );
                let result = decl_parser.parse(IterInput::new(
                    inner_relexed.iter().map(Clone::clone),
                    (inner_relexed.len()..inner_relexed.len()).into(),
                ));
                match result.output() {
                    Some((ty, ident)) => InlinePulseToken::Declare {
                        ident: ident.clone(),
                        ty: ty.clone(),
                    },
                    None => {
                        // Fallback: emit nothing
                        let _ = before;
                        InlinePulseToken::Verbatim(code.tokens[dollar_span.start].clone())
                    }
                }
            }
            RawToken::TickAntiquot {
                dollar_span,
                ident_span,
            } => {
                let ct = &code.tokens[dollar_span.start];
                let ident = &code.tokens[ident_span.start].text.val;
                InlinePulseToken::Verbatim(CodeToken {
                    before: ct.before,
                    text: Ast {
                        val: Rc::from(format!("'{}", ident).as_str()),
                        loc: ct.text.loc.clone(),
                    },
                })
            }
        })
        .collect();

    InlinePulseCode { tokens: result }
}
