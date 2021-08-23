// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s

int
add_em_up(int count, ...) // expected-error{{variadic functions are not allowed}}
{
	int sum = 0;
	return sum;
}

void
main(void)
{
	int i = add_em_up(3, 5, 5, 6);

	i = add_em_up(10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);

}
