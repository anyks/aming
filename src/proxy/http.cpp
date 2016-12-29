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
 * appconn Функция которая добавляет или удаляет в список склиента
 * @param flag флаг подключения или отключения клиента
 */
void BufferHttpProxy::appconn(bool flag){
	// Если такое подключение найдено
	if((*this->connects).count(this->client.host) > 0){
		// Получаем объект текущего коннекта
		Connects * connect = &((*this->connects).find(this->client.host)->second);
		// Выполняем захват мютекса
		connect->lock();
		// Если нужно добавить подключение
		if(flag) connect->inc();
		// Если нужно удалить подключение
		else {
			// Уменьшаем количество подключений
			connect->dec();
			// Отправляем сигнал
			connect->signal();
		}
		// Выполняем разблокировку мютекса
		connect->unlock();
		// Проверяем есть ли еще подключения
		if(connect->end()) (*this->connects).erase(this->client.host);
	// Если нужно добавить подключение
	} else if(flag){
		// Создаем объект подключения
		Connects connect;
		// Добавляем в список новое подключение
		(*this->connects).insert(pair <string, Connects> (this->client.host, connect));
	}
}
/**
 * getmac Метод определения мак адреса клиента
 * @param  address структура параметров подключения
 * @return данные мак адреса
 */
string HttpProxy::getmac(struct sockaddr * address){
	// Буфер для копирования мак адреса
	char buff[256];
	// Получаем указатель на мак адрес
	unsigned char * ptr = (unsigned char *) address->sa_data;
	// Записываем в буфер данные мак адреса
	sprintf(
		buff, "%02X:%02X:%02X:%02X:%02X:%02X",
		(ptr[0] & 0xff), (ptr[1] & 0xff), (ptr[2] & 0xff),
		(ptr[3] & 0xff), (ptr[4] & 0xff), (ptr[5] & 0xff)
	);
	// Выводим данные мак адреса
	return buff;
}
/**
 * gethost Функция получения данных хоста
 * @param  address структура параметров подключения
 * @param  socklen размер структуры
 * @return         данные полученного хоста
 */
