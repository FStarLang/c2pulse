// Legacy source range info JSON format

use std::rc::Rc;

use serde::{Deserialize, Serialize};

use crate::{
    emit,
    ir::{Location, Position, Range},
};

#[derive(Serialize)]
struct SourceMapPos {
    line: u32,   // starts with 1
    column: u32, // starts with 1
}

#[derive(Serialize)]
struct SourceMapRange {
    start: SourceMapPos,
    end: SourceMapPos,
}

type PulseSourceRange = SourceMapRange;

struct RcStr(Rc<str>);
impl Serialize for RcStr {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        self.0.serialize(serializer)
    }
}

#[derive(Serialize)]
struct CSourceRange {
    #[serde(flatten)]
    range: SourceMapRange,
    #[serde(rename = "fileName")]
    file_name: RcStr,
    #[serde(rename = "isVerbatim")]
    is_verbatim: bool,
}

#[derive(Serialize)]
struct SourceRangeInfoItem {
    #[serde(rename = "pulseRange")]
    pulse_range: PulseSourceRange,
    #[serde(rename = "cRange")]
    c_range: CSourceRange,
}

type SourceRangeInfo = Vec<SourceRangeInfoItem>;

fn to_smp(p: &Position) -> SourceMapPos {
    SourceMapPos {
        line: p.line,
        column: p.character,
    }
}

fn to_srm(r: &Range) -> SourceMapRange {
    SourceMapRange {
        start: to_smp(&r.start),
        end: to_smp(&r.end),
    }
}

fn to_srii(l: &Location, r: &Range) -> SourceRangeInfoItem {
    SourceRangeInfoItem {
        pulse_range: to_srm(&r),
        c_range: CSourceRange {
            range: to_srm(&l.range),
            file_name: RcStr(l.file_name.clone()),
            is_verbatim: false, // TODO
        },
    }
}

fn to_sri(ranges: &emit::SourceRangeMap) -> SourceRangeInfo {
    ranges.iter().map(|(l, r)| to_srii(l, r)).collect()
}

pub fn serialize(ranges: &emit::SourceRangeMap) -> String {
    serde_json::to_string_pretty(&to_sri(ranges)).unwrap()
}
