#include<stdio.h>
int getint(){
    int n;
    scanf("%d",&n);
    return n;
}
const int a = 1, b = 2;
const int c = 3;
int d, e = 1 + 2;
int f;


int double2(int x1) {
    if (!x1) {
        return 2*x1;
    }
    else {
        return (2*-x1);
    }
    return 0;
}

int add(int x1, int x2) {
    int sum = 0;
    sum = x1 + x2;
    return sum;
}

int sub(int x1, int x2) {
    int sum = 0;
    sum = x1 - x2;
    if (sum >= 0) {
        return sum;
    }
    if (sum <= 0) {
        return 0;
    }
    return 0;
}

void print_none() {
    return;
}

int main() {
    int i =0;
    for (i = 0;i <4; i =i+1){
        printf("%d\n",i);
    }
    return 0;
}
