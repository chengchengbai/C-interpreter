//	EBNF representation:
//		program :: = { global_declaration }+
//		global_declaration :: = enum_decl | variable_decl | function_decl
//		enum_decl :: = 'enum'[id] '{' id['=' 'num']{ ',' id['=' 'num'] '}'
//		variable_decl :: = type{ '*' } id{ ',' {'*'} id } ';'
//		function_decl :: = type{ '*' } id '(' parameter_decl ')' '{' body_decl '}'
//		parameter_decl :: = type{ '*' } id{ ',' type{ '*' } id }
//		body_decl :: = { variable_decl },{ statement }
//		statement :: = non_empty_statement | empty_statement
//		non_empty_statement :: = if_statement | while_statement | '{' statement '}'
//								| 'return' expression | expression ';'
//		if_statement :: = 'if' '(' expression ')' statement['else' non_empty_statement]
//		while_statement :: = 'while' '(' expression ')' non_empty_statement

#include"global.h"

extern int token;
extern int line;
extern int *current_id;
extern int *symbols;
extern int *text;
extern char *data;
extern int token_val;

int basetype;    // the type of a declaration, make it global for convenience
int expr_type;   // the type of an expression
int index_of_bp; // index of bp pointer on stack

void program()
{	
	std::cout << get_token() << std::endl;		// get next token
	while (token > 0) {
		global_declaration();
	}
}

void match(int tk) {
	if (token == tk) {
		std::cout << get_token() << std::endl;
	}
	else {
		std::cout<<"Expected token "<<(char)tk<<" at line: "<< line<<std::endl;
		system("pause");
		exit(-1);
	}
}

void expression(int level)
{
	// do nothing
}

void global_declaration()
{
	// global_declaration ::= enum_decl | variable_decl | function_decl
	//
	// enum_decl ::= 'enum' [id] '{' id ['=' 'num'] {',' id ['=' 'num'} '}'
	//
	// variable_decl ::= type {'*'} id { ',' {'*'} id } ';'
	//
	// function_decl ::= type {'*'} id '(' parameter_decl ')' '{' body_decl '}'

	int type;
	int i;

	basetype = INT;

	if (token == Enum) {
		// enum [id] { a = 10, b = 20, ... }

		match(Enum);
		if (token == Id) {
			match(Id);		//skip Id
		}
		if (token == '{') {
			match('{');
			enum_declaration();
			match('}');
		}
		match(';');
		return;
	}
	if (token == Int) {
		match(Int);
	}
	else if (token == Char) {
		match(Char);
		basetype = CHAR;
	}
	while (token != ';'&&token != '}') {

		type = basetype;

		while (token == Mul) {
			//maybe multiple pointer type such as char **a

			match(Mul);
			type = type + PTR;
		}
		if (token != Id) {
			//invalid declaration
			std::cout << "A bad global declaration! at line: "<<line << std::endl;
			system("pause");
			exit(-1);
		}
		if (current_id[Class]) {
			//Id has the same name with keywords
			std::cout << "Duplicate global declaration! at line: " << line << std::endl;
			system("pause");
			exit(-1);
		}
		match(Id);
		current_id[Type] = type;

		if (token == '(') {
			//function definition
			current_id[Class] = Fun;
			current_id[Value] = (int)(text + 1);	//record the function address on virtual machine
			function_declaration();
		}
		else {
			//variable declaration
			current_id[Class] = Glo;			//global declaration
			current_id[Value] = (int)data;		//assign memory address
			data = data + sizeof(int);
		}
		if (token == ',')
			match(',');
	}
	std::cout << get_token() << std::endl;
}

void enum_declaration()
{	
	//enum [id] {a=1,b=2,...};

	int i = 0;
	while (token != '}')
	{
		if (token != Id) {
			std::cout << "Bad enum identifier! at line " << line << std::endl;
			system("pause");
			exit(-1);
		}

		std::cout << get_token() << std::endl;
		if (token == Assign) {
			std::cout << get_token() << std::endl;
			if (token != Num) {
				std::cout << "Wrong assign value! at line " << line << std::endl;
				system("pause");
				exit(-1);
			}
			i = token_val;
			std::cout << get_token() << std::endl;
		}
		current_id[Class] = Num;
		current_id[Type] = INT;
		current_id[Value] = i++;

		if (token == ',') {
			std::cout << get_token() << std::endl;
		}
	}
}

void function_declaration()
{
	// type func_name (...) {...}
	// parse parameter list and main body: (...) {...}

	match('(');
	function_parameter();
	match(')');
	match('{');
	function_body();

	// look for symbol table and save the local var imformation
	current_id = symbols;
	while (current_id[Token])
	{
		if (current_id[Class] == Loc) {
			current_id[Class] = current_id[BClass];
			current_id[Type] = current_id[BType];
			current_id[Value] = current_id[BValue];
		}
		current_id = current_id + IdSize;
	}
}

void function_parameter()
{
	// parameter_decl ::= type {'*'} id {',' type {'*'} id}
	int type;
	int params = 0;
	while (token!=')')
	{
		type = INT;
		if (token == Int) {
			match(Int);
		}
		else if (token == Char) {
			type = CHAR;
			match(Char);
		}

		while (token == Mul) {
			//maybe multiple pointer type such as char **a
			match(Mul);
			type = type + PTR;
		}
		if (token != Id) {
			//invalid variable name
			std::cout << "A bad parameter declaration! at line: " << line << std::endl;
			system("pause");
			exit(-1);
		}
		if (current_id[Class] == Loc) {
			// if parameters name are already existed
			std::cout << "Duplicate declaration! at line: " << line << std::endl;
			system("pause");
			exit(-1);
		}
		
		match(Id);
		// save local variable information
		current_id[BClass] = current_id[Class]; current_id[Class] = Loc;
		current_id[BType] = current_id[Type]; current_id[Type] = type;
		current_id[BValue] = current_id[Value]; current_id[Value] = params++;

		if (token == ','){
			match(',');
		}
	}
	// fix the location of bp pointer as some parameters are sent to the stack after that processing
	index_of_bp = params + 1;
}

void function_body()
{
	// type func_name (...) {...}

	// parse the main body of a function which is in the brace:
	// ... {
	// 1. local declarations
	// 2. statements
	// }

	int pos_local; // position of local variables on the stack.
	int type;
	pos_local = index_of_bp;

	while (token == Int || token == Char) {
		basetype = (token == Int) ? INT : CHAR;
		match(token);

		while (token != ';')
		{
			type = basetype;

			while (token == Mul)
			{
				match(Mul);
				type = type + PTR;
			}

			if (token != Id) {
				std::cout << "A bad local declaration! at line: " << line << std::endl;
				system("pause");
				exit(-1);
			}
			if (current_id[Class] == Loc) {
				// if parameters name are already existed
				std::cout << "Duplicate local declaration! at line: " << line << std::endl;
				system("pause");
				exit(-1);
			}

			match(Id);
			// save local variable information
			current_id[BClass] = current_id[Class]; current_id[Class] = Loc;
			current_id[BType] = current_id[Type]; current_id[Type] = type;
			current_id[BValue] = current_id[Value]; current_id[Value] = ++pos_local;

			if (token == ',') {
				match(',');
			}
		}
		match(';');
	}

	//save stack size for local variables
	*++text = ENT;
	*++text = pos_local - index_of_bp;

	//start to parse statements
	while (token != '}'){
		statement();
	}

	//emit code for leaving sub function
	*++text = LEV;
}

void statement()
{

}