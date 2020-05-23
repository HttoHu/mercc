#include <stdio.h>
struct Test {
	int a = 1, b = 2, c = 3;
	int d = 7;
};
Test foo() {
	Test ret;
	ret.d = 0;
	return ret;
}
Test glo;
int main() {
	Test ret = foo();
	glo.a = -23;
	bool passed = true;
	passed = passed && ret.d == 0;
	if (ret.d == 0)
		printf("1. Function Call is OKay\n");
	Test v2 = ret, v3 = glo;
	passed = passed && v2.d == 0 && v3.a == -23;
	if (v2.d == 0 && v3.a == -23)
		printf("2. Struct init by a variable is okay\n");
	(v2.b)++;
	glo = v2;
	if (glo.b == 3)
		printf("3. Ctor is ok \n");
	Test* ptr = &ret;
	ptr->a = 111;
	v2 = *ptr;
	passed = passed && v2.a == 111;
	if (v2.a == 111)
		printf("4. struct ptr decay is okay\n");
	*ptr = glo;
	passed = passed && glo.b == 3;
	if (glo.b == 3)
		printf("5. struct ptr test 2 is ok \n");
	if (passed)
		printf("Congratulations, everything is fine");
	return 0;
}