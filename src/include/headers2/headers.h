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
#include <unordered_map>
#include <stdlib.h>
#include "config/conf.h"
#include "nwk/nwk.h"
#include "log/log.h"
#include "http/http.h"
#include "groups/groups.h"
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
 		* Node Структура данных рабочей ноды
 		*/
		struct Node {
			u_int prefix;	// Префикс сети
			string mac;		// Мак адрес ноды
			string ip4;		// IP адрес протокола версии 4
			string ip6;		// IP адрес протокола версии 6
			string domain;	// Домен ноды (домены могут быть с маской вида www.domain.com, *.domain.com, domain.com, *.domain.*, *.com, *.*)
		};
		/**
 		* Rules Структура правил заголовков
 		*/
		struct Rules {
			string agent;				// UserAgent пользователя
			vector <Node> clients;		// Данные клиента
			vector <Node> servers;		// Данные сервера
			vector <string> paths;		// Путь запроса на сервере
			vector <string> queries;	// Параметры запроса
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
		// Название файла конфигурации
		deque <string> names;
		// Системные параметры
		LogApp * log = nullptr;
		Config * config = nullptr;
		Groups * groups = nullptr;
		// Время в течение которого обновлять нельзя
		time_t maxUpdate = 0;
		// Время последнего обновления данных
		time_t lastUpdate = 0;
		// Тип поиска параметров заголовков (0 - Из файла, 1 - из LDAP)
		u_short typeSearch = 0;
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
		 * Headers Конструктор
		 * @param config конфигурационные данные
		 * @param log    объект лога для вывода информации
		 */
		Headers2(Config * config = nullptr, LogApp * log = nullptr, Groups * groups = nullptr);
};

#endif // _HEADERS2_AMING_