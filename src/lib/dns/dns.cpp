/* DNS РЕСОЛВЕР ДЛЯ ПРОКСИ AMING */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/

#include "dns/dns.h"

// Устанавливаем область видимости
using namespace std;

/**
 * DNSResolver::callback Событие срабатывающееся при получении данных с dns сервера
 * @param errcode ошибка dns сервера
 * @param addr    структура данных с dns сервера
 * @param ctx     объект с данными для запроса
 */
void DNSResolver::callback(int errcode, struct evutil_addrinfo * addr, void * ctx){
	// Получаем объект доменного имени
	DomainData * domainData = reinterpret_cast <DomainData *> (ctx);
	// Если данные получены
	if(domainData){
		// Полученный ip адрес
		string ip;
		// Полученные ip адреса
		vector <string> ips;
		// Если возникла ошибка
		if(errcode){
			// Выводим в лог сообщение
			if(domainData->log) domainData->log->write(LOG_ERROR, 0, "%s %s", domainData->domain.c_str(), evutil_gai_strerror(errcode));
		} else {
			// Создаем структуру данных, доменного имени
			struct evutil_addrinfo * ai;
			// Переходим по всей структуре и выводим ip адреса
			for(ai = addr; ai; ai = ai->ai_next){
				// Создаем буфер для получения ip адреса
				char buf[128];
				// IP адрес
				const char * ip = NULL;
				// Если это искомый тип интернет протокола
				if((ai->ai_family == domainData->family) || (domainData->family == AF_UNSPEC)){
					// Получаем структуру для указанного интернет протокола
					switch(ai->ai_family){
						// Если это IPv4 адрес
						case AF_INET: {
							// Создаем структуру для получения ip адреса
							struct sockaddr_in * sin = (struct sockaddr_in *) ai->ai_addr;
							// Выполняем запрос и получает ip адрес
							ip = evutil_inet_ntop(ai->ai_family, &sin->sin_addr, buf, 128);
						} break;
						// Если это IPv6 адрес
						case AF_INET6: {
							// Создаем структуру для получения ip адреса
							struct sockaddr_in6 * sin6 = (struct sockaddr_in6 *) ai->ai_addr;
							// Выполняем запрос и получает ip адрес
							ip = evutil_inet_ntop(ai->ai_family, &sin6->sin6_addr, buf, 128);
						} break;
					}
					// Запоминаем полученный ip адрес
					ips.push_back(ip);
				}
			}
			// Очищаем структуру данных домена
			evutil_freeaddrinfo(addr);
		}
		// Если ip адреса получены, выводим ip адрес в случайном порядке
		if(!ips.empty()){
			// Если количество элементов больше 1
			if(ips.size() > 1){
				// рандомизация генератора случайных чисел
				srand(time(0));
				// Получаем ip адрес
				ip = ips[rand() % ips.size()];
			// Выводим только первый элемент
			} else ip = ips[0];
		}
		// Записываем данные в кэш
		domainData->cache->setDomain(domainData->domain, ip);
		// Выводим готовый результат
		domainData->fn(ip, domainData->ctx);
		// Удаляем передаваемый объект
		delete domainData;
	}
}
/**
 * resolve Метод ресолвинга домена
 * @param domain название домена
 * @param family тип интернет протокола IPv4 или IPv6
 * @param fn     функция обратного вызова срабатывающая при получении данных
 * @param ctx    указатель на объект передаваемый пользователем
 */
