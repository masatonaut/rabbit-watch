#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

void signal_handler(int signum){
    printf("Signal with number %i has arrived\n", signum);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <number_of_expeditions> <observation_duration>\n", argv[0]);
        return 1;
    }

    int number_of_expeditions = atoi(argv[1]);
    int observation_duration = atoi(argv[2]);

    signal(SIGUSR1, signal_handler);  // SIGUSR1 でカスタムハンドラを使用

    printf("Starting %d expeditions for %d seconds each.\n", number_of_expeditions, observation_duration);

    for (int i = 0; i < number_of_expeditions; i++) {
        pid_t pid = fork();
        if (pid == 0) {  // 子プロセス
            printf("Child process started, will notify the parent after arriving.\n");
            sleep(1);  // 到着までのシミュレーション時間
            kill(getppid(), SIGUSR1);  // 親プロセスに到着通知
            sleep(observation_duration);  // 観察時間
            exit(0);
        } else if (pid > 0) {  // 親プロセス
            pause();  // シグナルを待つ
            printf("Parent received signal from child.\n");
        } else {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all children to complete
    while (wait(NULL) > 0);
    printf("All expeditions have completed.\n");
    return 0;
}
