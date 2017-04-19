#include<stdio.h>
int a;
int b;
int c;
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
