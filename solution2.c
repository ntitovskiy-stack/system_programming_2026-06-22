/*В текущем каталоге есть 2 канала in1 in2, в которые в случайном порядке поступают числа, которые необходимо просуммировать и вывести окончательную сумму на экран. 
Сумма выводится в отдельной строке, завершающейся символом конца строки. Признаком окончания подачи символов в каналы является закрытие этих каналов посылающей стороной.
Подсказка: для неблокирующего чтения использовать select.
Замечание: ﻿протокол обмена по каналу текстовый, то есть числа представлены строками*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <errno.h>
#include <string.h>

#define BUFFER_SIZE 4096

// Функция для парсинга числа из строки
long long parse_number(const char *str, int len) {
    long long num = 0;
    for (int i = 0; i < len; i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            num = num * 10 + (str[i] - '0');
        }
    }
    return num;
}

int main(void) {
    // Открываем каналы
    int fd1 = open("in1", O_RDONLY | O_NONBLOCK);
    int fd2 = open("in2", O_RDONLY | O_NONBLOCK);
    if (fd1 == -1 || fd2 == -1) {
        perror("open");
        return 1;
    }
    long long sum = 0;
    char buffer[BUFFER_SIZE];
    int active = 2;
    while (active > 0) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        int max_fd = -1;
        if (fd1 != -1) {
            FD_SET(fd1, &read_fds);
            max_fd = fd1;
        }
        if (fd2 != -1) {
            FD_SET(fd2, &read_fds);
            if (fd2 > max_fd) max_fd = fd2;
        }
        if (max_fd == -1) break;
        int ready = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (ready == -1) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }
        // Обработка первого канала
        if (fd1 != -1 && FD_ISSET(fd1, &read_fds)) {
            ssize_t n = read(fd1, buffer, BUFFER_SIZE - 1);
            if (n == -1) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("read");
                }
            } 
            else if (n == 0) {
                close(fd1);
                fd1 = -1;
                active--;
            } 
            else {
                buffer[n] = '\0';
                // Ищем числа в буфере
                int start = 0;
                for (int i = 0; i <= n; i++) {
                    if (buffer[i] == '\n' || buffer[i] == '\0') {
                        if (i > start) {
                            sum += parse_number(buffer + start, i - start);
                        }
                        start = i + 1;
                    }
                }
            }
        }
        // Обработка второго канала
        if (fd2 != -1 && FD_ISSET(fd2, &read_fds)) {
            ssize_t n = read(fd2, buffer, BUFFER_SIZE - 1);
            if (n == -1) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("read");
                }
            } 
            else if (n == 0) {
                close(fd2);
                fd2 = -1;
                active--;
            } 
            else {
                buffer[n] = '\0';
                int start = 0;
                for (int i = 0; i <= n; i++) {
                    if (buffer[i] == '\n' || buffer[i] == '\0') {
                        if (i > start) {
                            sum += parse_number(buffer + start, i - start);
                        }
                        start = i + 1;
                    }
                }
            }
        }
    }
    printf("%lld\n", sum);
    return 0;
}
