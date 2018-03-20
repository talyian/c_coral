#include "llvm-c/Core.h"

#include "codegen/LLVMFunctionCompiler.hh"
#include "codegen/LLVMTypeMap.hh"
#include "core/expr.hh"
#include "core/prettyprinter.hh"
#include "utils/ansicolor.hh"
#include <algorithm>
#include <string>

LLVMTypeRef coral::codegen::LLVMFunctionCompiler::LLVMTypeFromCoral(coral::type::Type * t) {
  bool success = false;
  auto llvm_type = llvm::GetLLVMType(module, context, t, success);
  if (success)
    return llvm_type;
  std::cerr
    << COL_LIGHT_BLUE << "Codegen WARN: Using Int64 for Unhandled Type: '"
    << *t << "'" << COL_CLEAR << "\n";
  return LLVMInt64TypeInContext(context);
}

void coral::codegen::LLVMFunctionCompiler::visit(ast::Func * expr) {

  function = LLVMAddFunction(
    module,
    expr->name.c_str(),
    LLVMTypeFromCoral(expr->type.get()));
  (*info)[expr] = function;

  if (LLVMCountParams(function) != expr->params.size()) {
    std::cerr << "Warning: Parameter count mismatch for function " << expr->name << "\n";
    PrettyPrinter::print(expr);
    std::cerr << LLVMPrintValueToString(function) << "\n";
    exit(1);
  }

  for(size_t i=0; i<expr->params.size(); i++) {
    // std::cerr << "param " << i << ": " << expr->params[i].get() << "\n";
    (*info)[expr->params[i].get()] = LLVMGetParam(function, i);
  }
  if (expr->body) {
    basic_block = LLVMAppendBasicBlock(function, "entry");
    LLVMPositionBuilderAtEnd(builder, basic_block);
    expr->body->accept(this);

  }
}

void coral::codegen::LLVMFunctionCompiler::visit(ast::Match * match) {
  this->rawPointer = 1;
  match->condition->accept(this);
  this->rawPointer = 0;
  auto input_value = out;
  LLVMValueRef indices[2] = {
    LLVMConstInt(LLVMInt32TypeInContext(context), 0, false),
    LLVMConstInt(LLVMInt32TypeInContext(context), 0, false), };
  out = LLVMBuildGEP(builder, input_value, indices, 2, "cond");
  auto llvm_cond = out;
  auto else_block = LLVMAppendBasicBlock(function, "else");
  auto after_block = LLVMAppendBasicBlock(function, "after_match");
  auto llvm_switch = LLVMBuildSwitch(
    builder,
    LLVMBuildLoad(builder, llvm_cond, ""),
    // LLVMConstInt(LLVMInt16TypeInContext(context), 0, false),
    else_block,
    match->cases.size());

  for(size_t i = 0; i < match->cases.size(); i++) {
    auto case1 = LLVMAppendBasicBlock(function, ("case" + std::to_string(i)).c_str());
    LLVMAddCase(
      llvm_switch,
      LLVMConstInt(LLVMInt16TypeInContext(context), i, false),
      case1);
    LLVMPositionBuilderAtEnd(builder, case1);
    indices[1] = LLVMConstInt(LLVMInt32TypeInContext(context), 1, false);
    auto data_ptr =  LLVMBuildGEP(builder, input_value, indices, 2, "");
    auto typed_ptr = LLVMBuildBitCast(
      builder, data_ptr,
      LLVMPointerType(LLVMTypeFromCoral(match->cases[i]->def->type.get()), 0), "");

    (*info)[match->cases[i]->def.get()] =
      LLVMBuildLoad(builder, typed_ptr, match->cases[i]->def->name.c_str());
    match->cases[i]->body->accept(this);
    LLVMBuildBr(builder, after_block);
  }
  LLVMPositionBuilderAtEnd(builder, else_block);
  // LLVMValueRef arg1[1] = { LLVMBuildGlobalString(builder, "no match\n", "fmt") };
  // LLVMBuildCall(
  //   builder,
  //   LLVMGetNamedFunction(module, "printf"),
  //   arg1, 1, "print");
  LLVMBuildBr(builder, after_block);

  LLVMPositionBuilderAtEnd(builder, after_block);
  // LLVMValueRef args[3] = {
  //   LLVMBuildGlobalString(builder, "Printf (%hd)\n", "fmt"),
  //   LLVMBuildLoad(builder, llvm_cond, ""),
  // };
  // LLVMBuildCall(
  //   builder,
  //   LLVMGetNamedFunction(module, "printf"),
  //   args, 2, "print");
  out = 0;
}

