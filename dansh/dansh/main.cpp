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
#include <sstream>

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
#include "dansh_command_var.hpp"

#include <readline/readline.h>
#include <readline/history.h>
#include <termios.h>


using namespace std;


bool					gKeepRunning = true;
bool					gRunningScript = false;	// Set to true when running our initialization script.
bool					gIsTerminal = false;		// Any kind of Terminal, smart or dumb.
bool					gIsSmartTerminal = false;	// Smart terminal like Terminal.app, not dumb like Xcode's console.

dansh_statement_ptr	parse_one_statement( const vector<dansh_token> & tokens, vector<dansh_token>::const_iterator & currToken, bool isRoot = false, bool noPipes = false );
bool	include_script( const string& filePath );
void	run_stream( FILE* inFile );


void	initialize()
{
	gIsTerminal = isatty(fileno(stdin));
	const char*	theTerm = getenv("TERM");
	gIsSmartTerminal = gIsTerminal && theTerm && strlen(theTerm) > 0;
	if( gIsSmartTerminal )
	{
		rl_readline_name = (char*)"dansh";
		readline_echoing_p = false;

		rl_initialize();
	}
	
	string	userHomeDir = user_home_dir();
	if( userHomeDir.length() != 0 )
		chdir( userHomeDir.c_str() );
	
	gBuiltInCommands["exit"] = []( dansh_statement_ptr params )
	{
		gKeepRunning = false;
		return dansh_statement_ptr();
	};
	
    gBuiltInCommands["pwd"] = dansh_command_pwd;
    gBuiltInCommands["."] = dansh_command_pwd;
    gBuiltInCommands[".."] = dansh_command_parent_dir;
	
	gBuiltInCommands["cd"] = dansh_command_cd;
	gBuiltInCommands["cd.."] = dansh_command_cd_parent_dir;
	
	
	gBuiltInCommands["~"] = dansh_command_home_dir;
	
	gBuiltInCommands["/"] = []( dansh_statement_ptr params )
	{
		dansh_statement_ptr	currentDir( new dansh_statement );
		currentDir->type = DANSH_STATEMENT_TYPE_STRING;
		currentDir->name = "/";
		return currentDir;
	};
	
	gBuiltInCommands["which"] = dansh_command_which;

    gBuiltInCommands["env.*"] = dansh_command_env;
    gBuiltInCommands["=env.*"] = dansh_command_set_env;
    
    gBuiltInCommands["var.*"] = dansh_command_var;
    gBuiltInCommands["=var.*"] = dansh_command_set_var;
    gBuiltInCommands["$*"] = dansh_command_var;
    gBuiltInCommands["=$*"] = dansh_command_set_var;
	
	gBuiltInCommands["source"] = []( dansh_statement_ptr params )
	{
		dansh_statement_ptr	currentDir( new dansh_statement );
		currentDir->type = DANSH_STATEMENT_TYPE_STRING;
		if( params->params.size() < 1 )
		{
			currentDir->name.append("Expected path of script as parameter to 'source'.");
		}
		else if( !include_script( params->params[0]->name.c_str() ) )
		{
			currentDir->name.append("Can't find script file \"");
			currentDir->name.append(params->params[0]->name);
			currentDir->name.append("\".");
		}
		return currentDir;
	};
	
	gBuiltInCommands["echo"] = dansh_command_echo;
	
	string	runCommandsFile(userHomeDir);
	runCommandsFile.append(1,'/');
	runCommandsFile.append(".danshrc");
	include_script(runCommandsFile);
}


bool	include_script( const string& filePath )
{
	bool	didRun = false;
	bool	oldRunningScript = gRunningScript;
	
	gRunningScript = true;
	FILE*		scriptFile = fopen(filePath.c_str(), "r");
	if( scriptFile )
	{
		run_stream( scriptFile );
		fclose( scriptFile );
		didRun = true;
	}
	gRunningScript = oldRunningScript;
	
	return didRun;
}


dansh_statement_ptr	parse_one_value( const vector<dansh_token> & tokens, vector<dansh_token>::const_iterator & currToken, bool bracketsOptional = false, bool noPipes = false )
{
	if( currToken == tokens.end() )
		return dansh_statement_ptr();
	
	dansh_statement_ptr		currStatement( new dansh_statement );
	if( currToken->type == DANSH_TOKEN_TYPE_STRING )
	{
		currStatement->name = currToken->text;
		currStatement->type = DANSH_STATEMENT_TYPE_STRING;
		currToken++;
		
		return currStatement;
	}
	else if( currToken->type == DANSH_TOKEN_TYPE_NUMBER )
	{
		currStatement->name = currToken->text;
		currStatement->type = DANSH_STATEMENT_TYPE_NUMBER;
		currToken++;
		
		return currStatement;
	}
	else
		return parse_one_statement( tokens, currToken, bracketsOptional, noPipes );
}


