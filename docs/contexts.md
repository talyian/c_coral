# Block Contexts and Indentation

Normally the body of a block is indented. within this block, a context
for variables is established. For example, `let` and `for` add a new
symbol to the context; let evaluates immediately but for binds its body over
its argument.

```
func foo(a, b):
	let c = a + b:
		let d = a / b:
			for x in [a,b,c,d]:
				print x
```

### Parse Tree Pseudocode
```
Function("foo", ["a", "b"],
  Let("c", Op("+", "a", "b"),
    Let("d", Op("/", "a", "b"), 
	  For("x", List("a", "b", "c", "d"),
	    Call("print", "x")))))
```		
Of course, this leads to silly levels of indentation for `let` blocks, so we just allow implicit blocks that end when the containing block end. The following should still produce the same parse tree.

```
func foo(a, b):
	let c = a + b
	let d = a / b
	for x in [a, b, c, d]
	print x
```

Now, this looks weird for the for loop example, but for the sake of consistency we allow it. This supports the decision to keep colons as a block open indicator even if they aren't necessary.

```
for x in foobar:
   loopcontent
afterloop
```

```
for x in foobar:
afterloop
```

In this case, deleting loopcontent results in an empty loop body, and the colon saves us from an embarrassing bug where afterloop becomes the new loop content.
