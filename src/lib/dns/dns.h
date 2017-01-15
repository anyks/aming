/* DNS РЕСОЛВЕР ДЛЯ ПРОКСИ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _DNS_RESOLVER_ANYKS_
#define _DNS_RESOLVER_ANYKS_

#include <regex>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/socket.h>
#include <event2/dns.h>
#include <event2/util.h>
#include <event2/event.h>
#include "../log/log.h"
#include "../config/conf.h"

// Устанавливаем область видимости
using namespace std;

/**
 * DNSResolver Класс dns ресолвера
 */
class DNSResolver {
	private:
		// Структура доменного имени
		struct DomainData {
			int family;					// Тип протокола интернета AF_INET или AF_INET6
			string ip;					// IP адрес домена
			string domain;				// Название домена
			LogApp * log;				// Объект ведения логов
			struct event_base * base;	// База событий
		} __attribute__((packed));
		// Тип протокола интернета AF_INET или AF_INET6
		int family;
		// Объект ведения логов
		LogApp * log = NULL;
		// База событий
		struct event_base * base = NULL;
		// База dns
		struct evdns_base * dnsbase = NULL;
		/**
		 * DNSResolver::callback Событие срабатывающееся при получении данных с dns сервера
		 * @param errcode ошибка dns сервера
		 * @param addr    структура данных с dns сервера
		 * @param ctx     объект с данными для запроса
		 */
		static void callback(int errcode, struct evutil_addrinfo * addr, void * ctx);
	public:
		/**
		 * resolve Метод ресолвинга домена
		 * @param  domain название домена
		 * @return        ip адрес домена
		 */
		string resolve(const string domain);
		/**
		 * setFamily Метод установки интернет протокола
		 * @param family интернет протокол
		 */
		void setFamily(const int family);
		/**
		 * setLog Метод установки объекта лога
		 * @param log объект лога
		 */
		void setLog(LogApp * log);
		/**
		 * setNameServer Метод добавления сервера dns
		 * @param nameserver ip адрес dns сервера
		 */
		void setNameServer(const string nameserver);
		/**
		 * setNameServers Метод добавления серверов dns
		 * @param nameserver ip адреса dns серверов
		 */
		void setNameServers(vector <string> nameservers);
		/**
		 * DNSResolver Конструктор
		 * @param log         объект ведения логов
		 * @param family      тип интернет протокола IPv4 или IPv6
		 * @param nameservers массив dns серверов
		 */
		DNSResolver(LogApp * log = NULL, int family = AF_INET, vector <string> nameservers = {});
		/**
		 * ~DNSResolver Деструктор
		 */
		~DNSResolver();
};

#endif // _DNS_RESOLVER_ANYKS_
