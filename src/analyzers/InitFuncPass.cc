#include "analyzers/InitFuncPass.hh"
#include "core/expr.hh"

namespace coral {
  namespace analyzers {
    ast::Func * InitFuncPass::getInitFunc() {
      if (func) return func;

      for(auto &item: module->body->lines)
        if (ast::Func *f = dynamic_cast<ast::Func*>(item.get()))
          if (f->name == "..init")
            return func = f;

      auto block = new ast::Block({});
      func = new ast::Func(
        {"..init"},
        new type::Type(""),
        {}, block);
      module->body->lines.emplace_back(func);
      return func;
    }

    InitFuncPass::InitFuncPass(coral::ast::Module * m) : module(m) {
      auto init_func = getInitFunc();
      for(auto it = m->body->lines.begin(); it != m->body->lines.end();) {
        remove = false;
        if (*it) (*it)->accept(this);
        if (remove) {
          auto ptr = it->release();
          init_func->body->lines.push_back(std::unique_ptr<ast::BaseExpr>(ptr));
          it = m->body->lines.erase(it);
        } else
          it++;
      }
    }

    void InitFuncPass::visit(ast::Call * call) {
      remove = true;
      std::cerr << "Call\n";
    }

    void InitFuncPass::visit(ast::Let * let) {
      remove = true;
      std::cerr << "Let\n";
    }
  }
}
