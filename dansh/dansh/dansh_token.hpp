//
//  dansh_token.hpp
//  dansh
//
//  Created by Uli Kusterer on 16/04/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#ifndef dansh_token_hpp
#define dansh_token_hpp


#include <string>
#include <vector>


typedef enum
{
	DANSH_TOKEN_TYPE_WHITESPACE,
	DANSH_TOKEN_TYPE_IDENTIFIER,
	DANSH_TOKEN_TYPE_OPENING_BRACKET,
	DANSH_TOKEN_TYPE_CLOSING_BRACKET,
	DANSH_TOKEN_TYPE_COMMA,
	DANSH_TOKEN_TYPE_PIPE,
	DANSH_TOKEN_TYPE_LESS_THAN,
	DANSH_TOKEN_TYPE_GREATER_THAN,
	DANSH_TOKEN_TYPE_DOT,
	DANSH_TOKEN_TYPE_EQUAL,
	DANSH_TOKEN_TYPE_STRING,
	DANSH_TOKEN_TYPE_NUMBER,
	DANSH_TOKEN_TYPE_LABEL
} dansh_token_type;


class dansh_token
{
public:
	dansh_token() : type(DANSH_TOKEN_TYPE_WHITESPACE) {}
	
	dansh_token_type	type;
	std::string			text;
};


void	finish_token( const std::string & currTokenString, dansh_token_type currType, std::vector<dansh_token> & outTokens );
void	tokenize( const std::string & currLine, std::vector<dansh_token> & outTokens );


#endif /* dansh_token_hpp */