bool	parse_parameter_list( const vector<dansh_token> & tokens, vector<dansh_token>::const_iterator & currToken, dansh_statement_ptr targetStatement, bool bracketsOptional )
{
	if( currToken == tokens.end() )
		return true;
	
	bool	hadOpeningBracket = false;
	
	if( currToken->type == DANSH_TOKEN_TYPE_OPENING_BRACKET )
	{
		currToken++;
		hadOpeningBracket = true;
	}
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

		if( currToken->type == DANSH_TOKEN_TYPE_CLOSING_BRACKET && hadOpeningBracket )
		{
			currToken++;
			return true;
		}
		
		bool	hadLabel = false;
		if( currToken->type == DANSH_TOKEN_TYPE_LABEL )
		{
			hadLabel = true;
			dansh_statement_ptr		label( new dansh_statement );
			label->type = DANSH_STATEMENT_TYPE_STRING;
			label->name = currToken->text;
			targetStatement->params.push_back( label );
			
			currToken++;
		}
		
		if( currToken != tokens.end()
			&& (!hadLabel || (currToken->type != DANSH_TOKEN_TYPE_LABEL))
			&& currToken->type != DANSH_TOKEN_TYPE_COMMA && currToken->type != DANSH_TOKEN_TYPE_PIPE && currToken->type != DANSH_TOKEN_TYPE_LESS_THAN && currToken->type != DANSH_TOKEN_TYPE_GREATER_THAN && currToken->type != DANSH_TOKEN_TYPE_CLOSING_BRACKET )
		{
			targetStatement->params.push_back( parse_one_value( tokens, currToken, false, true ) );	// Don't swallow up pipes when parsing parameters, would give wrong results in bracketsOptional case, and if we're without paremeters and used as a param to a bracketsOptional command, bracketsOptional for us is actually false.
			if( (*targetStatement->params.rbegin())->type == DANSH_STATEMENT_INVALID )
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
		if( currToken->type == DANSH_TOKEN_TYPE_CLOSING_BRACKET && hadOpeningBracket )
		{
			currToken++;
			break;
		}
		if( (currToken->type == DANSH_TOKEN_TYPE_PIPE || currToken->type == DANSH_TOKEN_TYPE_LESS_THAN
			|| currToken->type == DANSH_TOKEN_TYPE_GREATER_THAN) && !hadOpeningBracket )
		{
			break;	// We're done, let outside parse piped command.
		}
		else if( currToken->type != DANSH_TOKEN_TYPE_COMMA )
		{
			cerr << "Expected \",\" to separate parameters, or \")\" to end parameter list. Found \"" << currToken->text.c_str() << "\"." << endl;
			return false;
		}
		currToken++;
	}
	
	return true;
}


