/* МОДУЛЬ ОПТИМИЗАЦИИ ОПЕРАЦИОННОЙ СИСТЕМЫ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2016
*/
#ifndef _OS_ANYKS_
#define _OS_ANYKS_

#include <regex>
#include <string>
#include <iostream>
#include <stdlib.h>
#include "log.h"

// Устанавливаем область видимости
using namespace std;

/**
 * OsOpt Класс оптимизации операционной системы
 */
class OsOpt {
	private:
		/**
		 * OsData Данные операционной системы
		 */
		struct OsData {
			string	name;	// Название операционной системы
			u_int	type;	// Тип операционной системы
		} __attribute__((packed));
		// Объект лога
		LogApp * log = NULL;
		// Активация модуля
		bool enabled = false;
		/**
		 * exec Метод запуска внешней оболочки
		 * @param cmd команда запуска
		 */
		string exec(string cmd);
		/**
		 * getOsName Функция определения операционной системы
		 * @return название операционной системы
		 */
		OsData getOsName();
		/**
		 * getCongestionControl Метод определения алгоритма сети
		 * @param  str строка с выводмом доступных алгоритмов из sysctl
		 * @return     строка с названием алгоритма
		 */
		string getCongestionControl(string str);
	public:
		/**
		 * run Метод запуска оптимизации
		 * @return результат работы
		 */
		void run();
		/**
		 * enable Метод активации модуля
		 */
		void enable();
		/**
		 * enable Метод деактивации модуля
		 */
		void disable();
		/**
		 * OsOpt Конструктор
		 * @param log     объект лога для вывода информации
		 * @param enabled модуль активирован или деактивирован
		 */
		OsOpt(LogApp * log = NULL, bool enabled = false);
};

#endif // _OS_ANYKS_
