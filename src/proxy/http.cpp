/* СЕРВЕР HTTP ПРОКСИ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2016
*/
#include "http.h"

// Устанавливаем пространство имен
using namespace std;

/**
 * HttpProxy::get_host Функция получения данных хоста
 * @param  address структура параметров подключения
 * @param  socklen размер структуры
 * @return         данные полученного хоста
 */
string HttpProxy::get_host(struct sockaddr * address, int socklen){
	// Буферы данных
	char hbuf[256], sbuf[256];
	// Извлекаем данные из структуры подключений
	int s = getnameinfo(address, socklen, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);
	// Если данные извлечены верно
	if(s == 0) return hbuf;
	// Сообщаем что ничего не найдено
	return "";
}
/**
 * free_http Функция очистки объекта http
 * @param arg объект для очистки
 */
void HttpProxy::free_http(BufferHttpProxy ** arg){
	// Если данные еще не удалены
	if(* arg != NULL){
		// Удаляем объект данных
		delete * arg;
		// Присваиваем пустой адрес
		* arg = NULL;
	}
}
/**
 * set_non_block Функция отключения алгоритма Нейгла
 * @param fd файловый дескриптор (сокет)
 * @return   результат работы функции
 */
int HttpProxy::set_tcpnodelay(evutil_socket_t fd){
	// Устанавливаем параметр
	int tcpnodelay = 1;
	// Устанавливаем TCP_NODELAY
	if(setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &tcpnodelay, sizeof(tcpnodelay)) < 0){
		// Выводим в консоль информацию
		printf("Cannot set TCP_NODELAY option on socket %d\n", fd);
		// Выходим
		return -1;
	}
	// Все удачно
	return 0;
}
/**
 * append_to_buffer Функция добавления в буфер новых данных
 * @param data       ссылка на буфер данных
 * @param chunk_size размер одной порции данных
 * @param buffer     буфер с входящими данными
 */
void HttpProxy::append_to_buffer(vector <char> &data, size_t chunk_size, const char * buffer){
	// Определяем количество общих данных
	size_t total = (!data.empty() ? data.size() - 1 : 0);
	// Устанавливаем параметры по умолчанию
	total += chunk_size;
	// Выполняем увеличение размера вектора
	data.resize(total + 1);
	// Вспомогательные переменные для заполнения вектора данными
	size_t i = total - chunk_size, j = 0;
	// Добавляем все полученные байты в массив
	while(i < total){
		// Заполняем вектор полученными символами
		data[i] = (char) buffer[j];
		i++;	// Изменяем смещение по вектору
		j++;	// Изменяем смещение по буферу данных
	}
	// Добавляем нулевой символ
	data[total] = '\0';
	// Выходим
	return;
}
/**
 * set_buffer_size Функция установки размеров буфера
 * @param  fd         файловый дескриптор (сокет)
 * @param  read_size  размер буфера на чтение
 * @param  write_size размер буфера на запись
 * @return            результат работы функции
 */
int HttpProxy::set_buffer_size(evutil_socket_t fd, int read_size, int write_size){
	// Определяем размер массива опции
	socklen_t read_optlen	= sizeof(read_size);
	socklen_t write_optlen	= sizeof(write_size);
	// Устанавливаем размер буфера для сокета клиента и сервера
	if(read_size > 0)	setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *) &read_size, read_optlen);
	if(write_size > 0)	setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *) &write_size, write_optlen);
	// Считываем установленный размер буфера
	if((getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &read_size, &read_optlen) < 0)
	|| (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &write_size, &write_optlen) < 0)){
		// Выводим в консоль информацию
		printf("Get buffer wrong!!!!\n");
		// Выходим
		return -1;
	}
	// Все удачно
	return 0;
}
/**
 * check_auth Функция проверки логина и пароля
 * @param ctx объект входящих данных
 * @return    результат проверки подлинности
 */
