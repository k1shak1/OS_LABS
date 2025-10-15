#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int pipe1[2], pipe2[2];
    pid_t pid;
    char filename[100];
    char buffer[1024];
    
    printf("Введите имя файла: ");
    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        printf("Ошибка ввода имени файла\n");
        exit(1);
    }
    
    filename[strcspn(filename, "\n")] = '\0';
    
    FILE* test_file = fopen(filename, "w");
    if (test_file == NULL) {
        printf("Ошибка: невозможно создать файл '%s'\n", filename);
        exit(1);
    }
    fclose(test_file); 
    
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        printf("Ошибка создания каналов\n");
        exit(1);
    }
    
    pid = fork();
    if (pid == -1) {
        printf("Ошибка создания процесса\n");
        exit(1);
    }
    
    if (pid == 0) {
        close(pipe1[1]);
        close(pipe2[0]);

        dup2(pipe1[0], STDIN_FILENO);
        close(pipe1[0]);

        dup2(pipe2[1], STDOUT_FILENO);
        close(pipe2[1]);
        
        execl("./child", "child", filename, NULL);
        printf("Ошибка запуска дочерней программы\n");
        exit(1);
    } else {
        close(pipe1[0]);
        close(pipe2[1]);
        
        printf("Введите числа через пробел:\n");
        
        while (1) {
            if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                break;
            }
            
            write(pipe1[1], buffer, strlen(buffer));
            
            int status;
            pid_t result = waitpid(pid, &status, WNOHANG);
            if (result != 0) {
                printf("Дочерний процесс завершил работу\n");
                break;
            }
        }
        
        close(pipe1[1]);
        close(pipe2[0]);
        wait(NULL);
    }
    
    return 0;
}