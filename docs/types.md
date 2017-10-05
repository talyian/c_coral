# Types

## Sum Types

```
type Foo:
  x (Bar)
  y (Baz)
  z     # implicit unit type
  m = 3 # unit-type with a value

frob(x barVal)
frob(y bazVal)
frob(z)
frob(m)
3 + (int m)

impl Foo:

  func this.tostr ():
    match this:
	  x v: "x"
	  y v: "y"
	  z: "z"
	  m: "m"
```

## Product Types

```
# tuples are untagged
let m = (Some(1), "foobar", SomeType.new())

type Foo:  # implicit Foo['T] because z was left generic
  x: Bar
  y: Baz
  z  # implicit 'T

type Foo { x: Bar; y: Baz; z }

frob(Foo(barVal, bazVal, []))
frob(Foo(barVal, bazVal, 3))
```
