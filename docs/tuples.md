# Tuples

## Precedence

Reading mixed equals and commas can get confusing. Currently the precedence is undefined and both the following line are a syntax error.

    # Reading this code in isolation might feel like let a = (1, 2)
    let a = 1, 2
    # Reading this code in isolation "feels like" [let (a = b), (c = d)]
    let a = b, c = d

## 1-Tuple

There is no 1-Tuple. The type system allows for it (`Tuple[T]`), but there's no way to construct one in pure Coral code.

## Assignment

    # Tuple[]
	let z = ()
	# Tuple[Int, Int]
    let x = (1, 2)
	# Tuple[Int, Str, Tuple[], Tuple[Int, Int]]
    let y = (1, "3", z, x)

## Destructuring

    # See ##Precedence on why parentheses are required
    let (a, b) = x
	let (a, b, c, d) = y
	let (a, b, c) = x # Compile Error, cannot unpack 3 values from 2-Tuple `x`
	let (a, b) = y    # Compile Error, cannot unpack 2 values from 4-tuple `y`
	# low priority ... Do I even want to implement this
	let (a, ...b) = y
	# super low priority ...
	let (...a, b) = y

## Indexing

	assert x[0] = 1
	assert x[1] = 2
	x[2] # compile-time error, cannot get element 3 from 2-Tuple `x`
	# low priority ... maybe at some point have mutable tuples
	set x[0] = 1
