# Modules

Modules don't need to be declared.

    export * # is the default
	export (a, b, c, d)

Modules can be imported by `import foo [{[x as y]}] [as bar]`,

	import foo
	import foo as bar
	import foo { x, y }
	import foo { x as fooX, y as fooY }
	import foo { x, y } as bar
	import foo { x as barX, y as barY } as bar

## Import Search Path

`CORAL_MODULE_PATH` contains a list of paths to search for the module `foo`

* System Modules installed with Coral
* (Not currently implemented, but some kind of virtualenv like space?)
* Project 3rd Party Modules Directory
* Project Root
* Current Directory, if script mode (__main__)
