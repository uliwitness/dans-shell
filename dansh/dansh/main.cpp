//
//  main.cpp
//  dansh
//
//  Created by Uli Kusterer on 15/04/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <memory>
#include <vector>
#include <cassert>
#include <map>


using namespace std;


typedef enum
{
	DANSH_TOKEN_TYPE_INVALID,
	DANSH_TOKEN_TYPE_IDENTIFIER,
	DANSH_TOKEN_TYPE_OPENING_BRACKET,
	DANSH_TOKEN_TYPE_CLOSING_BRACKET,
	DANSH_TOKEN_TYPE_COMMA,
	DANSH_TOKEN_TYPE_DOT,
	DANSH_TOKEN_TYPE_STRING,
	DANSH_TOKEN_TYPE_NUMBER
} dansh_token_type;


class dansh_token
{
public:
	dansh_token() : type(DANSH_TOKEN_TYPE_INVALID) {}
	
	dansh_token_type	type;
	string				text;
};


typedef enum
{
	DANSH_STATEMENT_TYPE_FUNCTION,
	DANSH_STATEMENT_TYPE_STRING,
	DANSH_STATEMENT_TYPE_NUMBER
} dansh_statement_type;


class dansh_statement
{
public:
	dansh_statement() : type(DANSH_STATEMENT_TYPE_FUNCTION) {}
	
	dansh_statement		eval() const;
	
	void	print( ostream& outStream ) const;
	
	string					name;
	vector<dansh_statement>	params;
	dansh_statement_type	type;
};


typedef function<dansh_statement(dansh_statement& stmt)> dansh_built_in_lambda;

map<string,dansh_built_in_lambda>	gBuiltInCommands;


dansh_statement		dansh_statement::eval() const
{
	if( type == DANSH_STATEMENT_TYPE_FUNCTION )
	{
		dansh_statement		evaluated;
		evaluated.name = name;
		evaluated.type = type;
		for( const dansh_statement& currParam : params )
		{
			evaluated.params.push_back( currParam.eval() );
		}
		
		map<string,dansh_built_in_lambda>::const_iterator	foundCommand = gBuiltInCommands.find(name);
		if( foundCommand == gBuiltInCommands.end() )
		{
			cerr << "Unknown command \"" << name << "\"." << endl;
			return dansh_statement();
		}
		return foundCommand->second( evaluated );
	}
	else
		return *this;
}


void	dansh_statement::print( ostream& outStream ) const
{
	if( type == DANSH_STATEMENT_TYPE_FUNCTION )
	{
		outStream << name << "( ";
		bool	isFirst = true;
		for( const dansh_statement& currParam : params )
		{
			if( isFirst )
				isFirst = false;
			else
				outStream << ", ";
			currParam.print( outStream );
		}
		outStream << " )";
	}
	else
	{
		outStream << name;
	}
}


bool		gKeepRunning = true;



dansh_statement	parse_one_statement( const vector<dansh_token> & tokens, vector<dansh_token>::const_iterator & currToken );


string	user_home_dir()
{
	string	resultPath;
	long	bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
	if( bufsize != -1 )
    {
		uid_t			uid = geteuid();
		struct passwd	pw = {};
		struct passwd*	result = nullptr;
		char			buffer[bufsize];
		int				success = getpwuid_r( uid, &pw, buffer, bufsize, &result );
		if( success == 0 && result )
		{
			resultPath = result->pw_dir;
		}
		endpwent();
	}
	
	return resultPath;
}


