/* МОДУЛЬ УПРАВЛЕНИЯ СИСТЕМНЫМИ НАСТРОЙКАМИ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _SYSTEM_PARAMS_ANYKS_
#define _SYSTEM_PARAMS_ANYKS_

#include <string>
#include <iostream>
#include "../os/os.h"
#include "../log/log.h"
#include "../config/conf.h"

// Устанавливаем область видимости
using namespace std;

/**
 * System Класс управляющий системными настройками
 */
class System {
	private:
		// Адрес конфигурационного файла
		string configfile;
	public:
		// Объект log модуля
		LogApp * log = NULL;
		// Объект конфигурационного файла
		Config * config = NULL;
		// Объект взаимодействия с ОС
		Os * os = NULL;
		/**
		 * reload Метод перезагрузки конфигурационных данных
		 */
		void reload();
		/**
		 * setConfigFile Метод установки адреса конфигурационного файла
		 * @param configfile адрес конфигурационного файла
		 */
		void setConfigFile(const string configfile);
		/**
		 * System Конструктор
		 * @param configfile адрес конфигурационного файла
		 */
		System(const string configfile);
		/**
		 * ~System Деструктор
		 */
		~System();

};

#endif // _SYSTEM_PARAMS_ANYKS_