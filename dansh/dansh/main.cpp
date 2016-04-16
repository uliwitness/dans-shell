//
//  main.cpp
//  dansh
//
//  Created by Uli Kusterer on 15/04/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <cassert>
#include <map>
#include <unistd.h>
#include <pwd.h>

#include "dansh_statement.hpp"
#include "dansh_token.hpp"
#include "dansh_command_pwd.hpp"
#include "dansh_command_cd.hpp"
#include "dansh_command_home_dir.hpp"
#include "dansh_command_parent_dir.hpp"
#include "dansh_command_cd_parent_dir.hpp"
#include "dansh_command_which.hpp"
#include "dansh_command_env.hpp"
#include "dansh_command_echo.hpp"


using namespace std;


bool		gKeepRunning = true;



dansh_statement	parse_one_statement( const vector<dansh_token> & tokens, vector<dansh_token>::const_iterator & currToken, bool isRoot = false );


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
	
    gBuiltInCommands["pwd"] = dansh_command_pwd;
    gBuiltInCommands["."] = dansh_command_pwd;
    gBuiltInCommands[".."] = dansh_command_parent_dir;
	
	gBuiltInCommands["cd"] = dansh_command_cd;
	gBuiltInCommands["cd.."] = dansh_command_cd_parent_dir;
	
	
	gBuiltInCommands["~"] = dansh_command_home_dir;
	
	gBuiltInCommands["/"] = []( dansh_statement params )
	{
		dansh_statement		currentDir;
		currentDir.type = DANSH_STATEMENT_TYPE_STRING;
		currentDir.name = "/";
		return currentDir;
	};
	
	gBuiltInCommands["which"] = dansh_command_which;

	gBuiltInCommands["env.*"] = dansh_command_env;
	
	gBuiltInCommands["=env.*"] = dansh_command_set_env;
	
	gBuiltInCommands["echo"] = dansh_command_echo;
}


dansh_statement	parse_one_value( const vector<dansh_token> & tokens, vector<dansh_token>::const_iterator & currToken, bool bracketsOptional = false )
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
		return parse_one_statement( tokens, currToken, bracketsOptional );
}


bool	parse_parameter_list( const vector<dansh_token> & tokens, vector<dansh_token>::const_iterator & currToken, dansh_statement &targetStatement, bool bracketsOptional )
{
	if( currToken == tokens.end() )
		return true;
	
	if( currToken->type == DANSH_TOKEN_TYPE_OPENING_BRACKET )
		currToken++;
	else if( !bracketsOptional )
		return true;

	while( true )
	{
		if( currToken == tokens.end() )
		{
			if( !bracketsOptional )
			{
				cerr << "Missing \")\" to close parameter list." << endl;
				return false;
			}
			else
				return true;
		}

		if( currToken->type == DANSH_TOKEN_TYPE_CLOSING_BRACKET )
		{
			currToken++;
			return true;
		}
		
		bool	hadLabel = false;
		if( currToken->type == DANSH_TOKEN_TYPE_LABEL )
		{
			hadLabel = true;
			dansh_statement		label;
			label.type = DANSH_STATEMENT_TYPE_STRING;
			label.name = currToken->text;
			targetStatement.params.push_back( label );
			
			currToken++;
		}
		
		if( currToken != tokens.end()
			&& (!hadLabel || (currToken->type != DANSH_TOKEN_TYPE_LABEL && currToken->type != DANSH_TOKEN_TYPE_COMMA && currToken->type != DANSH_TOKEN_TYPE_CLOSING_BRACKET)) )
		{
			targetStatement.params.push_back( parse_one_value( tokens, currToken ) );
			if( targetStatement.params.rbegin()->type == DANSH_STATEMENT_INVALID )
				return false;
		}
		
		if( currToken == tokens.end() )
		{
			if( !bracketsOptional )
			{
				cerr << "Missing \")\" to close parameter list." << endl;
				return false;
			}
			else
				return true;
		}
		if( currToken->type == DANSH_TOKEN_TYPE_CLOSING_BRACKET )
		{
			currToken++;
			break;
		}
		if( currToken->type != DANSH_TOKEN_TYPE_COMMA )
		{
			cerr << "Expected \",\" to separate parameters, or \")\" to end parameter list. Found \"" << currToken->text.c_str() << "\"." << endl;
			return false;
		}
		currToken++;
	}
	
	return true;
}


dansh_statement	parse_one_statement( const vector<dansh_token> & tokens, vector<dansh_token>::const_iterator & currToken, bool bracketsOptional )
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
			while( currToken != tokens.end() && currToken->type == DANSH_TOKEN_TYPE_DOT )
			{
				currStatement.name.append( currToken->text );	// Append this "." to the "." already in the name.
				currToken++;
			}
			if( currToken == tokens.end() || currToken->type != DANSH_TOKEN_TYPE_IDENTIFIER )	// No full identifier after this? Guess it's the special "." or ".." function.
			{
				currToken--;	// Backtrack, the currToken++ below will skip this token.
			}
			else
				currStatement.name.append( currToken->text );	// Append identifier to the "." already here.
		}
		currToken++;
		
		while( currToken != tokens.end() && currToken->type == DANSH_TOKEN_TYPE_DOT )
		{
			currToken++;
			if( currToken == tokens.end() || (currToken->type != DANSH_TOKEN_TYPE_IDENTIFIER && currToken->type != DANSH_TOKEN_TYPE_DOT) )
			{
				cerr << "Expected identifier after '.' operator." << endl;
				return dansh_statement();
			}
			currStatement.name.append(1,'.');
			currStatement.name.append( currToken->text );
			currToken++;
		}
		
		if( currToken != tokens.end() && currToken->type == DANSH_TOKEN_TYPE_EQUAL )
		{
			currToken++;
			currStatement.name.insert( 0, "=" );
			currStatement.params.push_back( parse_one_value( tokens, currToken ) );
		}
		// Now, parse parameter list, if any:
		else if( !parse_parameter_list( tokens, currToken,  currStatement, bracketsOptional ) )
			return dansh_statement();
		currStatement.type = DANSH_STATEMENT_TYPE_FUNCTION;
	}
	else if( currToken->type == DANSH_TOKEN_TYPE_OPENING_BRACKET )
	{
		currToken++;
		currStatement.params.push_back( parse_one_value( tokens, currToken ) );	// If we have no name, the first parameter is used as the name.
		if( currToken == tokens.end() || currToken->type != DANSH_TOKEN_TYPE_CLOSING_BRACKET )
		{
			cerr << "Unexpected \"" << currToken->text.c_str() << "\" after function name expression. Expected \")\" here." << endl;
		}
		currToken++;
		
		// Now, parse parameter list, if any:
		if( !parse_parameter_list( tokens, currToken,  currStatement, bracketsOptional ) )
			return dansh_statement();
		currStatement.type = DANSH_STATEMENT_TYPE_FUNCTION;
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
	
	dansh_statement	currStatement = parse_one_statement( tokens, currToken, true );
//	currStatement.print(cout);
//	cout << endl;
	
	currStatement = currStatement.eval();
	if( currStatement.type == DANSH_STATEMENT_TYPE_STRING || currStatement.name.length() != 0 )
		cout << currStatement.name << endl;
}


void	print_prompt()
{
	if( isatty(fileno(stdin)) )
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
		{
			process_one_line( currLine );
			gKeepRunning = false;
		}
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
	
    return EXIT_SUCCESS;
}