bool HttpProxy::check_auth(void * ctx){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	// Если подключение не передано
	if(http != NULL){
		// Логин
		const char * username = "zdD786KeuS";
		// Проль
		const char * password = "k.frolovv@gmail.com";
		// Проверяем логин и пароль
		if(!strcmp(http->parser->getLogin().c_str(), username)
		&& !strcmp(http->parser->getPassword().c_str(), password)) return true;
	}
	// Сообщаем что проверка не прошла
	return false;
}
/**
 * connect_server Функция создания сокета для подключения к удаленному серверу
 * @param ctx объект входящих данных
 * @return    результат подключения
 */
int HttpProxy::connect_server(void * ctx){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	// Если подключение не передано
	if(http != NULL){
		// Получаем данные хоста
		hostent * sh = gethostbyname(http->parser->getHost().c_str());
		// Если данные хоста найдены
		if(sh){
			// ip адрес ресурса
			string ip;
			// Получаем порт сервера
			u_int port = http->parser->getPort();
			// Извлекаем ip адрес
			for(u_int i = 0; sh->h_addr_list[i] != 0; ++i){
				struct in_addr addr;
				memcpy(&addr, sh->h_addr_list[i], sizeof(struct in_addr));
				ip = inet_ntoa(addr);
			}
			// Если хост и порт сервера не совпадают тогда очищаем данные
			if((http->events.server != NULL)
			&& ((http->server.host != ip)
			|| (http->server.port != port))) http->free_server();
			// Получаем текущий сокет клиента
			evutil_socket_t fd = bufferevent_getfd(http->events.client);
			// Если сервер еще не подключен
			if(http->events.server == NULL){
				// Запоминаем хост и порт сервера
				http->server.host = ip;
				http->server.port = port;
				// Буфер порта
				char port_buf[6];
				// Сокет подключения
				evutil_socket_t sock = -1;
				// Структура параметров подключения
				struct addrinfo param;
				// Указатель на результаты
				struct addrinfo * req;
				// Убедимся, что структура пуста
				memset(&param, 0, sizeof(param));
				// Неважно, IPv4 или IPv6
				param.ai_family = AF_UNSPEC;
				// TCP stream-sockets
				param.ai_socktype = SOCK_STREAM;
				// We want a TCP socket
				param.ai_protocol = IPPROTO_TCP;
				// Only return addresses we can use.
				param.ai_flags = EVUTIL_AI_ADDRCONFIG;
				// Convert the port to decimal.
				evutil_snprintf(port_buf, sizeof(port_buf), "%d", (int) port);
				// Если формат подключения указан не верно то сообщаем об этом
				if(evutil_getaddrinfo(ip.c_str(), port_buf, &param,  &req)){
					// Выводим в консоль информацию
					printf("Error in server address format! %s\n", ip.c_str());
					// Выходим
					return 0;
				}
				// Создаем сокет, если сокет не создан то сообщаем об этом
				if((sock = socket(req->ai_family, req->ai_socktype, req->ai_protocol)) < 0){
					// Выводим в консоль информацию
					printf("Error in creating socket to server!\n");
					// Выходим
					return 0;
				}
				// Если сокет не создан то выходим
				if(sock < 0) return 0;
				// Устанавливаем размеры буферов
				set_buffer_size(sock, http->buffer_read_size, http->buffer_write_size);
				// Создаем буфер событий для сервера
				http->events.server = bufferevent_socket_new(http->base, sock, BEV_OPT_CLOSE_ON_FREE);
				// Устанавливаем коллбеки
				bufferevent_setcb(http->events.server, &HttpProxy::read_server, NULL, &HttpProxy::event, http);
				// Активируем буферы событий на чтение и запись
				bufferevent_enable(http->events.server, EV_READ | EV_WRITE);
				// Очищаем буферы событий при завершении работы
				bufferevent_flush(http->events.server, EV_READ | EV_WRITE, BEV_FINISHED);
				// Выполняем подключение к удаленному серверу, если подключение не выполненно то сообщаем об этом
				if(bufferevent_socket_connect(http->events.server, req->ai_addr, req->ai_addrlen) < 0){
					// Выводим в консоль информацию
					printf("Error in connecting to server!\n");
					// Очищаем буфер сервера
					http->free_server();
					// Выходим
					return -1;
				}
				// И освобождаем связанный список
				evutil_freeaddrinfo(req);
				// Если подключение постоянное
				if(http->parser->isAlive()){
					// Получаем сокеты
					evutil_socket_t server_fd = bufferevent_getfd(http->events.server);
					evutil_socket_t client_fd = bufferevent_getfd(http->events.client);
					// Устанавливаем TCP_NODELAY для сервера и клиента
					set_tcpnodelay(server_fd);
					set_tcpnodelay(client_fd);
				}
				// Выводим в консоль сообщение о новом коннекте
				printf("connect to host = %s [%s:%d] path = %s, socket = %d\n", http->parser->getHost().c_str(), http->server.host.c_str(), http->server.port, http->parser->getPath().c_str(), fd);
				// Сообщаем что все удачно
				return 1;
			// Если сервер уже подключен, сообщаем что все удачно
			} else {
				// Выводим в консоль сообщение о новом коннекте
				printf("last connect to host = %s [%s:%d] path = %s, socket = %d\n", http->parser->getHost().c_str(), http->server.host.c_str(), http->server.port, http->parser->getPath().c_str(), fd);
				// Сообщаем что все удачно
				return 2;
			}
		// Очищаем буфер, если он существует
		} else http->free_server();
	}
	// Выходим
	return -1;
}
/**
 * event Функция обработка входящих событий
 * @param bev    буфер события
 * @param events произошедшее событие
 * @param ctx    объект входящих данных
 */
