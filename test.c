#include <stdio.h>
#include <string.h>
#include <stdint.h>

int32_t main(void)
{
	char buf[50] = "Hello, ";
	strcat(buf, "world!\n");
	printf("%s", buf);

	return 0;
}
