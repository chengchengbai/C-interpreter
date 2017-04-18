#include<stdio.h>
#include<memory.h>
#include<stdlib.h>
#include<iostream>
#include<string>

char *src, *old_src;
int token;
int poolsize;
int line;

char *data;                   // data segment

int token_val;                // value of current token (mainly for number)
int *current_id,              // current parsed ID
*symbols;                 // symbol table
int *idmain;                  // the `main` function

// instructions
enum {
	LEA, IMM, JMP, CALL, JZ, JNZ, ENT, ADJ, LEV, LI, LC, SI, SC, PUSH,
	OR, XOR, AND, EQ, NE, LT, GT, LE, GE, SHL, SHR, ADD, SUB, MUL, DIV, MOD,
	OPEN, READ, CLOS, PRTF, MALC, MSET, MCMP, EXIT
};

// tokens and classes (operators last and in precedence order)
enum {
	Num = 128, Fun, Sys, Glo, Loc, Id,
	Char, Else, Enum, If, Int, Return, Sizeof, While,
	Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};

// identifier fields
enum { Token, Hash, Name, Type, Class, Value, BType, BClass, BValue, IdSize };

// types of variable/function
enum { CHAR, INT, PTR };

int readfile()
{
	int i;
	long fSize;
	FILE *fp;
	std::string filename;

	printf("please input a source file name: \n");
	//std::cin >> filename;
	filename = "test.cpp";


	if ((fp = fopen((char *)filename.data(), "rb")) == NULL)
	{
		printf("Can not open source file\n");
		system("pause");
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	fSize = ftell(fp);
	rewind(fp);

	if (!(src = old_src = (char *)malloc(fSize + 1)))
	{
		printf("Can not malloc %d memeory for source area\n", poolsize);
		system("pause");
		return -1;
	}

	i = fread(src, 1, fSize, fp);
	if (i != fSize)
	{
		printf("Can not read source file\n");
		system("pause");
		return -1;
	}
	src[i + 1] = 0;
	std::cout << src << std::endl;

	system("pause");
	return fSize;
}

void lexer()
{
	char *last_pos;
	int hash;
	while (token = *src)
	{
		++src;
		if (token == '\n')
		{
			++line;
		}
		else if (token == '#') {
			while (*src != 0 && *src != '\n'){
				src++;
			}
		}
		else if ((token >= 'a'&&token <= 'z') || (token >= 'A'&&token <= 'Z') || (token == '_')) {

			// handle identifier
			last_pos = src - 1;
			hash = token;

			while ((*src >= 'a'&&*src <= 'z') || (*src >= 'A'&&*src <= 'Z') || (*src >= '0'&&*src <= '9') || (*src == '_')) {
				hash = hash * 147 + *src;
				src++;
			}

			//look for existing identifier, liner search
			current_id = symbols;
			while (current_id[Token]) {
				if (current_id[Hash] == hash && !memcmp((char *)current_id[Name], last_pos, src - last_pos)) {
					// if found, return
					token = current_id[Token];
					return;
				}
				current_id = current_id + IdSize;
			}
			// store new id
			current_id[Name] = (int)last_pos;
			current_id[Hash] = hash;
			token = current_id[Token] = Id;
			return;
		}
		else if (token >= '0'&&token <= '9') {
			//handle a number
			token_val = token - '0';
			while (*src >= '0'&&*src <= '9')
			{
				token_val = token_val * 10 + *src++ - '0';
			}
			token = Num;
			return;
		}
		else if (token == '"' || token == '\'') {
			//handle string literal
			last_pos = data;
			while (*src != 0 && *src != token) {
				token_val = *src++;
				if (token_val == '\\') {
					token_val = *src++;
					if (token_val == 'n') {
						token_val = '\n';
					}
				}
				if (token == '"') {
					*data++ = token_val;
				}
			}
			src++;
			//if it is a single character, return Num token
			if (token == '"') {
				token_val = (int)last_pos;
			}
			else {
				token = Num;
			}
			return;
		}
		else if (token == '/') {
			if (*src == '/') {
				//skip comments
				while (*src != 0 && *src != '\n'){
					++src;
				}
			}
			else {
				// not a comment but a divide operator
				token = Div;
				return;
			}
		}
		else if (token == '=') {
			if (*src == '=') {
				src++;
				token = Eq;
			}
			else {
				token = Assign;
			}
			return;
		}
		else if (token == '+') {
			if (*src == '+') {
				src++;
				token = Inc;
			}
			else {
				token = Add;
			}
			return;
		}
		else if (token == '-') {
			if (*src == '-') {
				src++;
				token = Dec;
			}
			else {
				token = Sub;
			}
			return;
		}
		else if (token == '!') {
			if (*src == '=') {
				src++;
				token = Ne;
			}
			return;
		}
		else if (token == '<') {
			if (*src == '=') {
				src++;
				token = Le;
			}
			else if (*src == '<') {
				src++;
				token = Shl;
			}
			else {
				token = Lt;
			}
			return;
		}
		else if (token == '>') {
			if (*src == '=') {
				src++;
				token = Ge;
			}
			else if (*src == '>') {
				src++;
				token = Shr;
			}
			else {
				token = Gt;
			}
			return;
		}
		else if (token == '|') {
			if (*src == '|') {
				src++;
				token = Lor;
			}
			else {
				token = Or;
			}
			return;
		}
		else if (token == '&') {
			if (*src == '&') {
				src++;
				token = Lan;
			}
			else {
				token = And;
			}
			return;
		}
		else if (token == '^') {
			token = Xor;
			return;
		}
		else if (token == '%') {
			token = Mod;
			return;
		}
		else if (token == '*') {
			token = Mul;
			return;
		}
		else if (token == '[') {
			token = Brak;
			return;
		}
		else if (token == '?') {
			token = Cond;
			return;
		}
		else if (token == '~' || token == ';' || token == '{' || token == '}' || token == '(' || token == ')' || token == ']' || token == ',' || token == ':') {
			// directly return the character as token;
			return;
		}
	}
	return;
}

void expr(int level) {
	// do nothing
}

void program() {
	lexer(); // get next token
	while (token > 0) {
		printf("The token is: %3d(%c)\n", token, (char)token);
		lexer();
	}
}

void eval()
{
	//virtual machine to interpreter code
}

int main()
{
	int i;
	line = 1;
	poolsize = 256 * 1024;

	if (!(symbols =(int*)malloc(poolsize))) {
		printf("could not malloc(%d) for symbol table\n", poolsize);
		return -1;
	}
	memset(symbols, 0, poolsize);

	src = "char else enum if int return sizeof while "
		"open read close printf malloc memset memcmp exit void main";

	// add keywords to symbol table
	i = Char;
	while (i <= While)
	{
		lexer();
		current_id[Token] = i++;
	}
	// add library to symbol table
	i = OPEN;
	while (i <= EXIT) {
		lexer();
		current_id[Class] = Sys;
		current_id[Type] = INT;
		current_id[Value] = i++;
	}
	lexer(); current_id[Token] = Char;
	lexer(); idmain = current_id;

	readfile();
	program();
	system("pause");
	return 0;
}