void HttpProxy::event(struct bufferevent * bev, short events, void * ctx){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	// Если подключение не передано
	if(http != NULL){
		// Получаем текущий сокет
		evutil_socket_t current_fd = bufferevent_getfd(bev);
		// Получаем сокет клиента
		evutil_socket_t client_fd = bufferevent_getfd(http->events.client);
		// Определяем для кого вызвано событие
		string subject = (current_fd == client_fd ? "client" : "server");
		// Если подключение удачное
		if(events & BEV_EVENT_CONNECTED){
			// Выводим сообщение в консоль
			printf("Connect %s okay., socket = %d\n", subject.c_str(), current_fd);
		// Если это ошибка или завершение работы
		} else if(events & (BEV_EVENT_ERROR | BEV_EVENT_EOF | BEV_EVENT_TIMEOUT)) {
			// Если это ошибка
			if(events & BEV_EVENT_ERROR){
				// Получаем данные ошибки
				int err = bufferevent_socket_get_dns_error(bev);
				// Если ошибка существует, выводим сообщение в консоль
				if(err) printf("DNS error: %s\n", evutil_gai_strerror(err));
			}
			// Сообщаем что произошло отключение
			printf("Closing %s, socket = %d\n", subject.c_str(), current_fd);
			// Если это код не разрешающий коннект
			if(http->response.code != 200) free_http(&http);
			// Если это разрешенный запрос
			else {
				// Отключаемся если сработал таймаут или подключение не должно жить
				if((events & BEV_EVENT_TIMEOUT) || !http->parser->isAlive()) free_http(&http);
				// Если это не таймаут а постоянное подключение и это клиент
				else if(http->parser->isAlive() && (subject == "client")){
					// Устанавливаем таймаут ожидания результата
					struct timeval tread = {http->keepalive_timeout, 0};
					// Устанавливаем таймауты
					bufferevent_set_timeouts(bev, &tread, NULL);
				}
			}
		}
	// Отключаемся
	} else bufferevent_free(bev);
	// Выходим
	return;
}
/**
 * read_server Функция чтения данных с сокета сервера
 * @param bev буфер события
 * @param ctx передаваемый объект
 */
