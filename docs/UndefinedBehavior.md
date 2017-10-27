# Undefined Behavior

Coral has specific situations marked as "undefined behavior". We try to translate this to "currently invalid behavior that could change in the future." These are designed to be future-proof, and we could consider taking a page out of Haskell's book and adding compiler-option headers. For an  example of such invalid behavior, there is no total ordering on operator precedence, and unspecified pairs must currently be clarified by parentheses.