void coral::codegen::LLVMFunctionCompiler::visit(ast::IfExpr * expr) {
  auto thenblock = LLVMAppendBasicBlock(function, "then");
  auto elseblock = LLVMAppendBasicBlock(function, "else");
  auto endblock = LLVMAppendBasicBlock(function, "end");
  out = LLVMBuildCondBr(
    builder,
    compile(expr->cond.get()),
    thenblock,
    elseblock);
  int branchreturns = 0;
  returns = 0;
  LLVMPositionBuilderAtEnd(builder, thenblock);
  expr->ifbody->accept(this);
  if (!returns) LLVMBuildBr(builder, endblock);
  branchreturns += returns > 0 ? 1 : 0;

  returns = 0;
  LLVMPositionBuilderAtEnd(builder, elseblock);
  if (expr->elsebody) expr->elsebody->accept(this);
  if (!returns) LLVMBuildBr(builder, endblock);
  branchreturns += returns > 0 ? 1 : 0;

  // if we're returning from both branches, we didn't generate any
  // jumps to the ending basic block
  if (branchreturns == 2) {
    LLVMRemoveBasicBlockFromParent(endblock);
  } else {
    LLVMPositionBuilderAtEnd(builder, endblock);
  }
}

void coral::codegen::LLVMFunctionCompiler::visit(ast::IntLiteral * expr) {
  out = LLVMConstInt(LLVMInt32TypeInContext(context), std::stol(expr->value), false);
}

void coral::codegen::LLVMFunctionCompiler::visit(ast::FloatLiteral * expr) {
  out = LLVMConstReal(LLVMDoubleTypeInContext(context), std::stof(expr->value));
}

void coral::codegen::LLVMFunctionCompiler::visit(ast::StringLiteral * expr) {
  auto stringval = expr->getString();
  auto global = LLVMAddGlobal(
    module,
    LLVMArrayType(LLVMInt8TypeInContext(context), 1 + stringval.size()), "");
  auto llval = LLVMConstStringInContext(context, stringval.c_str(), stringval.size(), false);
  LLVMSetInitializer(global, llval);
  out = LLVMBuildBitCast(builder, global, LLVMPointerType(LLVMInt8TypeInContext(context), 0), "");
}

