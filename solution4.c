/*В задании требуется доработать демон, разработанный ранее в задании 6 модуля 3.5. 
Задача -- снабдить демон обработчиком сигнала SIGURG, по приходу которого демон должен завершать свою работу.*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>

static volatile sig_atomic_t should_exit = 0;

void handle_sigurg(int sig) {
	if (sig == SIGURG) {
		should_exit = 1;
	}
}

int main(void) {
	pid_t pid;
	
	// Первый fork
	pid = fork();
	if (pid < 0) {
		perror("fork");
		return 1;
	}
	if (pid > 0) {
		// Родительский процесс завершается
		return 0;
	}
	
	// Создаем новую сессию - процесс становится лидером сессии
	if (setsid() < 0) {
		perror("setsid");
		return 1;
	}
	
	// Устанавливаем маску режима создания файлов
	umask(0);
	
	// Переходим в корневой каталог
	if (chdir("/") < 0) {
		perror("chdir");
		return 1;
	}
	
	// Закрываем все открытые файловые дескрипторы
	int fd;
	int max_fd = sysconf(_SC_OPEN_MAX);
	if (max_fd == -1) {
		max_fd = 1024;
	}
	for (fd = 0; fd < max_fd; fd++) {
		close(fd);
	}
	
	// Перенаправляем stdin, stdout, stderr на /dev/null
	int fd_null = open("/dev/null", O_RDWR);
	if (fd_null != -1) {
		dup2(fd_null, STDIN_FILENO);
		dup2(fd_null, STDOUT_FILENO);
		dup2(fd_null, STDERR_FILENO);
		if (fd_null > 2) {
			close(fd_null);
		}
	}
	
	// Получаем PID демона
	pid_t daemon_pid = getpid();
	
	// Открываем системный лог
	openlog("solution", LOG_PID, LOG_DAEMON);
	syslog(LOG_INFO, "Daemon started with PID %d", daemon_pid);
	
	// Настраиваем обработчик сигнала SIGURG
	struct sigaction sa;
	sa.sa_handler = handle_sigurg;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	
	if (sigaction(SIGURG, &sa, NULL) == -1) {
		syslog(LOG_ERR, "Failed to set SIGURG handler: %s", strerror(errno));
		closelog();
		return 1;
	}
	
	// Выводим PID
	char pid_str[32];
	int len = snprintf(pid_str, sizeof(pid_str), "%d\n", daemon_pid);
	write(STDOUT_FILENO, pid_str, len);
	
	syslog(LOG_INFO, "Daemon running, waiting for SIGURG");
	
	// Блокируем SIGURG для использования sigwait
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGURG);
	sigprocmask(SIG_BLOCK, &mask, NULL);
	
	// Основной цикл ожидания SIGURG
	while (!should_exit) {
		int sig;
		if (sigwait(&mask, &sig) == 0) {
			if (sig == SIGURG) {
				should_exit = 1;
				syslog(LOG_INFO, "Received SIGURG, exiting");
			}
		}
	}
	
	// Завершаем работу
	syslog(LOG_INFO, "Daemon exiting");
	closelog();
	
	return 0;
}
