#include "generated.h"
#include "clang/Driver/Driver.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/MacroArgs.h"
#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Tooling/Tooling.h"
#include <dlfcn.h>

using namespace clang;
using namespace clang::tooling;
using rust::Ref;
using rust::RefMut;
using rust::std::rc::Rc;
using rust::std::vec::Vec;
using namespace rust::crate::clang;
namespace ir = rust::crate::ir;

llvm::StringRef toStringRef(Ref<rust::Str> str) {
  return llvm::StringRef((char const *)str.as_ptr(), str.len());
}

std::string toString(Ref<rust::Str> str) {
  return std::string((char const *)str.as_ptr(), str.len());
}

Ref<rust::Str> toStr(llvm::StringRef const &str) {
  return str_from_parts((uint8_t const *)str.data(), str.size());
}

Ref<rust::Str> toStr(std::string const &str) {
  return str_from_parts((uint8_t const *)str.data(), str.size());
}

using SnipMap = rust::crate::hauntedc::SnippetMap;

template <> struct std::hash<FileID> {
  std::size_t operator()(FileID const &s) const noexcept {
    return s.getHashValue();
  }
};
struct RangeMap {
  RangeMap(RefMut<Ctx> &c) : ctx(c) {}
  RefMut<Ctx> ctx;
  std::unordered_map<FileID, Rc<rust::Str>> files;

  Rc<rust::Str> getFileName(SourceManager &sm, FileID id) {
    if (auto result = files.find(id); result != files.end()) {
      return result->second.clone();
    } else {
      auto res = ctx.intern_str(
          id.isValid() ? toStr(sm.getFileEntryRefForID(id)->getName())
                       : "<unknown>"_rs);
      files[id] = res.clone();
      return res;
    }
  }

  Rc<ir::SourceInfo> getExpansionRange(SourceManager &sm, SourceRange range) {
    return mk_original_location(
        getFileName(sm, sm.getFileID(sm.getExpansionLoc(range.getBegin()))),
        sm.getExpansionLineNumber(range.getBegin()),
        sm.getExpansionColumnNumber(range.getBegin()),
        sm.getExpansionLineNumber(range.getEnd()),
        sm.getExpansionColumnNumber(range.getEnd()));
  }
};

class MacroTracker : public PPCallbacks {
public:
  MacroTracker(RangeMap &m, SnipMap &s, CompilerInstance &ci)
      : rangeMap(m), snippets(s), compilerInst(ci) {}
  RangeMap &rangeMap;
  SnipMap &snippets;
  CompilerInstance &compilerInst;

  void MacroExpands(Token const &MacroNameTok, MacroDefinition const &MD,
                    SourceRange Range, MacroArgs const *Args) override {
    auto &sm = compilerInst.getSourceManager();
    auto &langOpts = compilerInst.getLangOpts();
    if (Args) {
      InlineCodeBuilder toks = InlineCodeBuilder::new_();
      unsigned numArgs = Args->getNumMacroArguments();
      for (unsigned i = 0; i < numArgs; ++i) {
        Token const *argTokens = Args->getUnexpArgument(i);
        unsigned numTokens = Args->getArgLength(argTokens);

        for (unsigned j = 0; j < numTokens; ++j) {
          auto &tok = argTokens[j];
          std::string spelling = Lexer::getSpelling(tok, sm, langOpts);

          Rc<ir::SourceInfo> loc;
          SourceLocation tokLoc = sm.getSpellingLoc(tok.getLocation());
          if (auto fileID = sm.getFileID(tokLoc); fileID.isValid()) {
            unsigned beginLine = sm.getSpellingLineNumber(tokLoc);
            unsigned beginChar = sm.getSpellingColumnNumber(tokLoc);
            unsigned endLine = beginLine;
            unsigned endChar = beginChar + spelling.length();
            loc = mk_original_location(rangeMap.getFileName(sm, fileID),
                                       beginLine, beginChar, endLine, endChar);
          } else {
            auto expansionRange = sm.getExpansionRange(
                SourceRange(tok.getLocation(), tok.getEndLoc()));
            unsigned beginLine =
                sm.getExpansionLineNumber(expansionRange.getBegin());
            unsigned beginChar =
                sm.getExpansionColumnNumber(expansionRange.getBegin());
            unsigned endLine =
                sm.getExpansionLineNumber(expansionRange.getEnd());
            unsigned endChar =
                sm.getExpansionColumnNumber(expansionRange.getEnd());
            loc = mk_fallback_sourceinfo(mk_original_location(
                rangeMap.getFileName(sm,
                                     sm.getFileID(expansionRange.getBegin())),
                beginLine, beginChar, endLine, endChar));
          }

          Ref<rust::Str> before = tok.isAtStartOfLine()   ? "\n"_rs
                                  : tok.hasLeadingSpace() ? " "_rs
                                                          : ""_rs;
          toks.push_token(before, std::move(loc), toStr(spelling));
        }
      }
      unsigned ctr = compilerInst.getPreprocessor().getCounterValue();
      toks.insert_into_map(ctr, snippets);
    }
  }
};

