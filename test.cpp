#include<stdio.h>
int getint(){
	int n;
	scanf("%d",&n);
	return n;
}

const int N = 10;
int a[10] = {0,1,2,3,4,5,6,7,8,9};
int fib(int i) {
    if (i == 1) {
        return 1;
    }
    if (i == 2) {
        return 2;
    }
    return fib(i - 1) + fib(i - 2);
}
int main()
{
    int i = 0;
    i = fib(5);
    return 0;
}