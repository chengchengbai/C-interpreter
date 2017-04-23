#include<stdio.h>
int a;
int b;
int c;

enum test{A,B,C};
enum {D=1,E,F};

int main()
{
	char *s;	
	*s = "ahdoahfo";
	a = b + c;
	
	return 0;
}

void test_if()
{
	if(a==b){
		return 1;
	}
	else{
		c = a;
	}
}

void test_expr(int a,int b,char c)
{
	int *d;
	a=a*b-c+1;
	b=((1+2-3*a)/2)-b;
	c = ~c;
	*d=a++;
}

int test_while()
{
	while((a&b)==0){
		return a+b;
	}
}