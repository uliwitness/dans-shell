#Dan's Shell

##Syntax

To call a command, use the general syntax

	name( param1, param2, param3 )

Where name can be a package name consisting of several identifiers separated by dots.
If there are no parameters, you can leave away the brackets as well, or you can specify
empty brackets, as you wish. For the outermost statement on a line, you can even leave
away the brackets, so it looks almost like Bash or other shell languages.

	(functionReturningName())( param1, param2, param3 )

If you want to build the name of a command using code (e.g. to generate the pathname for
an executable), you replace the name in your function call with the expression in brackets.

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

Output the current working directory. Another way to write this command is

	.

or even

	.()

Which has the same effect.

	..

similarly returns the directory containing the current directory, so that is the same as pwd() but with the last path component removed.

	~

Is also a valid function, and returns the current user's home directory.

	cd( path )

Change the current working directory to the given path, specified as a string.

	which( "pathOrCommandName" )

Returns the full path of the executable that Dan's Shell will run if you invoke the given command. This will for example look up the search paths in the PATH environment variable and check if a command of which only the name has been specified exists there. Note that you must put the name of the desired command in quotes, otherwise the command will be executed and which() will try to find a tool named like the result of that command.

	exit()

Quit Dan's Shell.

	# This is a comment

To add comments to a line (most useful when running scripts), use the "#" character. Everything
starting with the "#" character to the end of the line will be ignored.


##Concept

The idea behind Dan's Shell is to provide a simple, familiar and regular syntax that
encourages correct and secure code. In particular, the desire is to make it impossible
to incorrectly quote variables and cause them to be executed, or to cause empty/undefined
variables to quietly disappear, causing parameter labels to be interpreted as values or
causing two concatenated paths to be interpreted as "/" and thus delete your hard disk
or whatever.
