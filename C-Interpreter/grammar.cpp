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
	// 1. unit_unary ::= unit | unit unary_op | unary_op unit
	// 2. expr ::= unit_unary (bin_op unit_unary ...)

	//unit_unary
	int *id;
	int tmp;
	int *addr;
	{
		if (!token) {
			std::cout << " unexpected token EOF of expression! at line: " << line << std::endl;
			system("pause");
			exit(-1);
		}
		if (token == Num) {
			match(Num);

			//emit code
			*++text = IMM;
			*++text = token_val;
			expr_type = INT;
		}
		else if (token == '"') {
			// string: such as "asadfs"
			*++text = IMM;
			*++text = token_val;

			match('"');
			while (token == '"') {
				match('"');
			}

			data = (char *)(((int)data + sizeof(int)) & (-sizeof(int)));
			expr_type = PTR;
		}
		else if (token == Sizeof) {
			//sizeof is actually a unary operator
			match(Sizeof);
			match(')');
			expr_type = INT;
			if (token == Int){
				match(Int);
			}
			else if (token == Char) {
				match(Char);
				expr_type = CHAR;
			}
			while (token == Mul) {
				match(Mul);
				expr_type = expr_type + PTR;
			}
			match(')');

			//emit code
			*++text = IMM;
			*++text = (expr_type == CHAR) ? sizeof(char) : sizeof(int);
			expr_type = INT;
		}
		else if (token == Id) {
			// there are several type when occurs to Id
			// but this is unit, so it can only be
			// 1. function call
			// 2. Enum variable
			// 3. global/local variable
			match(Id);
			id = current_id;

			if (token == '(') {
				match('(');

				tmp = 0;				//number of arguments
				while (token!=')')
				{
					expression(Assign);
					*++text = PUSH;
					tmp++;

					if (token == ',') {
						match(',');
					}
				}
				match(')');

				//emit code
				if (id[Class] == Sys) {
					*++text = id[Value];
				}
				else if (id[Class] == Fun) {
					//function call
					*++text = CALL;
					*++text = id[Value];
				}
				else {
					std::cout << "Bad function call! at line: " << line << std::endl;
					system("pause");
					exit(-1);
				}
				if (tmp > 0) {
					*++text = ADJ;
					*++text = tmp;
				}
				expr_type = id[Type];
			}
			else if (id[Class] == Num) {
				//enum 
				*++text = IMM;
				*++text = id[Value];
				expr_type = INT;
			}
			else {
				//variable
				if (id[Class] == Loc) {
					*++text = LEA;
					*++text = index_of_bp - id[Value];
				}
				else if (id[Class] == Glo) {
					*++text = IMM;
					*++text = id[Value];
				}
				else {
					std::cout << "Undefined variable! at line: " << line << std::endl;
					system("pause");
					exit(-1);
				}
				// emit code, default behaviour is to load the value of the
				// address which is stored in `ax`
				expr_type = id[Type];
				*++text = (expr_type == Char) ? LC : LI;
			}
		}
		else if (token == '(') {
			// cast or parenthesis
			match('(');
			if (token == Int || token == Char) {
				tmp = (token == Char) ? CHAR : INT;
				match(token);
				while (token == Mul) {
					match(Mul);
					tmp = tmp + PTR;
				}
				match(')');

				expression(Inc);

				expr_type = tmp;
			}
			else {
				expression(Assign);
				match(')');
			}
		}
		else if (token == Mul) {
			// dereference *<addr>
			match(Mul);
			expression(Inc);

			if (expr_type >= PTR) {
				expr_type = expr_type - PTR;
			}
			else {
				std::cout << "Bad dereference! at line: " << line << std::endl;
				system("pause");
				exit(-1);
			}
			*++text = (expr_type == CHAR) ? LC : LI;
		}
		else if (token == And) {
			// get address of 
			match(And);
			expression(Inc);
			if (*text == LC || *text == LI) {
				text--;
			}
			else {
				std::cout << "A bad address! at line: " << line << std::endl;
				system("pause");
				exit(-1);
			}
			expr_type = expr_type + PTR;
		}
		else if (token == '!') {
			//not
			match('!');
			expression(Inc);

			//emit code: use <expr> == 0
			*++text = PUSH;
			*++text = IMM;
			*++text = 0;
			*++text = EQ;

			expr_type = INT;
		}
		else if (token == '~') {
			//bitwise not
			match('~');
			expression(Inc);

			//emit code: use <expr> XOR -1
			*++text = PUSH;
			*++text = IMM;
			*++text = -1;
			*++text = XOR;

			expr_type = INT;
		}
		else if (token == Add) {
			match(Add);
			expression(Inc);

			expr_type = INT;
		}
		else if (token == Sub) {
			match(Sub);

			if (token == Num) {
				*++text = IMM;
				*++text = -token_val;
				match(Num);
			}
			else {
				*++text = IMM;
				*++text = -1;
				*++text = PUSH;
				expression(Inc);
				*++text = MUL;
			}
			expr_type = INT;
		}
		else if (token == Inc || token == Dec) {
			tmp = token;
			match(token);
			expression(Inc);
			if (*text == LC) {
				*text = PUSH;
				*++text = LC;
			}
			else if (*text == LI) {
				*text = PUSH;
				*++text = LI;
			}
			else {
				std::cout << "Bad value of pre-increment! at line: " << line << std::endl;
				system("pause");
				exit(-1);
			}
			*++text = PUSH;
			*++text = IMM;
			*++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
			*++text = (tmp == Inc) ? ADD : SUB;
			*++text = (expr_type == CHAR) ? SC : SI;
		}
		else {
			std::cout << "Bad expression! at line: " << line << std::endl;
			system("pause");
			exit(-1);
		}
	}
	// binary operator and postfix operators.
	{
	while (token >= level) {
		// handle according to current operator's precedence
		tmp = expr_type;
		if (token == Assign) {
			// var = expr;
			match(Assign);
			if (*text == LC || *text == LI) {
				*text = PUSH; // save the lvalue's pointer
			}
			else {
				std::cout << "Bad lvalue in assignment! at line: " << line << std::endl;
				system("pause");
				exit(-1);
			}
			expression(Assign);

			expr_type = tmp;
			*++text = (expr_type == CHAR) ? SC : SI;
		}
		else if (token == Cond) {
			// expr ? a : b;
			match(Cond);
			*++text = JZ;
			addr = ++text;
			expression(Assign);
			if (token == ':') {
				match(':');
			}
			else {
				std::cout << "Missing colon in conditional! at line: " << line << std::endl;
				system("pause");
				exit(-1);
			}
			*addr = (int)(text + 3);
			*++text = JMP;
			addr = ++text;
			expression(Cond);
			*addr = (int)(text + 1);
		}
		else if (token == Lor) {
			// logic or
			match(Lor);
			*++text = JNZ;
			addr = ++text;
			expression(Lan);
			*addr = (int)(text + 1);
			expr_type = INT;
		}
		else if (token == Lan) {
			// logic and
			match(Lan);
			*++text = JZ;
			addr = ++text;
			expression(Or);
			*addr = (int)(text + 1);
			expr_type = INT;
		}
		else if (token == Or) {
			// bitwise or
			match(Or);
			*++text = PUSH;
			expression(Xor);
			*++text = OR;
			expr_type = INT;
		}
		else if (token == Xor) {
			// bitwise xor
			match(Xor);
			*++text = PUSH;
			expression(And);
			*++text = XOR;
			expr_type = INT;
		}
		else if (token == And) {
			// bitwise and
			match(And);
			*++text = PUSH;
			expression(Eq);
			*++text = AND;
			expr_type = INT;
		}
		else if (token == Eq) {
			// equal ==
			match(Eq);
			*++text = PUSH;
			expression(Ne);
			*++text = EQ;
			expr_type = INT;
		}
		else if (token == Ne) {
			// not equal !=
			match(Ne);
			*++text = PUSH;
			expression(Lt);
			*++text = NE;
			expr_type = INT;
		}
		else if (token == Lt) {
			// less than
			match(Lt);
			*++text = PUSH;
			expression(Shl);
			*++text = LT;
			expr_type = INT;
		}
		else if (token == Gt) {
			// greater than
			match(Gt);
			*++text = PUSH;
			expression(Shl);
			*++text = GT;
			expr_type = INT;
		}
		else if (token == Le) {
			// less than or equal to
			match(Le);
			*++text = PUSH;
			expression(Shl);
			*++text = LE;
			expr_type = INT;
		}
		else if (token == Ge) {
			// greater than or equal to
			match(Ge);
			*++text = PUSH;
			expression(Shl);
			*++text = GE;
			expr_type = INT;
		}
		else if (token == Shl) {
			// shift left
			match(Shl);
			*++text = PUSH;
			expression(Add);
			*++text = SHL;
			expr_type = INT;
		}
		else if (token == Shr) {
			// shift right
			match(Shr);
			*++text = PUSH;
			expression(Add);
			*++text = SHR;
			expr_type = INT;
		}
		else if (token == Add) {
			// add
			match(Add);
			*++text = PUSH;
			expression(Mul);

			expr_type = tmp;
			if (expr_type > PTR) {
				// pointer type, and not `char *`
				*++text = PUSH;
				*++text = IMM;
				*++text = sizeof(int);
				*++text = MUL;
			}
			*++text = ADD;
		}
		else if (token == Sub) {
			// sub
			match(Sub);
			*++text = PUSH;
			expression(Mul);
			if (tmp > PTR && tmp == expr_type) {
				// pointer subtraction
				*++text = SUB;
				*++text = PUSH;
				*++text = IMM;
				*++text = sizeof(int);
				*++text = DIV;
				expr_type = INT;
			}
			else if (tmp > PTR) {
				// pointer movement
				*++text = PUSH;
				*++text = IMM;
				*++text = sizeof(int);
				*++text = MUL;
				*++text = SUB;
				expr_type = tmp;
			}
			else {
				// numeral subtraction
				*++text = SUB;
				expr_type = tmp;
			}
		}
		else if (token == Mul) {
			// multiply
			match(Mul);
			*++text = PUSH;
			expression(Inc);
			*++text = MUL;
			expr_type = tmp;
		}
		else if (token == Div) {
			// divide
			match(Div);
			*++text = PUSH;
			expression(Inc);
			*++text = DIV;
			expr_type = tmp;
		}
		else if (token == Mod) {
			// Modulo
			match(Mod);
			*++text = PUSH;
			expression(Inc);
			*++text = MOD;
			expr_type = tmp;
		}
		else if (token == Inc || token == Dec) {
			// postfix inc(++) and dec(--)
			// we will increase the value to the variable and decrease it
			// on `ax` to get its original value.
			if (*text == LI) {
				*text = PUSH;
				*++text = LI;
			}
			else if (*text == LC) {
				*text = PUSH;
				*++text = LC;
			}
			else {
				std::cout << "A bad value in increment! at line: " << line << std::endl;
				system("pause");
				exit(-1);
			}

			*++text = PUSH;
			*++text = IMM;
			*++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
			*++text = (token == Inc) ? ADD : SUB;
			*++text = (expr_type == CHAR) ? SC : SI;
			*++text = PUSH;
			*++text = IMM;
			*++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
			*++text = (token == Inc) ? SUB : ADD;
			match(token);
		}
		else if (token == Brak) {
			// array access var[xx]
			match(Brak);
			*++text = PUSH;
			expression(Assign);
			match(']');

			if (tmp > PTR) {
				// pointer, `not char *`
				*++text = PUSH;
				*++text = IMM;
				*++text = sizeof(int);
				*++text = MUL;
			}
			else if (tmp < PTR) {
				std::cout << "Pointer type expected! at line: " << line << std::endl;
				system("pause");
				exit(-1);
			}
			expr_type = tmp - PTR;
			*++text = ADD;
			*++text = (expr_type == CHAR) ? LC : LI;
		}
		else {
			std::cout << "Compiler error, token = " << token << "! at line: " << line << std::endl;
			system("pause");
			exit(-1);
		}
	}
	}

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
	// there are 6 kinds of statements here:
	// 1. if (...) <statement> [else <statement>]
	// 2. while (...) <statement>
	// 3. { <statement> }
	// 4. return xxx;
	// 5. <empty statement>;
	// 6. expression; (expression end with semicolon)

	int *a, *b;

	if (token == If) {
		// if (...) <statement> [else <statement>]
		//
		//   if (...)           <cond>
		//                      JZ a
		//     <statement>      <statement>
		//   else:              JMP b
		// a:                 a:
		//     <statement>      <statement>
		// b:                 b:
		//

		match(If);
		match('(');
		expression(Assign);
		match(')');

		*++text = JZ;
		b = ++text;

		statement();
		if (token == Else) {
			match(Else);

			// emit code for JMP B
			*b = (int)(text + 3);
			*++text = JMP;
			b = ++text;

			statement();
		}
		*b = (int)(text + 1);
	}
	else if (token == While) {
		// a:                     a:
		//    while (<cond>)        <cond>
		//                          JZ b
		//     <statement>          <statement>
		//                          JMP a
		// b:                     b:

		match(While);
		a = text + 1;

		match('(');
		expression(Assign);
		match(')');

		*++text = JZ;
		b = ++text;

		statement();

		*++text = JMP;
		*++text = (int)a;
		*b = (int)(text + 1);

	}
	else if (token == '{') {
		// { <statement> ... }

		match('{');
		while (token != '}')
		{
			statement();
		}
		match('}');
	}
	else if (token == Return) {
		// return [expression];

		match(Return);
		if (token != ';') {
			expression(Assign);
		}
		match(';');

		//emit code for return
		*++text = LEV;
	}
	else if (token == ';') {
		//empty statement
		match(';');
	}
	else {
		expression(Assign);
		match(';');
	}
}