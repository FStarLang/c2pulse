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
enum CoreToken<'src> {
    String(&'src str),
    Integer(&'src str, IntegerSuffix),
    Ident(&'src str),

    Punct(Punct),

    Error,
}

impl<'src> Display for CoreToken<'src> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            CoreToken::String(tok) => write!(f, "{}", tok),
            CoreToken::Integer(i, suffix) => write!(f, "{}{}", i, suffix),
            CoreToken::Ident(id) => write!(f, "{}", id),
            CoreToken::Punct(punct) => write!(f, "{}", punct.to_str()),
            CoreToken::Error => write!(f, "(LEXING ERROR)"),
        }
    }
}

#[derive(Debug, Clone, Hash, PartialEq, Eq)]
struct Token<'src> {
    ws_before: bool,
    tok: CoreToken<'src>,
}

impl<'src> Display for Token<'src> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.tok)
    }
}

fn lex_core_token<'src>() -> impl Parser<'src, &'src str, CoreToken<'src>> {
    let integer_suffix_l = Parser::or(
        one_of("lL").ignore_then(
            one_of("lL")
                .to(IntegerSuffix::LL)
                .or(empty().to(IntegerSuffix::L)),
        ),
        one_of("zZ").to(IntegerSuffix::Z),
    );
    let integer_suffix = choice((
        one_of("uU").ignore_then(integer_suffix_l.map(IntegerSuffix::make_unsigned)),
        integer_suffix_l
            .then(one_of("uU").or_not())
            .map(|(s, u)| if u.is_some() { s.make_unsigned() } else { s }),
        empty().to(IntegerSuffix::None),
    ));

    let decimal_literal = text::int(10); // also happens to accept 0, which should be an octal literal in C

    let integer_literal = decimal_literal
        .then(integer_suffix)
        .map(|(i, s)| CoreToken::Integer(i, s));

    let op = Punct::lexer().map(CoreToken::Punct);

    let ident = text::ident().map(CoreToken::Ident); // as C demands: XID_Start XID_Continue*

    // TODO FIXME
    let string = just('"')
        .not()
        .repeated()
        .to_slice()
        .delimited_by(just('"'), just('"'))
        .map(CoreToken::String);

    let fallback = text::whitespace()
        .not()
        .repeated()
        .at_least(1)
        .to(CoreToken::Error);

    integer_literal.or(op).or(ident).or(string).or(fallback)
}

fn punct<'tokens, 'src: 'tokens, I, Span>(
    op: Punct,
) -> impl Parser<'tokens, I, (), extra::Err<Rich<'tokens, Token<'src>, Span>>> + Clone
where
    I: ValueInput<'tokens, Token = Token<'src>, Span = Span>,
{
    select! {Token{tok: CoreToken::Punct(o), ..} if op == o => ()}
}

#[derive(Debug, Default, Clone)]
pub struct SnippetMap {
    pub snippets: HashMap<u32, InlineCode>,
}