string HttpProxy::gethost(struct sockaddr * address, int socklen){
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
 * set_nonblock Функция установки неблокирующего сокета
 * @param  fd   файловый дескриптор (сокет)
 * @param  log  указатель на объект ведения логов
 * @return      результат работы функции
 */
int HttpProxy::set_nonblock(evutil_socket_t fd, LogApp * log){
	int flags;
	flags = fcntl(fd, F_GETFL);
	if(flags < 0) return flags;
	flags |= O_NONBLOCK;
	// Устанавливаем неблокирующий режим
	if(fcntl(fd, F_SETFL, flags) < 0){
		// Выводим в лог информацию
		log->write(LOG_ERROR, "cannot set NON_BLOCK option on socket %d", fd);
		// Выходим
		return -1;
	}
	// Все удачно
	return 0;
}
/**
 * set_non_block Функция отключения алгоритма Нейгла
 * @param  fd   файловый дескриптор (сокет)
 * @param  log  указатель на объект ведения логов
 * @return      результат работы функции
 */
int HttpProxy::set_tcpnodelay(evutil_socket_t fd, LogApp * log){
	// Устанавливаем параметр
	int tcpnodelay = 1;
	// Устанавливаем TCP_NODELAY
	if(setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &tcpnodelay, sizeof(tcpnodelay)) < 0){
		// Выводим в лог информацию
		log->write(LOG_ERROR, "cannot set TCP_NODELAY option on socket %d", fd);
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
 * @param  log        указатель на объект ведения логов
 * @return            результат работы функции
 */
int HttpProxy::set_buffer_size(evutil_socket_t fd, int read_size, int write_size, LogApp * log){
	// Определяем размер массива опции
	socklen_t read_optlen	= sizeof(read_size);
	socklen_t write_optlen	= sizeof(write_size);
	// Устанавливаем размер буфера для сокета клиента и сервера
	if(read_size > 0)	setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *) &read_size, read_optlen);
	if(write_size > 0)	setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *) &write_size, write_optlen);
	// Считываем установленный размер буфера
	if((getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &read_size, &read_optlen) < 0)
	|| (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &write_size, &write_optlen) < 0)){
		// Выводим в лог информацию
		log->write(LOG_ERROR, "get buffer wrong on socket %d", fd);
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
		// Выводим в лог информацию о неудачном подключении
		http->proxy.log->write(LOG_MESSAGE, "auth client [%s] to proxy wrong!", http->client.host.c_str());
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
			// Если ip адрес существует
			if(!ip.empty()){
				// Если хост и порт сервера не совпадают тогда очищаем данные
				if((http->events.server != NULL)
				&& ((http->server.host != ip)
				|| (http->server.port != port))) http->free_server();
				// Если сервер еще не подключен
				if(http->events.server == NULL){
					// Сокет подключения
					evutil_socket_t sock = -1;
					// Запоминаем хост и порт сервера
					http->server.host = ip;
					http->server.port = port;
					// Структуры серверного и локального подключений
					struct sockaddr_in server_addr, local_addr;
					// Очищаем всю структуру для клиента
					memset(&local_addr, 0, sizeof(local_addr));
					// Очищаем всю структуру для сервера
					memset(&server_addr, 0, sizeof(server_addr));
					// Создаем сокет для подключения
					if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
						// Выводим в лог сообщение
						http->proxy.log->write(LOG_ERROR, "creating socket to server = %s, port = %d, client = %s", ip.c_str(), port, http->client.host.c_str());
						// Выходим
						return 0;
					}
					// Запоминаем сокет сервера
					http->sockets.server = sock;
					// Неважно, IPv4 или IPv6
					local_addr.sin_family = AF_UNSPEC;
					// Устанавливаем произвольный порт для локального подключения
					local_addr.sin_port = htons(0);
					// Устанавливаем адрес для локальго подключения
					local_addr.sin_addr.s_addr = inet_addr(http->proxy.external.c_str());
					// Выполняем бинд на сокет
					if(::bind(http->sockets.server, (struct sockaddr *) &local_addr, sizeof(local_addr)) < 0){
						// Выводим в лог сообщение
						http->proxy.log->write(LOG_ERROR, "bind local network [%s] error", "192.168.3.43");
						// Выходим
						return 0;
					}
					// Неважно, IPv4 или IPv6
					server_addr.sin_family = AF_UNSPEC;
					// Устанавливаем порт для локального подключения
					server_addr.sin_port = htons(http->server.port);
					// Устанавливаем адрес для удаленного подключения
					server_addr.sin_addr.s_addr = inet_addr(http->server.host.c_str());
					// Обнуляем серверную структуру
					bzero(&(server_addr.sin_zero), 8);
					// Разблокируем сокет
					// set_nonblock(http->sockets.server, http->log);
					// Устанавливаем размеры буферов
					set_buffer_size(http->sockets.server, http->proxy.bsize.read, http->proxy.bsize.write, http->proxy.log);
					// Если подключение постоянное
					if(http->parser->isAlive()){
						// Устанавливаем TCP_NODELAY для сервера и клиента
						set_tcpnodelay(http->sockets.server, http->proxy.log);
						set_tcpnodelay(http->sockets.client, http->proxy.log);
					}
					// Создаем буфер событий для сервера
					http->events.server = bufferevent_socket_new(http->base, http->sockets.server, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
					// Устанавливаем коллбеки
					bufferevent_setcb(http->events.server, &HttpProxy::read_server, NULL, &HttpProxy::event, http);
					// Активируем буферы событий на чтение и запись
					bufferevent_enable(http->events.server, EV_READ | EV_WRITE);
					// Очищаем буферы событий при завершении работы
					bufferevent_flush(http->events.server, EV_READ | EV_WRITE, BEV_FINISHED);
					// Выполняем подключение к удаленному серверу, если подключение не выполненно то сообщаем об этом
					if(bufferevent_socket_connect(http->events.server, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) < 0){
						// Выводим в лог сообщение
						http->proxy.log->write(LOG_ERROR, "connecting to server = %s, port = %d, client = %s", ip.c_str(), port, http->client.host.c_str());
						// Очищаем буфер сервера
						http->free_server();
						// Выходим
						return -1;
					}
					// Получаем данные мак адреса клиента
					http->server.mac = getmac((struct sockaddr *) &server_addr);
					// Выводим в лог сообщение о новом коннекте
					http->proxy.log->write(
						LOG_MESSAGE,
						"connect client [%s] to host = %s [%s:%d], mac = %s, method = %s, path = %s, useragent = %s, socket = %d",
						http->client.host.c_str(),
						http->parser->getHost().c_str(),
						http->server.host.c_str(),
						http->server.port,
						http->server.mac.c_str(),
						http->parser->getMethod().c_str(),
						http->parser->getPath().c_str(),
						http->parser->getUseragent().c_str(),
						http->sockets.client
					);
					// Сообщаем что все удачно
					return 1;
				// Если сервер уже подключен, сообщаем что все удачно
				} else {
					// Выводим в лог сообщение о новом коннекте
					http->proxy.log->write(
						LOG_MESSAGE,
						"last connect client [%s] to host = %s [%s:%d], mac = %s, method = %s, path = %s, useragent = %s, socket = %d",
						http->client.host.c_str(),
						http->parser->getHost().c_str(),
						http->server.host.c_str(),
						http->server.port,
						http->server.mac.c_str(),
						http->parser->getMethod().c_str(),
						http->parser->getPath().c_str(),
						http->parser->getUseragent().c_str(),
						http->sockets.client
					);
					// Сообщаем что все удачно
					return 2;
				}
			}
		}
		// Выводим в лог сообщение
		http->proxy.log->write(
			LOG_ERROR,
			"host server = %s not found, port = %d, client = %s",
			http->parser->getHost().c_str(),
			http->parser->getPort(),
			http->client.host.c_str()
		);
		// Выходим
		return 0;
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
		// Определяем для кого вызвано событие
		string subject = (current_fd == http->sockets.client ? "client" : "server");
		// Если подключение удачное
		if(events & BEV_EVENT_CONNECTED){
			// Если это сервер
			if(subject == "server"){
				// Выводим в лог сообщение
				http->proxy.log->write(
					LOG_ACCESS,
					"connect client [%s] to server host = %s [%s:%d], method = %s, path = %s, useragent = %s, socket = %d",
					http->client.host.c_str(),
					http->parser->getHost().c_str(),
					http->server.host.c_str(),
					http->server.port,
					http->parser->getMethod().c_str(),
					http->parser->getPath().c_str(),
					http->parser->getUseragent().c_str(),
					current_fd
				);
			}
		// Если это ошибка или завершение работы
		} else if(events & (BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT)) {
			// Если это ошибка
			if(events & BEV_EVENT_ERROR){
				// Получаем данные ошибки
				int err = bufferevent_socket_get_dns_error(bev);
				// Если ошибка существует, выводим сообщение в консоль
				if(err) http->proxy.log->write(LOG_ERROR, "DNS error: %s", evutil_gai_strerror(err));
			}
			// Если отключился клиент
			if(subject == "client"){
				// Выводим в лог сообщение
				http->proxy.log->write(
					LOG_ACCESS,
					"closing client [%s] from server [%s:%d], socket = %d",
					http->client.host.c_str(),
					http->parser->getHost().c_str(),
					http->parser->getPort(),
					current_fd
				);
			// Если отключился сервер
			} else {
				// Выводим в лог сообщение
				http->proxy.log->write(
					LOG_ACCESS,
					"closing server [%s:%d] from client [%s], socket = %d",
					http->parser->getHost().c_str(),
					http->parser->getPort(),
					http->client.host.c_str(),
					current_fd
				);
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
		// Определяем тип прокси
		bool smart = (http->proxy.options & OPT_SMART);
		// Определяем разрешено ли выводить название агента
		bool agent = (http->proxy.options & OPT_AGENT);
		// Получаем буферы входящих данных и исходящих
		struct evbuffer * input		= bufferevent_get_input(http->events.server);
		struct evbuffer * output	= bufferevent_get_output(http->events.client);
		// Устанавливаем таймаут ожидания результата
		struct timeval tread = {http->proxy.timeout.keepalive, 0};
		// Устанавливаем таймаут записи результата
		struct timeval twrite = {http->proxy.timeout.write, 0};
		// Устанавливаем таймауты
		bufferevent_set_timeouts(http->events.client, &tread, &twrite);
		// Если заголовки менять не надо тогда просто обмениваемся данными
		if(smart && !agent) evbuffer_add_buffer(output, input);
		// Иначе изменяем заголовки
		else {
			// Получаем размер входящих данных
			size_t len = evbuffer_get_length(input);
			// Создаем буфер данных
			char * buffer = new char[len];
			// Копируем в буфер полученные данные
			evbuffer_copyout(input, buffer, len);
			// Создаем буфер для исходящих данных
			struct evbuffer * tmp = evbuffer_new();
			// Создаем указатель поиска в буфере
			struct evbuffer_ptr ptr;
			// Выполняем инициализацию указателя
			evbuffer_ptr_set(input, &ptr, 0, EVBUFFER_PTR_SET);
			// Строка для поиска конца блока с заголовками
			const char * str = "\r\n\r\n";
			// Выполняем поиск конца блока с заголовками
			ptr = evbuffer_search(input, str, strlen(str), &ptr);
			// Если в буфере нашли заголовки
			if(ptr.pos > -1){
				// Добавляем полученный буфер в вектор
				vector <char> headers(buffer, buffer + len);
				// Выполняем модификацию заголовков
				http->parser->modify(headers);
				// Добавляем в новый буфер модифицированные заголовки
				evbuffer_add(tmp, headers.data(), headers.size());
			// Если в буфере заголовки не найдены тогда просто записываем данные в новый буфер
			} else evbuffer_add(tmp, buffer, len);
			// Удаляем данные из буфера
			evbuffer_drain(input, len);
			// Отправляем данные клиенту
			evbuffer_add_buffer(output, tmp);
			// Удаляем временный буфер
			evbuffer_free(tmp);
			// Удаляем буфер данных
			delete [] buffer;
		}
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
		// Определяем connect прокси разрешен
		bool conn_enabled = (http->proxy.options & OPT_CONNECT);
		// Получаем буферы входящих данных и исходящих
		struct evbuffer * input = bufferevent_get_input(bev);
		// Если авторизация прошла, и коннект произведен
		if((http->response.code == 200)
		&& http->parser->isConnect()
		&& (conn_enabled || http->parser->isHttps())){
			// Получаем буферы входящих данных и исходящих
			struct evbuffer * output = bufferevent_get_output(http->events.server);
			// Устанавливаем таймаут ожидания результата
			struct timeval tread = {http->proxy.timeout.read, 0};
			// Устанавливаем таймаут записи результата
			struct timeval twrite = {http->proxy.timeout.write, 0};
			// Устанавливаем таймауты
			bufferevent_set_timeouts(http->events.server, &tread, &twrite);
			// Выводим ответ сервера
			evbuffer_add_buffer(output, input);
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
						if(http->parser->isConnect() && (conn_enabled || http->parser->isHttps()))
							// Формируем ответ клиенту
							http->response = http->parser->authSuccess();
						// Если connect разрешен только для https подключений
						else if(!conn_enabled && http->parser->isConnect())
							// Сообращем что подключение запрещено
							http->response = http->parser->faultConnect();
						// Иначе делаем запрос на получение данных
						else {
							// Указываем что нужно отключится сразу после отправки запроса
							if(!http->parser->isAlive()) http->parser->setClose();
							// Формируем запрос на сервер
							http->response = http->parser->getQuery();
							// Устанавливаем таймаут ожидания результата
							struct timeval tread = {http->proxy.timeout.read, 0};
							// Устанавливаем таймаут записи результата
							struct timeval twrite = {http->proxy.timeout.write, 0};
							// Устанавливаем таймауты
							bufferevent_set_timeouts(http->events.server, &tread, &twrite);
							// Отправляем серверу сообщение
							bufferevent_write(http->events.server, http->response.data(), http->response.size());
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
					// Устанавливаем таймаут записи результата
					struct timeval twrite = {http->proxy.timeout.write, 0};
					// Если это код разрешающий коннект
					if(http->response.code == 200){
						// Устанавливаем таймаут ожидания результата
						struct timeval tread = {http->proxy.timeout.keepalive, 0};
						// Устанавливаем таймауты
						bufferevent_set_timeouts(bev, &tread, &twrite);
					// Если это завершение работы то устанавливаем таймер только на запись
					} else bufferevent_set_timeouts(bev, NULL, &twrite);
					// Отправляем клиенту сообщение
					bufferevent_write(bev, http->response.data(), http->response.size());
				}
			}
		}
	}
	// Выходим
	return;
}
/**
 * connection Функция обработки данных подключения в треде
 * @param ctx передаваемый объект
 */
