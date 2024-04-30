#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main(){
    char str[10000] = "";
    fgets(str, 10000, stdin);
    printf("%s\n", str);
    printf("\nNew!!!\n");
    return 1;
}