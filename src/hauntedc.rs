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
    BangEq => "!=",
    Hat => "^",
    PipePipe => "||",
    Pipe => "|",

    Question => "?",
    Colon => ":",
    Semi => ";",

    Eq => "=",

    Comma => ",",
    HashHash => "##",
    Hash => "#",

    ColonColon => "::",
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

fn expr_parser<
    'tokens,
    'src: 'tokens,
    I: ValueInput<'tokens, Token = Token<'src>, Span = SimpleSpan>,
    SIFT: SourceInfoForTokens,
>(
    snip_map: &'src SnippetMap,
    sift: &'src SIFT,
) -> impl Parser<'tokens, I, Expr, Extra<'tokens, 'src>> + Clone {
    recursive(|expr| {
        let assignment_expression = expr.clone();

        // let expression = assignment_expression;

        let type_name = select! {
            Token::Ident("_slprop") => TypeT::SLProp,
            Token::Ident("_specint") => TypeT::SpecInt,
        }
        .map_with(|ty, e| ty.with_loc(sift.resolve_source_info(&e.span())));

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
                    Some(snip) => Ok(Rc::new(snip.clone())),
                    None => Err(Rich::custom(span, format!("snippet {} not found", i))),
                }
            })
            .boxed();

        let ident = select! { Token::Ident(ident) => ident }
            .map_with(|ident, e| {
                Rc::<str>::from(ident).with_loc(sift.resolve_source_info(&e.span()))
            })
            .padded_by(ws());
        let identifier = ident.clone().map(|i| {
            let loc = i.loc.clone();
            ExprT::Var(i).with_loc(loc).into()
        });

        let integer_constant =
            select! { Token::Integer(i, suf) => (i,suf) }.try_map(|(i, _suf), span| {
                match BigInt::from_str(i) {
                    Ok(i) => {
                        let loc = sift.resolve_source_info(&span);
                        let ty = TypeT::Int {
                            signed: false,
                            width: 32,
                        } // TODO
                        .with_loc(loc.clone());
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

        let postfix_expression_nonrec = choice((
            ident
                .clone() // TODO: function should be postfix_expression
                .then(
                    assignment_expression
                        .clone()
                        .separated_by(punct(Punct::Comma))
                        .collect::<Vec<_>>()
                        .delimited_by(punct(Punct::LParen), punct(Punct::RParen)),
                )
                .try_map(|(f, args), s| match &*f.val {
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
                }),
            primary_expression,
        ))
        .boxed();
        let postfix_expression = left_recursion!(postfix_expression_nonrec, {
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
            ));

            and_then!(type_name.delimited_by(punct(Punct::LParen), punct(Punct::RParen)), {
                InlinePulse(ty, code: Rc<InlineCode>: inline_pulse) = e =>
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

        let shift_expression = additive_expression;

        let relational_expression = left_rec_binop!(shift_expression, {
            LEq(lhs, punct(Punct::LtEq), rhs) = e =>
                mk_binop(BinOp::LEq, lhs, rhs, sift.resolve_source_info(&e.span())),
            Lt(lhs, punct(Punct::Lt), rhs) = e =>
                mk_binop(BinOp::Lt, lhs, rhs, sift.resolve_source_info(&e.span())),
        })
        .boxed();

        let equality_expression = left_rec_binop!(relational_expression, {
            Eq(lhs, punct(Punct::EqEq), rhs) = e =>
                mk_binop(BinOp::Eq, lhs, rhs, sift.resolve_source_info(&e.span())),
        })
        .boxed();

        let and_expression = equality_expression;

        let exclusive_or_expression = and_expression;
        let inclusive_or_expression = exclusive_or_expression;
        let logical_and_expression = left_rec_binop!(inclusive_or_expression, {
            LogAnd(lhs, punct(Punct::AmpAmp), rhs) = e =>
                mk_binop(BinOp::LogAnd, lhs, rhs, sift.resolve_source_info(&e.span())),
        })
        .boxed();
        let logical_or_expression = logical_and_expression;

        let conditional_expression = logical_or_expression;

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

pub fn parse_expr(
    diagnostics: &mut Diagnostics,
    fallback_loc: &Rc<SourceInfo>,
    code: &InlineCode,
    snippets: &SnippetMap,
) -> Rc<crate::ir::Expr> {
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
        let result = lex_core_token().parse(token);
        diagnostics
            .diags
            .extend(result.errors().map(|err| Diagnostic {
                loc: loc.location().clone(),
                level: DiagnosticLevel::Error,
                msg: format!("{}", err),
            }));
        if !before.is_empty() {
            tokens.push((Token::Whitespace, (i..i).into()))
        }
        tokens.push((
            *result.output().unwrap_or(&Token::Error),
            (i..(i + 1)).into(),
        ));
        source_infos.push(loc.clone());
    }
    let source_infos = TokenSI {
        source_infos,
        fallback: fallback_loc.clone(),
    };
    let result = expr_parser(snippets, &source_infos).parse(IterInput::new(
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
