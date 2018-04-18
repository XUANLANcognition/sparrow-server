#include <stdio.h>

int main(int argc, char *argv[], char *envp[]){
    fprintf(stdout, "<h1>%d + %d = %d</h1>", atoi(argv[1]), atoi(argv[2]), atoi(argv[1]) + atoi(argv[2]));
    return 0;
}
