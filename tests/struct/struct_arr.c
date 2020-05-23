#include <stdio.h>
struct Test {
	int a = 1, b = 2, c = 3;
	int d = 7;
};
int main() {
	Test t[3][3] = { { Test{ 11,22,33,44 },Test{ 11,22,33,44 },Test{ 11,22,33,44 }},{Test{ 11,22,33,445 }, Test{ 11,22,33,44 },Test{ 11,22,33,44 } }, {Test{ 11,22,33,44 }, Test{ 11,22,33,445 },Test{ 11,22,33,44 }} };
	Test t2[4] = { Test{1,2,3,4},Test{12,2,3,4}, Test{1,2,37,4}, Test{1,2,3,45} };
	Test tests[10][10][10];
	for (int i = 0; i < 10; i++)
		printf("arr[%d] is %d \n", i, tests[i][1][2].a);
	for (int i = 0; i < 4; i++)
		printf("%d\n", t2[i].a);
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			printf("t[%d][%d]=%d\n", i, j, t[i][j].a);
	return 0;
}