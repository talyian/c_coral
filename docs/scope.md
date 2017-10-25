# Scope

A Scope controls where any particular name can be referenced.

## Scope is Not Lifetime
(!) It doesn't relate to the variable's lifetime, since they could be lexically in scope but captured by a closure or heap-allocated.

## Scopes can be nested

```
auto scope = module_scope;

scope["a"].name = "a"
scope["a"].decltype = (global | param | var | function)
scope["a"].globalname = [class].[func].a
scope["a"].type
scope["a"].inferredType
scope["a"].declaredType
scope["a"].astNode = (Expr *)

auto func_scope = scope.nested();
func_scope["b"] = null | ScopeInfo


LetAssign("a", Tuple(1, "3"))

```
