
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>

#define MAX_CLIENTS 50
#define MAX_GROUPS 20
#define MAX_GROUP_MEMBERS 20
#define MAX_LOGIN_LEN 32
#define MAX_MSG_LEN 1024
#define SERVER_PIPE "/tmp/msg_server_pipe"

typedef struct {
    char login[MAX_LOGIN_LEN];
    int active;
    char pipe_name[64];
} Client;

typedef struct {
    char name[MAX_LOGIN_LEN];
    char members[MAX_GROUP_MEMBERS][MAX_LOGIN_LEN];
    int member_count;
    int active;
} Group;

Client clients[MAX_CLIENTS];
Group groups[MAX_GROUPS];
int running = 1;

void handle_sigint(int sig) {
    running = 0;
}

int find_client(const char *login) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && strcmp(clients[i].login, login) == 0) {
            return i;
        }
    }
    return -1;
}

int find_group(const char *name) {
    for (int i = 0; i < MAX_GROUPS; i++) {
        if (groups[i].active && strcmp(groups[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

void send_to_client(const char *pipe_name, const char *message) {
    int fd = open(pipe_name, O_WRONLY | O_NONBLOCK);
    if (fd != -1) {
        write(fd, message, strlen(message) + 1);
        close(fd);
    }
}

void handle_login(const char *login) {
    if (find_client(login) != -1) {
        printf("Клиент %s уже подключен\n", login);
        return;
    }
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i].active) {  // если клиент не активен
            strcpy(clients[i].login, login);
            snprintf(clients[i].pipe_name, 64, "/tmp/msg_client_%s", login); // создаем имя канал клиента
            clients[i].active = 1;
            printf("Клиент %s подключился\n", login);
            
            char response[MAX_MSG_LEN];
            snprintf(response, MAX_MSG_LEN, "SERVER: Успешное подключение как %s", login);
            send_to_client(clients[i].pipe_name, response);
            return;
        }
    }
    printf("Достигнут лимит клиентов\n");
}

void handle_logout(const char *login) {
    int idx = find_client(login);
    if (idx != -1) {
        clients[idx].active = 0;
        printf("Клиент %s отключился\n", login);
    }
}

void handle_message(const char *from, const char *to, const char *msg) { // отправка соо от кому
    int idx = find_client(to);
    if (idx == -1) {
        printf("Получатель %s не найден\n", to);
        int sender_idx = find_client(from);
        if (sender_idx != -1) {
            char err[MAX_MSG_LEN];
            snprintf(err, MAX_MSG_LEN, "ERROR: Пользователь %s не в сети", to);
            send_to_client(clients[sender_idx].pipe_name, err);
        }
        return;
    }
    
    char full_msg[MAX_MSG_LEN];
    snprintf(full_msg, MAX_MSG_LEN, "[%s]: %s", from, msg);
    send_to_client(clients[idx].pipe_name, full_msg);
    printf("Сообщение от %s к %s: %s\n", from, to, msg);
}

void handle_create_group(const char *creator, const char *group_name) {
    if (find_group(group_name) != -1) {
        int idx = find_client(creator);
        if (idx != -1) {
            char err[MAX_MSG_LEN];
            snprintf(err, MAX_MSG_LEN, "ERROR: Группа %s уже существует", group_name);
            send_to_client(clients[idx].pipe_name, err);
        }
        return;
    }
    
    for (int i = 0; i < MAX_GROUPS; i++) {
        if (!groups[i].active) {
            strcpy(groups[i].name, group_name);
            strcpy(groups[i].members[0], creator);
            groups[i].member_count = 1;
            groups[i].active = 1;
            printf("Группа %s создана пользователем %s\n", group_name, creator);
            
            int idx = find_client(creator);
            if (idx != -1) {
                char response[MAX_MSG_LEN];
                snprintf(response, MAX_MSG_LEN, "SERVER: Группа %s создана", group_name);
                send_to_client(clients[idx].pipe_name, response);
            }
            return;
        }
    }
}

void handle_join_group(const char *login, const char *group_name) {
    int g_idx = find_group(group_name);
    if (g_idx == -1) {
        int idx = find_client(login);
        if (idx != -1) {
            char err[MAX_MSG_LEN];
            snprintf(err, MAX_MSG_LEN, "ERROR: Группа %s не найдена", group_name);
            send_to_client(clients[idx].pipe_name, err);
        }
        return;
    }
    
    for (int i = 0; i < groups[g_idx].member_count; i++) {
        if (strcmp(groups[g_idx].members[i], login) == 0) {
            int idx = find_client(login);
            if (idx != -1) {
                char err[MAX_MSG_LEN];
                snprintf(err, MAX_MSG_LEN, "ERROR: Вы уже в группе %s", group_name);
                send_to_client(clients[idx].pipe_name, err);
            }
            return;
        }
    }
    
    if (groups[g_idx].member_count < MAX_GROUP_MEMBERS) {
        strcpy(groups[g_idx].members[groups[g_idx].member_count], login);
        groups[g_idx].member_count++;
        printf("%s присоединился к группе %s\n", login, group_name);
        
        int idx = find_client(login);
        if (idx != -1) {
            char response[MAX_MSG_LEN];
            snprintf(response, MAX_MSG_LEN, "SERVER: Вы присоединились к группе %s", group_name);
            send_to_client(clients[idx].pipe_name, response);
        }
    }
}

void handle_group_message(const char *from, const char *group_name, const char *msg) {
    int g_idx = find_group(group_name);
    if (g_idx == -1) {
        int idx = find_client(from);
        if (idx != -1) {
            char err[MAX_MSG_LEN];
            snprintf(err, MAX_MSG_LEN, "ERROR: Группа %s не найдена", group_name);
            send_to_client(clients[idx].pipe_name, err);
        }
        return;
    }
    
    char full_msg[MAX_MSG_LEN];
    snprintf(full_msg, MAX_MSG_LEN, "[%s@%s]: %s", from, group_name, msg);
    
    for (int i = 0; i < groups[g_idx].member_count; i++) {
        if (strcmp(groups[g_idx].members[i], from) != 0) {
            int c_idx = find_client(groups[g_idx].members[i]);
            if (c_idx != -1) {
                send_to_client(clients[c_idx].pipe_name, full_msg);
            }
        }
    }
    printf("Сообщение от %s в группу %s: %s\n", from, group_name, msg);
}

int main() {
    signal(SIGINT, handle_sigint);
    
    memset(clients, 0, sizeof(clients));
    memset(groups, 0, sizeof(groups));
    
    unlink(SERVER_PIPE);
    if (mkfifo(SERVER_PIPE, 0666) == -1) {
        perror("mkfifo");
        exit(1);
    }
    
    printf("Сервер запущен. Ожидание клиентов...\n");
    
    int server_fd = open(SERVER_PIPE, O_RDONLY | O_NONBLOCK);
    if (server_fd == -1) {
        perror("open");
        exit(1);
    }
    
    char buffer[MAX_MSG_LEN * 2];
    while (running) {
        int n = read(server_fd, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';
            
            char arg1[MAX_LOGIN_LEN], arg2[MAX_LOGIN_LEN];
            char msg[MAX_MSG_LEN];
            
            if (sscanf(buffer, "LOGIN %s", arg1) == 1) {
                handle_login(arg1);
            }
            else if (sscanf(buffer, "LOGOUT %s", arg1) == 1) {
                handle_logout(arg1);
            }
            else if (sscanf(buffer, "MSG %s %s %[^\n]", arg1, arg2, msg) == 3) {
                handle_message(arg1, arg2, msg);
            }
            else if (sscanf(buffer, "CREATE_GROUP %s %s", arg1, arg2) == 2) {
                handle_create_group(arg1, arg2);
            }
            else if (sscanf(buffer, "JOIN_GROUP %s %s", arg1, arg2) == 2) {
                handle_join_group(arg1, arg2);
            }
            else if (sscanf(buffer, "GROUP_MSG %s %s %[^\n]", arg1, arg2, msg) == 3) {
                handle_group_message(arg1, arg2, msg);
            }
        }
        usleep(10000);
    }
    
    close(server_fd);
    unlink(SERVER_PIPE);
    printf("\nСервер остановлен\n");
    
    return 0;
}
