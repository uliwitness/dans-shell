#Dan's Shell

##Concept

The idea behind Dan's Shell is to provide a simple, familiar and regular syntax that
encourages correct and secure code. In particular, the desire is to make it impossible
to incorrectly quote variables and cause them to be executed, or to cause empty/undefined
variables to quietly disappear, causing parameter labels to be interpreted as values or
causing two concatenated paths to be interpreted as "/" and thus delete your hard disk
or whatever.


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

Currently, four kinds of values are supported:

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

	$myVariable

A variable local to this shell. You can use the assignment operator to assign
a value to this variable, like so:

	$myVariable = 15

In addition, equivalent syntax is provided that looks more Dan's Shell-ish:

	var.myVariable

This is consistent with environment variables, which are passed on to
commands that are executed and are referenced by writing:

	env.PATH

(to get the "PATH" environment variable, for example)

	name( -label "paramOne", --label2, "param3" )

While Dan's Shell requires commas between parameters, it has a special understanding of parameter labels. A label is an identifier that starts with a dash or two dashes, which is passed verbatim as if it was a string as a parameter to the called executable. Usually, command line tools use labels either to indicate what parameter is to follow (in the above case that "paramOne" is a label), or as switches whose presence indicates what the program is to do (like "--label2" above).

	name( "-label", "paramOne", "--label2", "param3" )

is 100% equivalent to the above labeled command, just that it's less arduous to type. Similarly, since you can leave out the brackets, you can end up at near-Bash-syntax:

	name -label "paramOne", --label2, "param3"




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

	echo( "text" )

Logs all its arguments to standard output, followed by a line break.

	source( "/path/to/file" )

Runs the given Dan's Shell script in the context of the current shell. This lets you pre-define variables etc.

	exit()

Quit Dan's Shell.

	# This is a comment

To add comments to a line (most useful when running scripts), use the "#" character. Everything
starting with the "#" character to the end of the line will be ignored.


##Shell Scripts

Dan's Shell is suitable for running shell scripts. Just pipe them into dansh's stdin, like
you would do for any other shell. You can of course also start a script text file with a
shebang to have the OS automatically run it in Dan's Shell.
