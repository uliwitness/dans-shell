//
//  dansh_statement.hpp
//  dansh
//
//  Created by Uli Kusterer on 16/04/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#ifndef dansh_statement_hpp
#define dansh_statement_hpp

#include <string>
#include <ostream>
#include <vector>
#include <map>
#include <functional>
#include <memory>


class dansh_statement;


typedef std::shared_ptr<dansh_statement> dansh_statement_ptr;


typedef enum
{
	DANSH_STATEMENT_INVALID,		// Error indicator.
	DANSH_STATEMENT_TYPE_COMMAND,	// Identical to function, but we don't capture its in/output.
	DANSH_STATEMENT_TYPE_FUNCTION,
	DANSH_STATEMENT_TYPE_STRING,
	DANSH_STATEMENT_TYPE_NUMBER,
	DANSH_STATEMENT_TYPE_PIPE
} dansh_statement_type;


class dansh_statement : public std::enable_shared_from_this<dansh_statement>
{
public:
    dansh_statement() : type(DANSH_STATEMENT_INVALID) {}
	
	dansh_statement_ptr		eval();
	
	void	print( std::ostream& outStream ) const;
	
	std::string                         name;	// If name length is 0, the first parameter's value is used as the name.
	std::vector<dansh_statement_ptr>	params;
	dansh_statement_type                type;
};


typedef std::function<dansh_statement_ptr(dansh_statement_ptr stmt)> dansh_built_in_lambda;

extern std::map<std::string,dansh_built_in_lambda>	gBuiltInCommands;

extern std::string	path_for_command( const std::string & inCommandName );
dansh_statement_ptr	launch_executable( const std::string& name, std::vector<dansh_statement_ptr> params );

#endif /* dansh_statement_hpp */
