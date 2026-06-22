/*Некоторая утилита генерирует довольно большой вывод, а вам требуется всего-лишь посчитать количество символов '0' в этом выводе. Утилита при запуске принимает 1 параметр. 
Вам требуется разработать программу, которая вызывает указанную утилиту, с заданным параметром и подсчитывает количество символов '0' в ее выводе. 
Ваша программа принимает на вход 2 параметра -- имя утилиты, в текущем каталоге и ее параметр. Ваша программа должна после подсчета вывести найденное число '0' в отдельной строке, заканчивающейся символом конца строки.*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <utility> <parameter>\n", argv[0]);
        return 1;
    }
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    }
    if (pid == 0) {
        // Перенаправление stdout в pipe и запуск утилиты
        close(pipefd[0]);  // Закрытие чтения
        // Перенаправление stdout в pipe
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(1);
        }
        close(pipefd[1]);  // Закрытие оригинального дескриптора после dup2
        // Запуск утилиты с переданным параметром
        execl(argv[1], argv[1], argv[2], NULL);
        perror("execl");
        exit(1);
    } 
    else {
        // Чтение из pipe и считывание символов '0'
        close(pipefd[1]);  // Закрытие записи (нам нужно только читать)
        char buffer[4096];
        ssize_t bytes_read;
        long long count = 0;  // Счетчик символов '0'
        ssize_t i;
        // Чтение данных из pipe, пока есть что читать
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            for (i = 0; i < bytes_read; i++) {
                if (buffer[i] == '0') {
                    count++;  // Увеличение счетчика при обнаружении '0'
                }
            }
        }
        close(pipefd[0]);  // Закрытие чтения
        // Ожидание завершения дочернего процесса
        int status;
        waitpid(pid, &status, 0);
        // Вывод количества найденных символов '0'
        printf("%lld\n", count);
        return 0;
    }
}