void	initialize()
{
	string	userHomeDir = user_home_dir();
	if( userHomeDir.length() != 0 )
		chdir( userHomeDir.c_str() );
	
	gBuiltInCommands["exit"] = []( dansh_statement params )
	{
		gKeepRunning = false;
		return dansh_statement();
	};
	
	gBuiltInCommands["pwd"] = []( dansh_statement params )
	{
		dansh_statement		currentDir;
		currentDir.type = DANSH_STATEMENT_TYPE_STRING;
		char*	currentWorkingDirectory = getcwd( NULL, 0 );
		if( currentWorkingDirectory )
		{
			currentDir.name = currentWorkingDirectory;
			free( currentWorkingDirectory );
		}
		return currentDir;
	};
	
	gBuiltInCommands["cd"] = []( dansh_statement params )
	{
		if( params.params.size() < 1 )
			cerr << "Expected directory path as first parameter of 'cd' command." << endl;
		else
			chdir( params.params[0].name.c_str() );
		return dansh_statement();
	};
}


void	finish_token( const string & currTokenString, dansh_token_type currType, vector<dansh_token> & outTokens )
{
	if( currTokenString.length() == 0 && currType != DANSH_TOKEN_TYPE_STRING )
		return;
	
	dansh_token		newToken;
	newToken.type = currType;
	newToken.text = currTokenString;
	
	outTokens.push_back( newToken );
}


void	tokenize( const string & currLine, vector<dansh_token> & outTokens )
{
	dansh_token_type		currType = DANSH_TOKEN_TYPE_INVALID;
	string::difference_type	lineLength = currLine.length();
	string					currTokenString;
	bool					inEscapeSequence = false;
	
	for( string::difference_type x = 0; x < lineLength; x++ )
	{
		char	currChar = currLine[x];
		switch( currType )
		{
			case DANSH_TOKEN_TYPE_INVALID:
				switch( currChar )
				{
					case '"':
						currType = DANSH_TOKEN_TYPE_STRING;
						break;
					
					case '0'...'9':
						currType = DANSH_TOKEN_TYPE_NUMBER;
						currTokenString.append(1, currChar);
						break;
						
					case ' ':
					case '\t':
					case '\r':
					case '\n':
						// Nothing to do, still in whitespace.
						break;
					
					case '(':
						finish_token( "(", DANSH_TOKEN_TYPE_OPENING_BRACKET, outTokens );
						break;

					case ')':
						finish_token( ")", DANSH_TOKEN_TYPE_CLOSING_BRACKET, outTokens );
						break;

					case ',':
						finish_token( ",", DANSH_TOKEN_TYPE_COMMA, outTokens );
						break;

					case '.':
						finish_token( ".", DANSH_TOKEN_TYPE_DOT, outTokens );
						break;

					default:
						currType = DANSH_TOKEN_TYPE_IDENTIFIER;
						currTokenString.append(1, currChar);
				}
				break;
			
			case DANSH_TOKEN_TYPE_IDENTIFIER:
				switch( currChar )
				{
					case '"':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_STRING;
						break;
					
					case '0'...'9':
						currTokenString.append(1, currChar);
						break;
						
					case ' ':
					case '\t':
					case '\r':
					case '\n':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_INVALID;
						break;
					
					case '(':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_INVALID;
						finish_token( "(", DANSH_TOKEN_TYPE_OPENING_BRACKET, outTokens );
						break;

					case ')':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_INVALID;
						finish_token( ")", DANSH_TOKEN_TYPE_CLOSING_BRACKET, outTokens );
						break;

					case ',':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_INVALID;
						finish_token( ",", DANSH_TOKEN_TYPE_COMMA, outTokens );
						break;

					case '.':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_INVALID;
						finish_token( ".", DANSH_TOKEN_TYPE_DOT, outTokens );
						break;

					default:
						currTokenString.append(1, currChar);
				}
				break;
			
			case DANSH_TOKEN_TYPE_STRING:
				switch( currChar )
				{
					case '"':
						if( inEscapeSequence )
						{
							currTokenString.append(1, currChar);
							inEscapeSequence = false;
						}
						else
						{
							finish_token( currTokenString, currType, outTokens );
							currTokenString.erase();
							currType = DANSH_TOKEN_TYPE_INVALID;
						}
						break;
					
					case 'n':
						if( inEscapeSequence )
						{
							currChar = '\n';
							inEscapeSequence = false;
						}
						currTokenString.append(1, currChar);
						break;

					case 'r':
						if( inEscapeSequence )
						{
							currChar = '\r';
							inEscapeSequence = false;
						}
						currTokenString.append(1, currChar);
						break;
					
					case 't':
						if( inEscapeSequence )
						{
							currChar = '\t';
							inEscapeSequence = false;
						}
						currTokenString.append(1, currChar);
						break;
					
					case '\\':
						if( inEscapeSequence )
						{
							currTokenString.append(1, currChar);
							inEscapeSequence = false;
						}
						else
						{
							inEscapeSequence = true;
						}
						break;
					
					default:
						inEscapeSequence = false;
						currTokenString.append(1, currChar);
				}
				break;
				
			case DANSH_TOKEN_TYPE_NUMBER:
				switch( currChar )
				{
					case '"':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_STRING;
						break;
					
					case '0'...'9':
					case '.':
						currTokenString.append(1, currChar);
						break;
						
					case ' ':
					case '\t':
					case '\r':
					case '\n':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_INVALID;
						break;
					
					case '(':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_INVALID;
						finish_token( "(", DANSH_TOKEN_TYPE_OPENING_BRACKET, outTokens );
						break;

					case ')':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_INVALID;
						finish_token( ")", DANSH_TOKEN_TYPE_CLOSING_BRACKET, outTokens );
						break;

					case ',':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_INVALID;
						finish_token( ",", DANSH_TOKEN_TYPE_COMMA, outTokens );
						break;

					default:
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_IDENTIFIER;
						break;
				}
				break;
			
			case DANSH_TOKEN_TYPE_OPENING_BRACKET:
			case DANSH_TOKEN_TYPE_CLOSING_BRACKET:
			case DANSH_TOKEN_TYPE_COMMA:
			case DANSH_TOKEN_TYPE_DOT:
				assert(false);	// Should never happen, brackets are single-character tokens that immediately return to whitespace.
				break;
			
		}
	}
	
	finish_token( currTokenString, currType, outTokens );	// Close last token, if we haven't yet.
}


