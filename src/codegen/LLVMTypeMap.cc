#include "llvm-c/Core.h"
#include "core/type.hh"
#include "LLVMTypeMap.hh"

#include <iostream>
#include <algorithm>

namespace coral {
  namespace llvm {
    LLVMTypeRef GetLLVMType(LLVMModuleRef module, LLVMContextRef context, type::Type * t, bool &success) {
      success = true;
      if (!t) return LLVMVoidTypeInContext(context);
      if (t->name == "Void") return LLVMVoidTypeInContext(context);
      if (t->name == "Ptr") {
        if (t->params.size() == 0)
          return LLVMPointerType(LLVMInt8TypeInContext(context), 0);
        else
          return LLVMPointerType(GetLLVMType(module, context, &t->params[0], success), 0);
      }
      if (t->name == "Bool") return LLVMInt1TypeInContext(context);
      if (t->name == "UInt8") return LLVMInt8TypeInContext(context);
      if (t->name == "Int8") return LLVMInt8TypeInContext(context);
      if (t->name == "Int16") return LLVMInt16TypeInContext(context);
      if (t->name == "Int32") return LLVMInt32TypeInContext(context);
      if (t->name == "Int64") return LLVMInt64TypeInContext(context);
      // TODO: This is for printf compatibility, since c automatically casts to double
      if (t->name == "Float32") return LLVMDoubleTypeInContext(context);
      if (t->name == "Float64") return LLVMDoubleTypeInContext(context);
      if (t->name == "Field") return GetLLVMType(module, context, &t->params[1], success);
      if (t->name == "FixedArray") {
        auto ret_type = GetLLVMType(module, context, &t->params.front(), success);
        auto length = std::stoul(t->params[1].name);
        return LLVMArrayType(ret_type, length);
      }
      if (t->name == "Union") {
        LLVMTypeRef fields[2];
        fields[0] = LLVMInt16TypeInContext(context);
        coral::type::Type raw("RawUnion", t->params);
        fields[1] = GetLLVMType(module, context, &raw, success);
        if (success)
          return LLVMStructTypeInContext(context, fields, 2, true);
      }
      if (t->name == "RawUnion") {
        auto size = SizeOfType(t);
        return LLVMArrayType(LLVMInt8TypeInContext(context), size);
      }
      if (t->name == "Func" || t->name == "Method") {
        // std::cout << "LLVMTyping: " << *t << "\n";
        auto ret_type = GetLLVMType(module, context, &t->params.back(), success);
        auto nparam = t->params.size() - 1;
        std::vector<LLVMTypeRef> params;
        auto is_variadic = false;
        for(size_t i=0; i<nparam; i++) {
          if (t->params[i].name == "...")
            is_variadic = true;
          else
            params.push_back(GetLLVMType(module, context, &t->params[i], success));
        }
        // std::cerr << t << "success ? " << success << "\n";
        return params.empty() ?
          LLVMFunctionType(ret_type, 0, 0, is_variadic) :
          LLVMFunctionType(ret_type, &params[0], params.size(), is_variadic);
      }
      if (t->name == "Struct") {
        auto params = new LLVMTypeRef[t->params.size()];
        for(size_t i=0; i<t->params.size(); i++) {
          params[i] = GetLLVMType(module, context, &t->params[i], success);
        }
        return LLVMStructTypeInContext(context, params, t->params.size(), true);
      }
      if (t->name == "String") {
        return LLVMPointerType(LLVMInt8TypeInContext(context), 0);
      }
      if (t->name == "Tuple") {
        auto params = new LLVMTypeRef[t->params.size()];
        for(size_t i=0; i<t->params.size(); i++) {
          params[i] = GetLLVMType(module, context, &t->params[i], success);
        }
        auto oo = LLVMStructTypeInContext(context, params, t->params.size(), true);
        delete [] params;
        return oo;
      }

      auto defined_type = LLVMGetTypeByName(module, t->name.c_str());
      if (defined_type) {
        return defined_type;
      }

      success = false;
      return LLVMVoidTypeInContext(context);
    }

    size_t SizeOfType(coral::type::Type * t) {
      if (t->name == "Int1") return 1;
      if (t->name == "Int8") return 1;
      if (t->name == "UInt8") return 1;
      if (t->name == "Int32") return 4;
      if (t->name == "Int64") return 8;

      if (t->name == "Float32") return 4;
      if (t->name == "Float64") return 8;

      if (t->name == "Ptr") return 8;

      if (t->name == "Union") {
        size_t size = 0;
        for(auto &p: t->params) size = std::max(size, SizeOfType(&p));
        return size;
      }
      if (t->name == "RawUnion") {
        size_t size = 0;
        for(auto &p: t->params) size = std::max(size, SizeOfType(&p));
        return size;
      }
      if (t->name == "Tuple") {
        size_t size = 0;
        for(auto &p: t->params) size += SizeOfType(&p);
        return size;
      }
      std::cerr << "\033[1;31mTrying to sizeof an unknown type " << t << "\033[0m\n";
      return 8;
    }
  }
}
