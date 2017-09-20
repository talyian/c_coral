# Blocks

## Indent / Dedent are the default block format

```
def foo:    # start
  if bar:   #   indent
    baz     #     indent
  else:     #     dedent
    speem   #     indent
            #     dedent
	    #   dedent
	    # eof

block : ':' NL INDENT lines NL DEDENT
```

## TODO: Braces are the alternative block format

For a precedent, think about YAML's flow- vs block- contexts. Using braces makes Coral look like Rust.

```
def foo:
  # we can mix and match blocks- and flow- contexts
  if bar {
    for baz:
        bar
  } else {
    speem 
  }

block : ':' NL INDENT lines NL DEDENT

```
