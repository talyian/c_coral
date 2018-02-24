from collections import namedtuple
import regex as re
print("\033[32mType Test\033[0m")
src = '''    i1 :: Int32
           op:= :: Call(Func(*T0, *T0, *T0), collatz.n, i1)
           op:% :: Call(Func(*T1, *T1, *T1), collatz.n, i2)
           i1.0 :: Int32
         op:=.0 :: Call(Func(*T2, *T2, *T2), op:%, i1.0)
   call.collatz :: Call(collatz, op:+, op:+.0)
             i3 :: Int32
           op:* :: Call(Func(*T3, *T3, *T3), i3, collatz.n)
           i1.1 :: Int32
           op:+ :: Call(Func(*T4, *T4, *T4), op:*, i1.1)
 collatz_loop.a :: Int32
 collatz_loop.b :: Int32
           i1.2 :: Int32
         op:+.0 :: Call(Func(*T5, *T5, *T5), collatz.m, i1.2)
 call.collatz.0 :: Call(collatz, op:/, op:+.1)
           i2.0 :: Int32
           op:/ :: Call(Func(*T6, *T6, *T6), collatz.n, i2.0)
           i1.3 :: Int32
         op:+.1 :: Call(Func(*T7, *T7, *T7), collatz.m, i1.3)
             if :: call.collatz
             if :: call.collatz.0
           if.0 :: collatz.m
           if.0 :: if
   collatz_loop :: Func(collatz_loop.a, collatz_loop.b, call.collatz.1)
 call.collatz.1 :: Call(collatz, collatz_loop.a, i0)
             i0 :: Int32
           main :: Func(i0.0)
 call.collatz.2 :: Call(collatz, i1.4, i30)
           i1.4 :: Int32
            i30 :: Int32
           i0.0 :: Int32
        collatz :: Func(collatz.n, collatz.m, if.0)
             i2 :: Int32'''

def parsecons(cons):
  tok = re.split('([(), ])', cons)
  def loopparse(tok, stack=[[]]):
    if tok == []:
        return stack
    if tok[0] == '(':
        return loopparse(tok[1:], stack + [[]])
    elif tok[0] == ')':
        y = stack.pop()
        if stack:
            stack[-1].append(y)
        return loopparse(tok[1:], stack[:-1])
    elif type(tok[0]) == list:
        return tok[0]
    elif tok[0] == ',':
        return loopparse(tok[1:], stack)
    elif not tok[0].strip():
        return loopparse(tok[1:], stack)
    else:
        stack[-1].append(tok[0])
        return loopparse(tok[1:], stack)
  print(loopparse(tok))

parsecons('Call(Func(*T5, *T5, *T5), collatz.m, i1.2)')

import sys; sys.exit(0)

rules = [x.strip().split(' :: ') for x in src.splitlines()]
terms = []
constraints = []
class Cons:
    def __init__(self, term, cons):
        self.term = term
        self.cons = cons
    def __str__(self): return "\033[35m{0:>20}\033[0m :: {1}".format(self.term, self.cons)

class IsType(namedtuple('IsType', 'name params')):
    def __str__(self): return self.name + ('' if not self.params else str(self.params))
    def __repr__(self): return str(self)
def parsecons(cons, term):
    def type(s):
        return IsType('{' + s + '}', [])
        return IsType(m.group(2), [type(x) for x in m.captures(3)])
    print("{:50} :: {}".format(cons, type(cons)))
    return Cons(term, type(cons))

def addConstraint(term, cons):
    constraints.append(parsecons(cons, term))

def addTerm(s):
    terms.append(s)

def show():
  for x in constraints:
    print(x)

for x in rules:
    addTerm(x[0])
    addConstraint(x[0] , x[1])
show()
