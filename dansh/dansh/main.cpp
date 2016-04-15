//
//  main.cpp
//  dansh
//
//  Created by Uli Kusterer on 15/04/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#include <iostream>
#include <string>

using namespace std;


bool		gKeepRunning = true;


void	process_one_line( const string & currLine )
{
	if( currLine.compare("exit") == 0 )
		gKeepRunning = false;
	else
		cout << "Unknown command \"" << currLine.c_str() << "\"" << endl;
}


void	print_prompt()
{
	cout << "> ";
}


int main(int argc, const char * argv[])
{
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
