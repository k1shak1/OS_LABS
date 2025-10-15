#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (argc != 2) return 1;

    FILE* out = fopen(argv[1], "w");

    char* line = NULL;
    size_t len = 0;

    while (getline(&line, &len, stdin) != -1) {
        int first_num, num;
        int result;
        int count = 0;
        char* ptr = line;

        if (sscanf(ptr, "%d", &first_num) != 1) continue;

        result = first_num;
        count++;
        fprintf(out, "%d", first_num); 

        while (*ptr && *ptr != ' ' && *ptr != '\n' && *ptr != '\t') ptr++;

        while (sscanf(ptr, "%d", &num) == 1) {
            fprintf(out, " / %d", num); 
            
            if (num == 0) {
                fprintf(out, " - Деление на 0! Завершение работы.\n");
                fclose(out);
                free(line);
                exit(1);
            }
            
            result /= num;
            count++;

            while (*ptr && *ptr != ' ' && *ptr != '\n' && *ptr != '\t') ptr++;
            while (*ptr == ' ' || *ptr == '\t') ptr++;
        }

        if (count > 1) {
            fprintf(out, " = %d\n", result);
        } else {
            fprintf(out, "\n"); 
        }
        fflush(out);
    }

    free(line);
    fclose(out);
    return 0;
}