void HttpProxy::read_server(struct bufferevent * bev, void * ctx){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	// Если подключение не передано
	if(http != NULL){
		// Получаем буферы входящих данных и исходящих
		struct evbuffer * input		= bufferevent_get_input(http->events.server);
		struct evbuffer * output	= bufferevent_get_output(http->events.client);
		// Устанавливаем таймаут ожидания результата
		struct timeval tread = {http->keepalive_timeout, 0};
		// Устанавливаем таймаут записи результата
		struct timeval twrite = {http->write_timeout, 0};
		// Устанавливаем таймауты
		bufferevent_set_timeouts(http->events.client, &tread, &twrite);
		// Выводим ответ сервера
		if(evbuffer_add_buffer(output, input) < 0) free_http(&http);
	}
	// Выходим
	return;
}
/**
 * read_client Функция чтения данных с сокета клиента
 * @param bev буфер события
 * @param ctx передаваемый объект
 */
void HttpProxy::read_client(struct bufferevent * bev, void * ctx){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	// Если подключение не передано
	if(http != NULL){
		// Получаем буферы входящих данных и исходящих
		struct evbuffer * input = bufferevent_get_input(bev);
		// Если авторизация прошла, и коннект произведен
		if((http->response.code == 200) && http->parser->isConnect()){
			// Получаем буферы входящих данных и исходящих
			struct evbuffer * output = bufferevent_get_output(http->events.server);
			// Устанавливаем таймаут ожидания результата
			struct timeval tread = {http->read_timeout, 0};
			// Устанавливаем таймаут записи результата
			struct timeval twrite = {http->write_timeout, 0};
			// Устанавливаем таймауты
			bufferevent_set_timeouts(http->events.server, &tread, &twrite);
			// Выводим ответ сервера
			if(evbuffer_add_buffer(output, input) < 0) free_http(&http);
		// Если это обычный запрос
		} else {
			// Получаем размер входящих данных
			size_t len = evbuffer_get_length(input);
			// Создаем буфер данных
			char * buffer = new char[len];
			// Копируем в буфер полученные данные
			evbuffer_copyout(input, buffer, len);
			// Склеиваем полученные данные
			append_to_buffer(http->request.data, len, buffer);
			// Удаляем данные из буфера
			evbuffer_drain(input, len);
			// Удаляем буфер данных
			delete [] buffer;
			// Выполняем парсинг полученных данных
			if(!http->request.data.empty() && http->parse()){
				// Очищаем таймауты
				bufferevent_set_timeouts(bev, NULL, NULL);
				// Скидываем количество отправляемых данных
				http->request.offset = 0;
				// Очищаем буфер данных
				http->request.data.clear();
				// Очищаем объект ответа
				http->response.clear();
				// Если авторизация не прошла
				if(!http->auth) http->auth = check_auth(http);
				// Если нужно запросить пароль
				if(!http->auth && (http->parser->getLogin().empty()
				|| http->parser->getPassword().empty())){
					// Формируем ответ клиенту
					http->response = http->parser->requiredAuth();
				// Сообщаем что авторизация не удачная
				} else if(!http->auth) {
					// Формируем ответ клиенту
					http->response = http->parser->faultAuth();
				// Если авторизация прошла
				} else {
					// Выполняем подключение к удаленному серверу
					int connect = connect_server(http);
					// Если сокет существует
					if(connect > 0){
						// Определяем порт, если это метод connect
						if(http->parser->isConnect())
							// Формируем ответ клиенту
							http->response = http->parser->authSuccess();
						// Иначе делаем запрос на получение данных
						else {
							// Указываем что нужно отключится сразу после отправки запроса
							if(!http->parser->isAlive()) http->parser->setClose();
							// Формируем запрос на сервер
							http->response = http->parser->getQuery();
							// Устанавливаем таймаут ожидания результата
							struct timeval tread = {http->read_timeout, 0};
							// Устанавливаем таймаут записи результата
							struct timeval twrite = {http->write_timeout, 0};
							// Устанавливаем таймауты
							bufferevent_set_timeouts(http->events.server, &tread, &twrite);
							// Отправляем серверу сообщение
							if(bufferevent_write(http->events.server, http->response.data(), http->response.size()) < 0) free_http(&http);
							// Выходим
							return;
						}
					// Если подключение не удачное то сообщаем об этом
					} else if(connect == 0) http->response = http->parser->faultConnect();
					// Если подключение удалено, выходим
					else return;
				}
				// Ответ готов
				if(!http->response.empty()){
					// Если это код разрешающий коннект
					if(http->response.code == 200){
						// Устанавливаем таймаут ожидания результата
						struct timeval tread = {http->keepalive_timeout, 0};
						// Устанавливаем таймаут записи результата
						struct timeval twrite = {http->write_timeout, 0};
						// Устанавливаем таймауты
						bufferevent_set_timeouts(bev, &tread, &twrite);
					}
					// Отправляем клиенту сообщение
					if(bufferevent_write(bev, http->response.data(), http->response.size()) < 0) free_http(&http);
				// Отключаемся
				} else free_http(&http);
			}
		}
	}
	// Выходим
	return;
}
/**
 * accept_error Событие возникновения ошибки подключения
 * @param listener объект подключения
 * @param ctx      передаваемый объект
 */
