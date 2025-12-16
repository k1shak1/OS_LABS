#include <stdio.h>
#include <stdlib.h>
#include "lib.h"

int main() {
    printf("  1 K         - вычислить Pi\n");
    printf("  2 a1 a2 ... - отсортировать числа\n");
    printf("  -1          - выход\n");
    printf("Введите команду: ");

    int cmd;
    
    while (1) {
        if (scanf("%d", &cmd) == EOF) break;
        
        if (cmd == -1) {
            printf("Выход\n");
            break;
        }
        
        if (cmd == 1) {
            int K;
            scanf("%d", &K);
            printf("Pi = %f\n", Pi(K));
        }
        
        else if (cmd == 2) {
            int numbers[100];
            int count = 0;
            int num;
            
            while (scanf("%d", &num) == 1 && count < 100) {
                numbers[count++] = num;
                if (getchar() == '\n') break;
            }
            
            if (count > 0) {
                printf("Исходный: ");
                for (int i = 0; i < count; i++) printf("%d ", numbers[i]);
                printf("\n");
                
                int* sorted = Sort(numbers, count);
                
                printf("Отсортированный: ");
                for (int i = 0; i < count; i++) printf("%d ", sorted[i]);
                printf("\n");
                
                free(sorted);
            }
        }
        
        else {
            printf("нет такой команды\n");
        }
    }
    
    return 0;
}