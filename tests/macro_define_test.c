#define DAMN "试试中文 f()=%d 1+34=%d"
#define RESULT f(),1+34
#define IF if
#include <stdio.h>
#ifdef ABC
int f() {
	return 133;
}
#else
int f() {
	return 232;
}
#endif
int main() {
	IF(1)
	{
		printf(DAMN,RESULT);
	}
	return 0;
}
