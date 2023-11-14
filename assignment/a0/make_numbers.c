#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int main(int argc, char *argv[]) {
    srand((unsigned)time(NULL));
    if(argc < 4) {
        printf("More arguments need. Format: n lo lo\n");
        return 1;
    }
    if(argc > 4) {
        printf("Extra arguments input. Format: n hi lo\n");
        return 1;
    }

    int n = 0;
    int len = strlen(argv[1]);
    int ptr = 0;
    bool isPositive = true;
    bool isLeast = false;
    if(argv[1][0] == '-') {
        isPositive = false;
        ptr++;
    }
    for(ptr; ptr < len; ptr++) {
        n *= 10;
        n += (int)argv[1][ptr] - 48;
        if(ptr < len - 1) {
            if(n > 214748364) {
                printf("n out of INT range\n");
                return 1;
            } else if(n == 214748364) {
                if(isPositive && ((int)argv[1][ptr+1]-48)>7) {
                    printf("n out of INT range\n");
                    return 1;
                } else if(!isPositive && ((int)argv[1][ptr+1]-48)>8) {
                    printf("n out of INT range\n");
                    return 1;
                } else if(!isPositive && ((int)argv[1][ptr+1]-48)==8) {
                    isLeast = true;
                }
            }
        }
    }
    if(!isPositive && !isLeast)
        n = 0 - n;
    if(isLeast)
        n = INT_MIN;

    int lo = 0;
    len = strlen(argv[2]);
    ptr = 0;
    isPositive = true;
    isLeast = false;
    if(argv[2][0] == '-') {
        isPositive = false;
        ptr++;
    }
    for(ptr; ptr < len; ptr++) {
        lo *= 10;
        lo += (int)argv[2][ptr] - 48;
        if(ptr < len - 1) {
            if(lo > 214748364) {
                printf("lo out of INT range\n");
                return 1;
            } else if(lo == 214748364) {
                if(isPositive && ((int)argv[2][ptr+1]-48)>7) {
                    printf("lo out of INT range\n");
                    return 1;
                } else if(!isPositive && ((int)argv[2][ptr+1]-48)>8) {
                    printf("lo out of INT range\n");
                    return 1;
                } else if(!isPositive && ((int)argv[2][ptr+1]-48)==8) {
                    isLeast = true;
                }
            }
        }
    }
    if(!isPositive)
        lo = 0 - lo;
    if(isLeast)
        lo = INT_MIN;

    int hi = 0;
    ptr = 0;
    isPositive = true;
    isLeast = false;
    len = strlen(argv[3]);
    if(argv[3][0] == '-') {
        isPositive = false;
        ptr++;
    }
    for(ptr; ptr < len; ptr++) {
        hi *= 10;
        hi += (int)argv[3][ptr] - 48;
        if(ptr < len - 1) {
            if(hi > 214748364) {
                printf("hi out of INT range\n");
                return 1;
            } else if(hi == 214748364) {
                if(isPositive && ((int)argv[3][ptr+1]-48)>7) {
                    printf("hi out of INT range\n");
                    return 1;
                } else if(!isPositive && ((int)argv[3][ptr+1]-48)>8) {
                    printf("hi out of INT range\n");
                    return 1;
                } else if(!isPositive && ((int)argv[3][ptr+1]-48)==8) {
                    isLeast = true;
                }
            }
        }
    }
    if(!isPositive)
        hi = 0 - hi;
    if(isLeast)
        hi = INT_MIN;

    FILE *f = fopen("log.txt", "w");

    if(f == NULL) {
        printf("Fail to open the file log.txt\n");
        return 1;
    }

    if(hi == lo) {
        fprintf(f, "%d\n", n);
        for(int i = 0; i < n; ++i)
            fprintf(f, "%d\n", hi);
    } else if (hi > lo) {
        fprintf(f, "%d\n", n);
        for(int i = 0; i < n; ++i){
            int rand_num = rand();
            int range = hi - lo + 1;
            rand_num = lo + (rand_num % range);
            fprintf(f, "%d\n", rand_num);
        }
    } else {
        printf("Error: hi is less than lo\nrequirement: hi > lo\nFormat: n lo hi\n");
        fclose(f);
        return 1;
    }

    fclose(f);

    return 0;
}
