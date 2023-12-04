#include<stdio.h>
int getint(){
	int n;
	scanf("%d",&n);
	return n;
}

//检测乘法中数是2的幂次    除法中数是2的幂次
int main(){
    int a,b,c;
    a = getint();
//    b = getint();
//    c = getint();
    printf("%d\n",a%8);
    printf("%d\n",a%2);
    //printf("%d\n",b%2);
    //printf("%d\n",c%2);
    //printf("%d\n",a%3);
    //printf("%d\n",a/3);
    return 0;
}