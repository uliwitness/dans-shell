//
//  dansh_command_var.cpp
//  dansh
//
//  Created by Uli Kusterer on 16/04/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#include "dansh_command_var.hpp"
#include <iostream>
#include <map>


using namespace std;


map<string,string>      gVariables;


dansh_built_in_lambda	dansh_command_var = []( dansh_statement_ptr params )
{
    dansh_statement_ptr		currentDir( new dansh_statement );
    
    currentDir->type = DANSH_STATEMENT_TYPE_STRING;
    string		varName = (params->name[0] == '$') ? params->name.substr(1,string::npos) : params->name.substr(4,string::npos);
    currentDir->name = gVariables[varName];
    return currentDir;
};


dansh_built_in_lambda	dansh_command_set_var = []( dansh_statement_ptr params )
{
    if( params->params.size() < 1 || (params->params[0]->type != DANSH_STATEMENT_TYPE_STRING && params->params[0]->type != DANSH_STATEMENT_TYPE_NUMBER) )
    {
        cerr << "Expected expression to the right of '=' symbol." << endl;
    }
    else
    {
        string		varName = (params->name[1] == '$') ? params->name.substr(2,string::npos) : params->name.substr(5,string::npos);
        gVariables[varName] = params->params[0]->name;
    }
    return dansh_statement_ptr();
};