Rc<rust::num_bigint::BigInt> toBigInt(llvm::APInt const &n) {
  llvm::SmallString<16> out;
  n.toStringSigned(out);
  return mk_bigint(toStr(out));
}

class C2PulseConsumer : public ASTConsumer {
public:
  C2PulseConsumer(RefMut<Ctx> c, RangeMap &m, SnipMap &s, CompilerInstance &ci)
      : ctx(c), rangeMap(m), snippets(s), sm(ci.getSourceManager()) {}

  RefMut<Ctx> ctx;
  RangeMap &rangeMap;
  SnipMap &snippets;
  SourceManager &sm;
  ASTContext *astCtx = nullptr;

  // TODO: should probably wait with translation until after parsing

  void Initialize(ASTContext &Context) override { astCtx = &Context; }

  virtual bool HandleTopLevelDecl(DeclGroupRef DG) override {
    for (auto D : DG)
      HandleDecl(D);
    return ASTConsumer::HandleTopLevelDecl(DG);
  }

  Rc<ir::Ident> getDeclName(NamedDecl *d) {
    auto loc = mk_fallback_sourceinfo(
        getRange(d->getSourceRange())); // TODO: get range of name token
    return ctx.mk_ident(toStr(d->getName()), std::move(loc));
  }

  Rc<ir::SourceInfo> getRange(SourceRange const &range) {
    return rangeMap.getExpansionRange(sm, range);
  }

  void reportUnsupported(SourceRange const &rng, Rc<ir::SourceInfo> const &loc,
                         Ref<rust::Str> msg) {
    if (!sm.isInMainFile(sm.getExpansionLoc(rng.getBegin()))) {
      // only complain about unsupported syntax in main file
      return;
    }
    ctx.report_diag(loc.clone(), true, msg);
  }

  Rc<ir::Type> trQualType(QualType t, SourceRange range) {
    auto loc = getRange(range);

    if (t.getAsString() == "size_t") {
      return mk_sizet(std::move(loc));
    } else if (t->isPointerType()) {
      return mk_pointer_unknown(
          std::move(loc), trQualType(t->getPointeeType(), /*TODO*/ range));
    }
    if (t->isVoidType()) {
      return mk_void_type(std::move(loc));
    }
    if (isBoolType(t)) {
      return mk_bool_type(std::move(loc));
    }
    if (t->isSignedIntegerType() || t->isUnsignedIntegerType()) {
      bool isSigned = t->isSignedIntegerType();
      unsigned width = astCtx->getIntWidth(t);
      return mk_int_type(std::move(loc), isSigned, width);
    }

    reportUnsupported(range, loc, "unsupported type"_rs);
    return mk_type_err(std::move(loc));
  }

