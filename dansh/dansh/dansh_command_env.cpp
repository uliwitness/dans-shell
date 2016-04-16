//
//  dansh_command_env.cpp
//  dansh
//
//  Created by Uli Kusterer on 16/04/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#include "dansh_command_env.hpp"
#include <iostream>


using namespace std;


dansh_built_in_lambda	dansh_command_env = []( dansh_statement params )
{
    dansh_statement		currentDir;
    
    currentDir.type = DANSH_STATEMENT_TYPE_STRING;
    string		varName = params.name.substr(4,string::npos);
    const char*	envStr = getenv( varName.c_str() );
    if( envStr )
        currentDir.name = envStr;
    return currentDir;
};


dansh_built_in_lambda	dansh_command_set_env = []( dansh_statement params )
{
    if( params.params.size() < 1 || (params.params[0].type != DANSH_STATEMENT_TYPE_STRING && params.params[0].type != DANSH_STATEMENT_TYPE_NUMBER) )
    {
        cerr << "Expected expression to the right of '=' symbol." << endl;
    }
    else
    {
        string		varName = params.name.substr(5,string::npos);
        setenv( varName.c_str(), params.params[0].name.c_str(), 1 );
    }
    return dansh_statement();
};
