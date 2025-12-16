#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>

int main() {
    char *libs[] = {"./lib1.so", "./lib2.so"};
    int cur = 0;
    
    void *handle = dlopen(libs[cur], RTLD_LAZY);
    if (!handle) {
        printf("Ошибка загрузки библиотеки: %s\n", dlerror());
        return 1;
    }
    
    float (*PiFunc)(int) = dlsym(handle, "Pi");
    int* (*SortFunc)(int*, int) = dlsym(handle, "Sort");

    printf("  0           - переключить библиотеку\n");
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
        
        if (cmd == 0) {
            dlclose(handle);
            cur = 1 - cur;
            
            handle = dlopen(libs[cur], RTLD_LAZY);
            if (!handle) {
                printf("Ошибка загрузки библиотеки: %s\n", dlerror());
                return 1;
            }
            
            PiFunc = dlsym(handle, "Pi");
            SortFunc = dlsym(handle, "Sort");
            
            printf("Библиотека переключена на реализацию %d\n", cur + 1);
        }
        
        else if (cmd == 1) {
            int K;
            scanf("%d", &K);
            printf("Pi = %f (библиотека %d)\n", PiFunc(K), cur + 1);
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
                
                int* sorted = SortFunc(numbers, count);
                
                printf("Отсортированный: ");
                for (int i = 0; i < count; i++) printf("%d ", sorted[i]);
                printf(" (библиотека %d)\n", cur + 1);
                
                free(sorted);
            }
        }
        
        else {
            printf("нет такой команды\n");
        }
    }
    
    dlclose(handle);
    return 0;
}