dansh_statement	parse_one_value( const vector<dansh_token> & tokens, vector<dansh_token>::const_iterator & currToken )
{
	if( currToken == tokens.end() )
		return dansh_statement();
	
	dansh_statement		currStatement;
	if( currToken->type == DANSH_TOKEN_TYPE_STRING )
	{
		currStatement.name = currToken->text;
		currStatement.type = DANSH_STATEMENT_TYPE_STRING;
		currToken++;
		
		return currStatement;
	}
	else if( currToken->type == DANSH_TOKEN_TYPE_NUMBER )
	{
		currStatement.name = currToken->text;
		currStatement.type = DANSH_STATEMENT_TYPE_NUMBER;
		currToken++;
		
		return currStatement;
	}
	else
		return parse_one_statement( tokens, currToken );
}


dansh_statement	parse_one_statement( const vector<dansh_token> & tokens, vector<dansh_token>::const_iterator & currToken )
{
	if( currToken == tokens.end() )
		return dansh_statement();
	
	dansh_statement		currStatement;
		
	if( currToken->type == DANSH_TOKEN_TYPE_IDENTIFIER || currToken->type == DANSH_TOKEN_TYPE_DOT )
	{
		// First, parse package name of the current command:
		currStatement.name = currToken->text;
		if( currToken->type == DANSH_TOKEN_TYPE_DOT )	// We support package names with a leading dot as a shorthand for current directory.
		{
			currToken++;
			if( currToken == tokens.end() || currToken->type != DANSH_TOKEN_TYPE_IDENTIFIER )	// Grab first full identifier. Subsequent identifiers can then be parsed by our regular code:
			{
				cerr << "Expected identifier after '.' operator." << endl;
				return dansh_statement();
			}
			currStatement.name.append( currToken->text );
		}
		currToken++;
		
		while( currToken != tokens.end() && currToken->type == DANSH_TOKEN_TYPE_DOT )
		{
			currToken++;
			if( currToken == tokens.end() || currToken->type != DANSH_TOKEN_TYPE_IDENTIFIER )
			{
				cerr << "Expected identifier after '.' operator." << endl;
				return dansh_statement();
			}
			currStatement.name.append(1,'.');
			currStatement.name.append( currToken->text );
			currToken++;
		}
		
		// Now, parse parameter list, if any:
		if( currToken != tokens.end() && currToken->type == DANSH_TOKEN_TYPE_OPENING_BRACKET )
		{
			while( true )
			{
				currToken++;
				if( currToken == tokens.end() )
				{
					cerr << "Missing \")\" to close parameter list." << endl;
					return dansh_statement();
				}

				if( currToken->type == DANSH_TOKEN_TYPE_CLOSING_BRACKET )
				{
					currToken++;
					break;
				}
				
				currStatement.params.push_back( parse_one_value( tokens, currToken ) );
				
				if( currToken->type == DANSH_TOKEN_TYPE_CLOSING_BRACKET )
				{
					currToken++;
					break;
				}
				if( currToken->type != DANSH_TOKEN_TYPE_COMMA )
				{
					cerr << "Expected \",\" to separate parameters, or \")\" to end parameter list. Found \"" << currToken->text.c_str() << "\"." << endl;
					return dansh_statement();
				}
			}
		}
	}
	else
	{
		cerr << "Unexpected \"" << currToken->text.c_str() << "\" at start of line." << endl;
	}
		
	return currStatement;
}


