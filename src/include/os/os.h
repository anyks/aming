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
#ifndef __linux__
#include <sys/sysctl.h>
#endif
#include <sys/resource.h>
#include "log/log.h"
#include "config/conf.h"

// Устанавливаем область видимости
using namespace std;

/**
 * Os Класс оптимизации операционной системы
 */
class Os {
	private:
		/**
		 * OsData Данные операционной системы
		 */
		struct OsData {
			string	name;	// Название операционной системы
			u_int	type;	// Тип операционной системы
		};
		// Объект лога
		LogApp * log = NULL;
		// Объект конфигурационного файла
		Config ** config = NULL;
// Если это не Linux
#ifndef __linux__
		/**
		 * getNumberParam Метод получения заначений ядра sysctl
		 * @param  name название параметра
		 * @return      значение параметра
		 */
		long getNumberParam(string name);
		/**
		 * getParam Метод получения заначений ядра sysctl
		 * @param  name название параметра
		 * @return      значение параметра
		 */
		string getStringParam(string name);
		/**
		 * setParam Метод установки значений ядра sysctl
		 * @param name  название параметра
		 * @param param данные параметра
		 */
		void setParam(string name, int param);
		/**
		 * setParam Метод установки значений ядра sysctl
		 * @param name  название параметра
		 * @param param данные параметра
		 */
		void setParam(string name, string param);
#endif
		/**
		 * enableCoreDumps Функция активации создания дампа ядра
		 * @return результат установки лимитов дампов ядра
		 */
		bool enableCoreDumps();
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
		 * @param cmd       команда запуска
		 * @param multiline данные должны вернутся многострочные
		 */
		string exec(string cmd, bool multiline = false);
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
		/**
		 * getCPU Метод получения данных процессора
		 */
		void getCPU();
	public:
		/**
		 * optimos Метод запуска оптимизации
		 * @return результат работы
		 */
		void optimos();
		/**
		 * set_fd_limit Функция установки количество разрешенных файловых дескрипторов
		 * @return количество установленных файловых дескрипторов
		 */
		int setFdLimit();
		/**
		 * mkPid Функция создания pid файла
		 */
		void mkPid();
		/**
		 * rmPid Функция удаления pid файла
		 * @param ext тип ошибки
		 */
		void rmPid(int ext);
		/**
		 * privBind Функция запускает приложение от имени указанного пользователя
		 */
		void privBind();
		/**
		 * Os Конструктор
		 * @param log     объект лога для вывода информации
		 * @param config  объект конфигурационных файлов
		 */
		Os(LogApp * log = NULL, Config ** config = NULL);
};

#endif // _OS_ANYKS_
