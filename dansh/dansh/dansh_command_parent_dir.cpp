//
//  dansh_command_parent_dir.cpp
//  dansh
//
//  Created by Uli Kusterer on 16/04/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#include "dansh_command_parent_dir.hpp"
#include <unistd.h>


using namespace std;


string	parent_dir()
{
    string	outDir;
    char*	currentWorkingDirectory = getcwd( NULL, 0 );
    if( currentWorkingDirectory )
    {
        char*		lastPathComponent = strrchr( currentWorkingDirectory, '/' );
        if( lastPathComponent && lastPathComponent != currentWorkingDirectory )	// We found a '/' and the whole path is not "/"?
            *lastPathComponent = '\0';
        if( lastPathComponent == currentWorkingDirectory )
            currentWorkingDirectory[1] = '\0';	// Truncate, but leave the "/" there.
        outDir = currentWorkingDirectory;
        free( currentWorkingDirectory );
    }
    return outDir;
}


dansh_built_in_lambda	dansh_command_parent_dir = []( dansh_statement_ptr params )
{
    dansh_statement_ptr	currentDir( new dansh_statement );
    currentDir->type = DANSH_STATEMENT_TYPE_STRING;
    currentDir->name = parent_dir();
    return currentDir;
};
