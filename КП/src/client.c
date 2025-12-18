
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <signal.h>

#define MAX_COMMAND_LEN 2048
#define MAX_LOGIN_LEN 32
#define MAX_MSG_LEN 1024
#define SERVER_PIPE "/tmp/msg_server_pipe"

char my_login[MAX_LOGIN_LEN];
char my_pipe[64];
int running = 1;

void *receive_thread(void *arg) {
    int fd = open(my_pipe, O_RDONLY);
    if (fd == -1) {
        perror("open client pipe");
        return NULL;
    }
    
    char buffer[MAX_MSG_LEN];
    while (running) {
        int n = read(fd, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';
            printf("\n%s\n> ", buffer);
            fflush(stdout);
        }
    }
    
    close(fd);
    return NULL;
}

void send_to_server(const char *message) {
    int fd = open(SERVER_PIPE, O_WRONLY);
    if (fd == -1) {
        printf("Ошибка: не удается подключиться к серверу\n");
        return;
    }
    write(fd, message, strlen(message) + 1);
    close(fd);
}

void print_help() {
    printf("\nДоступные команды:\n");
    printf("  /msg <логин> <сообщение>     - отправить личное сообщение\n");
    printf("  /create <имя_группы>         - создать группу\n");
    printf("  /join <имя_группы>           - присоединиться к группе\n");
    printf("  /gmsg <имя_группы> <сообщ>   - отправить сообщение в группу\n");
    printf("  /help                        - показать эту справку\n");
    printf("  /quit                        - выйти\n\n");
}

void handle_sigint(int sig) {
    running = 0;
}

int main() {
    signal(SIGINT, handle_sigint);
    
    printf("=== Клиент мгновенных сообщений ===\n");
    printf("Введите ваш логин: ");
    fgets(my_login, MAX_LOGIN_LEN, stdin);
    my_login[strcspn(my_login, "\n")] = 0;
    
    if (strlen(my_login) == 0) {
        printf("Некорректный логин\n");
        return 1;
    }
    
    snprintf(my_pipe, 64, "/tmp/msg_client_%s", my_login);
    unlink(my_pipe);
    
    if (mkfifo(my_pipe, 0666) == -1) {
        perror("mkfifo");
        return 1;
    }
    
    char login_msg[MAX_MSG_LEN];
    snprintf(login_msg, MAX_MSG_LEN, "LOGIN %s", my_login);
    send_to_server(login_msg);
    
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_thread, NULL);
    
    printf("\nВы подключены как '%s'\n", my_login);
    print_help();
    
    char input[MAX_MSG_LEN];
    char command[2048];
    
    while (running) {
        printf("> ");
        fflush(stdout);
        
        if (fgets(input, MAX_MSG_LEN, stdin) == NULL) {
            break;
        }
        
        input[strcspn(input, "\n")] = 0;
        
        if (strlen(input) == 0) {
            continue;
        }
        
        if (strcmp(input, "/quit") == 0) {
            break;
        }
        else if (strcmp(input, "/help") == 0) {
            print_help();
        }
        else if (strncmp(input, "/msg ", 5) == 0) {
            char to[MAX_LOGIN_LEN];
            char msg[MAX_MSG_LEN];
            
            if (sscanf(input + 5, "%s %[^\n]", to, msg) == 2) {
                snprintf(command, sizeof(command), "MSG %s %s %s", my_login, to, msg);
                send_to_server(command);
                printf("Сообщение отправлено пользователю %s\n", to);
            } else {
                printf("Использование: /msg <логин> <сообщение>\n");
            }
        }
        else if (strncmp(input, "/create ", 8) == 0) {
            char group_name[MAX_LOGIN_LEN];
            
            if (sscanf(input + 8, "%s", group_name) == 1) {
                snprintf(command, MAX_MSG_LEN, "CREATE_GROUP %s %s", my_login, group_name);
                send_to_server(command);
            } else {
                printf("Использование: /create <имя_группы>\n");
            }
        }
        else if (strncmp(input, "/join ", 6) == 0) {
            char group_name[MAX_LOGIN_LEN];
            
            if (sscanf(input + 6, "%s", group_name) == 1) {
                snprintf(command, MAX_MSG_LEN, "JOIN_GROUP %s %s", my_login, group_name);
                send_to_server(command);
            } else {
                printf("Использование: /join <имя_группы>\n");
            }
        }
        else if (strncmp(input, "/gmsg ", 6) == 0) {
            char group_name[MAX_LOGIN_LEN];
            char msg[MAX_MSG_LEN];
            
            if (sscanf(input + 6, "%s %[^\n]", group_name, msg) == 2) {
                snprintf(command, sizeof(command), "GROUP_MSG %s %s %s", my_login, group_name, msg);
                send_to_server(command);
                printf("Сообщение отправлено в группу %s\n", group_name);
            } else {
                printf("Использование: /gmsg <имя_группы> <сообщение>\n");
            }
        }
        else {
            printf("Неизвестная команда. Введите /help для справки\n");
        }
    }
    
    running = 0;
    
    char logout_msg[MAX_MSG_LEN];
    snprintf(logout_msg, MAX_MSG_LEN, "LOGOUT %s", my_login);
    send_to_server(logout_msg);
    
    pthread_join(recv_thread, NULL);
    
    unlink(my_pipe);
    printf("\nОтключение от сервера...\n");
    
    return 0;
}