void coral::codegen::LLVMFunctionCompiler::visit(ast::Var * var) {
  out = 0;
  if (info->find(var->expr) == info->end()) {
    std::cerr << var->expr << "\t";
    std::cerr << ast::ExprNameVisitor::of(var->expr) << "\t";
    std::cerr << ((ast::Def *)(var->expr))->name << "\n";
    std::cerr << COL_LIGHT_RED << "Not Found: " << var->name << "\033[0m\n";
    return;
  }
  switch (ast::ExprTypeVisitor::of(var->expr)) {
  case ast::ExprTypeKind::DefKind:
    out = info->find(var->expr)->second;
    return;
  case ast::ExprTypeKind::FuncKind:
    // std::cerr << "var codegen" << var->name << "--------------------\n";
    out = info->find(var->expr)->second;
    // PrettyPrinter::print(var->expr);
    // std::cerr << LLVMPrintValueToString(out) << "\n";
    return;
  case ast::ExprTypeKind::LetKind:
    if (this->rawPointer) {
      out = info->find(var->expr)->second;
    } else {
      // A Let-expr usually generates a local -> Alloca
      // std::cerr << "Loading " << var->name << " = "
      //           << LLVMPrintValueToString(info->find(var->expr)->second) << " | "
      //           << LLVMPrintTypeToString(LLVMTypeOf(info->find(var->expr)->second))
      //           << "\n";
      out = LLVMBuildLoad(builder, info->find(var->expr)->second, var->name.c_str()) ;
    }
    return;
  case ast::ExprTypeKind::ExternKind:
    out = info->find(var->expr)->second;
    return;
  default:
    std::cerr << "unknown var kind : " << var->name << " :: " << ast::ExprNameVisitor::of(var->expr) << "\n";
    break;
  }
  out = LLVMConstInt(LLVMInt32TypeInContext(context), 0, false);
}
void coral::codegen::LLVMFunctionCompiler::visit(ast::BinOp * expr) {
  auto lhs = compile(expr->lhs.get());
  auto rhs = compile(expr->rhs.get());
  if (expr->funcptr) {
    auto func = (*info)[expr->funcptr];
    if (!func) {
      std::cerr << "Warning: missing info for function pointer " << expr->funcptr->name << "\n";
    }
    LLVMValueRef args[2] = {lhs, rhs};
    out = LLVMBuildCall(builder, func, args, 2, expr->op.c_str());
    return;
  }

  if (LLVMGetTypeKind(LLVMTypeOf(lhs))==LLVMPointerTypeKind) {
    if (expr->op == "+") {
      // LLVMValueRef indices[1] = { rhs };
      out = LLVMBuildGEP(builder, lhs, &rhs, 1, "");
    } else {
      std::cerr << "Unknown Operator " << expr->op << " for Pointers\n";
      out = 0;
    }
    return;
  }


  if (LLVMGetTypeKind(LLVMTypeOf(lhs)) == LLVMIntegerTypeKind) {
    if (expr->op == "-")
      out = LLVMBuildSub(builder, lhs, rhs, "");
    else if (expr->op == "+")
      out = LLVMBuildAdd(builder, lhs, rhs, "");
    else if (expr->op == "%")
      out = LLVMBuildSRem(builder, lhs, rhs, "");
    else if (expr->op == "*")
      out = LLVMBuildMul(builder, lhs, rhs, "");
    else if (expr->op == "/")
      out = LLVMBuildSDiv(builder, lhs, rhs, "");
    else if (expr->op == "=")
      out = LLVMBuildICmp(builder, LLVMIntEQ, lhs, rhs, "");
    else if (expr->op == "!=")
      out = LLVMBuildICmp(builder, LLVMIntNE, lhs, rhs, "");
    else if (expr->op == "<")
      out = LLVMBuildICmp(builder, LLVMIntSLT, lhs, rhs, "");
    else if (expr->op == "<=")
      out = LLVMBuildICmp(builder, LLVMIntSLE, lhs, rhs, "");
    else if (expr->op == ">")
      out = LLVMBuildICmp(builder, LLVMIntSGT, lhs, rhs, "");
    else if (expr->op == ">=")
      out = LLVMBuildICmp(builder, LLVMIntSGE, lhs, rhs, "");
  } else if (LLVMGetTypeKind(LLVMTypeOf(lhs)) == LLVMDoubleTypeKind) {
    if (expr->op == "-")
      out = LLVMBuildFSub(builder, lhs, rhs, "");
    else if (expr->op == "+")
      out = LLVMBuildFAdd(builder, lhs, rhs, "");
    else if (expr->op == "%")
      out = LLVMBuildFRem(builder, lhs, rhs, "");
    else if (expr->op == "*")
      out = LLVMBuildFMul(builder, lhs, rhs, "");
    else if (expr->op == "/")
      out = LLVMBuildFDiv(builder, lhs, rhs, "");
    else if (expr->op == "=")
      out = LLVMBuildFCmp(builder, LLVMRealOEQ, lhs, rhs, "");
    else if (expr->op == "!=")
      out = LLVMBuildFCmp(builder, LLVMRealONE, lhs, rhs, "");
    else if (expr->op == "<")
      out = LLVMBuildFCmp(builder, LLVMRealOLT, lhs, rhs, "");
    else if (expr->op == "<=")
      out = LLVMBuildFCmp(builder, LLVMRealOLE, lhs, rhs, "");
    else if (expr->op == ">")
      out = LLVMBuildFCmp(builder, LLVMRealOGT, lhs, rhs, "");
    else if (expr->op == ">=")
      out = LLVMBuildFCmp(builder, LLVMRealOGE, lhs, rhs, "");
  }
}
void coral::codegen::LLVMFunctionCompiler::visit(ast::Return * expr) {
  returns++;
  if (expr->val) {
    out = compile(expr->val.get());
    if (LLVMGetTypeKind(LLVMTypeOf(out)) == LLVMVoidTypeKind)
      out = LLVMBuildRetVoid(builder);
    else
      out = LLVMBuildRet(builder, out);
  }
  else
    out = LLVMBuildRetVoid(builder);
}

