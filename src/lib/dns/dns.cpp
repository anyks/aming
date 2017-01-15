/* DNS РЕСОЛВЕР ДЛЯ ПРОКСИ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/

#include "dns.h"

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
	if(domainData != NULL){
		// Если возникла ошибка
		if(errcode){
			// Выводим в лог сообщение
			if(domainData->log != NULL) domainData->log->write(LOG_ERROR, "%s %s", domainData->domain.c_str(), evutil_gai_strerror(errcode));
		} else {
			// Создаем структуру данных, доменного имени
			struct evutil_addrinfo * ai;
			// Вектор полученных ip адресов
			vector <string> ips;
			// Переходим по всей структуре и выводим ip адреса
			for(ai = addr; ai; ai = ai->ai_next){
				// Создаем буфер для получения ip адреса
				char buf[128];
				// IP адрес
				const char * ip = NULL;
				// Если это искомый тип интернет протокола
				if(ai->ai_family == domainData->family){
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
			// Если ip адреса получены, выводим ip адрес в случайном порядке
			if(!ips.empty()){
				// рандомизация генератора случайных чисел
				srand(time(0));
				// Получаем ip адрес
				domainData->ip = ips[0 + rand() % ips.size()];
			}
			// Очищаем структуру данных домена
			evutil_freeaddrinfo(addr);
		}
		// Удаляем события из базы событий
		event_base_loopbreak(domainData->base);
	}
}
/**
 * resolve Метод ресолвинга домена
 * @param  domain название домена
 * @return        ip адрес домена
 */
string DNSResolver::resolve(const string domain){
	// IP адрес домена
	string ip;
	// Если домен передан
	if(!domain.empty()){
		// Результат работы регулярного выражения
		smatch match;
		// Устанавливаем правило регулярного выражения
		regex e("^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$", regex::ECMAScript | regex::icase);
		// Выполняем поиск протокола
		regex_search(domain, match, e);
		// Если данные найдены
		if(match.empty()){
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
			// Запоминаем объект лога
			domainData->log = this->log;
			// Запоминаем объект базы событий
			domainData->base = this->base;
			// Устанавливаем тип протокола интернета
			domainData->family = this->family;
			// Добавляем доменное имя
			domainData->domain = domain;
			// Выполняем dns запрос
			struct evdns_getaddrinfo_request * req = evdns_getaddrinfo(this->dnsbase, domain.c_str(), NULL, &hints, &DNSResolver::callback, domainData);
			// Выводим в лог сообщение
			if((req == NULL) && (this->log != NULL)) this->log->write(LOG_ERROR, "request for %s returned immediately", domain.c_str());
			// Запускаем базу событий
			event_base_dispatch(this->base);
			// Запоминаем полученный ip адрес
			ip = domainData->ip;
			// Удаляем объект домена
			delete domainData;
		// Если передан домен то возвращаем его
		} else ip = domain;
	}
	// Выводим ip адрес
	return ip;
}
/**
 * setFamily Метод установки интернет протокола
 * @param family интернет протокол
 */
void DNSResolver::setFamily(const int family){
	// Если интернет протокол передан
	if(family) this->family = family;
}
/**
 * setLog Метод установки объекта лога
 * @param log объект лога
 */
void DNSResolver::setLog(LogApp * log){
	// Если интернет протокол передан
	if(log != NULL) this->log = log;
}
/**
 * setNameServer Метод добавления сервера dns
 * @param nameserver ip адрес dns сервера
 */
void DNSResolver::setNameServer(const string nameserver){
	// Если dns сервер передан
	if(!nameserver.empty()){
		// Добавляем dns сервер в базу dns
		if(evdns_base_nameserver_ip_add(this->dnsbase, nameserver.c_str()) != 0){
			// Выводим в лог сообщение
			if(this->log != NULL) this->log->write(LOG_ERROR, "name server [%s] does not add!", nameserver.c_str());
		}
	}
}
/**
 * setNameServers Метод добавления серверов dns
 * @param nameserver ip адреса dns серверов
 */
void DNSResolver::setNameServers(vector <string> nameservers){
	// Если нейм сервера переданы
	if(!nameservers.empty()){
		// Переходим по всем нейм серверам и добавляем их
		for(u_int i = 0; i < nameservers.size(); i++){
			// Добавляем сервер dns в базу dns
			setNameServer(nameservers[i]);
		}
	}
}
/**
 * DNSResolver Конструктор
 * @param log         объект ведения логов
 * @param family      тип интернет протокола IPv4 или IPv6
 * @param nameservers массив dns серверов
 */
DNSResolver::DNSResolver(LogApp * log, const int family, vector <string> nameservers){
	// Запоминаем объект лога
	this->log = log;
	// Запоминаем тип интернет протокола
	this->family = family;
	// Создаем базу данных событий
	this->base = event_base_new();
	// Создаем базу данных dns
	this->dnsbase = evdns_base_new(this->base, 1);
	// Если база событий не создана
	if(!this->base){
		// Выводим в лог сообщение
		if(this->log != NULL) this->log->write(LOG_ERROR, "event base does not created!");
		// Выходим из приложения
		exit(1);
	}
	// Если база dns не создана
	if(!this->dnsbase){
		// Выводим в лог сообщение
		if(this->log != NULL) this->log->write(LOG_ERROR, "dns base does not created!");
		// Выходим из приложения
		exit(1);
	}
	// Если нейм сервера переданы
	if(!nameservers.empty()){
		// Переходим по всем нейм серверам и добавляем их
		for(u_int i = 0; i < nameservers.size(); i++){
			// Добавляем сервер dns в базу dns
			setNameServer(nameservers[i]);
		}
	}
}
/**
 * ~DNSResolver Деструктор
 */
DNSResolver::~DNSResolver(){
	// Удаляем базу данных dns
	evdns_base_free(this->dnsbase, 0);
	// Удаляем базу данных событий
	event_base_free(this->base);
}