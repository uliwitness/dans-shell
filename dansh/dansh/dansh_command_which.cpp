//
//  dansh_command_which.cpp
//  dansh
//
//  Created by Uli Kusterer on 16/04/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#include "dansh_command_which.hpp"
#include <iostream>


using namespace std;


dansh_built_in_lambda	dansh_command_which = []( dansh_statement_ptr params )
{
    dansh_statement_ptr	currentDir( new dansh_statement );
    
    if( params->params.size() < 1 )
        cerr << "Expected command name as first parameter of 'which' command." << endl;
    else
    {
        currentDir->type = DANSH_STATEMENT_TYPE_STRING;
        currentDir->name = path_for_command( params->params[0]->name );
    }
    return currentDir;
};
