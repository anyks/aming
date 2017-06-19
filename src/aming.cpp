/* СЕРВЕР ПРОКСИ AMING */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2016 - 2017
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