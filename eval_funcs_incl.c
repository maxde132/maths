#include <stdio.h>
#include "config.h"

double print_double(double a)
{
	printf("%.*f", global_config.precision, a);
	return a;
}
double println_double(double a)
{
	printf("%.*f\n", global_config.precision, a);
	return a;
}
