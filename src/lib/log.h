/* МОДУЛЬ ЛОГОВ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2016
*/
#ifndef _LOG_ANYKS_
#define _LOG_ANYKS_

#include <iostream>
#include <cstdio>
#include <string>
#include <time.h>
#include <zlib.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>

// Устанавливаем область видимости
using namespace std;

// Типы логов
#define LOG_ERROR 1
#define LOG_ACCESS 2
#define LOG_MESSAGE 3

// Типы хранилищь для логов
#define TOLOG_CONSOLE 0x01
#define TOLOG_FILES 0x02
#define TOLOG_DATABASE 0x04

// Адрес расположения логов
#define DIR_LOG "/var/log"
// Размер файла лога в кбт
#define SIZE_LOG 1024

/**
 * LogApp Класс для работы с логами
 */
class LogApp {
	private:
		// Название системы
		string name;
		// Адрес каталога где хранятся логи
		string dirlog;
		// Разрешено использовать лог или запрещено
		bool enabled;
		// Тип логов (1 - запись в файл, 2 - запись в коносль, 3 - запись в базу данных)
		u_short type;
		// Размер максимального лог файла в килобайтах
		size_t size;
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
		 */
		void write_to_file(u_short type, const char * message);
		/**
		 * write_to_console Функция записи лога в консоль
		 * @param type    тип лога (1 - Ошибка, 2 - Доступ, 3 - Сообщение)
		 * @param message сообщение для записи
		 */
		void write_to_console(u_short type, const char * message);
		/**
		 * write_to_db Функция записи лога в базу данных
		 * @param type    тип лога (1 - Ошибка, 2 - Доступ, 3 - Сообщение)
		 * @param message сообщение для записи
		 */
		void write_to_db(u_short type, const char * message);
	public:
		/**
		 * write Метод записи данных в лог
		 * @param type    тип лога (1 - Ошибка, 2 - Доступ, 3 - Сообщение)
		 * @param message текст сообщения
		 * @param ...     дополнительные параметры
		 */
		void write(u_short type, const char * message, ...);
		/**
		 * enable Метод активации модуля
		 */
		void enable();
		/**
		 * enable Метод деактивации модуля
		 */
		void disable();
		/**
		 * LogApp Конструктор log класса
		 * @param type    тип логов (TOLOG_FILES - запись в файл, TOLOG_CONSOLE - запись в коносль, TOLOG_DATABASE - запись в базу данных)
		 * @param name    название системы
		 * @param dir     адрес куда следует сохранять логи
		 * @param size    размер файла лога
		 * @param enabled активирован модуль или деактивирован
		 */
		LogApp(u_short type = TOLOG_CONSOLE, const char * name = "anyks", const char * dir = DIR_LOG, size_t size = SIZE_LOG, bool enabled = true);
};
#endif