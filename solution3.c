/* Разработать приложение, умеющее обрабатывать сигналы SIGUSR1 ,SIGUSR2, SIGTERM. После старта Ваше приложение должно по приходу одного из сигналов SIGUSR1,  
SIGUSR2 выполнять суммирование числа срабатываний каждого из сигналов, а после прихода сигнала SIGTERM, требуется вывести в стандартный поток вывода 2 числа, разделенных пробелом, 
соответствующих количеству обработанных сигналов SIGUSR1, SIGUSR2, и завершить программу. Вывод оканчивается символом конца строки. */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

// Использование volatile для защиты от оптимизаций
static volatile int sigusr1_count = 0;
static volatile int sigusr2_count = 0;
static volatile int sigterm_received = 0;

void handle_signal(int sig) {
	if (sig == SIGUSR1) {
		sigusr1_count++;
	} 
    else if (sig == SIGUSR2) {
		sigusr2_count++;
	} 
    else if (sig == SIGTERM) {
		sigterm_received = 1;
	}
}

int main(void) {
	struct sigaction sa;
	// Настройка обработчика для SIGUSR1
	sa.sa_handler = handle_signal;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGUSR1, &sa, NULL) == -1) {
		perror("sigaction SIGUSR1");
		return 1;
	}
	// Настройка обработчика для SIGUSR2
	if (sigaction(SIGUSR2, &sa, NULL) == -1) {
		perror("sigaction SIGUSR2");
		return 1;
	}
	// Настройка обработчика для SIGTERM
	if (sigaction(SIGTERM, &sa, NULL) == -1) {
		perror("sigaction SIGTERM");
		return 1;
	}
	// Блокировка всех сигналов, кроме SIGTERM, для sigwait
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGTERM);
	// Основной цикл: ожидание SIGTERM
	while (!sigterm_received) {
		int sig;
		// Ожидание только SIGTERM, другие сигналы будут обработаны асинхронно
		if (sigwait(&mask, &sig) == 0) {
			if (sig == SIGTERM) {
				sigterm_received = 1;
			}
		}
	}
	// Вывод результата
	printf("%d %d\n", sigusr1_count, sigusr2_count);
	return 0;
}