void HttpProxy::accept_error(struct evconnlistener * listener, void * ctx){
	// Получаем базу событий
	struct event_base * base = evconnlistener_get_base(listener);
	// Получаем сообщение об ошибке
	int err = EVUTIL_SOCKET_ERROR();
	// Выводим сообщение в консоль
	fprintf(stderr, "Got an error %d (%s) on the listener. Shutting down.\n", err, evutil_socket_error_to_string(err));
	// Удаляем базу событий
	event_base_loopexit(base, NULL);
	// Выходим
	return;
}
/**
 * accept_connect Событие подключения к серверу
 * @param listener объект подключения
 * @param fd       файловый дескриптор (сокет) клиента
 * @param address  адрес клиента
 * @param socklen  размер входящих данных
 * @param ctx      передаваемый объект
 */
void HttpProxy::accept_connect(struct evconnlistener * listener, evutil_socket_t fd, struct sockaddr * address, int socklen, void * ctx){
	// Получаем объект прокси-сервера
	HttpProxy * proxy = reinterpret_cast <HttpProxy *> (ctx);
	// Если прокси существует
	if(proxy != NULL){
		// Получаем данные подключившегося клиента
		string host = proxy->get_host(address, socklen);
		// Выводим в консоль сообщение
		printf("client host = %s\n", host.c_str());
		// Создаем новый объект подключения
		BufferHttpProxy * http = new BufferHttpProxy(proxy->name_app);
		// Запоминаем таймауты
		http->read_timeout		= proxy->read_timeout;
		http->write_timeout		= proxy->write_timeout;
		http->keepalive_timeout	= proxy->keepalive_timeout;
		// Запоминаем размеры буферов
		http->buffer_read_size	= proxy->buffer_read_size;
		http->buffer_write_size	= proxy->buffer_write_size;
		// Запоминаем базу событий
		http->base = evconnlistener_get_base(listener);
		// Создаем буфер событий
		http->events.client = bufferevent_socket_new(http->base, fd, BEV_OPT_CLOSE_ON_FREE);
		// Устанавливаем размеры буферов
		set_buffer_size(fd, http->buffer_read_size, http->buffer_write_size);
		// Устанавливаем таймаут ожидания запроса
		struct timeval tread = {http->keepalive_timeout, 0};
		// Устанавливаем таймауты
		bufferevent_set_timeouts(http->events.client, &tread, NULL);
		// Устанавливаем коллбеки
		bufferevent_setcb(http->events.client, &HttpProxy::read_client, NULL, &HttpProxy::event, http);
		// Активируем буферы событий на чтение и запись
		bufferevent_enable(http->events.client, EV_READ | EV_WRITE);
		// Очищаем буферы событий при завершении работы
		bufferevent_flush(http->events.client, EV_READ | EV_WRITE, BEV_FINISHED);
	}
	// Выходим
	return;
}
/**
 * HttpProxy Конструктор
 * @param name       название прокси-сервера
 * @param host       хост прокси-сервера
 * @param port       порт прокси-сервера
 * @param buffrsize  размер буфера сокета на чтение
 * @param buffwsize  размер буфера сокета на запись
 * @param maxcls     максимальное количество подключаемых клиентов к прокси-серверу (-1 автоматически)
 * @param rtm        таймаут на чтение данных из сокета сервера
 * @param wtm        таймаут на запись данных из сокета клиента и сервера
 * @param katm       таймаут на чтение данных из сокета клиента
 */
