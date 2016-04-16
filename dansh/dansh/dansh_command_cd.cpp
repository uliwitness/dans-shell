//
//  dansh_command_cd.cpp
//  dansh
//
//  Created by Uli Kusterer on 16/04/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#include "dansh_command_cd.hpp"
#include <iostream>
#include <unistd.h>


using namespace std;


dansh_built_in_lambda	dansh_command_cd = []( dansh_statement params )
{
    if( params.params.size() < 1 )
        cerr << "Expected directory path as first parameter of 'cd' command." << endl;
    else
        chdir( params.params[0].name.c_str() );
    return dansh_statement();
};
