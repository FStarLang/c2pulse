use std::{collections::HashMap, fmt::Display, rc::Rc, str::FromStr};

use chumsky::{
    Parser,
    input::{IterInput, ValueInput},
    prelude::*,
};
use num_bigint::BigInt;

use crate::{
    diag::{Diagnostic, DiagnosticLevel},
    ir::{
        Ast, CodeToken, InlineCode, LValue, LValueT, Location, Position, RValue, RValueT, Range,
        SourceInfo, TypeT, WithLoc,
    },
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
    LBracket => "[",
    RBracket => "]",
    LParen => "(",
    RParen => ")",
    LBrace => "{",
    RBrace => "}",
    Dot => ".",
    DashGt => "->",

    PlusPlus => "++",
    DashDash => "--",
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
    Lt => "<",
    Gt => ">",
    LtEq => "<=",
    GtEq => ">=",
    EqEq => "==",
    BangEq => "!=",
    Hat => "^",
    Pipe => "|",
    AmpAmp => "&&",
    PipePipe => "||",

    Question => "?",
    Colon => ":",
    Semi => ";",
    DotDotDot => "...",

    Eq => "=",
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

    Comma => ",",
    Hash => "#",
    HashHash => "##",

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
    ($base:expr, { $($n:ident($acc:ident, $x:ident: $t:ty: $p:expr) => $cb:expr,)* }) => {{
        enum Rhs {
            $($n($t),)*
        }
        let rhs = choice((
            $(($p).map(Rhs::$n),)*
        ));
        ($base).foldl(rhs.repeated(), |acc, rhs| match rhs {
            $(Rhs::$n($x) => { let $acc = acc; $cb },)*
        })
    }}
}

macro_rules! left_rec_binop {
    ($base:expr, { $($n:ident($acc:ident, $p:expr, $x:ident) => $cb:expr,)* }) => {{
        let base = $base;
        left_recursion!(base, {$($n($acc, $x: Expr: $p.ignore_then(base.clone())) => $cb,)*})
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

fn expr_parser<'tokens, 'src: 'tokens, I, SIFT>(
    snip_map: &'src SnippetMap,
    sift: &'src SIFT,
) -> impl Parser<'tokens, I, Expr, extra::Err<Rich<'tokens, Token<'src>, SimpleSpan>>> + Clone
where
    I: ValueInput<'tokens, Token = Token<'src>, Span = SimpleSpan>,
    SIFT: SourceInfoForTokens,
{
    recursive(|expr| {
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

        let identifier =
            select! { Token { tok: CoreToken::Ident(ident), .. } => ident}.map_with(|ident, e| {
                let loc = sift.resolve_source_info(&e.span());
                LValueT::Var(Rc::<str>::from(ident).with_loc(loc.clone()))
                    .with_loc(loc)
                    .into()
            });

        let integer_constant = select! { Token { tok: CoreToken::Integer(i, suf), .. } => (i,suf)}
            .try_map(|(i, suf), span| {
                match BigInt::from_str(i) {
                    Ok(i) => {
                        let loc = sift.resolve_source_info(&span);
                        let ty = TypeT::Int {
                            signed: false,
                            width: 32,
                        } // TODO
                        .with_loc(loc.clone());
                        Ok(RValueT::IntLit {
                            val: Rc::new(i),
                            ty,
                        }
                        .with_loc(loc)
                        .into())
                    }
                    Err(err) => Err(Rich::custom(span, err)),
                }
            });

        let constant = integer_constant;

        let parenthesized = expr
            .clone()
            .delimited_by(punct(Punct::LParen), punct(Punct::RParen));

        let primary_expression = identifier.or(constant).or(parenthesized);

        let postfix_expression = primary_expression;

        let unary_expression = postfix_expression.boxed();

        let cast_expression = recursive(|cast_expression| {
            and_then!(type_name.delimited_by(punct(Punct::LParen), punct(Punct::RParen)), {
                InlinePulse(ty, code: Rc<InlineCode>: inline_pulse) = e =>
                    RValueT::InlinePulse { val: code, ty }.with_loc(sift.resolve_source_info(&e.span())).into(),
                Plain(ty, x: Expr: cast_expression) = e =>
                    RValueT::Cast { val: x.to_rvalue(), ty }.with_loc(sift.resolve_source_info(&e.span())).into(),
            }).or(unary_expression)
        });

        let multiplicative_expression = left_rec_binop!(cast_expression, {
            Mul(e, punct(Punct::Star), rhs) => todo!(),
            Div(e, punct(Punct::Slash), rhs) => todo!(),
            Mod(e, punct(Punct::Perc), rhs) => todo!(),
        })
        .boxed();

        let additive_expression = left_rec_binop!(multiplicative_expression, {
            Plus(e, punct(Punct::Plus), rhs) => todo!(),
            Minus(e, punct(Punct::Dash), rhs) => todo!(),
        })
        .boxed();

        let shift_expression = additive_expression;

        let relational_expression = shift_expression;

        let equality_expression = relational_expression;

        let and_expression = equality_expression;

        let exclusive_or_expression = and_expression;
        let inclusive_or_expression = exclusive_or_expression;
        let logical_and_expression = inclusive_or_expression;
        let logical_or_expression = logical_and_expression;

        let conditional_expression = logical_or_expression;

        let constant_expression = conditional_expression;

        constant_expression
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

pub fn parse_rvalue(code: &InlineCode, snippets: &SnippetMap) -> (Rc<RValue>, Vec<Diagnostic>) {
    let mut diags = vec![];
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
                diags.extend(result.errors().map(|err| Diagnostic {
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
    diags.extend(result.errors().map(|err| {
        Diagnostic {
            loc: source_infos
                .resolve_error_location(err.span())
                .unwrap_or_else(default_location),
            level: DiagnosticLevel::Error,
            msg: format!("{}", err), // TODO
        }
    }));
    (output, diags)
}
