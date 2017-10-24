/* МОДУЛЬ УПРАВЛЕНИЯ ЗАГОЛОВКАМИ AMING */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _HEADERS2_AMING_
#define _HEADERS2_AMING_

#include <map>
#include <regex>
#include <deque>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <stdlib.h>
#include "config/conf.h"
#include "nwk/nwk.h"
#include "log/log.h"
#include "http/http.h"
#include "ldap2/ldap.h"
#include "ausers/ausers.h"
#include "general/general.h"

// Устанавливаем область видимости
using namespace std;

/**
 * Headers Класс управления заголовками
 * Параметры в файле заголовков
 * [action traffic server method path query agent user group headers]
 */
class Headers2 {
	private:
		/**
		 * IsNot Структура результата проверки на инверсию
		 */
		struct IsNot {
			bool inv;	// Результат проверки
			string str;	// Строка в чистом виде
		};
		/**
		 * Ldap Структура ldap
		 */
		struct Ldap {
			string dn;
			string scope;
			string filter;
		};
		/**
 		* Node Структура данных рабочей ноды
 		*/
		struct Node {
			u_int prefix;	// Префикс сети
			u_short type;	// Тип данных
			string data;	// Данные ноды
		};
		/**
 		* Rules Структура правил заголовков
 		*/
		struct Rules {
			string query;				// Регулярное выражение параметров запроса
			string userAgent;			// Регулярное выражение UserAgent
			vector <Node> clients;		// Данные клиента
			vector <Node> servers;		// Данные сервера
			vector <string> paths;		// Путь запроса на сервере
			vector <string> headers;	// Список заголовков
		};
		/**
 		* Params Структура списков параметров для создания правил
 		*/
		struct Params {
			string query;				// Регулярное выражение параметров запроса
			string userAgent;			// Регулярное выражение UserAgent
			vector <string> actions;	// Список экшенов
			vector <string> traffic;	// Список направлений трафика
			vector <string> clients;	// Список клиентов
			vector <string> servers;	// Список серверов
			vector <string> methods;	// Список методов
			vector <string> paths;		// Список путей
			vector <string> users;		// Список пользователей
			vector <string> groups;		// Список групп
			vector <string> headers;	// Список заголовков
		};
		/*
		* Список правил (gid -> uid -> action -> traffic -> method -> {client, server, headers})
		*/
		/*
		* gid ->
		*       | uid ->
		*       |       | action ->
		*       |       |          | traffic ->
		*       |       |          |           | method ->
		*       |       |          |           |          | rules
		* -------------------------------------------------------
		*/
		map <
			gid_t,
			map <
				uid_t,
				map <
					bool,
					map <
						bool,
						unordered_map <
							string,
							Rules
						>
					>
				>
			>
		> rules;
		// Объект ldap подклчюения
		Ldap ldap;
		// Название файла конфигурации
		deque <string> names;
		// Системные параметры
		LogApp * log = nullptr;
		Config * config = nullptr;
		AUsers * ausers = nullptr;
		// Время в течение которого обновлять нельзя
		time_t maxUpdate = 0;
		// Время последнего обновления данных
		time_t lastUpdate = 0;
		// Тип поиска параметров заголовков (0 - Из файла, 1 - из LDAP)
		u_short typeSearch = 0;
		/**
		 * isNot Метод проверки на инверсию
		 * @param  str строка для проверки
		 * @return     результат проверки
		 */
		const IsNot isNot(const string str);
		/**
		 * modifyHeaders Метод модификации заголовков
		 * @param rules   правила фильтрации
		 * @param http    блок с http данными
		 */
		void modifyHeaders(const vector <string> rules, HttpData &http);
		/**
		 * modifyHeaders Метод модификации заголовков
		 * @param rules  правила фильтрации
		 * @param data   строка с данными запроса или ответа
		 * @param http   блок с http данными
		 */
		void modifyHeaders(const vector <string> rules, string &data, HttpData &http);
		/**
		 * createRulesList Метод созданий списка правил
		 * @param params список параметров
		 */
		void createRulesList(const Params params);
		/**
		 * readFromLDAP Метод чтения данных из LDAP сервера
		 */
		void readFromLDAP();
		/**
		 * readFromFile Метод чтения данных из файла
		 */
		void readFromFile();
	public:
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
		/**
		 * findRules Метод поиска заголовков
		 * @param ip     IP адрес пользователя
		 * @param mac    MAC адрес пользователя
		 * @param sip    IP адрес сервера
		 * @param sdm    доменное имя сервера
		 * @param uat    юзер-агент браузера
		 * @param path   путь запроса 
		 * @param query  параметры запроса
		 * @param method метод запроса
		 * @param rules  список правил
		 * @return       сформированный список заголовков
		 */
		const vector <string> findHeaders(const string ip, const string mac, const string sip, const string sdm, const string uat, const string path, const string query, const string method, const Rules * rules = nullptr);
		/**
		 * get Метод получения правил клиента
		 * @param gid     идентификатор группы
		 * @param uid     идентификатор пользователя
		 * @param ip      IP адрес пользователя
		 * @param mac     MAC адрес пользователя
		 * @param sip     IP адрес сервера
		 * @param sdm     доменное имя сервера
		 * @param uat     юзер-агент браузера
		 * @param path    путь запроса 
		 * @param query   параметры запроса
		 * @param method  метод запроса
		 * @param traffic направление трафика
		 * @param action  экшен
		 * @return        сформированный список правил
		 */
		const vector <string> get(const gid_t gid, const uid_t uid, const string ip, const string mac, const string sip, const string sdm, const string uat, const string path, const string query, const string method, const bool traffic, const bool action);
		/**
		 * get Метод получения правил клиента
		 * @param ip      IP адрес пользователя
		 * @param mac     MAC адрес пользователя
		 * @param sip     IP адрес сервера
		 * @param sdm     доменное имя сервера
		 * @param uat     юзер-агент браузера
		 * @param path    путь запроса 
		 * @param query   параметры запроса
		 * @param method  метод запроса
		 * @param traffic направление трафика
		 * @param action  экшен
		 * @return        сформированный список правил
		 */
		const vector <string> get(const string ip, const string mac, const string sip, const string sdm, const string uat, const string path, const string query, const string method, const bool traffic, const bool action);
		/**
		 * add Метод добавления новых параметров фильтрации заголовков
		 * @param gid     идентификатор группы
		 * @param uid     идентификатор пользователя
		 * @param action  экшен
		 * @param traffic направление трафика
		 * @param method  метод запроса
		 * @param ctx     правила работы с заголовками
		 */
		void add(const gid_t gid, const uid_t uid, const bool action = false, const bool traffic = false, const string method = "*", void * ctx = nullptr);
		/**
		 * rm Метод удаления параметров фильтрации заголовков
		 * @param gid идентификатор группы
		 * @param uid идентификатор пользователя
		 */
		void rm(const gid_t gid, const uid_t uid);
		/**
		 * read Метод чтения из параметров
		 */
		void read();
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
		 * modify Метод модификации заголовков
		 * @param client данные клиента
		 * @param http   блок с данными запроса или ответа
		 */
		void modify(AParams::Client client, HttpData &http);
		/**
		 * modify Метод модификации заголовков
		 * @param client данные клиента
		 * @param data   строка с данными запроса или ответа
		 */
		void modify(AParams::Client client, string &data);
		/**
		 * Headers Конструктор
		 * @param config конфигурационные данные
		 * @param log    объект лога для вывода информации
		 * @param ausers объект пользователей
		 */
		Headers2(Config * config = nullptr, LogApp * log = nullptr, AUsers * ausers = nullptr);
};

#endif // _HEADERS2_AMING_