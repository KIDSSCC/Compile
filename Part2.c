#include<stdio.h> 
#define INIT 0
const int iter=1;

int adder(int a, int b) {
	return a + b;
}
int main() {
	//2.综合运用：斐波那契非递归
	int a, b, i, t, n;
	a = INIT;
	b = INIT+1;
	i = iter;
	scanf("%d", &n);
	if (n == 1) {
		printf("%d\n", a);
	}
	else if (n == 2) {
		printf("%d\n", b);
	}
	else if (n < 1) {
		printf("%s\n", "enter the number greater than 0!");
	}
	else {
		while (i < n)
		{
			t = b;
			b = adder(a, b);
			a = t;
			i = i + 1;
		}
		printf("%d\n", a);
	}
	return 0;
}
