# Functions

func f (a, b):
	b
func id (a): a
func flip (f): func (a, b): (b, a)

func add (a, b): 
	a + b

# Partial Application

`@` is partial application operator, `_` is the placeholder marker

```
add @ 3
add @ (3, _)
add @ (_, 4)
flip add (3, 4)
flip (add @ 3) 4
```
