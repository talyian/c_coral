#include "parser/parser.hh"
#include "core/expr.hh"
#include "analyzers/ImportResolver.hh"
#include <string>
#include <algorithm>
namespace coral {
  namespace analyzers {
    ImportResolver::ImportResolver(ast::Module * m) : module(m) {
      m->accept(this);
      for(auto it = imported_lines.rbegin();
          it != imported_lines.rend();
          it++)
        m->body->lines.insert(m->body->lines.begin(), std::unique_ptr<ast::BaseExpr>(*it));

      for(auto &it: m->body->lines)
        if (dynamic_cast<ast::Import *>(it.get()))
          it.release();
    }


    void ImportResolver::visit(ast::Import * m) {
      std::string path;
      for(auto &part : m->path) {
        path += part;
        if (&part != &m->path.back())
          path += "/";
      }

      std::string moo = module->path;
      moo = moo.substr(0, moo.rfind('/') + 1);

      std::vector<std::string> search_paths {
          moo,
          "/home/jimmy/coral/src/libs/",
      };
      for(auto &search_dir: search_paths) {
        moo = search_dir + path + ".coral";
        FILE * f = fopen(moo.c_str(), "r");
        if (f) {
          auto _parser = coralParseModule(moo.c_str());
          auto imported_module = (ast::Module *)_coralModule(_parser);
          for(auto &newline: imported_module->body->lines) {
            ast::BaseExpr * rawptr = newline.release();
            if (rawptr)
              imported_lines.push_back(rawptr);
          }
          return;
        }
      }
      std::cerr << "Failed import: " << path << "\n";
    }
  }
}