void * HttpProxy::connection(void * ctx){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	// Если подключение не передано
	if(http != NULL){
		// Если такое подключение найдено
		if((*http->connects).count(http->client.host) > 0){
			// Получаем объект текущего коннекта
			Connects * connect = &((*http->connects).find(http->client.host)->second);
			// Выполняем захват мютекса
			connect->lock();
			// Если количество подключений достигло предела
			if(connect->get() >= MAX_CONNECTS) connect->wait();
			// Выполняем разблокировку мютекса
			connect->unlock();
		}
		// Выполняем инициализацию подключения
		http->begin();
		// Создаем новую базу событий
		struct event_base * base = event_base_new();
		// Запоминаем базу событий
		http->base = base;
		// Создаем буфер событий
		http->events.client = bufferevent_socket_new(http->base, http->sockets.client, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
		// Устанавливаем размеры буферов
		set_buffer_size(http->sockets.client, http->proxy.bsize.read, http->proxy.bsize.write, http->proxy.log);
		// Устанавливаем таймаут ожидания запроса
		struct timeval tread = {http->proxy.timeout.keepalive, 0};
		// Устанавливаем таймауты
		bufferevent_set_timeouts(http->events.client, &tread, NULL);
		// Устанавливаем коллбеки
		bufferevent_setcb(http->events.client, &HttpProxy::read_client, NULL, &HttpProxy::event, http);
		// Активируем буферы событий на чтение и запись
		bufferevent_enable(http->events.client, EV_READ | EV_WRITE);
		// Очищаем буферы событий при завершении работы
		bufferevent_flush(http->events.client, EV_READ | EV_WRITE, BEV_FINISHED);
		// Активируем перебор базы событий
		event_base_dispatch(http->base);
		// Удаляем базу
		event_base_free(base);
		// Отключаем подключение для сокетов
		shutdown(http->sockets.client, SHUT_RDWR);
		shutdown(http->sockets.server, SHUT_RDWR);
		// Закрываем сокеты
		close(http->sockets.client);
		close(http->sockets.server);
		// Удаляем объект подключения
		delete http;
	}
	// Выходим
	return 0;
}
/**
 * accept_error Событие возникновения ошибки подключения
 * @param listener объект подключения
 * @param ctx      передаваемый объект
 */
void HttpProxy::accept_error(struct evconnlistener * listener, void * ctx){
	// Получаем объект прокси сервера
	HttpProxy * proxy = reinterpret_cast <HttpProxy *> (ctx);
	// Если прокси существует
	if(proxy != NULL){
		// Получаем базу событий
		struct event_base * base = evconnlistener_get_base(listener);
		// Получаем сообщение об ошибке
		int err = EVUTIL_SOCKET_ERROR();
		// Выводим сообщение в лог
		proxy->server.log->write(LOG_ERROR, "Got an error %d (%s) on the listener. Shutting down.", err, evutil_socket_error_to_string(err));
		// Удаляем базу событий
		event_base_loopexit(base, NULL);
	}
	// Выходим
	return;
}
/**
 * spawn_thread Функция создания треда
 * @param  thread объект треда
 * @param  ctx    передаваемый объект
 * @return        результат работы функции
 */
bool HttpProxy::spawn_thread(pthread_t * thread, void * ctx){
	// Создаем атрибут потока управления
	pthread_attr_t attr;
	// Инициализируем описатель атрибута потока управления
	pthread_attr_init(&attr);
	// Устанавливаем атрибут "размер стека" потока управления
	pthread_attr_setstacksize(&attr, 64 * 1024);
	// Устанавливаем статус освобождения ресурсов
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	// Создаем поток управления
	return !pthread_create(thread, &attr, &HttpProxy::connection, ctx);
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
	// Получаем объект прокси сервера
	HttpProxy * proxy = reinterpret_cast <HttpProxy *> (ctx);
	// Если прокси существует
	if(proxy != NULL){
		// Устанавливаем неблокирующий режим для сокета
		set_nonblock(fd, proxy->server.log);
		// Получаем данные подключившегося клиента
		string host = gethost(address, socklen);
		// Получаем данные мак адреса клиента
		string mac = getmac(address);
		// Выводим в лог сообщение
		proxy->server.log->write(LOG_ACCESS, "client connect to proxy server, host = %s, mac = %s, socket = %d", host.c_str(), mac.c_str(), fd);
		// Создаем новый объект подключения
		BufferHttpProxy * http = new BufferHttpProxy(proxy->server.name, proxy->server.version, proxy->server.options);
		// Запоминаем параметры прокси сервера
		http->proxy = proxy->server;
		// Запоминаем список подключений
		http->connects = &proxy->connects;
		// Запоминаем файловый дескриптор текущего подключения
		http->sockets.client = fd;
		// Запоминаем данные мак адреса
		http->client.mac = mac;
		// Запоминаем данные клиента
		http->client.host = host;
		// Создаем поток
		pthread_t thread;
		// Выполняем активацию потока
		spawn_thread(&thread, http);
	}
	// Выходим
	return;
}
/**
 * HttpProxy Конструктор
 * @param log        указатель на объект ведения логов
 * @param name       название прокси сервера
 * @param version    версия прокси сервера
 * @param internal   внутренний хост прокси сервера
 * @param external   внешний хост прокси сервера
 * @param port       порт прокси сервера
 * @param buffrsize  размер буфера сокета на чтение
 * @param buffwsize  размер буфера сокета на запись
 * @param maxcls     максимальное количество подключаемых клиентов к прокси серверу (-1 автоматически)
 * @param rtm        таймаут на чтение данных из сокета сервера
 * @param wtm        таймаут на запись данных из сокета клиента и сервера
 * @param katm       таймаут на чтение данных из сокета клиента
 * @param options    опции прокси сервера
 */
HttpProxy::HttpProxy(
	LogApp * log,
	const char * name,
	const char * version,
	const char * internal,
	const char * external,
	u_int port,
	int buffrsize,
	int buffwsize,
	int maxcls,
	u_short rtm,
	u_short wtm,
	u_short katm,
	u_short options
){
	// Если лог существует
	if(log != NULL){
		// Запоминаем параметры прокси сервера
		this->server = {
			name,		// Запоминаем название системы
			version,	// Запоминаем версию системы
			internal,	// Запоминаем внутренний адрес прокси сервера
			external,	// Запоминаем внешний адрес прокси сервера
			options,	// Запоминаем тип прокси сервера
			{
				rtm,	// Запоминаем Read timeout
				wtm,	// Запоминаем Write timeout
				katm,	// Запоминаем Keep-Alive timeout
			},{
				buffrsize,	// Размер буфера на чтение
				buffwsize	// Размер буфера на запись
			},
			log // Запоминаем объект ведения логов
		};
		// Структура для создания сервера приложения
		struct sockaddr_in sin;
		// Создаем новую базу
		struct event_base * base = event_base_new();
		// Структура определяющая параметры сервера приложений
		struct hostent * server = gethostbyname(internal);
		// Очищаем всю структуру
		memset(&sin, 0, sizeof(sin));
		// Listen on 0.0.0.0
		// sin.sin_addr.s_addr = htonl(0);
		// Устанавливаем тип подключения что это INET адрес
		sin.sin_family = AF_INET;
		// Указываем адрес прокси сервера
		sin.sin_addr.s_addr = * ((unsigned long *) server->h_addr);
		// Указываем локальный порт
		sin.sin_port = htons(port);
		// Вешаем приложение на порт
		listener = evconnlistener_new_bind(
			base, &HttpProxy::accept_connect, this,
			LEV_OPT_REUSEABLE |
			// LEV_OPT_THREADSAFE |
			LEV_OPT_CLOSE_ON_FREE,// |
			// LEV_OPT_LEAVE_SOCKETS_BLOCKING,
			maxcls, (struct sockaddr *) &sin, sizeof(sin)
		);
		// Если подключение не удалось
		if(!listener){
			// Выводим в консоль информацию
			this->server.log->write(LOG_ERROR, "[-] Couldn't create listener");
			// Выходим
			exit(1);
		}
		// Получаем сокет приложения
		evutil_socket_t socket = evconnlistener_get_fd(listener);
		// Устанавливаем неблокирующий режим
		set_tcpnodelay(socket, this->server.log);
		// Устанавливаем обработчик на получение ошибок
		evconnlistener_set_error_cb(listener, &HttpProxy::accept_error);
		// Активируем перебор базы событий
		event_base_dispatch(base);
		// Активируем перебор базы событий
		// event_base_loop(base, EVLOOP_NO_EXIT_ON_EMPTY);
	}
}
/**
 * HttpProxy Конструктор
 */
HttpProxy::~HttpProxy(){
	// Удаляем слушателя
	evconnlistener_free(listener);
}