namespace coral {
  LLVMValueRef ParseStruct(coral::codegen::LLVMFunctionCompiler * cc, coral::ast::Call * expr) {
    auto size = expr->arguments.size();
    LLVMTypeRef * fieldTypes = new LLVMTypeRef[size];
    LLVMValueRef * fieldValues = new LLVMValueRef[size];
    std::string * fieldNames  = new std::string[size];
    int i=0;
    for(auto && arg : expr->arguments) {
      if (ast::ExprTypeVisitor::of(arg.get()) != ast::ExprTypeKind::BinOpKind) return 0;
      auto binop = (ast::BinOp *)arg.get();
      if (binop->op != "=") return 0;
      auto lhs = dynamic_cast<ast::Var *>(binop->lhs.get());
      if (lhs == 0) {
        std::cerr << "FAIL " << ast::ExprNameVisitor::of(binop->lhs.get()) << "\n";
        return 0;
      }
      fieldNames[i] = lhs->name;
      fieldValues[i] = cc->compile(binop->rhs.get());
      fieldTypes[i] = LLVMTypeOf(fieldValues[i]);
      i++;
    }
    auto type = LLVMStructTypeInContext(cc->context, fieldTypes, size, true);
    auto val = LLVMBuildAlloca(cc->builder, type, "");
    LLVMValueRef index[2] = {
      LLVMConstInt(LLVMInt32TypeInContext(cc->context), 0, false),
      LLVMConstInt(LLVMInt32TypeInContext(cc->context), 0, false) };
    for(size_t j=0; j<expr->arguments.size(); j++) {
      index[1] = LLVMConstInt(LLVMInt32TypeInContext(cc->context), j, false);
      auto ptr = LLVMBuildGEP(cc->builder, val, index, 2, fieldNames[j].c_str());
      LLVMBuildStore(cc->builder, fieldValues[j], ptr);
    }
    // std::cerr << COL_LIGHT_RED;
    // std::cerr << "Struct Expression: " << LLVMPrintValueToString(val);
    // std::cerr << COL_CLEAR << "\n";
    return val;
  }
}