  Rc<ir::Type> trTypeAttrs(AttrVec const &attrs, Rc<ir::Type> &&ty) {
    for (auto it = attrs.rbegin(); it != attrs.rend(); ++it) {
      if (auto ann = dyn_cast<AnnotateAttr>(*it)) {
        auto loc = getRange(ann->getRange());
        if (auto req = isUnaryAttrOf(ann, "c2pulse-requires")) {
          ty = mk_type_requires(std::move(loc), std::move(ty),
                                std::move(req.value()));
        } else if (auto ens = isUnaryAttrOf(ann, "c2pulse-ensures")) {
          ty = mk_type_ensures(std::move(loc), std::move(ty),
                               std::move(ens.value()));
        } else if (ann->getAnnotation() == "c2pulse-plain" &&
                   ann->args_size() == 0) {
          ty = mk_type_plain(std::move(loc), std::move(ty));
        } else if (ann->getAnnotation() == "c2pulse-consumes" &&
                   ann->args_size() == 0) {
          ty = mk_type_consumes(std::move(loc), std::move(ty));
        }
      }
    }
    return ty;
  }

  bool isBoolType(QualType t) {
    return t->isUnsignedIntegerType() && astCtx->getIntWidth(t) == 1;
  }

  Rc<ir::LValue> trLValue(Expr *e) {
    auto loc = getRange(e->getSourceRange());

    if (auto dre = dyn_cast<DeclRefExpr>(e)) {
      auto id = ctx.mk_ident(toStr(dre->getDecl()->getName()), loc.clone());
      return mk_lvalue_var(std::move(loc), std::move(id));
    } else if (auto p = dyn_cast<ParenExpr>(e)) {
      return trLValue(p->getSubExpr());
    } else if (auto uo = dyn_cast<UnaryOperator>(e)) {
      switch (uo->getOpcode()) {
      case UO_Deref:
        return mk_deref(std::move(loc), trRValue(uo->getSubExpr()));

      default:;
        // continue to error case
      }
    }

    reportUnsupported(e->getSourceRange(), loc,
                      "unsupported lvalue expression"_rs);
    return mk_lvalue_err(std::move(loc),
                         trQualType(e->getType(), e->getSourceRange()));
  }

  Rc<ir::RValue> trRValue(Expr *e) {
    auto loc = getRange(e->getSourceRange());

    if (auto ic = dyn_cast<CastExpr>(e)) {
      switch (ic->getCastKind()) {
      case CK_LValueToRValue:
        return mk_rvalue_lvalue(std::move(loc), trLValue(ic->getSubExpr()));

      case CK_NoOp:
      case CK_IntegralCast:
      case CK_IntegralToBoolean:
        return mk_rvalue_cast(std::move(loc), trRValue(ic->getSubExpr()),
                              trQualType(ic->getType(), ic->getSourceRange()));

      default:;
        // continue to error case
      }
    } else if (auto p = dyn_cast<ParenExpr>(e)) {
      return trRValue(p->getSubExpr());
    } else if (auto il = dyn_cast<IntegerLiteral>(e)) {
      if (isBoolType(il->getType())) {
        return mk_bool_lit(std::move(loc), !il->getValue().isZero());
      } else {
        auto ty = trQualType(il->getType(), il->getSourceRange());
        return mk_int_lit(std::move(loc), toBigInt(il->getValue()),
                          std::move(ty));
      }
    } else if (auto uo = dyn_cast<UnaryOperator>(e)) {
      switch (uo->getOpcode()) {
      case UO_AddrOf:
        return mk_rvalue_ref(std::move(loc), trLValue(uo->getSubExpr()));

      default:;
        // continue to error case
      }
    } else if (auto *bo = dyn_cast<BinaryOperator>(e)) {
      auto m = [&](ir::BinOp op) {
        return mk_rvalue_binop(std::move(loc), std::move(op),
                               trRValue(bo->getLHS()), trRValue(bo->getRHS()));
      };
      switch (bo->getOpcode()) {
      case clang::BO_Add:
        return m(ir::BinOp::Add());
      case clang::BO_Sub:
        return m(ir::BinOp::Sub());
      case clang::BO_Mul:
        return m(ir::BinOp::Mul());
      case clang::BO_Div:
        return m(ir::BinOp::Div());
      case clang::BO_LAnd:
        return m(ir::BinOp::LogAnd());
      case clang::BO_EQ:
        return m(ir::BinOp::Eq());
      case clang::BO_LE:
        return m(ir::BinOp::LEq());

      default:;
        // continue to error case
      }
    } else if (auto *c = dyn_cast<CallExpr>(e)) {
      if (auto fd = c->getDirectCallee()) {
        auto fn = ctx.mk_ident(toStr(fd->getName()),
                               getRange(c->getCallee()->getSourceRange()));
        auto args = Vec<Rc<ir::RValue>>::new_();
        for (auto arg : c->arguments()) {
          args.push(trRValue(arg));
        }
        return mk_rvalue_fncall(std::move(loc), std::move(fn), std::move(args));
      }
    }

    reportUnsupported(e->getSourceRange(), loc,
                      "unsupported rvalue expression"_rs);
    return mk_rvalue_err(std::move(loc),
                         trQualType(e->getType(), e->getSourceRange()));
  }

