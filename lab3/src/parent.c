#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>

#define SHARED_SIZE 4096

int main() {
    char buffer[1024];
    char outfile[100];

    printf("Введите имя файла для результатов: ");
    fgets(outfile, sizeof(outfile), stdin);
    outfile[strcspn(outfile, "\n")] = 0;

    FILE* test = fopen(outfile, "w");
    if (!test) {
        perror("fopen");
        exit(1);
    }
    fclose(test);

    int fd = open("shared.dat", O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    ftruncate(fd, SHARED_SIZE);

    char* shared_data = mmap(NULL, SHARED_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        exit(1);
    }

    pid_t pid = fork();

    if (pid == 0) {
        char parent_pid_str[20];
        sprintf(parent_pid_str, "%d", getppid());

        execl("./child", "child", "shared.dat", outfile, parent_pid_str, NULL);
        perror("execl");
        exit(1);
    }

    printf("Введите числа:\n");

    while (1) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
            break;

        strcpy(shared_data, buffer);

        kill(pid, SIGUSR1);

        int status;
        pid_t res = waitpid(pid, &status, WNOHANG);
        if (res != 0) {
            printf("Дочерний процесс завершён.\n");
            break;
        }
    }

    munmap(shared_data, SHARED_SIZE);
    close(fd);

    wait(NULL);
    return 0;
}
