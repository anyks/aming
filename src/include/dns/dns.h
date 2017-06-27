/* DNS РЕСОЛВЕР ДЛЯ ПРОКСИ AMING */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _DNS_RESOLVER_AMING_
#define _DNS_RESOLVER_AMING_

#include <mutex>
#include <regex>
#include <string>
#include <vector>
#include <random>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <event2/dns.h>
#include <event2/util.h>
#include <event2/event.h>
#include "log/log.h"
#include "cache/cache.h"

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
			Cache * cache;	// Объект управления кэшем
		};
		// Мютекс для захвата потока
		mutex mtx;
		// Объект ведения логов
		LogApp * log = nullptr;
		// Объект управления кэшем
		Cache * cache = nullptr;
		// База событий
		struct event_base * base = nullptr;
		// База dns
		struct evdns_base * dnsbase = nullptr;
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
		 * @param family тип интернет протокола AF_INET, AF_INET6 или AF_UNSPEC
		 * @param fn     функция обратного вызова срабатывающая при получении данных
		 * @param ctx    указатель на объект передаваемый пользователем
		 */
		void resolve(const string domain, const int family, handler fn, void * ctx = nullptr);
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
		 * @param cache   объект кэша
		 * @param base    база данных событий
		 * @param servers массив dns серверов
		 */
		DNSResolver(LogApp * log = nullptr, Cache * cache = nullptr, struct event_base * base = nullptr, vector <string> servers = {});
		/**
		 * ~DNSResolver Деструктор
		 */
		~DNSResolver();
};

#endif // _DNS_RESOLVER_AMING_