trait SourceInfoForTokens {
    fn resolve_source_info(&self, span: &SimpleSpan) -> Rc<SourceInfo>;
    fn resolve_error_location(&self, span: &SimpleSpan) -> Option<Location>;
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
enum Expr {
    LValue(Rc<LValue>),
    RValue(Rc<RValue>),
}
impl Expr {
    fn to_rvalue(self) -> Rc<RValue> {
        match self {
            Expr::LValue(e) => {
                let loc = e.loc.clone();
                RValueT::LValue(e).with_loc(loc)
            }
            Expr::RValue(e) => e,
        }
    }
}
impl From<Rc<LValue>> for Expr {
    fn from(value: Rc<LValue>) -> Self {
        Expr::LValue(value)
    }
}
impl From<Rc<RValue>> for Expr {
    fn from(value: Rc<RValue>) -> Self {
        Expr::RValue(value)
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
        ($base).foldl_with(rhs.repeated(), |acc, rhs, _extra| match rhs {
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
        left_recursion!(base, {$($n($acc, $x: Expr: $p.ignore_then(base.clone())) $(= $extra)? => $cb,)*})
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
    RValueT::BinOp(binop, lhs.to_rvalue(), rhs.to_rvalue())
        .with_loc(loc)
        .into()
}

type Extra<'tokens, 'src> = extra::Err<Rich<'tokens, Token<'src>, SimpleSpan>>;

fn expr_parser<'tokens, 'src: 'tokens, I, SIFT>(
    snip_map: &'src SnippetMap,
    sift: &'src SIFT,
) -> impl Parser<'tokens, I, Expr, Extra<'tokens, 'src>> + Clone
where
    I: ValueInput<'tokens, Token = Token<'src>, Span = SimpleSpan>,
    SIFT: SourceInfoForTokens,
{
    recursive(|expr| {
        let assignment_expression = expr.clone();

        // let expression = assignment_expression;

        let type_name = select! { Token { tok: CoreToken::Ident("_slprop"), .. } => TypeT::SLProp }
            .map_with(|ty, e| ty.with_loc(sift.resolve_source_info(&e.span())));

        let inline_pulse = select! { Token { tok: CoreToken::Ident("_inline_pulse"), .. } => () }
            .ignore_then(
                select! { Token { tok: CoreToken::Integer(i, _), .. } => i }
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

        let ident =
            select! { Token { tok: CoreToken::Ident(ident), .. } => ident}.map_with(|ident, e| {
                Rc::<str>::from(ident).with_loc(sift.resolve_source_info(&e.span()))
            });
        let identifier = ident.map(|i| {
            let loc = i.loc.clone();
            LValueT::Var(i).with_loc(loc).into()
        });

        let integer_constant = select! { Token { tok: CoreToken::Integer(i, suf), .. } => (i,suf)}
            .try_map(|(i, _suf), span| {
                match BigInt::from_str(i) {
                    Ok(i) => {
                        let loc = sift.resolve_source_info(&span);
                        let ty = TypeT::Int {
                            signed: false,
                            width: 32,
                        } // TODO
                        .with_loc(loc.clone());
                        Ok(RValueT::IntLit(Rc::new(i), ty).with_loc(loc).into())
                    }
                    Err(err) => Err(Rich::custom(span, err)),
                }
            });

        let constant = integer_constant;

        let parenthesized = expr
            .clone()
            .delimited_by(punct(Punct::LParen), punct(Punct::RParen));

        let primary_expression = identifier.or(constant).or(parenthesized);

        let postfix_expression = choice((
            ident // TODO: function should be postfix_expression
                .then(
                    assignment_expression
                        .clone()
                        .separated_by(punct(Punct::Comma))
                        .collect::<Vec<_>>()
                        .delimited_by(punct(Punct::LParen), punct(Punct::RParen)),
                )
                .map_with(|(f, args), extra| {
                    RValueT::FnCall(f, args.into_iter().map(|e: Expr| e.to_rvalue()).collect())
                        .with_loc(sift.resolve_source_info(&extra.span()))
                        .into()
                }),
            primary_expression,
        ))
        .boxed();

        let cast_expression = recursive(|cast_expression| {
            let unary_expression = choice((
                postfix_expression,
                punct(Punct::Star)
                    .ignore_then(cast_expression.clone())
                    .map_with(|e: Expr, extra| {
                        LValueT::Deref(e.to_rvalue())
                            .with_loc(sift.resolve_source_info(&extra.span()))
                            .into()
                    }),
            ));

            and_then!(type_name.delimited_by(punct(Punct::LParen), punct(Punct::RParen)), {
                InlinePulse(ty, code: Rc<InlineCode>: inline_pulse) = e =>
                    RValueT::InlinePulse(code, ty).with_loc(sift.resolve_source_info(&e.span())).into(),
                Plain(ty, x: Expr: cast_expression) = e =>
                    RValueT::Cast(x.to_rvalue(), ty).with_loc(sift.resolve_source_info(&e.span())).into(),
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

        let relational_expression = shift_expression;

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

        expr
    })
}

fn default_location() -> Location {
    // FIXME TODO
    Location {
        file_name: Rc::from(""),
        range: Range {
            start: Position {
                line: 1,
                character: 1,
            },
            end: Position {
                line: 1,
                character: 1,
            },
        },
    }
}

fn location_of_sourceinfo(source_info: &SourceInfo) -> Location {
    match source_info {
        SourceInfo::None => default_location(),
        SourceInfo::Original(location) => location.clone(),
    }
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

impl SourceInfoForTokens for Vec<Rc<SourceInfo>> {
    fn resolve_source_info(&self, span: &SimpleSpan) -> Rc<SourceInfo> {
        let infos = &self[span.start..span.end];
        if let [info] = infos {
            return info.clone();
        }
        match location_of_source_infos(infos) {
            Some(span_loc) => Rc::new(SourceInfo::Original(span_loc)),
            None => Rc::new(SourceInfo::None),
        }
    }

    fn resolve_error_location(&self, span: &SimpleSpan) -> Option<Location> {
        let infos = &self[span.start..span.end];
        if let [info] = infos {
            if let SourceInfo::Original(loc) = &**info {
                return Some(loc.clone());
            }
        }
        location_of_source_infos(infos)
    }
}

pub fn parse_rvalue(
    diagnostics: &mut Diagnostics,
    code: &InlineCode,
    snippets: &SnippetMap,
) -> Rc<RValue> {
    let (tokens, source_infos): (Vec<(Token, SimpleSpan)>, Vec<Rc<SourceInfo>>) = code
        .tokens
        .iter()
        .enumerate()
        .map(
            |(
                i,
                CodeToken {
                    before,
                    text: Ast { loc, val: token },
                },
            )| {
                let result = lex_core_token().parse(token);
                diagnostics
                    .diags
                    .extend(result.errors().map(|err| Diagnostic {
                        loc: location_of_sourceinfo(loc),
                        level: DiagnosticLevel::Error,
                        msg: format!("{}", err),
                    }));
                (
                    (
                        Token {
                            ws_before: !before.is_empty(),
                            tok: *result.output().unwrap_or(&CoreToken::Error),
                        },
                        (i..(i + 1)).into(),
                    ),
                    loc.clone(),
                )
            },
        )
        .unzip();
    let result = expr_parser(snippets, &source_infos).parse(IterInput::new(
        tokens.iter().map(Clone::clone),
        (tokens.len()..tokens.len()).into(),
    ));
    let output = match result.output() {
        Some(output) => output.clone().to_rvalue(),
        None => {
            let loc = Rc::new(SourceInfo::None);
            RValueT::Error(TypeT::Error.with_loc(loc.clone())).with_loc(loc)
        }
    };
    diagnostics.diags.extend(result.errors().map(|err| {
        Diagnostic {
            loc: source_infos
                .resolve_error_location(err.span())
                .unwrap_or_else(default_location),
            level: DiagnosticLevel::Error,
            msg: format!("{} {:?}", err, tokens), // TODO
        }
    }));
    output
}
