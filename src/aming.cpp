/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  copyright:  © 2017 anyks.com
*/

#include "proxy/proxy.h"

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
	// Запускаем прокси сервер
	Proxy proxy(configfile);
	// Выходим
	return 0;
}
