#include <stdio.h>

int main(int argc, char *argv[], char *envp[]){
    fprintf(stdout, "<h1>%s</h2>\n", argv[0]);
    fprintf(stdout, "<h1>%s</h2>\n", argv[1]);
    fprintf(stdout, "<h1>%s</h2>\n", argv[2]);
    return 0;
}
