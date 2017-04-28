
#include"global.h"

//here gose the global definitions
char *src, *old_src;		//source file content buffer
int token;					//token number
int poolsize;
int line;					//current line number

int *text,                    // text segment
*old_text,                // for dump text segment
*stack;                   // stack
char *data;                   // data segment

int *pc, *bp, *sp, ax, cycle; // virtual machine registers

int token_val;                // value of current token (mainly for number)
int *current_id,              // current parsed ID
*symbols;                 // symbol table
int *idmain;                  // the `main` function

int readfile()
{
	int i;
	long fSize;
	FILE *fp;
	std::string filename;

	printf("please input a source file name: \n");
	std::cin >> filename;
	filename = "test.c";

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
	fclose(fp);

	//printf("%s\n", src);		
	std::cout << src << std::endl;		//printf will cause some issues on display so use 'cout' instead

	system("pause");
	return fSize;
}

int eval()
{
	//virtual machine to interpret or execute code
	int op, *tmp;
	while (1) {
		op = *pc++; // get next operation code

		if (op == IMM) { ax = *pc++; }                                     // load immediate value to ax
		else if (op == LC) { ax = *(char *)ax; }                               // load character to ax, address in ax
		else if (op == LI) { ax = *(int *)ax; }                                // load integer to ax, address in ax
		else if (op == SC) { ax = *(char *)*sp++ = ax; }                       // save character to address, value in ax, address on stack
		else if (op == SI) { *(int *)*sp++ = ax; }                             // save integer to address, value in ax, address on stack
		else if (op == PUSH) { *--sp = ax; }                                     // push the value of ax onto the stack
		else if (op == JMP) { pc = (int *)*pc; }                                // jump to the address
		else if (op == JZ) { pc = ax ? pc + 1 : (int *)*pc; }                   // jump if ax is zero
		else if (op == JNZ) { pc = ax ? (int *)*pc : pc + 1; }                   // jump if ax is zero
		else if (op == CALL) { *--sp = (int)(pc + 1); pc = (int *)*pc; }           // call subroutine
																				   //else if (op == RET)  {pc = (int *)*sp++;}                              // return from subroutine;
		else if (op == ENT) { *--sp = (int)bp; bp = sp; sp = sp - *pc++; }      // make new stack frame
		else if (op == ADJ) { sp = sp + *pc++; }                                // add esp, <size>
		else if (op == LEV) { sp = bp; bp = (int *)*sp++; pc = (int *)*sp++; }  // restore call frame and PC
		else if (op == ENT) { *--sp = (int)bp; bp = sp; sp = sp - *pc++; }      // make new stack frame
		else if (op == ADJ) { sp = sp + *pc++; }                                // add esp, <size>
		else if (op == LEV) { sp = bp; bp = (int *)*sp++; pc = (int *)*sp++; }  // restore call frame and PC
		else if (op == LEA) { ax = (int)(bp + *pc++); }                         // load address for arguments.

		else if (op == OR)  ax = *sp++ | ax;
		else if (op == XOR) ax = *sp++ ^ ax;
		else if (op == AND) ax = *sp++ & ax;
		else if (op == EQ)  ax = *sp++ == ax;
		else if (op == NE)  ax = *sp++ != ax;
		else if (op == LT)  ax = *sp++ < ax;
		else if (op == LE)  ax = *sp++ <= ax;
		else if (op == GT)  ax = *sp++ >  ax;
		else if (op == GE)  ax = *sp++ >= ax;
		else if (op == SHL) ax = *sp++ << ax;
		else if (op == SHR) ax = *sp++ >> ax;
		else if (op == ADD) ax = *sp++ + ax;
		else if (op == SUB) ax = *sp++ - ax;
		else if (op == MUL) ax = *sp++ * ax;
		else if (op == DIV) ax = *sp++ / ax;
		else if (op == MOD) ax = *sp++ % ax;


		else if (op == EXIT) { printf("exit(%d)", *sp); return *sp; }
		//else if (op == OPEN) { ax = open((char *)sp[1], sp[0]); }
		//else if (op == CLOS) { ax = close(*sp); }
		//else if (op == READ) { ax = read(sp[2], (char *)sp[1], *sp); }
		else if (op == PRTF) { tmp = sp + pc[1]; ax = printf((char *)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]); }
		else if (op == MALC) { ax = (int)malloc(*sp); }
		else if (op == MSET) { ax = (int)memset((char *)sp[2], sp[1], *sp); }
		else if (op == MCMP) { ax = memcmp((char *)sp[2], (char *)sp[1], *sp); }
		else {
			printf("unknown instruction:%d\n", op);
			return -1;
		}
	}
	return 0;
}

