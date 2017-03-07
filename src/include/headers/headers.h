/* МОДУЛЬ УПРАВЛЕНИЯ ЗАГОЛОВКАМИ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _HEADERS_ANYKS_
#define _HEADERS_ANYKS_

#include <map>
#include <regex>
#include <string>
#include <vector>
#include <iostream>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>
#include <sys/file.h>
#include "log/log.h"
#include "http/http.h"

// Устанавливаем область видимости
using namespace std;

/**
 * Headers Класс управления заголовками
 */
class Headers {
	private:
		/**
		 * Структура параметров заголовков
		 */
		struct Params {
			string action;
			string route;
			string method;
			string server;
			string regex;
			vector <string> headers;
		};
		// Объект лога
		LogApp * log = NULL;
		// Конфигурационные данные
		Config ** config = NULL;
		// Список правил
		map <string, vector <Params>> rules;
		/**
		 * add Метод добавления новых параметров фильтрации заголовков
		 * @param client идентификатор клиента
		 * @param params параметры фильтрации
		 */
		void add(const string client, Params params);
		/**
		 * rm Метод удаления параметров фильтрации заголовков
		 * @param client идентификатор клиента
		 */
		void rm(const string client);
		/**
		 * read Метод чтения из файла параметров
		 */
		void read();
		/**
		 * addToPath Метод формирования адреса из пути и названия файла
		 * @param  path путь где хранится файл
		 * @param  file название файла
		 * @return      сформированный путь
		 */
		const string addToPath(const string path, const string file);
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
		 * isAddress Метод проверки на то является ли строка адресом
		 * @param  address строка адреса для проверки
		 * @return         результат проверки
		 */
		bool isAddress(const string address);
		/**
		 * isIpV4 Метод проверки на ip адрес, интернет протокола версии 4
		 * @param  ip строка ip адреса для проверки
		 * @return    результат проверки
		 */
		bool isIpV4(const string ip);
		/**
		 * isIpV6 Метод проверки на ip адрес, интернет протокола версии 6
		 * @param  ip строка ip адреса для проверки
		 * @return    результат проверки
		 */
		bool isIpV6(const string ip);
		/**
		 * isIp Метод проверки на ip адрес
		 * @param  ip строка ip адреса для проверки
		 * @return    результат проверки
		 */
		bool isIp(const string ip);
		/**
		 * isMac Метод проверки на mac адрес
		 * @param  mac строка mac адреса для проверки
		 * @return     результат проверки
		 */
		bool isMac(const string mac);
		/**
		 * isDomain Метод проверки на доменное имя
		 * @param  domain строка названия домена для проверки
		 * @return        результат проверки
		 */
		bool isDomain(const string domain);
		/**
		 * isLogin Метод проверки на логин пользователя
		 * @param  login строка логина для проверки
		 * @return       результат проверки
		 */
		bool isLogin(const string login);
	public:
		/**
		 * clear Метод очистки данных
		 */
		void clear();
		/**
		 * modify Метод модификации заголовков
		 * @param client идентификатор клиента
		 * @param server адрес сервера
		 * @param http   блок с данными запроса или ответа
		 */
		void modify(const string client, const string server, HttpData & http);
		/**
		 * Headers Конструктор
		 * @param log    объект лога для вывода информации
		 * @param config конфигурационные данные
		 */
		Headers(LogApp * log = NULL, Config ** config = NULL);
};

#endif // _HEADERS_ANYKS_