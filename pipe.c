#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_EXPEDITIONS 10

volatile int ready_count = 0; // すべての子プロセスからの信号をカウント

void signal_handler(int signum){
    ready_count++;
    printf("Parent received signal that child is ready: Signal %i\n", signum);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <number_of_expeditions> <observation_duration>\n", argv[0]);
        return 1;
    }

    int number_of_expeditions = atoi(argv[1]);
    int observation_duration = atoi(argv[2]);
    int parent_to_child_fd[MAX_EXPEDITIONS][2];
    int child_to_parent_fd[MAX_EXPEDITIONS][2];
    char* identifications[] = {"Lion", "Bear", "Wolf", "Fox", "Deer"};
    pid_t pids[MAX_EXPEDITIONS];
    char buffer[100];

    signal(SIGUSR1, signal_handler);  // SIGUSR1 でカスタムハンドラを使用

    printf("Starting %d expeditions for %d seconds each.\n", number_of_expeditions, observation_duration);

    for (int i = 0; i < number_of_expeditions; i++) {
        if (pipe(parent_to_child_fd[i]) == -1 || pipe(child_to_parent_fd[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }        
        
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pids[i] == 0) {  // 子プロセス
            close(parent_to_child_fd[i][1]);  // 親からの書き込み用エンドを閉じる
            read(parent_to_child_fd[i][0], buffer, sizeof(buffer));  // 識別テキストの読み取り
            printf("Child %d ready to observe: %s\n", getpid(), buffer);
            close(parent_to_child_fd[i][0]);

            // 親プロセスに準備完了を通知
            char ready_message[100];
            sprintf(ready_message, "Child %d is ready", getpid());
            close(child_to_parent_fd[i][0]);
            write(child_to_parent_fd[i][1], ready_message, strlen(ready_message) + 1);
            close(child_to_parent_fd[i][1]);

            sleep(observation_duration); // 観察時間のシミュレーション
            exit(0);
        }
        // } else {
        //     close(parent_to_child_fd[i][0]);  // 子プロセスへの読み込み用エンドを閉じる
        //     write(parent_to_child_fd[i][1], identifications[i % 5], strlen(identifications[i % 5]) + 1);
        //     close(parent_to_child_fd[i][1]);  // 書き込み完了後に閉じる
        // }
    }

    // 親プロセスがすべての子プロセスから準備完了信号を受信するのを待つ
    while (ready_count < number_of_expeditions) {
        pause();
    }

    // 識別テキストの送信
    for (int i = 0; i < number_of_expeditions; i++) {
        close(child_to_parent_fd[i][1]);  // 書き込み用エンドを閉じる
        char confirm[100];
        read(child_to_parent_fd[i][0], confirm, sizeof(confirm));  // 子プロセスからの準備完了を確認
        printf("%s\n", confirm);
        close(child_to_parent_fd[i][0]);

        close(parent_to_child_fd[i][0]);  // 子プロセスへの読み込み用エンドを閉じる
        write(parent_to_child_fd[i][1], identifications[i % 5], strlen(identifications[i % 5]) + 1);
        close(parent_to_child_fd[i][1]);  // 書き込み完了後に閉じる
    }

    // 全ての子プロセスが準備完了するのを待つ
    for (int i = 0; i < number_of_expeditions; i++) {
        wait(NULL);
    }

    printf("Parent process has finished all observations\n");
    return 0;
}
