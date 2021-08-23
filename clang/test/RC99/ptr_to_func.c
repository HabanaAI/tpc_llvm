// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s

//GAUDI-1366
// XFAIL:*

int sum(int num1, int num2)
{
  return num1 + num2;
}
void add(int a, int b)
{
  int i = a + b;
}
void subtract(int a, int b)
{
  int i = a - b;
}

// A function that receives a pointer to function as parameter
void wrapper(int(*fun)())  // expected-error{{pointers to function are not allowed}}
{
	// <f2p> is a pointer to a function which returns an int and takes two int
  int(*f2p) (int, int);  // expected-error{{pointers to function are not allowed}}
  fun();
	// fun_ptr_arr is an array of function pointers
  void(*fun_ptr_arr[])(int, int) = { add, subtract }; // expected-error{{pointers to function are not allowed}}
  unsigned int a = 15, b = 10, ch = 0;
  (*fun_ptr_arr[ch])(a, b);
  int i = ((int(*)())(""))(""); // expected-error{{pointers to function are not allowed}}
}
// pointer to function as field of structure
typedef struct String_Struct* String;

struct String_Struct
{
  char* (*get)(const void* self);  // expected-error{{pointers to function are not allowed}}
};

// Direct solution: Function takes a char and returns a pointer to a
// function which is taking two floats and returns a float. <opCode>
// specifies which function to return
int(*GetPtr1(const char opCode))(int, int); // expected-error{{pointers to function are not allowed}}

// pointer to function in typedef
typedef int(*myFuncDef)(int, int); // expected-error{{pointers to function are not allowed}}
								   // note that the typedef name is indeed myFuncDef
myFuncDef functionFactory(int n) {
  return 0;
}

void test(void *);
void base();
void call_base() {
  void	*abc = &base;   // expected-error{{pointers to function are not allowed}}
  test(&base);   // expected-error{{pointers to function are not allowed}}
}
