#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <time.h>

typedef struct product{
    char id[100];
    int count;
}Prod;

int main(int argc, char *argv[]){
    struct product Prod;
    strcpy(Prod.id, argv[1]);
    srand(time(NULL));
    for(int i = 0; i <= 150; i++){
        Prod.count = i;
        write(STDOUT_FILENO, &Prod, sizeof(struct product));
        double random_number = (double)rand() / RAND_MAX * (0.2 - 0.01) + 0.01;
        usleep(1000000 * random_number);
    }
    Prod.count = -1;
    write(STDOUT_FILENO, &Prod, sizeof(struct product));
    return 1;
}