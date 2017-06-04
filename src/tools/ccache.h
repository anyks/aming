/* УТИЛИТА ЧИСТКИ УСТАРЕВШИХ ФЙЛОВ КЭША ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _CACHE_CLEANER_ANYKS_
#define _CACHE_CLEANER_ANYKS_

#include <string>
#include <iostream>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "../include/config/conf.h"
#include "../include/general/general.h"

// Устанавливаем область видимости
using namespace std;

/**
 * CCache Класс для работы с устаревшим кэшем
 */
class CCache {
	private:
		// Объект конфигурационных данных
		Config * config = NULL;
		/**
		 * handler Прототип колбека
		 */
		typedef void (* handler) (const string filename, void * ctx);
		/**
		 * checkDomains Функция проверки кэша доменов
		 * @param filename адрес файла записи
		 * @param ctx      объект приложения
		 */
		static void checkDomains(const string filename, void * ctx);
		/**
		 * checkCache Функция проверки кэша данных
		 * @param filename адрес файла записи
		 * @param ctx      объект приложения
		 */
		static void checkCache(const string filename, void * ctx);
		/**
		 * infoPatch Функция определяющая тип адреса
		 * @param  pathName  адрес файла или каталога
		 * @return           результат проверки (-1 - Не определено, 0 - Файл, 1 - Каталог)
		 */
		const short infoPatch(const string pathName);
		/**
		 * processDirectory Рекурсивная функция обхода содержимого каталога
		 * @param  curDir адрес текущего каталога
		 * @param  fn     функция обработчик содержимого файла
		 * @return        результат работы
		 */
		const u_long processDirectory(const string curDir, handler fn);
	public:
		/**
		 * CCache Конструктор
		 * @param config адрес конфигурационного файла
		 */
		CCache(const string config);
		/**
		 * ~CCache Деструктор
		 */
		~CCache();
};

#endif // _CACHE_CLEANER_ANYKS_