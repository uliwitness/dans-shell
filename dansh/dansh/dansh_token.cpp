//
//  dansh_token.cpp
//  dansh
//
//  Created by Uli Kusterer on 16/04/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#include "dansh_token.hpp"
#include <cassert>


using namespace std;


void	finish_token( const string & currTokenString, dansh_token_type currType, vector<dansh_token> & outTokens )
{
	if( currTokenString.length() == 0 && currType != DANSH_TOKEN_TYPE_STRING )
		return;
	
	if( currType == DANSH_TOKEN_TYPE_IDENTIFIER && currTokenString.find("-") == 0 )
		currType = DANSH_TOKEN_TYPE_LABEL;
	
	dansh_token		newToken;
	newToken.type = currType;
	newToken.text = currTokenString;
	
	outTokens.push_back( newToken );
}


void	tokenize( const string & currLine, vector<dansh_token> & outTokens )
{
	dansh_token_type		currType = DANSH_TOKEN_TYPE_WHITESPACE;
	string::difference_type	lineLength = currLine.length();
	string					currTokenString;
	bool					inEscapeSequence = false;
	
	for( string::difference_type x = 0; x < lineLength; x++ )
	{
		char	currChar = currLine[x];
		switch( currType )
		{
			case DANSH_TOKEN_TYPE_WHITESPACE:
				switch( currChar )
				{
					case '"':
						currType = DANSH_TOKEN_TYPE_STRING;
						break;
					
					case '0'...'9':
						currType = DANSH_TOKEN_TYPE_NUMBER;
						currTokenString.append(1, currChar);
						break;
						
					case ' ':
					case '\t':
					case '\r':
					case '\n':
						// Nothing to do, still in whitespace.
						break;
					
					case '(':
						finish_token( "(", DANSH_TOKEN_TYPE_OPENING_BRACKET, outTokens );
						break;

					case ')':
						finish_token( ")", DANSH_TOKEN_TYPE_CLOSING_BRACKET, outTokens );
						break;

					case ',':
						finish_token( ",", DANSH_TOKEN_TYPE_COMMA, outTokens );
						break;

					case '.':
						finish_token( ".", DANSH_TOKEN_TYPE_DOT, outTokens );
						break;
					
					case '=':
						finish_token( "=", DANSH_TOKEN_TYPE_EQUAL, outTokens );
						break;
						

					case '|':
						finish_token( "|", DANSH_TOKEN_TYPE_PIPE, outTokens );
						break;

					case '<':
						finish_token( "<", DANSH_TOKEN_TYPE_LESS_THAN, outTokens );
						break;

					case '>':
						finish_token( ">", DANSH_TOKEN_TYPE_GREATER_THAN, outTokens );
						break;

					case '#':
						return;	// We parse one line, comment goes to end of line. we're done.
						break;
					
					default:
						currType = DANSH_TOKEN_TYPE_IDENTIFIER;
						currTokenString.append(1, currChar);
				}
				break;
			
			case DANSH_TOKEN_TYPE_IDENTIFIER:
				switch( currChar )
				{
					case '"':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_STRING;
						break;
					
					case '0'...'9':
						currTokenString.append(1, currChar);
						break;
						
					case ' ':
					case '\t':
					case '\r':
					case '\n':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_WHITESPACE;
						break;
					
					case '(':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_WHITESPACE;
						finish_token( "(", DANSH_TOKEN_TYPE_OPENING_BRACKET, outTokens );
						break;

					case ')':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_WHITESPACE;
						finish_token( ")", DANSH_TOKEN_TYPE_CLOSING_BRACKET, outTokens );
						break;

					case ',':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_WHITESPACE;
						finish_token( ",", DANSH_TOKEN_TYPE_COMMA, outTokens );
						break;

					case '.':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_WHITESPACE;
						finish_token( ".", DANSH_TOKEN_TYPE_DOT, outTokens );
						break;

					case '=':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_WHITESPACE;
						finish_token( "=", DANSH_TOKEN_TYPE_EQUAL, outTokens );
						break;


					case '|':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_WHITESPACE;
						finish_token( "|", DANSH_TOKEN_TYPE_PIPE, outTokens );
						break;

					case '<':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_WHITESPACE;
						finish_token( "<", DANSH_TOKEN_TYPE_LESS_THAN, outTokens );
						break;

					case '>':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_WHITESPACE;
						finish_token( ">", DANSH_TOKEN_TYPE_GREATER_THAN, outTokens );
						break;
						
					case '#':
						finish_token( currTokenString, currType, outTokens );
						return;	// We parse one line, comment goes to end of line. we're done.
						break;
					
					default:
						currTokenString.append(1, currChar);
				}
				break;
			
			case DANSH_TOKEN_TYPE_STRING:
				switch( currChar )
				{
					case '"':
						if( inEscapeSequence )
						{
							currTokenString.append(1, currChar);
							inEscapeSequence = false;
						}
						else
						{
							finish_token( currTokenString, currType, outTokens );
							currTokenString.erase();
							currType = DANSH_TOKEN_TYPE_WHITESPACE;
						}
						break;
					
					case 'n':
						if( inEscapeSequence )
						{
							currChar = '\n';
							inEscapeSequence = false;
						}
						currTokenString.append(1, currChar);
						break;

					case 'r':
						if( inEscapeSequence )
						{
							currChar = '\r';
							inEscapeSequence = false;
						}
						currTokenString.append(1, currChar);
						break;
					
					case 't':
						if( inEscapeSequence )
						{
							currChar = '\t';
							inEscapeSequence = false;
						}
						currTokenString.append(1, currChar);
						break;
					
					case '\\':
						if( inEscapeSequence )
						{
							currTokenString.append(1, currChar);
							inEscapeSequence = false;
						}
						else
						{
							inEscapeSequence = true;
						}
						break;
					
					default:
						inEscapeSequence = false;
						currTokenString.append(1, currChar);
				}
				break;
				
			case DANSH_TOKEN_TYPE_NUMBER:
				switch( currChar )
				{
					case '"':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_STRING;
						break;
					
					case '0'...'9':
					case '.':
						currTokenString.append(1, currChar);
						break;
						
					case ' ':
					case '\t':
					case '\r':
					case '\n':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_WHITESPACE;
						break;
					
					case '(':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_WHITESPACE;
						finish_token( "(", DANSH_TOKEN_TYPE_OPENING_BRACKET, outTokens );
						break;

					case ')':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_WHITESPACE;
						finish_token( ")", DANSH_TOKEN_TYPE_CLOSING_BRACKET, outTokens );
						break;

					case ',':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_WHITESPACE;
						finish_token( ",", DANSH_TOKEN_TYPE_COMMA, outTokens );
						break;

					case '=':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_WHITESPACE;
						finish_token( "=", DANSH_TOKEN_TYPE_EQUAL, outTokens );
						break;

					case '|':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_WHITESPACE;
						finish_token( "|", DANSH_TOKEN_TYPE_PIPE, outTokens );
						break;

					case '<':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_WHITESPACE;
						finish_token( "<", DANSH_TOKEN_TYPE_LESS_THAN, outTokens );
						break;

					case '>':
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_WHITESPACE;
						finish_token( ">", DANSH_TOKEN_TYPE_GREATER_THAN, outTokens );
						break;

					case '#':
						finish_token( currTokenString, currType, outTokens );
						return;	// We parse one line, comment goes to end of line. we're done.
						break;
					
					default:
						finish_token( currTokenString, currType, outTokens );
						currTokenString.erase();
						currType = DANSH_TOKEN_TYPE_IDENTIFIER;
						break;
				}
				break;
			
			case DANSH_TOKEN_TYPE_LABEL:
				assert(false);	// Should never happen, labels are a variant of identifiers only generated by finish_token when its text starts with a dash.
				break;
			
			case DANSH_TOKEN_TYPE_OPENING_BRACKET:
			case DANSH_TOKEN_TYPE_CLOSING_BRACKET:
			case DANSH_TOKEN_TYPE_COMMA:
			case DANSH_TOKEN_TYPE_DOT:
			case DANSH_TOKEN_TYPE_EQUAL:
			case DANSH_TOKEN_TYPE_PIPE:
			case DANSH_TOKEN_TYPE_LESS_THAN:
			case DANSH_TOKEN_TYPE_GREATER_THAN:
				assert(false);	// Should never happen, operators are single-character tokens that immediately return to whitespace.
				break;
		}
	}
	
	finish_token( currTokenString, currType, outTokens );	// Close last token, if we haven't yet.
}
