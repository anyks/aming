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
#include <fstream>
#include <unistd.h>
#include <time.h>
#include <zlib.h>
#include <pwd.h>
#include <grp.h>
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
		// Пользователь от которого устанавливаются права на каталог
		string user;
		// Группа к которой принадлежит пользователь
		string group;
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
		 * getOsName Функция определения операционной системы
		 * @return название операционной системы
		 */
		const char * getOsName();
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
		 * welcome Функция выводящая приглашение
		 * @param appname     название приложения
		 * @param name        пользовательское название
		 * @param version     версия приложения
		 * @param host        хост на котором поднято приложение
		 * @param ipv4        активация IPv4
		 * @param ipv6        активация IPv6
		 * @param gzip_t      активация обмена сжатыми данными
		 * @param gzip_r      сжимать полученные не сжатые данные
		 * @param smart       активация умного прокси
		 * @param keepalive   активация постоянных подключений
		 * @param http        тип поднятого прокси
		 * @param socks5      тип поднятого прокси
		 * @param connect     активация коннект прокси
		 * @param maxcon      максимальное количество подключений
		 * @param http_port   порт http прокси
		 * @param socks5_port порт socks5 прокси
		 * @param copyright   копирайт автора прокси
		 * @param site        сайт автора прокси
		 * @param email       адрес электронной почты автора
		 * @param support     адрес электронной почты службы поддержки
		 * @param author      ник или имя автора
		 */
		void welcome(
			const char * appname,
			const char * name,
			const char * version,
			const char * host,
			bool ipv4,
			bool ipv6,
			bool gzip_t,
			bool gzip_r,
			bool smart,
			bool keepalive,
			bool http,
			bool socks5,
			bool connect,
			int maxcon,
			u_int http_port,
			u_int socks5_port,
			const char * copyright,
			const char * site,
			const char * email,
			const char * support,
			const char * author
		);
		/**
		 * LogApp Конструктор log класса
		 * @param type    тип логов (TOLOG_FILES - запись в файл, TOLOG_CONSOLE - запись в коносль, TOLOG_DATABASE - запись в базу данных)
		 * @param name    название системы
		 * @param dir     адрес куда следует сохранять логи
		 * @param size    размер файла лога
		 * @param enabled активирован модуль или деактивирован
		 * @param user    пользователь от которого устанавливается права на каталог
		 * @param group   группа к которому принадлежит пользователь
		 */
		LogApp(u_short type = TOLOG_CONSOLE, const char * name = "anyks", const char * dir = DIR_LOG, size_t size = SIZE_LOG, bool enabled = true, string user = "", string group = "");
};

#endif // _LOG_ANYKS_
