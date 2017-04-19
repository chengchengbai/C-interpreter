#pragma once

#include<stdio.h>
#include<memory.h>
#include<stdlib.h>
#include<iostream>
#include<string>


// mock assembly instructions
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


// here gose the function declarations
int readfile();
int preprocess();

std::string get_token();