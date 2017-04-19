#include<stdio.h>
int a;
int b;
int c;

enum test{A,B,C};
enum {D=1,E,F};

int main()
{
	char *s = "ahdoahfo";
	a = b + c;
	
	return 0;
}

void translation_unit()
{
	while(token != TK_EOF)
	{
		external_declaration(SC_GLOBAL);
	}
}