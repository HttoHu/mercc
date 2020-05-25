#include <stdio.h>
struct A
{
	int a=-1;
	int vec[3] = {1, 2, 3};
	int b = -2;
	char name[100]="Jar Jar Binks";
};
A print_s(A a)
{
	printf("%d %d \n", a.a, a.b);
	printf("%d %d %d\nName:%s\n", a.vec[0], a.vec[1], a.vec[2],a.name);
	a.a =a.a* 2;
	return a;
}
A glo;
int print_array(int arr[3])
{
	for (int i = 0; i < 3; i++)
		printf("%d ", arr[i]++);
	printf("\n");
	return 0;
}
int main() {
	A b;
	A c[10];
	print_array(glo.vec);
	glo=print_s(glo);
	return 0;
}