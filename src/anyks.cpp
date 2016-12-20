// MacOS X
// g++ -std=c++11 -D_BSD_SOURCE -Wall -pedantic -O3 -Werror=vla -o ./bin/http ./proxy/http.cpp ./lib/http.cpp ./lib/base64.cpp ./anyks.cpp -I/usr/local/include /usr/local/opt/libevent/lib/libevent.a
// Linux
// g++ -std=c++11 -Wall -pedantic -O3 -Werror=vla -o ./bin/http ./proxy/http.cpp ./lib/http.cpp ./lib/base64.cpp ./anyks.cpp /usr/lib/x86_64-linux-gnu/libevent.a /usr/lib/gcc/x86_64-linux-gnu/4.9/libstdc++.a
// g++ -std=c++11 -Wall -pedantic -O3 -Werror=vla -o ./bin/http ./proxy/http.cpp ./lib/http.cpp ./lib/base64.cpp ./anyks.cpp /usr/lib/x86_64-linux-gnu/libevent.a /usr/lib/x86_64-linux-gnu/5/libstdc++.a
// FreeBSD
// clang++ -std=c++11 -D_BSD_SOURCE -Wall -pedantic -O3 -Werror=vla -o ./bin/http ./proxy/http.cpp ./lib/http.cpp ./lib/base64.cpp ./anyks.cpp -I/usr/local/include /usr/local/lib/libevent.a
#include <sys/wait.h>
#include <sys/signal.h>
#include "./proxy/http.h"

/**
 * sigpipe_handler Функция обработки сигнала SIGPIPE
 * @param signum номер сигнала
 */
void sigpipe_handler(int signum){
	// Выводим в консоль информацию
	cout << "Сигнал попытки записи в отключенный сокет!!!!" << endl;
}
/**
 * sigterm_handler Функция обработки сигналов завершения процессов SIGTERM
 * @param signum номер сигнала
 */
void sigterm_handler(int signum){
	// Выводим в консоль информацию
	cout << "Процесс убит!!!!" << endl;
	// Выходим
	exit(0);
}
/**
 * sighup_handler Функция обработки сигналов потери терминала SIGHUP
 * @param signum номер сигнала
 */
void sighup_handler(int signum){
	// Выводим в консоль информацию
	cout << "Терминал потерял связь!!!" << endl;
}
/**
 * main Главная функция приложения
 * @param  argc длина массива параметров
 * @param  argv массив параметров
 * @return      код выхода из приложения
 */
int main(int argc, char * argv[]){
	// Устанавливаем сигнал установки подключения
	signal(SIGPIPE, sigpipe_handler);	// Сигнал обрыва соединения во время записи
	signal(SIGTERM, sigterm_handler);	// Процесс убит
	signal(SIGHUP, sighup_handler);		// Терминал потерял связь
	// Создаем объект для http прокси-сервера
	HttpProxy * http = new HttpProxy();
	// Очищаем выделенный объект
	delete http;
	// Выходим
	return 0;
}