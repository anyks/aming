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
#include "general/general.h"

// Устанавливаем область видимости
using namespace std;

/**
 * Headers Класс управления заголовками
 * Параметры в файле заголовков
 * [action traffic server method path query agent user group headers]
 */
class Headers {
	private:
		/**
 		* Servers Структура данных сервера
 		*/
		struct Servers {
			u_int prefix;	// Префикс сети
			string mac;		// Мак адрес сервера
			string ip4;		// IP адрес протокола версии 4
			string ip6;		// IP адрес протокола версии 6
			string path;	// Путь запроса на сервере
			string query;	// Параметры запроса
			string domain;	// Домен сервера (домены могут быть с маской вида www.domain.com, *.domain.com, domain.com, *.domain.*, *.com, *.*)
		};
		/**
 		* Client Структура данных клиента
 		*/
		struct Client {
			u_int prefix;	// Префикс сети
			string mac;		// Мак адрес клиента
			string ip4;		// IP адрес протокола версии 4
			string ip6;		// IP адрес протокола версии 6
			string agent;	// UserAgent пользователя
		};
		/**
 		* Rules Структура правил заголовков
 		*/
		struct Rules {
			Client client;				// Данные клиента
			Servers server;				// Данные сервера
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
							unordered_map <
								string,
								Rules
							>
						>
					>
				>
			>
		> rules;
	public:
		/**
		 * Headers Конструктор
		 * @param config конфигурационные данные
		 * @param log    объект лога для вывода информации
		 */
		Headers(Config * config = nullptr, LogApp * log = nullptr);
};

#endif // _HEADERS2_AMING_