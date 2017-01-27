/* СЕРВЕР ПРОКСИ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2016
*/
/*
*
* MacOS X:
* # clang++ -Wall -O3 -pedantic -ggdb -g -std=c++11 -Werror=vla -lz -lpthread -o ./bin/http ./proxy/http.cpp ./lib/http/http.cpp ./lib/base64/base64.cpp ./lib/log/log.cpp ./lib/os/os.cpp ./lib/ini/ini.cpp ./lib/config/conf.cpp ./lib/dns/dns.cpp ./lib/system/system.cpp ./lib/proxy/proxy.cpp ./anyks.cpp -I/usr/local/include /usr/local/opt/libevent/lib/libevent.a
*
* Linux (requre = liblz-dev):
* # g++ -std=c++11 -ggdb -Wall -pedantic -O3 -Werror=vla -Wno-unused-result -lz -pthread -o ./bin/http ./proxy/http.cpp ./lib/http/http.cpp ./lib/base64/base64.cpp ./lib/log/log.cpp ./lib/os/os.cpp ./lib/ini/ini.cpp ./lib/config/conf.cpp ./lib/dns/dns.cpp ./lib/system/system.cpp ./anyks.cpp /usr/lib/x86_64-linux-gnu/libevent.a /usr/lib/gcc/x86_64-linux-gnu/4.9/libstdc++.a
* # gcc -std=c++11 -lm -ggdb -Wall -pedantic -pthread -O3 -Werror=vla -Wno-unused-result -o ./bin/http ./proxy/http.cpp ./lib/http/http.cpp ./lib/base64/base64.cpp ./lib/log/log.cpp ./lib/os/os.cpp ./lib/ini/ini.cpp ./lib/config/conf.cpp ./lib/dns/dns.cpp ./lib/system/system.cpp ./anyks.cpp /usr/lib/x86_64-linux-gnu/libevent.a /usr/lib/gcc/x86_64-linux-gnu/5/libstdc++.a /usr/lib/x86_64-linux-gnu/libz.a /usr/lib/x86_64-linux-gnu/libm.a
*
* FreeBSD:
* # clang++ -std=c++11 -D_BSD_SOURCE -ggdb -Wall -pedantic -O3 -Werror=vla -lz -lpthread -o ./bin/http ./proxy/http.cpp ./lib/http/http.cpp ./lib/base64/base64.cpp ./lib/log/log.cpp ./lib/os/os.cpp ./lib/ini/ini.cpp ./lib/config/conf.cpp ./lib/dns/dns.cpp ./lib/system/system.cpp ./anyks.cpp -I/usr/local/include /usr/local/lib/libevent.a
*
* Запуск: # ./bin/http -c ./config.ini
*
*/
#include <string>
#include "./lib/proxy/proxy.h"

// Устанавливаем пространство имен
using namespace std;

/**
 * main Главная функция приложения
 * @param  argc длина массива параметров
 * @param  argv массив параметров
 * @return      код выхода из приложения
 */
int main(int argc, char * argv[]){
	// Активируем локаль приложения
	// setlocale(LC_ALL, "");
	setlocale(LC_ALL, "en_US.UTF-8");
	// Адрес конфигурационного файла
	string configfile;

	configfile = (argc >= 2 ? argv[1] : "");

	/*
	// Определяем параметр запуска
	string param = (argc >= 2 ? argv[1] : "");
	// Если это параметр поиска конфигурационного файла
	if(param.compare("-c") == 0)
		// Ищем адрес конфигурационного файла
		configfile = (argc >= 3 ? argv[2] : "");
	// Если параметр конфигурационного файла найден
	else if(param.find("--config=") != string::npos)
		// Удаляем параметр из адреса файла
		configfile = param.replace(0, 9, "");
	*/
	// Запускаем прокси сервер
	Proxy proxy(configfile);
	// Выходим
	return 0;
}