#include "utils/ansicolor.hh"
#include "core/expr.hh"
#include "core/prettyprinter.hh"
#include "analyzers/TypeResolver.hh"
#include "analyzers/inference/Builder.hh"

#include <iostream>
#include <iomanip>
#include <set>
#include <algorithm>

coral::analyzers::TypeResolver::TypeResolver(ast::Module * m): module(m) {
  typeinference::InferenceBuilder resolver(m);

  // rewrite types back into the AST
  for(auto &&pair : resolver.env.critical_constraints) {
    if (!pair.first) continue;
    auto expr = pair.first->expr;
    auto tvalue = dynamic_cast<typeinference::Type *>(pair.second);
    if (tvalue) {
      auto tvaluetype = tvalue->concrete_type();
      if (tvaluetype){
        if (auto let = dynamic_cast<ast::Let *>(expr)) {
          if (let->type.name == "")
            let->type = *tvaluetype;
        }
        else if (auto func = dynamic_cast<ast::Func *>(expr)) {
          if (func->type->returnType().name == "")
            func->type.reset(tvalue->concrete_type());
          for(size_t i = 0; i < func->params.size(); i++) {
            if (!func->params[i]->type || func->params[i]->type->name == "") {
              typeinference::Type * v = (typeinference::Type *)(tvalue->params[i]);
              func->params[i]->type.reset(v->concrete_type());
            }
          }
        }
        delete tvaluetype;
      }
    }
  }
}