//allocate memory for symbol table
//add keywords and system call to symbol table
int preprocess()
{
	int i;
	poolsize = 256 * 1024;

	if (!(text = old_text =(int*)malloc(poolsize))) {
		printf("could not malloc(%d) for text area\n", poolsize);
		return -1;
	}

	if (!(data =(char*)malloc(poolsize))) {

		printf("could not malloc(%d) for data area\n", poolsize);
		return -1;
	}

	if (!(stack = (int*)malloc(poolsize))) {
		printf("could not malloc(%d) for symbol table\n", poolsize);
		return -1;
	}

	if (!(symbols = (int*)malloc(poolsize))) {
		printf("could not malloc(%d) for symbol table\n", poolsize);
		return -1;
	}

	memset(text, 0, poolsize);
	memset(data, 0, poolsize);
	memset(stack, 0, poolsize);
	memset(symbols, 0, poolsize);
	bp = sp = (int *)((int)stack + poolsize);
	ax = 0;

	src = "char else enum if int return sizeof while "
		"open read close printf malloc memset memcmp exit void main";

	// add keywords to symbol table
	i = Char;
	while (i <= While)
	{
		get_token();
		current_id[Token] = i++;
	}
	// add library to symbol table
	i = OPEN;
	while (i <= EXIT) {
		get_token();
		current_id[Class] = Sys;
		current_id[Type] = INT;
		current_id[Value] = i++;
	}
	get_token(); current_id[Token] = Char;
	get_token(); idmain = current_id;

	return 0;
}

//int main()
//{
//	line = 1;		
//
//	if (preprocess() != 0)
//		return -1;
//
//	readfile();
//	program();
//	system("pause");
//	return 0;
//}

int readfile_arg(int argc, char *argv[])
{
	int i;
	FILE *fp;

	argc--;
	argv++;

	if ((fp = fopen(*argv, "rb")) == NULL) {
		printf("Can not open source file\n");
		system("pause");
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	long fSize = ftell(fp);
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
	fclose(fp);

	//printf("%s\n", src);		
	std::cout << src << std::endl;		//printf will cause some issues on display so use 'cout' instead

	system("pause");
	return fSize;
}

int main(int argc, char *argv[])
{

	line = 1;
	if (preprocess() != 0)
		return -1;

	readfile_arg(argc, argv);
	program();
	
	//int i = 0;
	//text[i++] = IMM;
	//text[i++] = 10;
	//text[i++] = PUSH;
	//text[i++] = IMM;
	//text[i++] = 20;
	//text[i++] = ADD;
	//text[i++] = PUSH;
	//text[i++] = EXIT;
	//pc = text;

	if (!(pc = (int *)idmain[Value])) {
		printf("main() not defined\n");
		system("pause");
		return -1;
	}

	int *tmp;
	sp = (int *)((int)stack + poolsize);
	*--sp = EXIT; // call exit if main returns
	*--sp = PUSH; tmp = sp;
	*--sp = argc;
	*--sp = (int)argv;
	*--sp = (int)tmp;

	std::cout << eval() << std::endl;
	system("pause");
	return 0;
}