/* МОДУЛЬ УПРАВЛЕНИЯ ЗАГОЛОВКАМИ AMING */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _HEADERS_AMING_
#define _HEADERS_AMING_

#include <regex>
#include <deque>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <unordered_map>
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
		// Название файла конфигурации
		deque <string> names;
		// Объект лога
		LogApp * log = nullptr;
		// Конфигурационные данные
		Config * config = nullptr;
		// Параметры для парсинга http данных
		u_short options = 0x00;
		// Список правил
		unordered_map <string, vector <Params>> rules;
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
		const IsNot isNot(const string str);
		/**
		 * checkTypeId Метод определения типа идентификатора
		 * @param  str строка идентификатора для определения типа
		 * @return     тип идентификатора
		 */
		const u_short checkTypeId(const string str);
		/**
		 * isIpV4 Метод проверки на ip адрес, интернет протокола версии 4
		 * @param  ip строка ip адреса для проверки
		 * @return    результат проверки
		 */
		const bool isIpV4(const string ip);
		/**
		 * isIpV6 Метод проверки на ip адрес, интернет протокола версии 6
		 * @param  ip строка ip адреса для проверки
		 * @return    результат проверки
		 */
		const bool isIpV6(const string ip);
		/**
		 * isIp Метод проверки на ip адрес
		 * @param  ip строка ip адреса для проверки
		 * @return    результат проверки
		 */
		const bool isIp(const string ip);
		/**
		 * isMac Метод проверки на mac адрес
		 * @param  mac строка mac адреса для проверки
		 * @return     результат проверки
		 */
		const bool isMac(const string mac);
		/**
		 * isDomain Метод проверки на доменное имя
		 * @param  domain строка названия домена для проверки
		 * @return        результат проверки
		 */
		const bool isDomain(const string domain);
		/**
		 * isLogin Метод проверки на логин пользователя
		 * @param  login строка логина для проверки
		 * @return       результат проверки
		 */
		const bool isLogin(const string login);
		/**
		 * checkAvailable Метод проверки на существование параметров заголовков
		 * @param  name название файла с параметрами
		 * @return      результат проверки
		 */
		const bool checkAvailable(const string name);
		/**
		 * getName Метод получения имени конфига
		 */
		const string getName();
	public:
		/**
		 * clear Метод очистки данных
		 */
		void clear();
		/**
		 * addName Метод добавления нового имени конфига
		 * @param name название файла с параметрами
		 */
		void addName(const string name);
		/**
		* setOptions Метод установки новых параметров для парсинга http данных
		* @param options параметры для парсинга http данных
		*/
		void setOptions(const u_short options = 0x00);
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
		 * @param config конфигурационные данные
		 * @param log    объект лога для вывода информации
		 */
		Headers(Config * config = nullptr, LogApp * log = nullptr);
};

#endif // _HEADERS_AMING_