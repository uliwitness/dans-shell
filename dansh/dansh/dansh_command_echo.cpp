//
//  dansh_command_echo.cpp
//  dansh
//
//  Created by Uli Kusterer on 16/04/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#include "dansh_command_echo.hpp"
#include <iostream>


using namespace std;


dansh_built_in_lambda	dansh_command_echo = []( dansh_statement_ptr params )
{
	bool	addLineBreak = true;
    for( const dansh_statement_ptr currParam : params->params )
    {
		if( currParam->name.compare("-n") == 0 )
		{
			addLineBreak = false;
			continue;
		}
		
        cout << currParam->name;
    }
	if( addLineBreak )
		cout << endl;
    return dansh_statement_ptr();
};
