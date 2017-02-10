/* МОДУЛЬ ЛОГОВ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _LOG_ANYKS_
#define _LOG_ANYKS_

#include <iostream>
#include <cstdio>
#include <string>
#include <thread>
#include <future>
#include <fstream>
#include <algorithm>
#include <unistd.h>
#include <time.h>
#include <zlib.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include "config/conf.h"

// Устанавливаем область видимости
using namespace std;

// Типы логов
#define LOG_ERROR 1
#define LOG_ACCESS 2
#define LOG_MESSAGE 3

// Типы хранилищь для логов
#define TOLOG_CONSOLE 0x01
#define TOLOG_FILES 0x02

/**
 * LogApp Класс для работы с логами
 */
class LogApp {
	private:
		// Название системы
		string name;
		// Разрешено использовать лог или запрещено
		bool enabled;
		// Тип логов (1 - запись в файл, 2 - запись в коносль, 3 - запись в базу данных)
		u_short type;
		// Размер максимального лог файла в килобайтах
		size_t size;
		// Конфигурационные данные
		Config ** config = NULL;
		/**
		 * is_number Функция проверки является ли строка числом
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
		 * setOwner Функция установки владельца на каталог
		 * @param path путь к файлу или каталогу для установки владельца
		 */
		void setOwner(const char * path);
		/**
		 * makePath Функция создания каталога для хранения логов
		 * @param  path адрес для каталога
		 * @return      результат создания каталога
		 */
		bool makePath(const char * path);
		/**
		 * isDirExist Функция проверки существования каталога
		 * @param  path адрес каталога
		 * @return      результат проверки
		 */
		bool isDirExist(const char * path);
		/**
		 * isFileExist Функция проверки существования файла
		 * @param  path адрес каталога
		 * @return      результат проверки
		 */
		bool isFileExist(const char * path);
		/**
		 * write_to_file Функция записи лога в файл
		 * @param type    тип лога (1 - Ошибка, 2 - Доступ, 3 - Сообщение)
		 * @param message сообщение для записи
		 * @param ctx     указатель на объект модуля логов
		 */
		static void write_to_file(u_short type, const char * message, void * ctx = NULL);
		/**
		 * write_to_console Функция записи лога в консоль
		 * @param type    тип лога (1 - Ошибка, 2 - Доступ, 3 - Сообщение)
		 * @param message сообщение для записи
		 * @param ctx     указатель на объект модуля логов
		 * @param sec     количество секунд через которое нужно вывести сообщение
		 */
		static void write_to_console(u_short type, const char * message, void * ctx, u_int sec = 0);
	public:
		/**
		 * write Метод записи данных в лог
		 * @param type    тип лога (1 - Ошибка, 2 - Доступ, 3 - Сообщение)
		 * @param sec     количество секунд через которое нужно вывести сообщение
		 * @param message текст сообщения
		 * @param ...     дополнительные параметры
		 */
		void write(u_short type, u_int sec, const char * message, ...);
		/**
		 * enable Метод активации модуля
		 */
		void enable();
		/**
		 * enable Метод деактивации модуля
		 */
		void disable();
		/**
		 * welcome Функция выводящая приглашение
		 */
		void welcome();
		/**
		 * LogApp Конструктор log класса
		 * @param config  конфигурационные данные
		 * @param type    тип логов (TOLOG_FILES - запись в файл, TOLOG_CONSOLE - запись в коносль)
		 */
		LogApp(Config ** config = NULL, u_short type = TOLOG_CONSOLE);
};

#endif // _LOG_ANYKS_
