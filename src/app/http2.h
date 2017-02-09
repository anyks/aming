/* СЕРВЕР HTTP ПРОКСИ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _HTTP_PROXY_ANYKS_
#define _HTTP_PROXY_ANYKS_

#include <mutex>
#include <thread>
#include <iostream>
#include <condition_variable>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <event2/listener.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include "dns/dns.h"
#include "nwk/nwk.h"
#include "http/http.h"
#include "system/system.h"

// Устанавливаем область видимости
using namespace std;

// Флаги базы данных событий
#define EVLOOP_ONCE				0x01
#define EVLOOP_NONBLOCK			0x02
#define EVLOOP_NO_EXIT_ON_EMPTY	0x04

// Флаги таймеров
#define TM_SERVER 0x01
#define TM_CLIENT 0x02

/**
 * ConnectClients Класс всех подключенных пользователей
 */
class ConnectClients {
	private:
		/**
		 * Client Класс всех подключений пользователя
		 */
		class Client {
			private:
				// Мютекс для блокировки сервера
				mutex mtx;
				// Идентификатор клиента
				string id;
				// Максимально возможное количество подключений
				u_int max;
				// Количество активных клиентов
				u_int active;
				// Коллбек для удаления текущего клиента
				function <void (const string)> remove;
				// Переменная состояния
				condition_variable cond;
				// Массив подключенных клиентов
				vector <std::thread> connects;
				/**
				 * isfull Метод проверки заполненности максимального количества коннектов
				 * @return результат проверки
				 */
				bool isfull();
				/**
				 * rm Метод удаления подключения из объекта клиента
				 * @param index индекс подключения
				 */
				void rm(size_t index);
			public:
				/**
				 * add Метод добавления нового подключения в объект клиента
				 * @param client родительский объект клиента
				 * @param ctx    передаваемый указатель на объект
				 */
				void add(void * ctx);
		};
		// Мютекс для блокировки сервера
		mutex mtx;
		// Массив всех активных клиентов
		map <string, unique_ptr <Client>> clients;
		/**
		 * rm Метод удаления объекта подключившихся клиентов
		 * @param id идентификатор клиента
		 */
		void rm(const string id);
	public:
		/**
		 * add Метод добавления нового подключения в объект пользователя
		 * @param ctx передаваемый указатель на объект
		 */
		void add(void * ctx);
};



#endif // _HTTP_PROXY_ANYKS_