  Vec<Rc<ir::Stmt>> trStmts(Stmt *stmt) {
    auto stmts = Vec<Rc<ir::Stmt>>::new_();
    if (stmt)
      trStmt(stmts, stmt);
    return stmts;
  }

  rust::Unit trStmt(Vec<Rc<ir::Stmt>> &stmts, Stmt *stmt) {
    auto loc = getRange(stmt->getSourceRange());

    if (auto *bo = dyn_cast<BinaryOperator>(stmt)) {
      switch (bo->getOpcode()) {
      case clang::BO_Assign:
        return stmts.push(mk_assign(std::move(loc), trLValue(bo->getLHS()),
                                    trRValue(bo->getRHS())));

      default:;
        // continue to error case
      }
    } else if (auto *i = dyn_cast<IfStmt>(stmt)) {
      return stmts.push(mk_if(loc.clone(), trRValue(i->getCond()),
                              trStmts(i->getThen()), trStmts(i->getElse())));
    } else if (auto *w = dyn_cast<WhileStmt>(stmt)) {
      auto body = w->getBody();
      auto invs = Vec<Rc<ir::RValue>>::new_();
      if (auto attrBody = dyn_cast<AttributedStmt>(body)) {
        for (auto attr : attrBody->getAttrs()) {
          if (auto inv = isUnaryAttrOf(attr, "c2pulse-invariant")) {
            invs.push(std::move(inv.value()));
          }
        }
        body = attrBody->getSubStmt();
      }
      return stmts.push(mk_while(loc.clone(), trRValue(w->getCond()),
                                 std::move(invs), trStmts(body)));
    } else if (auto *r = dyn_cast<ReturnStmt>(stmt)) {
      return stmts.push(mk_return(std::move(loc), trRValue(r->getRetValue())));
    } else if (auto *ds = dyn_cast<DeclStmt>(stmt)) {
      for (auto d : ds->decls()) {
        auto dloc = getRange(d->getSourceRange());
        if (auto vd = dyn_cast<VarDecl>(d)) {
          auto id = ctx.mk_ident(toStr(vd->getName()), dloc.clone());
          auto ty = trQualType(vd->getType(), vd->getSourceRange());
          stmts.push(mk_var_decl(dloc.clone(), id.clone(), std::move(ty)));
          if (vd->hasInit()) {
            stmts.push(mk_assign(dloc.clone(),
                                 mk_lvalue_var(dloc.clone(), id.clone()),
                                 trRValue(vd->getInit())));
          }
        } else {
          reportUnsupported(d->getSourceRange(), dloc,
                            "unsupported variable declaration"_rs);
          stmts.push(mk_stmt_err(dloc.clone()));
        }
      }
      return rust::Unit();
    } else if (auto *comp = dyn_cast<CompoundStmt>(stmt)) {
      // TODO: scope
      for (auto stmt : comp->body())
        trStmt(stmts, stmt);
      return rust::Unit();
    } else if (auto *c = dyn_cast<CallExpr>(stmt)) {
      return stmts.push(mk_call(std::move(loc), trRValue(c)));
    }

    reportUnsupported(stmt->getSourceRange(), loc, "unsupported statement"_rs);
    return stmts.push(mk_stmt_err(std::move(loc)));
  }

