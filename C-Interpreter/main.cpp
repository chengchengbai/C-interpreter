
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

	std::cout << src << std::endl;		//printf will cause some issues on display so use 'cout' instead

	system("pause");
	return fSize;
}

void eval()
{
	//virtual machine to interpret or execute code
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

	if (!(symbols = (int*)malloc(poolsize))) {
		printf("could not malloc(%d) for symbol table\n", poolsize);
		return -1;
	}
	memset(data, 0, poolsize);
	memset(symbols, 0, poolsize);

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

int main()
{
	line = 1;		

	if (preprocess() != 0)
		return -1;

	readfile();
	program();
	system("pause");
	return 0;
}