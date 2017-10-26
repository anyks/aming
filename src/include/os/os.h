/* МОДУЛЬ ОПТИМИЗАЦИИ ОПЕРАЦИОННОЙ СИСТЕМЫ AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  copyright:  © 2017 anyks.com
*/

#ifndef _OS_AMING_
#define _OS_AMING_

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
#include "general/general.h"

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
		LogApp * log = nullptr;
		// Объект конфигурационного файла
		Config ** config = nullptr;
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
		 * @param config  объект конфигурационных файлов
		 * @param log     объект лога для вывода информации
		 */
		Os(Config ** config = nullptr, LogApp * log = nullptr);
};

#endif // _OS_AMING_
