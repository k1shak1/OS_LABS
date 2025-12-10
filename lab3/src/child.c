#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define SHARED_SIZE 4096

static char* shared_data = NULL;
static int should_exit = 0;
static FILE* outfile = NULL;
static pid_t parent_pid = 0;

void handle_signal(int sig) {
    if (sig == SIGTERM) {
        should_exit = 1;
        return;
    }

    if (sig != SIGUSR1 || !shared_data) return;

    char buffer[SHARED_SIZE];
    strcpy(buffer, shared_data);

    int first, num;
    int result;
    int count = 0;

    char* ptr = buffer;

    if (sscanf(ptr, "%d", &first) != 1)
        return;

    result = first;
    count++;

    fprintf(outfile, "%d", first);

    while (*ptr && *ptr != ' ') ptr++;
    while (*ptr == ' ') ptr++;

    while (sscanf(ptr, "%d", &num) == 1) {
        fprintf(outfile, " / %d", num);

        if (num == 0) {
            fprintf(outfile, " - Деление на 0! Завершение работы.\n");
            fflush(outfile);

            // Завершаем родителя
            kill(parent_pid, SIGTERM);
            should_exit = 1;
            return;
        }

        result /= num;
        count++;

        while (*ptr && *ptr != ' ') ptr++;
        while (*ptr == ' ') ptr++;
    }

    if (count > 1)
        fprintf(outfile, " = %d\n", result);
    else
        fprintf(outfile, "\n");

    fflush(outfile);
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: child <shared_file> <output_file> <parent_pid>\n");
        return 1;
    }

    const char* shared_name = argv[1];
    const char* outname = argv[2];
    parent_pid = atoi(argv[3]);

    outfile = fopen(outname, "w");
    if (!outfile) {
        perror("fopen");
        return 1;
    }

    int fd = open(shared_name, O_RDONLY);
    if (fd < 0) {
        perror("open shared file");
        fclose(outfile);
        return 1;
    }

    shared_data = mmap(NULL, SHARED_SIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (shared_data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        fclose(outfile);
        return 1;
    }

    close(fd);

    signal(SIGUSR1, handle_signal);
    signal(SIGTERM, handle_signal);

    while (!should_exit)
        pause();

    munmap(shared_data, SHARED_SIZE);
    fclose(outfile);

    return 0;
}
