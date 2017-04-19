
#include"global.h"

/* see those extern definition details on main.cpp */
extern char *src;
extern int token;
extern int line;
extern int *current_id;
extern int *symbols;
extern char *data;                 
extern int token_val;

//lexical analysing
std::string get_token()
{
	char *last_pos;
	int hash;
	std::string tk_str;

	while (token = *src)
	{
		tk_str = *src;
		++src;
		if (token == '\n')
		{
			++line;
		}
		else if (token == '#') {
			while (*src != 0 && *src != '\n') {
				src++;
			}
		}
		else if ((token >= 'a'&&token <= 'z') || (token >= 'A'&&token <= 'Z') || (token == '_')) {

			// handle identifier
			last_pos = src - 1;
			hash = token;

			while ((*src >= 'a'&&*src <= 'z') || (*src >= 'A'&&*src <= 'Z') || (*src >= '0'&&*src <= '9') || (*src == '_')) {
				hash = hash * 147 + *src;
				tk_str += *src;
				src++;
			}

			//look for existing identifier at symbol table
			current_id = symbols;
			while (current_id[Token]) {
				if (current_id[Hash] == hash && !memcmp((char *)current_id[Name], last_pos, src - last_pos)) {
					// if found, return
					token = current_id[Token];
					return tk_str;
				}
				current_id = current_id + IdSize;
			}
			// store new id
			current_id[Name] = (int)last_pos;
			current_id[Hash] = hash;
			token = current_id[Token] = Id;
			return tk_str;
		}
		else if (token >= '0'&&token <= '9') {
			//handle a number
			token_val = token - '0';
			while (*src >= '0'&&*src <= '9')
			{
				tk_str += *src;
				token_val = token_val * 10 + *src++ - '0';
			}
			token = Num;
			return tk_str;
		}
		else if (token == '"' || token == '\'') {
			//handle string literal
			last_pos = data;
			while (*src != 0 && *src != token) {
				tk_str += *src;
				token_val = *src++;
				if (token_val == '\\') {
					tk_str += *src;
					token_val = *src++;
					if (token_val == 'n') {
						token_val = '\n';
					}
				}
				if (token == '"') {
					*data++ = token_val;
				}
			}
			tk_str += *src;
			src++;
			//if it is a single character, return Num token
			if (token == '"') {
				token_val = (int)last_pos;
			}
			else {
				token = Num;
			}
			return tk_str;
		}
		else if (token == '/') {
			if (*src == '/') {
				//skip comments
				while (*src != 0 && *src != '\n') {
					++src;
				}
			}
			else {
				// not a comment but a divide operator
				token = Div;
				return tk_str;
			}
		}
		else if (token == '=') {
			if (*src == '=') {
				tk_str += *src;
				src++;
				token = Eq;
			}
			else {
				token = Assign;
			}
			return tk_str;
		}
		else if (token == '+') {
			if (*src == '+') {
				tk_str += *src;
				src++;
				token = Inc;
			}
			else {
				token = Add;
			}
			return tk_str;
		}
		else if (token == '-') {
			if (*src == '-') {
				tk_str += *src;
				src++;
				token = Dec;
			}
			else {
				token = Sub;
			}
			return tk_str;
		}
		else if (token == '!') {
			if (*src == '=') {
				tk_str += *src;
				src++;
				token = Ne;
			}
			return tk_str;
		}
		else if (token == '<') {
			if (*src == '=') {
				tk_str += *src;
				src++;
				token = Le;
			}
			else if (*src == '<') {
				tk_str += *src;
				src++;
				token = Shl;
			}
			else {
				token = Lt;
			}
			return tk_str;
		}
		else if (token == '>') {
			if (*src == '=') {
				tk_str += *src;
				src++;
				token = Ge;
			}
			else if (*src == '>') {
				tk_str += *src;
				src++;
				token = Shr;
			}
			else {
				token = Gt;
			}
			return tk_str;
		}
		else if (token == '|') {
			if (*src == '|') {
				tk_str += *src;
				src++;
				token = Lor;
			}
			else {
				token = Or;
			}
			return tk_str;
		}
		else if (token == '&') {
			if (*src == '&') {
				tk_str += *src;
				src++;
				token = Lan;
			}
			else {
				token = And;
			}
			return tk_str;
		}
		else if (token == '^') {
			token = Xor;
			return tk_str;
		}
		else if (token == '%') {
			token = Mod;
			return tk_str;
		}
		else if (token == '*') {
			token = Mul;
			return tk_str;
		}
		else if (token == '[') {
			token = Brak;
			return tk_str;
		}
		else if (token == '?') {
			token = Cond;
			return tk_str;
		}
		else if (token == '~' || token == ';' || token == '{' || token == '}' || token == '(' || token == ')' || token == ']' || token == ',' || token == ':') {
			// return the token string directly;
			return tk_str;
		}
	}
	return tk_str;
}