  std::optional<Rc<ir::RValue>> isUnaryAttrOf(Attr const *attr,
                                              char const *name) {
    if (auto ann = dyn_cast<AnnotateAttr>(attr);
        ann && ann->args_size() == 1 && ann->getAnnotation() == name) {
      if (auto ctrVal = ann->args_begin()[0]->getIntegerConstantExpr(*astCtx)) {
        unsigned ctr = ctrVal->getZExtValue();
        return {ctx.parse_rvalue(getRange(attr->getRange()), ctr, snippets)};
      }
    }
    return {};
  }

  void HandleDecl(Decl *D) {
    if (auto *FD = dyn_cast<FunctionDecl>(D)) {
      // Include block
      if (FD->getName().starts_with("__c2pulse_include_anchor")) {
        std::optional<unsigned> code;
        for (auto attr : FD->getAttrs()) {
          if (auto ann = dyn_cast<AnnotateAttr>(attr);
              ann && ann->getAnnotation() == "c2pulse-includes" &&
              ann->args_size() == 1) {
            if (auto ctrVal =
                    ann->args_begin()[0]->getIntegerConstantExpr(*astCtx)) {
              unsigned ctr = ctrVal->getZExtValue();
              code = ctr;
            }
          }
        }
        auto loc = getRange(D->getSourceRange());
        if (code) {
          ctx.add_include(std::move(loc), *code, snippets);
        } else {
          ctx.report_diag(std::move(loc), true,
                          "internal error: invalid INCLUDES encoding"_rs);
        }
        return;
      }

      // Regular function decl
      auto ident = getDeclName(FD);
      auto builder =
          DeclBuilder::new_(getRange(FD->getSourceRange()), ident.clone());
      for (auto param : FD->parameters()) {
        auto ty = trQualType(param->getType(), param->getSourceRange());
        ty = trTypeAttrs(param->getAttrs(), std::move(ty));
        if (param->getDeclName().isIdentifier() &&
            param->getName().size() > 0) {
          builder.arg(ctx.mk_ident(toStr(param->getName()),
                                   getRange(param->getSourceRange())),
                      std::move(ty));
        } else {
          builder.arg_anon(std::move(ty));
        }
      }
      builder.return_type(
          trQualType(FD->getReturnType(), FD->getReturnTypeSourceRange()));
      for (auto attr : FD->getAttrs()) {
        if (auto req = isUnaryAttrOf(attr, "c2pulse-requires")) {
          builder.requires(std::move(req.value()));
        }
        if (auto ens = isUnaryAttrOf(attr, "c2pulse-ensures")) {
          builder.ensures(std::move(ens.value()));
        }
      }
      if (FD->hasBody()) {
        ctx.add_fn_defn(std::move(builder), trStmts(FD->getBody()));
      } else {
        ctx.add_fn_decl(std::move(builder));
      }
    }
  }
};

class C2PulseAction : public SyntaxOnlyAction {
public:
  C2PulseAction(RefMut<Ctx> c, RangeMap &m) : ctx(c), rangeMap(m) {}
  RefMut<Ctx> ctx;
  RangeMap &rangeMap;
  SnipMap snippets = SnipMap::default_();