dansh_statement_ptr	parse_one_statement( const vector<dansh_token> & tokens, vector<dansh_token>::const_iterator & currToken, bool bracketsOptional, bool noPipes )
{
	if( currToken == tokens.end() )
		return dansh_statement_ptr();
	
	dansh_statement_ptr	currStatement( new dansh_statement );
		
	if( currToken->type == DANSH_TOKEN_TYPE_IDENTIFIER || currToken->type == DANSH_TOKEN_TYPE_DOT )
	{
		// First, parse package name of the current command:
		currStatement->name = currToken->text;
		if( currToken->type == DANSH_TOKEN_TYPE_DOT )	// We support package names with a leading dot as a shorthand for current directory.
		{
			currToken++;
			while( currToken != tokens.end() && currToken->type == DANSH_TOKEN_TYPE_DOT )
			{
				currStatement->name.append( currToken->text );	// Append this "." to the "." already in the name.
				currToken++;
			}
			if( currToken == tokens.end() || currToken->type != DANSH_TOKEN_TYPE_IDENTIFIER )	// No full identifier after this? Guess it's the special "." or ".." function.
			{
				currToken--;	// Backtrack, the currToken++ below will skip this token.
			}
			else
				currStatement->name.append( currToken->text );	// Append identifier to the "." already here.
		}
		currToken++;
		
		while( currToken != tokens.end() && currToken->type == DANSH_TOKEN_TYPE_DOT )
		{
			currToken++;
			if( currToken == tokens.end() || (currToken->type != DANSH_TOKEN_TYPE_IDENTIFIER && currToken->type != DANSH_TOKEN_TYPE_DOT) )
			{
				cerr << "Expected identifier after '.' operator." << endl;
				return dansh_statement_ptr();
			}
			currStatement->name.append(1,'.');
			currStatement->name.append( currToken->text );
			currToken++;
		}
		
		if( currToken != tokens.end() && currToken->type == DANSH_TOKEN_TYPE_EQUAL )
		{
			currToken++;
			currStatement->name.insert( 0, "=" );
			currStatement->params.push_back( parse_one_value( tokens, currToken ) );
		}
		// Now, parse parameter list, if any:
		else if( !parse_parameter_list( tokens, currToken,  currStatement, bracketsOptional ) )
			return dansh_statement_ptr();
		currStatement->type = bracketsOptional ? DANSH_STATEMENT_TYPE_COMMAND : DANSH_STATEMENT_TYPE_FUNCTION;
		
		if( currToken != tokens.end() && currToken->type == DANSH_TOKEN_TYPE_PIPE && !noPipes )
		{
			dansh_statement_ptr	pipeStatement( new dansh_statement );
			pipeStatement->type = DANSH_STATEMENT_TYPE_PIPE;
			pipeStatement->params.push_back( currStatement );
			currStatement = pipeStatement;
			while( currToken != tokens.end() && currToken->type == DANSH_TOKEN_TYPE_PIPE )
			{
				currToken++;
				pipeStatement->params.push_back( parse_one_statement( tokens, currToken, bracketsOptional, true ) );
			}
		}
	}
	else if( currToken->type == DANSH_TOKEN_TYPE_OPENING_BRACKET )
	{
		currToken++;
		currStatement->params.push_back( parse_one_value( tokens, currToken ) );	// If we have no name, the first parameter is used as the name.
		if( currToken == tokens.end() || currToken->type != DANSH_TOKEN_TYPE_CLOSING_BRACKET )
		{
			cerr << "Unexpected \"" << currToken->text.c_str() << "\" after function name expression. Expected \")\" here." << endl;
		}
		currToken++;
		
		// Now, parse parameter list, if any:
		if( !parse_parameter_list( tokens, currToken,  currStatement, bracketsOptional ) )
			return dansh_statement_ptr();
		currStatement->type = bracketsOptional ? DANSH_STATEMENT_TYPE_COMMAND : DANSH_STATEMENT_TYPE_FUNCTION;
		
		if( currToken != tokens.end() && currToken->type == DANSH_TOKEN_TYPE_PIPE && !noPipes )
		{
			dansh_statement_ptr	pipeStatement( new dansh_statement );
			pipeStatement->type = DANSH_STATEMENT_TYPE_PIPE;
			pipeStatement->params.push_back( currStatement );
			currStatement = pipeStatement;
			while( currToken != tokens.end() && currToken->type == DANSH_TOKEN_TYPE_PIPE )
			{
				currToken++;
				pipeStatement->params.push_back( parse_one_statement( tokens, currToken, bracketsOptional, true ) );
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
	
	dansh_statement_ptr	currStatement = parse_one_statement( tokens, currToken, true );
    if( currStatement )
    {
//		currStatement->print(cout);
//		cout << endl;
		
        currStatement = currStatement->eval();
        if( currStatement && (currStatement->type == DANSH_STATEMENT_TYPE_STRING || currStatement->name.length() != 0) )
            cout << currStatement->name << endl;
    }
}


std::string	prompt_string()
{
	std::stringstream	outPrompt;
	if( gIsTerminal && !gRunningScript )
	{
		// Print current folder's name:
		char*	currentWorkingDirectory = getcwd( NULL, 0 );
		if( currentWorkingDirectory )
		{
			if( strcmp(currentWorkingDirectory,"/") == 0 )
			{
				outPrompt << "/ ";
			}
			else if( user_home_dir().compare(currentWorkingDirectory) == 0 )
			{
				outPrompt << "~ ";
			}
			else
			{
				char*		lastPathComponent = strrchr( currentWorkingDirectory, '/' );
				if( lastPathComponent )
					outPrompt << (lastPathComponent +1) << " ";
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
				outPrompt << result->pw_name;
			}
			endpwent();
		}
		
		// Print "type here" indicator:
		outPrompt << "> ";
	}
	
	return outPrompt.str();
}


void	run_stream( FILE* inFile )
{
	string		currLine;
	bool		localKeepRunning = true;
	
	while( localKeepRunning && gKeepRunning )
	{
		if( gIsSmartTerminal )
		{
			char*	cmdline = readline( prompt_string().c_str() );
			if( !cmdline )
				localKeepRunning = false;
			else if( strlen(cmdline) > 0 )
			{
				add_history( cmdline );
				process_one_line( cmdline );
				free(cmdline);
				cmdline = nullptr;
			}
		}
		else
		{
			int	currCh = 0;
			if( currLine.size() == 0 )
			{
				std::string		prompt = prompt_string();
				if( prompt.size() != 0 )
					cout << prompt;
			}
			currCh = getc(inFile);
			if( currCh == EOF || currCh == '\0' )
			{
				process_one_line( currLine );
				localKeepRunning = false;
			}
			else if( currCh == '\n' || currCh == '\r' )
			{
				process_one_line( currLine );
				currLine.erase();
			}
			else
				currLine.append( 1, currCh );
		}
	}
}


int	main(int argc, char *argv[])
{
	initialize();
	run_stream(stdin);
	
	return EXIT_SUCCESS;
}


