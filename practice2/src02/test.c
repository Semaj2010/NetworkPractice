#include <stdio.h>


int main(){
	char buf[40];
	int a;

	a = read(0, buf, 40);

	printf("out : %s",buf);

	return 0;
}
