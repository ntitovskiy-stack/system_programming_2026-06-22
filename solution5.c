/* В системе существуют 2 региона разделяемой памяти, заполненной некоторыми числами (типа int). Каждый из регионов имеет размер 1000 байт. Вам требуется разработать приложение, 
которое попарно суммирует первые 100 чисел в этих регионах и помещает суммы в новый (созданный вашим приложением) регион памяти размером 1000 байт. 
Таким образом, после завершения работы Вашего приложения в памяти должен существовать регион разделяемой памяти размером 1000 байт, содержащий в начале 100 сумм. 
Перед завершением работы приложение выводит в стандартный поток ввода-вывода ключ созданного региона, завершающийся символом конца строки. На вход ваше приложение принимает ключи существующих регионов памяти. */

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define SHM_SIZE 1000
#define NUM_INTS (SHM_SIZE / sizeof(int))
int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <key1> <key2>\n", argv[0]);
		return 1;
	}
	// Преобразуем ключи из строк в int
	key_t key1 = atoi(argv[1]);
	key_t key2 = atoi(argv[2]);
	if (key1 == 0 || key2 == 0) {
		fprintf(stderr, "Invalid keys\n");
		return 1;
	}
	// Получаем доступ к существующим регионам разделяемой памяти
	int shm_id1 = shmget(key1, SHM_SIZE, 0666);
	if (shm_id1 == -1) {
		perror("shmget key1");
		return 1;
	}
	int shm_id2 = shmget(key2, SHM_SIZE, 0666);
	if (shm_id2 == -1) {
		perror("shmget key2");
		return 1;
	}
	// Присоединяем существующие регионы к адресному пространству
	int *data1 = (int *)shmat(shm_id1, NULL, 0);
	if (data1 == (void *)-1) {
		perror("shmat key1");
		return 1;
	}
	int *data2 = (int *)shmat(shm_id2, NULL, 0);
	if (data2 == (void *)-1) {
		perror("shmat key2");
		shmdt(data1);
		return 1;
	}
	// Генерируем уникальный ключ для нового региона
	key_t new_key;
	int shm_id_new;
	// Пробуем создать регион с ключом, основанным на PID и времени для избежания конфликтов
	int attempt;
	for (attempt = 0; attempt < 100; attempt++) {
		// Используем комбинацию PID и счетчика попыток
		new_key = (getpid() << 16) | attempt;
		// Пробуем создать новый регион разделяемой памяти (IPC_CREAT | IPC_EXCL)
		shm_id_new = shmget(new_key, SHM_SIZE, IPC_CREAT | IPC_EXCL | 0666);
		if (shm_id_new != -1) {
			break;
		}
		if (errno != EEXIST) {
			perror("shmget new");
			shmdt(data1);
			shmdt(data2);
			return 1;
		}
	}
	if (shm_id_new == -1) {
		fprintf(stderr, "Failed to create new shared memory region\n");
		shmdt(data1);
		shmdt(data2);
		return 1;
	}
	// Присоединяем новый регион
	int *result = (int *)shmat(shm_id_new, NULL, 0);
	if (result == (void *)-1) {
		perror("shmat new");
		shmctl(shm_id_new, IPC_RMID, NULL);
		shmdt(data1);
		shmdt(data2);
		return 1;
	}
	// Суммируем первые 100 чисел
	int i;
	for (i = 0; i < 100; i++) {
		result[i] = data1[i] + data2[i];
	}
	// Отсоединяемся от всех регионов
	shmdt(data1);
	shmdt(data2);
	shmdt(result);
	// Выводим ключ созданного региона
	printf("%d\n", new_key);
	return 0;
}
