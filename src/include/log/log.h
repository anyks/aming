/* МОДУЛЬ ЛОГОВ AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  copyright:  © 2017 anyks.com
*/

#ifndef _LOG_AMING_
#define _LOG_AMING_

#include <regex>
#include <cstdio>
#include <string>
#include <thread>
#include <future>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <ctime>
#include <zlib.h>
#include <sys/types.h>
#include "config/conf.h"
#include "general/general.h"

// Устанавливаем область видимости
using namespace std;

// Типы логов
#define LOG_ERROR 1
#define LOG_ACCESS 2
#define LOG_WARNING 3
#define LOG_MESSAGE 4

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
		// Разрешено использовать запись в лог данных запроса
		bool dataEnabled;
		// Разрешить вывод лога в файлы
		bool filesEnabled;
		// Разрешить вывод лога в консоль
		bool consoleEnabled;
		// Тип логов (1 - запись в файл, 2 - запись в коносль, 3 - запись в базу данных)
		u_short type;
		// Размер максимального лог файла в килобайтах
		size_t size;
		// Конфигурационные данные
		Config ** config = nullptr;
		/**
		 * write_data_to_file Функция записи в лога полученных данных в файл
		 * @param id   идентификатор записи
		 * @param data полученные данные
		 * @param ctx  указатель на объект модуля логов
		 */
		static void write_data_to_file(const string id, const string data, void * ctx = nullptr);
		/**
		 * write_to_file Функция записи лога в файл
		 * @param type    тип лога (1 - Ошибка, 2 - Доступ, 3 - Сообщение)
		 * @param message сообщение для записи
		 * @param ctx     указатель на объект модуля логов
		 */
		static void write_to_file(u_short type, const char * message, void * ctx = nullptr);
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
		 * write_data Метод записи данных запроса в лог
		 * @param id   идентификатор записи
		 * @param data данные для записи
		 */
		void write_data(const string id, const string data);
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
		LogApp(Config ** config = nullptr, u_short type = TOLOG_CONSOLE);
};

#endif // _LOG_AMING_