void coral::codegen::LLVMFunctionCompiler::visit(ast::Call * expr) {
  // if this is a methodcall, convert it into a direct call
  expr->methodCallInvert();

  if (ast::Member * member = dynamic_cast<ast::Member *>(expr->callee.get())) {
    if (member->methodPtr)
      ;
    else if (ast::Var * var = dynamic_cast<ast::Var *>(member->base.get())) {
      if (ast::Union * union_type = dynamic_cast<ast::Union *>(var->expr)) {
        for(size_t i = 0; i < union_type->cases.size(); i++) {
          if (union_type->cases[i]->name == member->member) {
            auto var = new ast::Var("member..member");
            var->expr = union_type->cases[i].get();
            expr->callee.reset(var);
            break;
          }
        }
      }
    }
  }

  if (ast::ExprTypeVisitor::of(expr->callee.get()) == ast::ExprTypeKind::VarKind) {
    auto var = dynamic_cast<ast::Var*>(expr->callee.get());
    // Tuple Constructor..... need a better way to do this
    // std::cerr << "var " << var->name << " kind: " << ast::ExprNameVisitor::of(var->expr) << "\n";
    if (var->expr && ast::ExprTypeVisitor::of(var->expr) == ast::ExprTypeKind::TupleKind) {
      auto tuple_type = LLVMGetTypeByName(module, var->name.c_str());
      auto tupleval = LLVMBuildAlloca(builder, tuple_type, var->name.c_str());
      for(size_t i = 0; i < expr->arguments.size(); i++) {
        expr->arguments[i]->accept(this);
        LLVMBuildStore(
          builder, out,
          LLVMBuildStructGEP(builder, tupleval, i, ""));
      }
      out = LLVMBuildLoad(builder, tupleval, "");
      return;
    }
    // TODO: make this an actual operator

    else if (var->name.substr(0, 10) == "_llvmBuild") {
      if (var->name == "_llvmBuildAdd") {
        expr->arguments[0]->accept(this);
        auto lhs = out;
        expr->arguments[1]->accept(this);
        auto rhs = out;
        out = LLVMBuildAdd(builder, lhs, rhs, "");
      }
      if (var->name == "_llvmBuildFAdd") {
        expr->arguments[0]->accept(this);
        auto lhs = out;
        expr->arguments[1]->accept(this);
        auto rhs = out;
        out = LLVMBuildFAdd(builder, lhs, rhs, "");
      }
      return;
    }
    else if (var->name == "addrof") {
      this->rawPointer = 1;
      expr->arguments[0]->accept(this);
      this->rawPointer = 0;
      return;
    }
    else if (var->name == "negate") {
      expr->arguments[0]->accept(this);
      if (LLVMGetTypeKind(LLVMTypeOf(out)) == LLVMIntegerTypeKind)
        out = LLVMBuildMul(builder, LLVMConstInt(LLVMTypeOf(out), -1, false), out, "");
      else
        out = LLVMBuildFMul(builder, LLVMConstReal(LLVMTypeOf(out), -1.0), out, "");
      return;
    }
    else if (var->name == "int64") {
      expr->arguments[0]->accept(this);
      out = LLVMBuildSExt(builder, out, LLVMInt64TypeInContext(context), "");
      return;
    } else if (var->name == "ptr") {
      auto literal = dynamic_cast<ast::IntLiteral *>(expr->arguments[0].get());
      auto val = std::stoull(literal->value);
      auto ptrtype = coral::type::Type("Ptr");
      out = LLVMConstPointerCast(
        LLVMConstInt(LLVMInt64TypeInContext(context), val, false),
        LLVMTypeFromCoral(&ptrtype));
      return;
    }
    else if (var->name == "deref") {
      expr->arguments[0]->accept(this);
      out = LLVMBuildLoad(builder, out, var->name.c_str());
      return;
    }
    else if (var->name == "derefi") {
      // TODO typed pointers should eliminate the need for a "deref-as-integer" builtin
      expr->arguments[0]->accept(this);
      auto intval = out;
      auto inttype = LLVMTypeOf(intval);
      auto ptrval = LLVMBuildIntToPtr(builder, intval, LLVMPointerType(inttype, 0), "ptrcast");
      out = LLVMBuildLoad(builder, ptrval, var->name.c_str());
      return;
    } else if (var->name == "struct") {
      out = ParseStruct(this, expr);
      out = LLVMBuildLoad(builder, out, "");
      return;
    }

    expr->callee->accept(this);

    if (!out) {
      out = LLVMGetNamedFunction(module, var->name.c_str());
      if (!out)
        out = LLVMAddFunction(
          module, var->name.c_str(),
          LLVMFunctionType(LLVMVoidTypeInContext(context), 0, 0, true));
    }
  } else {
    expr->callee->accept(this);
    if (!out) {
      std::cerr << "missing var " << ast::ExprNameVisitor::of(expr->callee.get()) << "\n";
    }
  }
  auto llvmVarRef = out;
  auto llvmArgs = new LLVMValueRef[expr->arguments.size()];

  // std::cerr << "Function " << expr->callee.get() << "--------------------\n";
  // PrettyPrinter::print(expr->callee.get());
  // std::cerr << LLVMPrintValueToString(llvmVarRef) << "\n";

  for(size_t i=0; i<expr->arguments.size(); i++) {
    // std::cerr << "  arg[" << i << "]"<< expr->arguments[i].get() << "\n";
    expr->arguments[i]->accept(this);
    llvmArgs[i] = out;
  }
  out = LLVMBuildCall(
    builder, llvmVarRef,
    llvmArgs, expr->arguments.size(), "");
  delete [] llvmArgs;
}

void coral::codegen::LLVMFunctionCompiler::visit(ast::Block * expr) {
  for(auto && line : expr->lines) if (line) line->accept(this);
}