void DNSResolver::resolve(const string domain, const int family, handler fn, void * ctx){
	// Если домен передан
	if(!domain.empty()){
		// Результат работы регулярного выражения
		smatch match;
		// Устанавливаем правило регулярного выражения
		regex e("^\\[?(\\d{1,3}(?:\\.\\d{1,3}){3}|[A-Fa-f\\d\\:]{2,39})\\]?$", regex::ECMAScript | regex::icase);
		// Выполняем поиск протокола
		regex_search(domain, match, e);
		// Если данные найдены
		if(match.empty()){
			// Запрашиваем данные домена из кэша
			string ip = this->cache->getDomain(domain);
			// Если ip адрес найден тогда выводим результат
			if(!ip.empty()) fn(ip, ctx);
			// Если адрес не найден то запрашиваем его с ресолвера
			else {
				// Структура запроса
				struct evutil_addrinfo hints;
				// Заполняем структуру запроса нулями
				memset(&hints, 0, sizeof(hints));
				// Устанавливаем тип подключения
				hints.ai_family = AF_UNSPEC;
				// Устанавливаем флаг подключения что это канонническое имя
				hints.ai_flags = EVUTIL_AI_CANONNAME;
				// Устанавливаем что это потоковый сокет
				hints.ai_socktype = SOCK_STREAM;
				// Устанавливаем что это tcp подключение
				hints.ai_protocol = IPPROTO_TCP;
				// Создаем объект домен
				DomainData * domainData = new DomainData;
				// Устанавливаем функцию обратного вызова
				domainData->fn = fn;
				// Запоминаем указатель на объект пользователя
				domainData->ctx = ctx;
				// Запоминаем название искомого домена
				domainData->domain = domain;
				// Запоминаем объект лога
				domainData->log = this->log;
				// Запоминаем объект управления кэшем
				domainData->cache = this->cache;
				// Устанавливаем тип протокола интернета
				domainData->family = family;
				// Выполняем dns запрос
				struct evdns_getaddrinfo_request * req = evdns_getaddrinfo(this->dnsbase, domain.c_str(), NULL, &hints, &DNSResolver::callback, domainData);
				// Выводим в лог сообщение
				if((req == NULL) && this->log) this->log->write(LOG_ERROR, 0, "request for %s returned immediately", domain.c_str());
			}
		// Если передан домен то возвращаем его
		} else fn(match[1].str(), ctx);
	}
	// Выходим
	return;
}
/**
 * createDNSBase Метод создания dns базы
 */
void DNSResolver::createDNSBase(){
	// Если база событий существует
	if(this->base){
		// Очищаем базу данных dns
		if(this->dnsbase) evdns_base_free(this->dnsbase, 0);
		// Создаем базу данных dns
		this->dnsbase = evdns_base_new(this->base, 0);
		// Если база dns не создана
		if(!this->dnsbase && this->log){
			// Выводим в лог сообщение
			this->log->write(LOG_ERROR, 0, "dns base does not created!");
		}
	}
	// Если нейм сервера переданы
	setNameServers(this->servers);
}
/**
 * setLog Метод установки объекта лога
 * @param log объект лога
 */
void DNSResolver::setLog(LogApp * log){
	// Если интернет протокол передан
	if(log) this->log = log;
}
/**
 * setBase Установка базы данных событий
 * @param base указатель на объект базы данных событий
 */
void DNSResolver::setBase(struct event_base * base){
	// Создаем базу данных событий
	this->base = base;
	// Создаем dns базу
	createDNSBase();
}
/**
 * setNameServer Метод добавления сервера dns
 * @param server ip адрес dns сервера
 */
void DNSResolver::setNameServer(const string server){
	// Если dns сервер передан
	if(!server.empty() && this->dnsbase){
		// Добавляем dns сервер в базу dns
		if(evdns_base_nameserver_ip_add(this->dnsbase, server.c_str()) != 0){
			// Выводим в лог сообщение
			if(this->log) this->log->write(LOG_ERROR, 0, "name server [%s] does not add!", server.c_str());
		}
	}
}
/**
 * setNameServers Метод добавления серверов dns
 * @param server ip адреса dns серверов
 */
void DNSResolver::setNameServers(vector <string> servers){
	// Если нейм сервера переданы
	if(!servers.empty()){
		// Запоминаем dns сервера
		this->servers = servers;
		// Переходим по всем нейм серверам и добавляем их
		for(u_int i = 0; i < this->servers.size(); i++){
			// Добавляем сервер dns в базу dns
			setNameServer(this->servers[i]);
		}
	}
}
/**
 * DNSResolver Конструктор
 * @param log     объект ведения логов
 * @param cache   объект кэша
 * @param base    база данных событий
 * @param servers массив dns серверов
 */
DNSResolver::DNSResolver(LogApp * log, Cache * cache, struct event_base * base, vector <string> servers){
	// Запоминаем объект лога
	this->log = log;
	// Запоминаем объект кэша
	this->cache = cache;
	// Создаем базу данных событий
	this->base = base;
	// Запоминаем dns сервера
	this->servers = servers;
	// Создаем dns базу
	createDNSBase();
}
/**
 * ~DNSResolver Деструктор
 */
DNSResolver::~DNSResolver(){
	// Захватываем поток
	this->mtx.lock();
	// Удаляем базу данных dns
	if(this->dnsbase){
		// Очищаем базу данных dns
		evdns_base_free(this->dnsbase, 0);
		// Обнуляем указатель
		this->dnsbase = NULL;
	}
	// Освобождаем поток
	this->mtx.unlock();
}