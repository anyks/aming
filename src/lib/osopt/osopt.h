/* МОДУЛЬ ОПТИМИЗАЦИИ ОПЕРАЦИОННОЙ СИСТЕМЫ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _OS_ANYKS_
#define _OS_ANYKS_

#include <regex>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include "../log/log.h"

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
		 * set_fd_limit Функция установки количество разрешенных файловых дескрипторов
		 * @param  maxfd максимальное количество файловых дескрипторов
		 * @return       количество установленных файловых дескрипторов
		 */
		int setFdLimit(u_int maxfd);
		/**
		 * isNumber Функция проверки является ли строка числом
		 * @param  str строка для проверки
		 * @return     результат проверки
		 */
		bool isNumber(const string &str);
		/**
		 * getUid Функция вывода идентификатора пользователя
		 * @param  name имя пользователя
		 * @return      полученный идентификатор пользователя
		 */
		uid_t getUid(const char * name);
		/**
		 * getGid Функция вывода идентификатора группы пользователя
		 * @param  name название группы пользователя
		 * @return      полученный идентификатор группы пользователя
		 */
		gid_t getGid(const char * name);
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
		 * enableCoreDumps Функция активации создания дампа ядра
		 * @return результат установки лимитов дампов ядра
		 */
		bool enableCoreDumps();
		/**
		 * setPidfile Функция создания pid файла
		 * @param filename название файла pid
		 */
		void setPidfile(const char * filename);
		/**
		 * rmPidfile Функция удаления pid файла
		 * @param filename название файла pid
		 * @param ext      тип ошибки
		 */
		void rmPidfile(const char * filename, int ext);
		/**
		 * privBind Функция запускает приложение от имени указанного пользователя
		 * @param user  название или идентификатор пользователя
		 * @param group название или идентификатор группы пользователя
		 */
		void privBind(const string &user, const string &group);
		/**
		 * OsOpt Конструктор
		 * @param log     объект лога для вывода информации
		 * @param enabled модуль активирован или деактивирован
		 */
		OsOpt(LogApp * log = NULL, bool enabled = false);
};

#endif // _OS_ANYKS_
