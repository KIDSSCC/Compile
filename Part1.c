#include<stdio.h> 
#include <stdbool.h>


int main() {
	//1.简单运用：表达式测试
	int x, y;
	scanf("%d%d", &x,&y);
	//双目算数
	int add = x + y;
	int sub = x - y;
	int mul = x * y;
	int div = x / y;
	int mod = x % y;
	//单目算数
	int singleAdd = ++x;
	int singleSub = -y;
	//关系运算
	bool result1 = (x == y);
	bool result2 = (x > y);
	bool result3 = (x < y);
	bool result4 = (x >= y);
	bool result5 = (x <= y);
	bool result6 = (x != y);
	// 逻辑运算
	bool b1 = (x > 1) || (x < -1);
	bool b2 = !(y > 0);
	bool b3 = (b1 && b2) ? b1 : b2;
	//输出
	printf("input: x=%d,y=%d\n", x-1,y);
	printf("doubleAlu: add=%d,sub=%d,mul=%d,div=%d,mod=%d\n", add,sub,mul,div,mod);
	printf("singleAlu: singleAdd=%d,singleSub=%d,x=%d\n", singleAdd,singleSub,x);
	printf("relation: eq=%d,gt=%d,lt=%d,ge=%d,le=%d,ne=%d\n", result1,result2,result3,result4,result5,result6);
	printf("logic: b1=%d,b2=%d,b3=%d\n", b1,b2,b3);

	return 0;
}
