//
//  dansh_command_home_dir.cpp
//  dansh
//
//  Created by Uli Kusterer on 16/04/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#include "dansh_command_home_dir.hpp"
#include <iostream>
#include <unistd.h>
#include <pwd.h>


using namespace std;


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


dansh_built_in_lambda	dansh_command_home_dir = []( dansh_statement_ptr params )
{
    dansh_statement_ptr		currentDir( new dansh_statement );
    currentDir->type = DANSH_STATEMENT_TYPE_STRING;
    currentDir->name = user_home_dir();
    return currentDir;
};