void	process_one_line( const string & currLine )
{
	vector<dansh_token>	tokens;
	tokenize( currLine, tokens );
	
//	for( const dansh_token& currToken : tokens )
//	{
//		cout << "[" << currToken.text << "] ";
//	}
//	cout << endl;
	
	vector<dansh_token>::const_iterator	currToken = tokens.begin();
	
	dansh_statement	currStatement = parse_one_statement( tokens, currToken );
//	currStatement.print(cout);
//	cout << endl;
	
	currStatement = currStatement.eval();
	if( currStatement.type == DANSH_STATEMENT_TYPE_STRING || currStatement.name.length() != 0 )
		cout << currStatement.name << endl;
}


void	print_prompt()
{
	// Print current folder's name:
	char*	currentWorkingDirectory = getcwd( NULL, 0 );
	if( currentWorkingDirectory )
	{
		if( strcmp(currentWorkingDirectory,"/") == 0 )
		{
			cout << "/ ";
		}
		else if( user_home_dir().compare(currentWorkingDirectory) == 0 )
		{
			cout << "~ ";
		}
		else
		{
			char*		lastPathComponent = strrchr( currentWorkingDirectory, '/' );
			if( lastPathComponent )
				cout << (lastPathComponent +1) << " ";
		}
		free( currentWorkingDirectory );
	}
	
	// Print current user's name:
	long	bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
	if( bufsize != -1 )
    {
		uid_t			uid = geteuid();
		struct passwd	pw = {};
		struct passwd*	result = nullptr;
		char			buffer[bufsize];
		int				success = getpwuid_r( uid, &pw, buffer, bufsize, &result );
		if( success == 0 && result )
		{
			cout << result->pw_name;
		}
		endpwent();
	}
	
	// Print "type here" indicator:
  	cout << "> ";
}


int main(int argc, const char * argv[])
{
	initialize();
	
	string		currLine;
	
	print_prompt();
	
	while( gKeepRunning )
	{
		char	currCh = 0;
		currCh = getc(stdin);
		if( currCh == EOF || currCh == '\0' )
			gKeepRunning = false;
		else if( currCh == '\n' || currCh == '\r' )
		{
			process_one_line( currLine );
			currLine.erase();
			
			if( gKeepRunning )
				print_prompt();
		}
		else
			currLine.append( 1, currCh );
	}
	
    return 0;
}
