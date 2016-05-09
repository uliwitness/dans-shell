//
//  dansh_statement.cpp
//  dansh
//
//  Created by Uli Kusterer on 16/04/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#include "dansh_statement.hpp"
#include <map>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>


using namespace std;


map<string,dansh_built_in_lambda>	gBuiltInCommands;



// This is from Darwin's implementation of "which":
bool	file_is_accessible( const string& inPath )
{
	struct stat		fileInfo = {};

	return( access(inPath.c_str(), X_OK) == 0
			&& stat(inPath.c_str(), &fileInfo) == 0
			&& S_ISREG(fileInfo.st_mode)
			&& (getuid() != 0 || (fileInfo.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) != 0) );
}


string	path_for_command( const string & inCommandName )
{
	string	finalPath;
	if( inCommandName.find( "./" ) == 0 )	// Starts with "./"? User explicitly requested current directory.
	{
		char*	currentWorkingDirectory = getcwd( NULL, 0 );
		if( currentWorkingDirectory )
		{
			finalPath = currentWorkingDirectory;
			finalPath.append( inCommandName.substr(1) );
			free(currentWorkingDirectory);
		}
	}
	else if( inCommandName.find("/") == 0 )	// Starts with "/"? User explicitly requested this tool.
	{
		finalPath = inCommandName;
	}
	else
	{
		char*	thePath = strdup(getenv("PATH"));
		char*	currPath = thePath;
		char*	token = NULL;
		while( (token = strsep( &currPath, ":" )) != nullptr )
		{
			finalPath = token;
			finalPath.append( 1, '/' );
			finalPath.append( inCommandName );
			if( file_is_accessible( finalPath ) )
				break;
		}
		free( thePath );
		
		if( token == nullptr )
			finalPath.erase();
	}
	
	return finalPath;
}


dansh_statement_ptr	launch_executable( const string& name, vector<dansh_statement_ptr> params )
{
	if( name.length() == 0 && params.size() == 0 )
		return dansh_statement_ptr();
	
	int pipeOutputInputFDs[2];
	if( pipe(pipeOutputInputFDs) == -1 )
	{
		perror("dansh");
		return dansh_statement_ptr();
	}
	
	int		status = 0;
	pid_t	childPID = fork();
	if( childPID == 0 )	// Child process?
	{
		vector<char*>	args;
		if( (name.length() != 0) )
			args.push_back( strdup(name.c_str()) );
		for( const dansh_statement_ptr currParam : params )
		{
			args.push_back( strdup(currParam->name.c_str()) );
		}
		args.push_back(nullptr);
		
		// Replace our stdout with the pipe's input:
		while( (dup2(pipeOutputInputFDs[1], STDOUT_FILENO) == -1) && (errno == EINTR) )
		{
			// We repeat this until we're no longer interrupted by some signal.
		}
		close( pipeOutputInputFDs[1] );	// Close original, we just made a copy.
		close( pipeOutputInputFDs[0] );
		
		if( execvp(args[0], args.data()) == -1 )	// Replace ourselves with the executable to launch:
		{
			perror("dansh");
		}
		exit(EXIT_FAILURE);	// Give back control to the shell that launched us, don't run a 2nd copy of that shell.
	}
	else if( childPID < 0 )	// Error forking?
	{
		perror( "dansh" );
		return dansh_statement_ptr();
	}
	else	// Parent process?
	{
		close( pipeOutputInputFDs[1] );	// We don't need the entrance, only the child process.
		
		char				buffer[4096];
		dansh_statement_ptr	actualOutput( new dansh_statement );
		actualOutput->type = DANSH_STATEMENT_TYPE_STRING;
		//actualOutput.name.append(1,'[');
		while( true )
		{
			ssize_t count = read( pipeOutputInputFDs[0], buffer, sizeof(buffer) );
			if( count == -1 )
			{
				if (errno == EINTR)
				{
					continue;
				}
				else
				{
					perror("dansh");
					return dansh_statement_ptr();
				}
			}
			else if( count == 0 )
			{
				break;
			}
			else
			{
				actualOutput->name.append( buffer, count );
			}
		}
		close( pipeOutputInputFDs[0] );
		//actualOutput.name.append(1,']');

		do // Wait for the child process to quit:
		{
			waitpid( childPID, &status, WUNTRACED );
		}
		while( !WIFEXITED(status) && !WIFSIGNALED(status) );
		
		return actualOutput;
	}

	return dansh_statement_ptr();
}


dansh_statement_ptr		dansh_statement::eval()
{
	if( type == DANSH_STATEMENT_TYPE_FUNCTION )
	{
		dansh_statement_ptr	evaluated( new dansh_statement );
		bool				firstParamIsName = (name.length() == 0);	// No name? Use first param as name!
		evaluated->name = name;
		evaluated->type = type;
		bool				isFirst = true;
		for( const dansh_statement_ptr currParam : params )
		{
			dansh_statement_ptr	evalName = currParam->eval();
			if( !evalName )
				return nullptr;
			if( isFirst )
			{
				isFirst = false;
				if( firstParamIsName )
					evaluated->name = evalName->name;
				else
					evaluated->params.push_back( evalName );
			}
			else
				evaluated->params.push_back( evalName );
		}
		
		map<string,dansh_built_in_lambda>::const_iterator	foundCommand = gBuiltInCommands.find(evaluated->name);
		if( foundCommand == gBuiltInCommands.end() )
		{
			string::size_type pos = evaluated->name.find('.');
			if( pos != string::npos )
			{
				string	fuzzyName = evaluated->name.substr(0,pos);
				fuzzyName.append(".*");
				foundCommand = gBuiltInCommands.find(fuzzyName);
			}
		}
        if( foundCommand == gBuiltInCommands.end() )
        {
            bool    startsWithSymbol = (evaluated->name.length() >= 2 && evaluated->name[0] == '=' && !isalpha(evaluated->name[1]))
                                        || (evaluated->name.length() >= 1 && evaluated->name[0] != '=' && !isalpha(evaluated->name[0]));
            if( startsWithSymbol )
            {
                string	fuzzyName = evaluated->name.substr(0,(evaluated->name[0] == '=') ? 2 : 1);
                fuzzyName.append("*");
                foundCommand = gBuiltInCommands.find(fuzzyName);
            }
        }
		if( foundCommand == gBuiltInCommands.end() )
		{
			string	commandPath = path_for_command( evaluated->name );
			
			dansh_statement_ptr	commandOutput = launch_executable( commandPath, evaluated->params );
			if( commandOutput && commandOutput->type == DANSH_STATEMENT_TYPE_STRING )
			{
				return commandOutput;
			}
            else if( evaluated->name.find('=') == 0 )    // Starts with = sign? Assignment to something nonsensical.
            {
                cerr << "Don't know how to assign to \"" << evaluated->name << "\"." << endl;
                return dansh_statement_ptr();
            }
			else
			{
				cerr << "Unknown command \"" << evaluated->name << "\"." << endl;
				return dansh_statement_ptr();
			}
		}
		return foundCommand->second( evaluated );
	}
	else
		return shared_from_this();
}


void	dansh_statement::print( ostream& outStream ) const
{
	if( type == DANSH_STATEMENT_TYPE_FUNCTION )
	{
		outStream << name << "( ";
		bool	isFirst = true;
		for( const dansh_statement_ptr currParam : params )
		{
			if( isFirst )
				isFirst = false;
			else
				outStream << ", ";
			currParam->print( outStream );
		}
		outStream << " )";
	}
	else
	{
		outStream << name;
	}
}