HttpProxy::HttpProxy(
	const char * name,
	const char * host,
	u_int port,
	int buffrsize,
	int buffwsize,
	int maxcls,
	u_short rtm,
	u_short wtm,
	u_short katm
){
	// Запоминаем название системы
	name_app = name;
	// Запоминаем таймауты
	read_timeout		= rtm;
	write_timeout		= wtm;
	keepalive_timeout	= katm;
	// Структура для создания сервера приложения
	struct sockaddr_in sin;
	// Создаем новую базу
	struct event_base * base = event_base_new();
	// Структура определяющая параметры сервера приложений
	struct hostent * server = gethostbyname(host);
	// Очищаем всю структуру
	memset(&sin, 0, sizeof(sin));
	// Listen on 0.0.0.0
	// sin.sin_addr.s_addr = htonl(0);
	// This is an INET address
	sin.sin_family = AF_INET;
	// Указываем адрес прокси сервера
	sin.sin_addr.s_addr = * ((unsigned long *) server->h_addr);
	// Listen on the given port.
	sin.sin_port = htons(port);
	// Вешаем приложение на порт
	listener = evconnlistener_new_bind(
		base, &HttpProxy::accept_connect, this,
		LEV_OPT_REUSEABLE |
		//LEV_OPT_THREADSAFE |
		LEV_OPT_CLOSE_ON_FREE |
		LEV_OPT_LEAVE_SOCKETS_BLOCKING,
		maxcls, (struct sockaddr *) &sin, sizeof(sin)
	);
	// Если подключение не удалось
	if(!listener){
		// Выводим сообщение об ошибке
		perror("Couldn't create listener");
		// Выходим
		return;
	}
	// Получаем сокет приложения
	evutil_socket_t socket = evconnlistener_get_fd(listener);
	// Устанавливаем неблокирующий режим
	set_tcpnodelay(socket);
	// Размер буфера на чтение
	buffer_read_size = buffrsize;
	// Размер буфера на запись
	buffer_write_size = buffwsize;
	// Устанавливаем размеры буферов
	set_buffer_size(socket, ((maxcls > 0 ? maxcls : 1) * buffer_read_size), ((maxcls > 0 ? maxcls : 1) * buffer_write_size));
	// Устанавливаем обработчик на получение ошибок
	evconnlistener_set_error_cb(listener, &HttpProxy::accept_error);
	// Активируем перебор базы событий
	event_base_dispatch(base);
	// Активируем перебор базы событий
	// event_base_loop(base, EVLOOP_NO_EXIT_ON_EMPTY);
}
/**
 * HttpProxy Конструктор
 */
HttpProxy::~HttpProxy(){
	// Удаляем слушателя
	evconnlistener_free(listener);
}