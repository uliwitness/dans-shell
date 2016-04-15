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


using namespace std;


bool		gKeepRunning = true;


void	initialize()
{
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
			chdir( result->pw_dir );
		}
		endpwent();
	}
}


void	process_one_line( const string & currLine )
{
	if( currLine.compare("exit") == 0 || currLine.compare("exit()") == 0 )
		gKeepRunning = false;
	else if( currLine.compare("pwd()") == 0 )
	{
		char*	currentWorkingDirectory = getcwd( NULL, 0 );
		if( currentWorkingDirectory )
		{
			cout << currentWorkingDirectory << endl;
			free( currentWorkingDirectory );
		}
	}
	else
		cout << "Unknown command \"" << currLine.c_str() << "\"" << endl;
}


void	print_prompt()
{
	// Print current folder's name:
	char*	currentWorkingDirectory = getcwd( NULL, 0 );
	if( currentWorkingDirectory )
	{
		char*		lastPathComponent = strrchr( currentWorkingDirectory, '/' );
		if( lastPathComponent )
			cout << (lastPathComponent +1) << " ";
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
			cout << result->pw_name << " ";
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