  bool BeginSourceFileAction(CompilerInstance &CI) override {
    CI.getPreprocessor().addPPCallbacks(
        std::make_unique<MacroTracker>(rangeMap, snippets, CI));
    return SyntaxOnlyAction::BeginSourceFileAction(CI);
  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override {
    return std::make_unique<C2PulseConsumer>(ctx, rangeMap, snippets, CI);
  }

  void EndSourceFileAction() override {
    SyntaxOnlyAction::EndSourceFileAction();
  }
};

class C2PulseActionFactory : public FrontendActionFactory {
public:
  C2PulseActionFactory(RefMut<Ctx> c, RangeMap &m) : ctx(c), rangeMap(m) {}

  std::unique_ptr<FrontendAction> create() override {
    return std::make_unique<C2PulseAction>(ctx, rangeMap);
  }

  RefMut<Ctx> ctx;
  RangeMap &rangeMap;
};

class C2PulseDiagnosticConsumer : public DiagnosticConsumer {
public:
  C2PulseDiagnosticConsumer(RefMut<Ctx> c, RangeMap &m) : ctx(c), rangeMap(m) {}
  RefMut<Ctx> ctx;
  RangeMap &rangeMap;

  void HandleDiagnostic(DiagnosticsEngine::Level DiagLevel,
                        const Diagnostic &Info) override {
    auto &sm = Info.getSourceManager();
    if (Info.getNumRanges() > 0) {
      auto range = Info.getRange(0);
      auto file_name = rangeMap.getFileName(
          sm, sm.getFileID(sm.getExpansionLoc(range.getBegin())));
      unsigned begin_line = sm.getExpansionLineNumber(range.getBegin());
      unsigned begin_col = sm.getExpansionColumnNumber(range.getBegin());
      unsigned end_line = sm.getExpansionLineNumber(range.getEnd());
      unsigned end_col = sm.getExpansionColumnNumber(range.getEnd());
      if (end_line == 0 || end_col == 0) {
        // unknown end position
        end_line = begin_line;
        end_col = begin_col;
      }
      llvm::SmallString<0> out;
      Info.FormatDiagnostic(out);
      ctx.report_diag(mk_original_location(std::move(file_name), begin_line,
                                           begin_col, end_line, end_col),
                      DiagLevel >= DiagnosticsEngine::Level::Error, toStr(out));
    }
  }
};

std::string getBinaryForResourcesPath() {
  Dl_info info;
  if (dladdr((void *)&clang::driver::Driver::GetResourcesPath, &info)) {
    return info.dli_fname;
  } else {
    return "/usr/bin/clang";
  }
}

std::string getResourcesPath() {
  return clang::driver::Driver::GetResourcesPath(getBinaryForResourcesPath());
}

llvm::vfs::Status mkStatus(Ref<rust::crate::vfs::VFSEntry> entry) {
  auto fileName = entry.get_file_name();
  llvm::sys::fs::UniqueID unique(0, (uint64_t)fileName.as_ptr());
  llvm::sys::TimePoint<> time;
  return llvm::vfs::Status(
      toStringRef(fileName), unique, time, 0, 0, entry.get_contents().len(),
      llvm::sys::fs::file_type::regular_file, llvm::sys::fs::perms::all_all);
}

class CtxVFSFile : public llvm::vfs::File {
  Rc<rust::crate::vfs::VFSEntry> entry;

public:
  CtxVFSFile(Rc<rust::crate::vfs::VFSEntry> &&e) : entry(std::move(e)) {}

  llvm::ErrorOr<llvm::vfs::Status> status() override {
    return mkStatus(entry.deref());
  }

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>
  getBuffer(const Twine &Name, int64_t FileSize = -1,
            bool RequiresNullTerminator = true,
            bool IsVolatile = false) override {
    if (!RequiresNullTerminator)
      return llvm::MemoryBuffer::getMemBuffer(
          toStringRef(entry.deref().get_contents()),
          toStringRef(entry.deref().get_file_name()));

    return llvm::MemoryBuffer::getMemBufferCopy(
        toStringRef(entry.deref().get_contents()),
        toStringRef(entry.deref().get_file_name()));
  };

  std::error_code close() override { return {}; }