void coral::codegen::LLVMFunctionCompiler::visit(ast::Let * expr) {
  out = 0;
  expr->value->accept(this);
  auto llval = out;
  // this shouldn't happen usually
  if (!out) { std::cerr << "warning: null LLVMinstr: " << expr->var->name << "\n"; return; }
  // TODO: LLVMTypeFromCoral(expr->type)
  auto vartype = expr->type.name != "" ? LLVMTypeFromCoral(&expr->type) : LLVMTypeOf(llval);
  auto local = LLVMBuildAlloca(builder, vartype, expr->var->name.c_str());

  // std::cerr << COL_LIGHT_BLUE;
  // PrettyPrinter::print(expr->var.get());
  // auto str = LLVMPrintTypeToString(LLVMTypeOf(llval));
  // std::cerr << " -- " << str
  //           << COL_CLEAR << "\n";
  // LLVMDisposeMessage(str);
  LLVMBuildStore(builder, llval, local);
  (*info)[expr] = local;
}

void coral::codegen::LLVMFunctionCompiler::visit(ast::Set * expr) {
  out = 0;
  expr->value->accept(this);
  auto llval = out;
  auto local = (*info)[expr->var->expr];
  if (ast::ExprTypeVisitor::of(expr->var->expr) == ast::ExprTypeKind::DefKind) {
    std::cerr << "Warning: writing to a parameter is not currently supported\n";
    out = 0;
    return;
  }
  LLVMBuildStore(builder, llval, local);
}

void coral::codegen::LLVMFunctionCompiler::visit(ast::Comment * w) { }

void coral::codegen::LLVMFunctionCompiler::visit(ast::Member * w) {
  this->rawPointer = 1;
  w->base->accept(this);
  this->rawPointer = 0;
  auto baseinstr = out;

  if (!baseinstr) {
    std::cerr << "Warning: member base failed to codegen: ";
    PrettyPrinter::print(w);
    exit(2);
  } else {
    // PrettyPrinter::print(w);
    // std::cerr << "^ member debugging ";
    // std::cerr << (void *)baseinstr << " ";
    // std::cerr << LLVMPrintValueToString(baseinstr) << "\n";
  }
  if (w->methodPtr) {
    out = (*info)[w->methodPtr];
    return;
  }

  auto n = w->memberIndex;
  if (n < 0) {
    std::cerr << "Compile Error: Member Index not found: " << w->member << "\n";
    exit(5);
  }

  // build either a GEP or extractvalue instruction.
  // HACK: this is probably not the best way to handle this....
  if (LLVMGetTypeKind(LLVMTypeOf(baseinstr)) == LLVMPointerTypeKind) {
    LLVMValueRef index[2] = {
      LLVMConstInt(LLVMInt32TypeInContext(context), 0, false),
      LLVMConstInt(LLVMInt32TypeInContext(context), n, false)};
    out = LLVMBuildGEP(builder, baseinstr, index, 2, w->member.c_str());
    out = LLVMBuildLoad(builder, out, w->member.c_str());
  } else {
    out = LLVMBuildExtractValue(builder, baseinstr, n, w->member.c_str());
  }
}


void coral::codegen::LLVMFunctionCompiler::visit(ast::TupleLiteral * t) {
  auto tuple_type = LLVMTypeFromCoral(t->type.get());
  auto tupleval = LLVMBuildAlloca(builder, tuple_type, "");

  for(size_t i = 0; i < t->items.size(); i++) {
    t->items[i]->accept(this);
    LLVMBuildStore(
      builder, out,
      LLVMBuildStructGEP(builder, tupleval, i, ""));
  }
  out = LLVMBuildLoad(builder, tupleval, "");
}

void coral::codegen::LLVMFunctionCompiler::visit(ast::While * w) {

  auto whileblock = LLVMAppendBasicBlock(function, "while");
  auto body = LLVMAppendBasicBlock(function, "do");
  auto endblock = LLVMAppendBasicBlock(function, "end");
  LLVMBuildBr(builder, whileblock);

  LLVMPositionBuilderAtEnd(builder, whileblock);
  out = 0;
  w->cond->accept(this);
  LLVMBuildCondBr(builder, out, body, endblock);

  LLVMPositionBuilderAtEnd(builder, body);
  returns=0;
  w->body->accept(this);
  if (!returns)
    LLVMBuildBr(builder, whileblock);

  LLVMPositionBuilderAtEnd(builder, endblock);
}
