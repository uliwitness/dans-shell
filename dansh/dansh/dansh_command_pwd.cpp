//
//  dansh_command_pwd.cpp
//  dansh
//
//  Created by Uli Kusterer on 16/04/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#include "dansh_command_pwd.hpp"
#include <unistd.h>


dansh_built_in_lambda	dansh_command_pwd = []( dansh_statement params )
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
