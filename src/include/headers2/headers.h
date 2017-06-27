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
 		* Rules Структура правил заголовков
 		*/
		struct Rules {
			string agent;
			string query;
			vector <string> headers;
		};
		/*
		* Список правил (server -> ID -> action -> traffic -> method -> path -> {query, agent, headers})
		*
		* server может быть
		* mac
		* ip
		* network
		* domain (домены могут быть с маской вида www.domain.com, *.domain.com, domain.com, *.domain.*, *.com, *.*)
		*
		* ID Может быть
		* group
		* user
		* ident
		*
		* ident может быть
		* mac
		* ip
		* network
		*/
		/*
		* id_server ->
		*             | name ->
		*             |        | id_user ->
		*             |        |           | name ->
		*             |        |           |        | action ->
		*             |        |           |        |          | traffic ->
		*             |        |           |        |          |           | method ->
		*             |        |           |        |          |           |          | path ->
		*             |        |           |        |          |           |          |        | rules
		*/
		map <
			u_int,
			unordered_map <
				string,
				map <
					u_int,
					unordered_map <
						string,
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
				>
			>
		> rules;
	public:
		/**
		 * Headers Конструктор
		 * @param config конфигурационные данные
		 * @param log    объект лога для вывода информации
		 */
		Headers(Config * config = NULL, LogApp * log = NULL);
};

#endif // _HEADERS2_AMING_