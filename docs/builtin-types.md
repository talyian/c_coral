# Builtin Types

## Number

Integers can be `Int` (arbitrary length), or fixed-length at `[U]Int(1/8/16/32/64/128/256)`

Fractions can be `Dec` (arbitrary-precision decimals), `Rat` (arbitrary-precision rational), or IEEE-754-compatible fixed-length (`Float16/32/64/128` + `Dec64/128`)

### Numbers and casting

Integer Literals can be assigned directly to all integer types, but require an zero-cost upcast to convert between non-literal values.

    let a : Int32 = 3;
	let b : Int1 = 1;
	let c : Int32 = int32 b;
	let d : Int = upcast b;

Ditto for Floats.

## String Builtins
`Bytes` is the standard byte array class and is backed by a contiguous byte buffer.
`ZString` is a null-terminated bytestring and is used for compabitility and interop purposes.

`Text` is an abstract type, with UTF-8 the default implementation
`CodedString` is a combination of a byte and a code page
`Utf8Text` is a UTF-8 string 

TODO: we probably need this stuff
`Byteslice` ...
`Bytestream` ...
`Charstream` ...