  llvm::ErrorOr<std::string> getName() override {
    return toString(entry.deref().get_file_name());
  }
};

class CtxVFS : public llvm::vfs::FileSystem {
  RefMut<Ctx> ctx;
  IntrusiveRefCntPtr<llvm::vfs::FileSystem> realFS;

public:
  CtxVFS(RefMut<Ctx> c) : ctx(c), realFS(llvm::vfs::getRealFileSystem()) {}

  static IntrusiveRefCntPtr<llvm::vfs::FileSystem> make(RefMut<Ctx> ctx) {
    return llvm::makeIntrusiveRefCnt<CtxVFS>(ctx);
  }

  llvm::ErrorOr<llvm::vfs::Status> status(const Twine &Path) override {
    auto res = ctx.read_vfs_file(toStr(Path.str()));
    if (!res.is_ok()) {
      // TODO: fallback for directories
      return realFS->status(Path);
    }
    return mkStatus(res.unwrap().deref());
  };

  llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>>
  openFileForRead(const Twine &Path) override {
    auto res = ctx.read_vfs_file(toStr(Path.str()));
    if (!res.is_ok()) {
      return llvm::errc::no_such_file_or_directory;
    }
    return std::make_unique<CtxVFSFile>(res.unwrap());
  }

  llvm::vfs::directory_iterator dir_begin(const Twine &Dir,
                                          std::error_code &EC) override {
    return realFS->dir_begin(Dir, EC);
  }

  std::error_code setCurrentWorkingDirectory(const Twine &Path) override {
    return realFS->setCurrentWorkingDirectory(Path);
  }

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() const override {
    return realFS->getCurrentWorkingDirectory();
  }

  std::error_code getRealPath(const Twine &Path,
                              SmallVectorImpl<char> &Output) override {
    // TODO: implement using VFS
    return realFS->getRealPath(Path, Output);
  }

  bool exists(const Twine &Path) override {
    auto res = ctx.read_vfs_file(toStr(Path.str()));
    if (res.is_ok())
      return true;

    // TODO: fallback for directories
    return realFS->exists(Path);
  }

  std::error_code isLocal(const Twine &Path, bool &Result) override {
    Result = true;
    return {};
  }

  std::error_code makeAbsolute(SmallVectorImpl<char> &Path) const override {
    // TODO: implement using VFS
    return realFS->makeAbsolute(Path);
  }
};

static void parse_file(RefMut<Ctx> ctx) {
  std::string fileName = toString(ctx.get_input_file_name());
  std::vector<std::string> sourcePathList{fileName};

  std::string compDBErrMsg;
  auto compDB =
      CompilationDatabase::autoDetectFromSource(fileName, compDBErrMsg);
  std::vector<std::string> argsForCompDB;
  if (!compDB) {
    compDB = std::make_unique<FixedCompilationDatabase>(".", argsForCompDB);
  }

  ClangTool Tool(*compDB, sourcePathList,
                 std::make_shared<PCHContainerOperations>(), CtxVFS::make(ctx));

  RangeMap rangeMap(ctx);
  C2PulseDiagnosticConsumer consumer(ctx, rangeMap);
  Tool.setDiagnosticConsumer(&consumer);

  // Tool.appendArgumentsAdjuster(OptionsParser->getArgumentsAdjuster());

  Tool.appendArgumentsAdjuster(getInsertArgumentAdjuster(
      {"-DC2PULSE", "-fno-builtin"}, ArgumentInsertPosition::BEGIN));
  Tool.appendArgumentsAdjuster(getInsertArgumentAdjuster(
      {"-resource-dir", getResourcesPath()}, ArgumentInsertPosition::BEGIN));

  C2PulseActionFactory factory(ctx, rangeMap);
  Tool.run(&factory);
}

namespace rust::exported_functions {
Unit parse_file(RefMut<Ctx> ctx) {
  ::parse_file(ctx);
  return {};
}
} // namespace rust::exported_functions