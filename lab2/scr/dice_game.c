#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

typedef struct {
    int k;              
    int player1_score;  
    int player2_score;  
    int experiments;    
    int thread_id;      

    long player1_wins;
    long player2_wins;
    long draws;
} thread_data_t;

long global_player1_wins = 0;
long global_player2_wins = 0;
long global_draws = 0;

int roll_dice(unsigned int* seed) {
    return (rand_r(seed) % 6 + 1) + (rand_r(seed) % 6 + 1);
}

void* simulate_games(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    
    unsigned int seed = time(NULL) ^ data->thread_id;
    
    for (int exp = 0; exp < data->experiments; exp++) {
        int player1_current = data->player1_score;
        int player2_current = data->player2_score;
        
        for (int i = 0; i < data->k; i++) {
            player1_current += roll_dice(&seed);
            player2_current += roll_dice(&seed);
        }
        
        if (player1_current > player2_current) {
            data->player1_wins++;
        } else if (player2_current > player1_current) {
            data->player2_wins++;
        } else {
            data->draws++;
        }
    }
    
    return NULL;
}

void print_help() {
    printf("Исп: ./dice_game K раунд очки1 очки2 эксперименты потоки\nПример: ./dice_game 5 3 25 30 1000000 4\n");
}

int main(int argc, char* argv[]) {
    if (argc != 7) {
        printf("Ошибка: Неверное количество параметров!\n");
        printf("Ожидается 6 параметров, получено: %d\n\n", argc - 1);
        print_help();
        return 1;
    }
    
    int k = atoi(argv[1]);  // atoi преобразоует строку в число
    int current_round = atoi(argv[2]);
    int player1_total = atoi(argv[3]);
    int player2_total = atoi(argv[4]);
    int total_experiments = atoi(argv[5]);
    int max_threads = atoi(argv[6]);
    
    if (k <= 0 || current_round <= 0 || player1_total < 0 || player2_total < 0 || 
        total_experiments <= 0 || max_threads <= 0) {
        printf("Ошибка: Все параметры должны быть положительными числами!\n");
        printf("Очки игроков могут быть 0 или больше.\n");
        return 1;
    }
        
    pthread_t threads[max_threads];
    thread_data_t thread_data[max_threads];
    
    int experiments_per_thread = total_experiments / max_threads;
    int remaining_experiments = total_experiments % max_threads;
    
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    
    for (int i = 0; i < max_threads; i++) {
        thread_data[i].k = k;
        thread_data[i].player1_score = player1_total;
        thread_data[i].player2_score = player2_total;
        thread_data[i].experiments = experiments_per_thread;
        thread_data[i].thread_id = i;
        thread_data[i].player1_wins = 0;
        thread_data[i].player2_wins = 0;
        thread_data[i].draws = 0;
        
        if (i < remaining_experiments) {
            thread_data[i].experiments++;
        }
        
        if (pthread_create(&threads[i], NULL, simulate_games, &thread_data[i]) != 0) {
            perror("Ошибка при создании потока");
            return 1;
        }
    }
    
    for (int i = 0; i < max_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    for (int i = 0; i < max_threads; i++) {
        global_player1_wins += thread_data[i].player1_wins;
        global_player2_wins += thread_data[i].player2_wins;
        global_draws += thread_data[i].draws;
    }
    
    gettimeofday(&end_time, NULL);
    double execution_time = (end_time.tv_sec - start_time.tv_sec) + 
                          (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
    
    printf("=== РЕЗУЛЬТАТЫ ===\n");
    printf("Количество потоков: %d\n", max_threads);
    printf("Всего экспериментов: %d\n", total_experiments);
    printf("Побед игрока 1: %ld (%.2f%%)\n", 
           global_player1_wins, 
           (double)global_player1_wins / total_experiments * 100);
    printf("Побед игрока 2: %ld (%.2f%%)\n", 
           global_player2_wins, 
           (double)global_player2_wins / total_experiments * 100);
    printf("Ничьих: %ld (%.2f%%)\n", 
           global_draws, 
           (double)global_draws / total_experiments * 100);
    printf("Время выполнения: %.3f секунд\n", execution_time);
    
    return 0;
}