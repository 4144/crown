#pragma once

#include "Types.h"
#include "OS.h"


namespace crown
{

enum json_type
{
	JSON_PRIMITIVE 	= 0,	// Number, boolean or null
	JSON_OBJECT 	= 1,	// Object
	JSON_ARRAY 		= 2,	// Array
	JSON_STRING 	= 3		// String
};

enum json_error
{
	JSON_NO_MEMORY	= 0,	// Not enough token provided
	JSON_INV_CHAR	= 1,	// Invalid character inside JSON string
	JSON_INV_PART	= 2,	// JSON string is incompleted
	JSON_NO_INIT	= 3,	// JSON parser not initialized
	JSON_SUCCESS	= 4		// Everything OK!
};

struct JSONToken
{
	json_type	m_type;
	int32_t 	m_start;
	int32_t 	m_end;
	size_t 		m_size;
	int32_t 	m_parent;

	inline void print()
	{
		os::printf("Type:\t%d\n",m_type);
		os::printf("Start:\t%d\n",m_start);
		os::printf("End:\t%d\n",m_end);
		os::printf("Parent:\t%d\n",m_parent);
		os::printf("Size:\t%d\n",m_size);
		os::printf("\n");		
	}

};

class JSONParser
{
public:
	/// Constructor
					JSONParser(size_t size = 1024);
	/// Init JSON parser, must be called for each different JSON string
	void 			init();
	/// Parse JSON data
	json_error 		parse(const char* src);
	/// Parse string in JSON data
	json_error		parse_string(const char* src);
	/// Parse number or boolean in JSON data
	json_error		parse_primitive(const char* src);
	/// Allocate token node
	JSONToken* 		allocate_token();
	/// Fill token and set boundaries
	void			fill_token(JSONToken* token, json_type type, int32_t start, int32_t end);
	/// Get all tokens
	JSONToken*		get_tokens();
	/// Get next token
	int32_t			get_tokens_number();

private:
	/// JSON string offset
	uint32_t 		m_pos;
	/// Next token to allocate				
	int32_t			m_next_token;
	/// Previous token e.g parent or array		
	int32_t			m_prev_token;

	/// JSON tokens list, used as default
	JSONToken		m_tokens_list[1024];
	/// JSON tokens ptr (used only if we need more then 1024 tokens)
	JSONToken* 		m_tokens;
	/// m_tokens default size, default 1024
	size_t			m_size;

	// is JSON parser initilized
	bool 			is_init;		
};

} // namespace crown