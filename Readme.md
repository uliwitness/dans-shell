#Dan's Shell

##Syntax

To call a command, use the general syntax

	name( param1, param2, param3 )

Where name can be a package name consisting of several identifiers separated by dots.
If there are no parameters, you can leave away the brackets as well, or you can specify
empty brackets, as you wish.

Currently, two kinds of values are supported:

	"Quoted String"

Strings delimited by double quotes. These may contain spaces. Like C, you can use
the backslash to escape characters in a string. E.g. to write a quote in your
string, write \" so the quote will become part of the string instead of ending it.
You can also specify \n to indicate a Unix line break (ASCII 10) and \r to indicate
a carriage return (ASCII 13, classic MacOS line break). For a Windows line break,
\r\n would work. \t will produce a tab character (ASCII 9).

	123.45
	
Number. A number can consist only of numeric characters and may optionally contain
a decimal point.


##Built-in commands

Currently, the built-in commands are:

	pwd()

Output the current working directory.

	exit()

Quit Dan's Shell.


##Concept

The idea behind Dan's Shell is to provide a simple, familiar and regular syntax that
encourages correct and secure code. In particular, the desire is to make it impossible
to incorrectly quote variables and cause them to be executed, or to cause empty/undefined
variables to quietly disappear, causing parameter labels to be interpreted as values or
causing two concatenated paths to be interpreted as "/" and thus delete your hard disk
or whatever.
