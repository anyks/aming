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
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include "config/conf.h"
#include "nwk/nwk.h"
#include "log/log.h"
#include "http/http.h"
#include "general/general.h"

// Устанавливаем область видимости
using namespace std;

/**
 * Headers Класс управления заголовками
 */
class Headers {
	private:
		/**
		 * IsNot Структура результата проверки на инверсию
		 */
		struct IsNot {
			bool inv;	// Результат проверки
			string str;	// Строка в чистом виде
		};
		/**
		 * Структура параметров заголовков
		 */
		struct Params {
			u_short utype;				// Тип идентификатора пользователя (0 - не определен, 1 - IPv4, 2 - PIv6, 3 - MAC, 4 - Домен)
			u_short stype;				// Тип идентификатора сервера (0 - не определен, 1 - IPv4, 2 - PIv6, 3 - MAC, 4 - Домен)
			string action;				// Экшен правила (add - добавление, rm - удаление)
			string route;				// Направление данных (in - входящие данные, out - исходящие данные)
			string server;				// Идентификатор сервера (ip адрес, домен)
			string path;				// Путь запроса на сервере
			string regex;				// Регулярное выражение проверки User-Agent
			vector <string> methods;	// Массив методов запросов (get, post, head ...)
			vector <string> headers;	// Массив заголовков
		};
		// Объект лога
		LogApp * log = NULL;
		// Конфигурационные данные
		Config * config = NULL;
		// Список правил
		map <const string, vector <Params>> rules;
		/**
		 * get Метод получения правил клиента
		 * @param client     идентификатор клиента
		 * @param addGeneral добавлять в список общие правила
		 * @return           сформированный список правил
		 */
		vector <Params> get(const string client, bool addGeneral = true);
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
		 * modifyHeaders Метод модификации заголовков
		 * @param server идентификатор сервера
		 * @param rules  правила фильтрации
		 * @param http   блок с http данными
		 */
		void modifyHeaders(const string server, vector <Params> rules, HttpData &http);
		/**
		 * modifyHeaders Метод модификации заголовков
		 * @param server идентификатор сервера
		 * @param rules  правила фильтрации
		 * @param data   строка с данными запроса или ответа
		 * @param http   блок с http данными
		 */
		void modifyHeaders(const string server, vector <Params> rules, string &data, HttpData &http);
		/**
		 * isNot Метод проверки на инверсию
		 * @param  str строка для проверки
		 * @return     результат проверки
		 */
		IsNot isNot(const string str);
		/**
		 * checkTypeId Метод определения типа идентификатора
		 * @param  str строка идентификатора для определения типа
		 * @return     тип идентификатора
		 */
		u_short checkTypeId(const string str);
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
		 * @param ip     ip адрес клиента
		 * @param mac    мак адрес клиента
		 * @param server адрес сервера
		 * @param http   блок с данными запроса или ответа
		 */
		void modify(const string ip, const string mac, const string server, HttpData &http);
		/**
		 * modify Метод модификации заголовков
		 * @param ip     ip адрес клиента
		 * @param mac    мак адрес клиента
		 * @param server адрес сервера
		 * @param data   строка с данными запроса или ответа
		 */
		void modify(const string ip, const string mac, const string server, string &data);
		/**
		 * Headers Конструктор
		 * @param log    объект лога для вывода информации
		 * @param config конфигурационные данные
		 */
		Headers(LogApp * log = NULL, Config * config = NULL);
};

#endif // _HEADERS_ANYKS_