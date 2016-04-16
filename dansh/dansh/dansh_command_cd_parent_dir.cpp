//
//  dansh_command_cd_parent_dir.cpp
//  dansh
//
//  Created by Uli Kusterer on 16/04/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#include "dansh_command_cd_parent_dir.hpp"
#include "dansh_command_parent_dir.hpp"
#include <iostream>
#include <unistd.h>


using namespace std;


dansh_built_in_lambda	dansh_command_cd_parent_dir = []( dansh_statement_ptr params )
{
    if( params->params.size() > 0 )
        cerr << "Too many parameters to 'cd' command." << endl;
    else
        chdir( parent_dir().c_str() );
    return dansh_statement_ptr();
};
