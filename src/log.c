#include "../include/log.h"

void logInit()
{
    FILE *file = fopen("log.txt", "w");
    fclose(file);
}

void messageLog(const char *str)
{
    FILE *file = fopen("log.txt", "a");
    fprintf(file, "%s", str);
    fclose(file);
}

void integerLog(int x)
{
    FILE *file = fopen("log.txt", "a");
    fprintf(file, "%d", x);
    fclose(file);
}