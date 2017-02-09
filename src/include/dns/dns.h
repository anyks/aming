/* DNS РЕСОЛВЕР ДЛЯ ПРОКСИ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _DNS_RESOLVER_ANYKS_
#define _DNS_RESOLVER_ANYKS_

#include <mutex>
#include <regex>
#include <string>
#include <vector>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <event2/dns.h>
#include <event2/util.h>
#include <event2/event.h>
#include "log/log.h"

// Устанавливаем область видимости
using namespace std;

/**
 * DNSResolver Класс dns ресолвера
 */
class DNSResolver {
	private:
		/**
		 * handler Прототип колбека
		 */
		typedef void (* handler) (const string ip, void * ctx);
		// Структура доменного имени
		struct DomainData {
			handler fn;		// Указатель на функцию обратного вызова
			int family;		// Тип протокола интернета AF_INET или AF_INET6
			void * ctx;		// указатель на объект передаваемый пользователем
			string domain;	// название искомого домена
			LogApp * log;	// Объект ведения логов
		};
		// Мютекс для захвата потока
		recursive_mutex mtx;
		// Тип протокола интернета AF_INET или AF_INET6
		int family;
		// Объект ведения логов
		LogApp * log = NULL;
		// База событий
		struct event_base * base = NULL;
		// База dns
		struct evdns_base * dnsbase = NULL;
		// Адреса серверов dns
		vector <string> servers;
		/**
		 * createDNSBase Метод создания dns базы
		 */
		void createDNSBase();
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
		 * @param domain название домена
		 * @param fn     функция обратного вызова срабатывающая при получении данных
		 * @param ctx    указатель на объект передаваемый пользователем
		 */
		void resolve(const string domain, handler fn, void * ctx = NULL);
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
		 * setBase Установка базы данных событий
		 * @param base указатель на объект базы данных событий
		 */
		void setBase(struct event_base * base);
		/**
		 * setNameServer Метод добавления сервера dns
		 * @param server ip адрес dns сервера
		 */
		void setNameServer(const string server);
		/**
		 * setNameServers Метод добавления серверов dns
		 * @param servers ip адреса dns серверов
		 */
		void setNameServers(vector <string> servers);
		/**
		 * DNSResolver Конструктор
		 * @param log     объект ведения логов
		 * @param base    база данных событий
		 * @param family  тип интернет протокола IPv4 или IPv6
		 * @param servers массив dns серверов
		 */
		DNSResolver(LogApp * log = NULL, struct event_base * base = NULL, int family = AF_INET, vector <string> servers = {});
		/**
		 * ~DNSResolver Деструктор
		 */
		~DNSResolver();
};

#endif // _DNS_RESOLVER_ANYKS_
