/* СЕРВЕР HTTP ПРОКСИ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#include "http.h"

// Устанавливаем пространство имен
using namespace std;

/**
 * get Метод получения количества подключений
 * @return количество активных подключений
 */
inline size_t Connects::get(){
	// Выводим количество подключений
	return this->connects;
}
/**
 * end Метод проверки на конец всех подключений
 * @return проверка на достижения нуля
 */
inline bool Connects::end(){
	// Если количество подключений меньше 1 то сообщаем об этом
	return !(this->connects > 0);
}
/**
 * inc Метод инкреминации количества подключений
 */
inline void Connects::inc(){
	// Выполняем инкремент
	this->connects++;
}
/**
 * dec Метод декрементации количества подключений
 */
inline void Connects::dec(){
	// Выполняем декрементацию
	if(this->connects > 0) this->connects--;
}
/**
 * lock Метод блокировки мютекса
 */
inline void Connects::lock(){
	// Лочим мютекс
	pthread_mutex_lock(&this->mutex);
}
/**
 * unlock Метод разблокировки мютекса
 */
inline void Connects::unlock(){
	// Разлочим мютекс
	pthread_mutex_unlock(&this->mutex);
}
/**
 * signal Метод отправки сигнала первому блокированному потоку
 */
inline void Connects::signal(){
	// Отправляем сигнал
	pthread_cond_signal(&this->condition);
}
/**
 * broadcastSignal Метод отправки сигналов всем блокированным потокам
 */
inline void Connects::broadcastSignal(){
	// Выполняем вещание
	pthread_cond_broadcast(&this->condition);
}
/**
 * wait Метод блокировки потока
 */
inline void Connects::wait(){
	// Блокируем поток
	pthread_cond_wait(&this->condition, &this->mutex);
}
/**
 * Connects Конструктор
 */
Connects::Connects(){
	// Устанавливаем первоначальное значение коннекта
	this->connects = 1;
	// Инициализируем мютекс
	pthread_mutex_init(&this->mutex, 0);
	// Инициализируем переменную состояния
	pthread_cond_init(&this->condition, 0);
}
/**
 * ~Connects Деструктор
 */
Connects::~Connects(){
	// Удаляем мютекс
	pthread_mutex_destroy(&this->mutex);
	// Удаляем переменную состояния
	pthread_cond_destroy(&this->condition);
}
/**
 * lock Метод блокировки мютекса
 */
inline void BufferHttpProxy::lock(){
	// Лочим мютекс
	pthread_mutex_lock(&this->mutex);
}
/**
 * unlock Метод разблокировки мютекса
 */
inline void BufferHttpProxy::unlock(){
	// Разлочим мютекс
	pthread_mutex_unlock(&this->mutex);
}
/**
 * appconn Функция которая добавляет или удаляет в список склиента
 * @param flag флаг подключения или отключения клиента
 */
void BufferHttpProxy::appconn(bool flag){
	// Если такое подключение найдено
	if((*this->connects).count(this->client.ip) > 0){
		// Получаем объект текущего коннекта
		Connects * connect = &((*this->connects).find(this->client.ip)->second);
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
		if(connect->end()) (*this->connects).erase(this->client.ip);
	// Если нужно добавить подключение
	} else if(flag){
		// Создаем объект подключения
		Connects connect;
		// Добавляем в список новое подключение
		(*this->connects).insert(pair <string, Connects> (this->client.ip, connect));
	}
}
/**
 * free_socket Метод отключения сокета
 * @param fd ссылка на файловый дескриптор (сокет)
 */
void BufferHttpProxy::free_socket(evutil_socket_t * fd){
	// Если сокет существует
	if(*fd > -1){
		// Отключаем подключение для сокета
		shutdown(*fd, SHUT_RDWR);
		// Закрываем сокет
		close(*fd);
		// Сообщаем что сокет закрыт
		*fd = -1;
	}
}
/**
 * free_event Метод удаления буфера события
 * @param event указатель на объект буфера события
 */
void BufferHttpProxy::free_event(struct bufferevent ** event){
	// Удаляем событие
	if(*event != NULL){
		// Очищаем таймауты
		bufferevent_set_timeouts(*event, NULL, NULL);
		// Удаляем буфер события
		bufferevent_free(*event);
		// Устанавливаем что событие удалено
		*event = NULL;
	}
}
/**
 * begin Метод активации подключения
 */
void BufferHttpProxy::begin(){
	// Добавляем в список подключений
	this->appconn(true);
}
/**
 * close_client Метод закрытия соединения клиента
 */
void BufferHttpProxy::close_client(){
	// Захватываем поток
	this->lock();
	// Закрываем сокет
	free_socket(&this->sockets.client);
	// Закрываем буфер события
	free_event(&this->events.client);
	// Отпускаем поток
	this->unlock();
}
/**
 * close_server Метод закрытия соединения сервера
 */
void BufferHttpProxy::close_server(){
	// Захватываем поток
	this->lock();
	// Закрываем сокет
	free_socket(&this->sockets.server);
	// Закрываем буфер события
	free_event(&this->events.server);
	// Отпускаем поток
	this->unlock();
}
/**
 * set_timeout Метод установки таймаутов
 * @param type  тип подключения (клиент или сервер)
 * @param read  таймаут на чтение
 * @param write таймаут на запись
 */
void BufferHttpProxy::set_timeout(u_short type, bool read, bool write){
	// Устанавливаем таймаут ожидания результата на чтение с сервера
	struct timeval _read = {this->proxy.config->timeouts.read, 0};
	// Устанавливаем таймаут записи результата на запись
	struct timeval _write = {this->proxy.config->timeouts.write, 0};
	// Устанавливаем таймаут ожидания результата на чтение с клиента
	struct timeval _keepalive = {this->proxy.config->timeouts.keepalive, 0};
	// Устанавливаем таймауты для сервера
	if((type & TM_SERVER) && (this->events.server != NULL)){
		// Если время постоянного подключения меньше 1 секунды значит таймер ставить не надо
		if(this->proxy.config->timeouts.keepalive < 1) read = false;
		// Устанавливаем таймауты
		bufferevent_set_timeouts(this->events.server, (read ? &_read : NULL), (write ? &_write : NULL));
	}
	// Устанавливаем таймауты для клиента
	if((type & TM_CLIENT) && (this->events.client != NULL)){
		// Если время постоянного подключения меньше 1 секунды значит таймер ставить не надо
		if(this->proxy.config->timeouts.keepalive < 1) read = false;
		// Устанавливаем таймауты
		bufferevent_set_timeouts(this->events.client, (read ? &_keepalive : NULL), (write ? &_write : NULL));
	}
}
/**
 * BufferHttpProxy Конструктор
 * @param string  name    имя ресурса
 * @param u_short options параметры прокси сервера
 */
BufferHttpProxy::BufferHttpProxy(string name, u_short options){
	// Инициализируем мютекс
	pthread_mutex_init(&this->mutex, 0);
	// Создаем объект для работы с http заголовками
	this->parser = Http(name, options);
}
/**
 * ~BufferHttpProxy Деструктор
 */
BufferHttpProxy::~BufferHttpProxy(){
	// Очищаем буфер событий клиента
	close_client();
	// Очищаем буфер событий сервера
	close_server();
	// Удаляем из списока подключений
	appconn(false);
	// Удаляем базу событий
	event_base_loopexit(this->base, NULL);
	// Удаляем мютекс
	pthread_mutex_destroy(&this->mutex);
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
		if(!strcmp(http->httpData.getLogin().c_str(), username)
		&& !strcmp(http->httpData.getPassword().c_str(), password)) return true;
		// Выводим в лог информацию о неудачном подключении
		http->proxy.log->write(LOG_MESSAGE, "auth client [%s] to proxy wrong!", http->client.ip.c_str());
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
		hostent * sh = gethostbyname(http->httpData.getHost().c_str());
		// Если данные хоста найдены
		if(sh){
			// ip адрес ресурса
			string ip;
			// Получаем порт сервера
			u_int port = http->httpData.getPort();
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
				&& ((http->server.host.compare(ip) != 0)
				|| (http->server.port != port))) http->close_server();
				// Если сервер еще не подключен
				if(http->events.server == NULL){
					// Создаем хост подключения
					string host;
					// Определяем тип подключения
					switch(http->proxy.config->proxy.ipver){
						// Для протокола IPv4
						case 4: host = http->proxy.config->ipv4.external; break;
						// Для протокола IPv6
						case 6: host = http->proxy.config->ipv6.external; break;
					}
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
						http->proxy.log->write(LOG_ERROR, "creating socket to server = %s, port = %d, client = %s", ip.c_str(), port, http->client.ip.c_str());
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
					local_addr.sin_addr.s_addr = inet_addr(host.c_str());
					// Выполняем бинд на сокет
					if(::bind(http->sockets.server, (struct sockaddr *) &local_addr, sizeof(local_addr)) < 0){
						// Выводим в лог сообщение
						http->proxy.log->write(LOG_ERROR, "bind local network [%s] error", host.c_str());
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
					set_nonblock(http->sockets.server, http->proxy.log);
					// Устанавливаем размеры буферов
					set_buffer_size(http->sockets.server, http->proxy.config->buffers.read, http->proxy.config->buffers.write, http->proxy.log);
					// Если подключение постоянное
					if(http->client.alive){
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
						http->proxy.log->write(LOG_ERROR, "connecting to server = %s, port = %d, client = %s", ip.c_str(), port, http->client.ip.c_str());
						// Выходим
						return -1;
					}
					// Получаем данные мак адреса клиента
					http->server.mac = getmac((struct sockaddr *) &server_addr);
					// Выводим в лог сообщение о новом коннекте
					http->proxy.log->write(
						LOG_MESSAGE,
						"connect client [%s] to host = %s [%s:%d], mac = %s, method = %s, path = %s, useragent = %s, socket = %d",
						http->client.ip.c_str(),
						http->httpData.getHost().c_str(),
						http->server.host.c_str(),
						http->server.port,
						http->server.mac.c_str(),
						http->httpData.getMethod().c_str(),
						http->httpData.getPath().c_str(),
						http->httpData.getUseragent().c_str(),
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
						http->client.ip.c_str(),
						http->httpData.getHost().c_str(),
						http->server.host.c_str(),
						http->server.port,
						http->server.mac.c_str(),
						http->httpData.getMethod().c_str(),
						http->httpData.getPath().c_str(),
						http->httpData.getUseragent().c_str(),
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
			http->httpData.getHost().c_str(),
			http->httpData.getPort(),
			http->client.ip.c_str()
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
			if(subject.compare("server") == 0){
				// Выводим в лог сообщение
				http->proxy.log->write(
					LOG_ACCESS,
					"connect client [%s], useragent = %s, socket = %d to server [%s:%d]",
					http->client.ip.c_str(),
					http->client.useragent.c_str(),
					current_fd,
					http->server.host.c_str(),
					http->server.port
				);
			}
		// Если это ошибка или завершение работы
		} else if(events & (BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT | BEV_EVENT_EOF)) {
			// Если это ошибка
			if(events & BEV_EVENT_ERROR){
				// Получаем данные ошибки
				int err = bufferevent_socket_get_dns_error(bev);
				// Если ошибка существует, выводим сообщение в консоль
				if(err) http->proxy.log->write(LOG_ERROR, "DNS error: %s", evutil_gai_strerror(err));
			}
			// Если отключился клиент
			if(subject.compare("client") == 0){
				// Выводим в лог сообщение
				http->proxy.log->write(
					LOG_ACCESS,
					"closing client [%s] from server [%s:%d], socket = %d",
					http->client.ip.c_str(),
					http->server.host.c_str(),
					http->server.port,
					current_fd
				);
				// Закрываем подключение к клиенту
				http->close_client();
				// Закрываем подключение к серверу
				http->close_server();
			// Если отключился сервер
			} else {
				// Выводим в лог сообщение
				http->proxy.log->write(
					LOG_ACCESS,
					"closing server [%s:%d] from client [%s], socket = %d",
					http->server.host.c_str(),
					http->server.port,
					http->client.ip.c_str(),
					current_fd
				);
				// Если сервер закрыл сове соединение
				// Закрываем соединение с клиентом
				if((!http->client.connect && (events | BEV_EVENT_EOF))
				|| !(events | BEV_EVENT_EOF)) http->close_client();
				// Закрываем подключение к серверу
				http->close_server();
			}
		}
	}
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
		// Если соединение постоянное, устанавливаем таймер на клиента
		if(http->client.alive) http->set_timeout(TM_CLIENT, true, true);
		// Если соединение не постоянное то ставим таймер только на запись
		else http->set_timeout(TM_CLIENT, false, true);
		// Если заголовки менять не надо тогда просто обмениваемся данными
		if(http->client.connect) evbuffer_add_buffer(output, input);
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
				http->parser.modify(headers);
				// Добавляем в новый буфер модифицированные заголовки
				evbuffer_add(tmp, headers.data(), headers.size());
				// Отправляем данные клиенту
				evbuffer_add_buffer(output, tmp);
			// Выводим данные так как они есть
			} else evbuffer_add_buffer(output, input);
			// Удаляем данные из буфера
			evbuffer_drain(input, len);
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
 * do_request Функция запроса данных у сервера
 * @param bev  буфер события
 * @param ctx  передаваемый объект
 * @param flag флаг разрешающий новый запрос данных
 */
void HttpProxy::do_request(struct bufferevent * bev, void * ctx, bool flag){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	// Если подключение не передано
	if(http != NULL){
		// Определяем connect прокси разрешен
		bool conn_enabled = (http->proxy.config->options & OPT_CONNECT);
		// Если данные еще не заполнены, но они есть в массиве
		if(!http->parser.httpData.empty() && (!http->httpData.size() || flag)){
			// Очищаем таймеры для клиента и сервера
			http->set_timeout(TM_CLIENT | TM_SERVER);
			// Очищаем объект ответа
			http->response.clear();
			// Получаем первый элемент из массива
			vector <HttpData>::iterator httpData = http->parser.httpData.begin();
			// Запоминаем данные объекта http запроса
			http->httpData = * httpData;
			// Если авторизация не прошла
			if(!http->auth) http->auth = check_auth(http);
			// Если нужно запросить пароль
			if(!http->auth && (http->httpData.getLogin().empty()
			|| http->httpData.getPassword().empty())){
				// Формируем ответ клиенту
				http->response = http->httpData.requiredAuth();
			// Сообщаем что авторизация не удачная
			} else if(!http->auth) {
				// Формируем ответ клиенту
				http->response = http->httpData.faultAuth();
			// Если авторизация прошла
			} else {
				// Выполняем подключение к удаленному серверу
				int connect = connect_server(http);
				// Если сокет существует
				if(connect > 0){
					// Заполняем структуру клиента
					http->client.alive		= http->httpData.isAlive();
					http->client.https		= http->httpData.isHttps();
					http->client.connect	= http->httpData.isConnect();
					http->client.useragent	= http->httpData.getUseragent();
					// Определяем порт, если это метод connect
					if(http->client.connect && (conn_enabled || http->client.https))
						// Формируем ответ клиенту
						http->response = http->httpData.authSuccess();
					// Если connect разрешен только для https подключений
					else if(!conn_enabled && http->client.connect)
						// Сообращем что подключение запрещено
						http->response = http->httpData.faultConnect();
					// Иначе делаем запрос на получение данных
					else {
						// Указываем что нужно отключится сразу после отправки запроса
						if(!http->client.alive) http->httpData.setClose();
						// Формируем запрос на сервер
						http->response = http->httpData.getRequest();
						// Устанавливаем таймер на сервер
						http->set_timeout(TM_SERVER, true, true);
						// Отправляем серверу сообщение
						bufferevent_write(http->events.server, http->response.data(), http->response.size());
						// Удаляем объект подключения
						http->parser.httpData.erase(httpData);
						// Если данные в массиве существуют тогда продолжаем загрузку
						if(!http->parser.httpData.empty()) do_request(bev, http, true);
						// Очищаем объект http данных если запросов больше нет
						else http->httpData.clear();
						// Выходим
						return;
					}
				// Если подключение не удачное то сообщаем об этом
				} else if(connect == 0) http->response = http->httpData.faultConnect();
				// Если подключение удалено, выходим
				else return;
			}
			// Ответ готов
			if(!http->response.empty()){
				// Если это код разрешающий коннект, устанавливаем таймер для клиента
				if(http->response.code == 200) http->set_timeout(TM_CLIENT, true, true);
				// Если это завершение работы то устанавливаем таймер только на запись
				else http->set_timeout(TM_CLIENT, false, true);
				// Отправляем клиенту сообщение
				bufferevent_write(bev, http->response.data(), http->response.size());
				// Удаляем объект подключения
				http->parser.httpData.erase(httpData);
			}
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
		// Метод подключения, не connect
		bool not_connect = true;
		// Определяем connect прокси разрешен
		bool conn_enabled = (http->proxy.config->options & OPT_CONNECT);
		// Получаем буферы входящих данных и исходящих
		struct evbuffer * input = bufferevent_get_input(bev);
		// Если авторизация прошла, и коннект произведен
		if((http->response.code == 200) && http->httpData.size()){
			// Если это метод connect
			if(http->client.connect && (conn_enabled || http->client.https)){
				// Запоминаем что это метод connect
				not_connect = false;
				// Если сервер подключен
				if(http->events.server != NULL){
					// Получаем буферы входящих данных и исходящих
					struct evbuffer * output = bufferevent_get_output(http->events.server);
					// Удаляем таймер для клиента
					http->set_timeout(TM_CLIENT);
					// Устанавливаем таймер для сервера
					http->set_timeout(TM_SERVER, true, true);
					// Выводим ответ сервера
					evbuffer_add_buffer(output, input);
				}
			}
		}
		// Если это обычный запрос
		if(not_connect){
			// Удаляем таймер для сервера
			http->set_timeout(TM_SERVER);
			// Получаем размер входящих данных
			size_t len = evbuffer_get_length(input);
			// Создаем буфер данных
			char * buffer = new char[len];
			// Копируем в буфер полученные данные
			evbuffer_copyout(input, buffer, len);
			// Выполняем парсинг данных
			size_t size = http->parser.parse(buffer, len);
			// Удаляем данные из буфера
			evbuffer_drain(input, size);
			// Удаляем буфер данных
			delete [] buffer;
			// Выполняем загрузку данных
			do_request(bev, http);
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
		if((*http->connects).count(http->client.ip) > 0){
			// Получаем объект текущего коннекта
			Connects * connect = &((*http->connects).find(http->client.ip)->second);
			// Выполняем захват мютекса
			connect->lock();
			// Если количество подключений достигло предела
			if(connect->get() >= http->proxy.config->proxy.maxcon) connect->wait();
			// Выполняем разблокировку мютекса
			connect->unlock();
		}
		// Выполняем инициализацию подключения
		http->begin();
		// Создаем новую базу событий
		http->base = event_base_new();
		// Создаем буфер событий
		http->events.client = bufferevent_socket_new(http->base, http->sockets.client, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
		// Устанавливаем размеры буферов
		set_buffer_size(http->sockets.client, http->proxy.config->buffers.read, http->proxy.config->buffers.write, http->proxy.log);
		// Устанавливаем таймер для клиента
		http->set_timeout(TM_CLIENT, true);
		// Устанавливаем коллбеки
		bufferevent_setcb(http->events.client, &HttpProxy::read_client, NULL, &HttpProxy::event, http);
		// Активируем буферы событий на чтение и запись
		bufferevent_enable(http->events.client, EV_READ | EV_WRITE);
		// Очищаем буферы событий при завершении работы
		bufferevent_flush(http->events.client, EV_READ | EV_WRITE, BEV_FINISHED);
		// Активируем перебор базы событий
		event_base_dispatch(http->base);
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
		string ip = gethost(address, socklen);
		// Получаем данные мак адреса клиента
		string mac = getmac(address);
		// Выводим в лог сообщение
		proxy->server.log->write(LOG_ACCESS, "client connect to proxy server, host = %s, mac = %s, socket = %d", ip.c_str(), mac.c_str(), fd);
		// Создаем новый объект подключения
		BufferHttpProxy * http = new BufferHttpProxy(proxy->server.config->proxy.name, proxy->server.config->options);
		// Запоминаем параметры прокси сервера
		http->proxy = proxy->server;
		// Запоминаем список подключений
		http->connects = &proxy->connects;
		// Запоминаем файловый дескриптор текущего подключения
		http->sockets.client = fd;
		// Запоминаем данные мак адреса
		http->client.mac = mac;
		// Запоминаем данные клиента
		http->client.ip = ip;
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
 * @param log    объект ведения логов
 * @param config объект конфигурационных данных
 */
HttpProxy::HttpProxy(LogApp * log, Config * config){
	// Если лог существует
	if((log != NULL) && (config != NULL)){
		// Запоминаем параметры прокси сервера
		this->server = {log, config};
		// Структура для создания сервера приложения
		struct sockaddr_in sin;
		// Создаем новую базу
		struct event_base * base = event_base_new();
		// Создаем хост подключения
		string host;
		// Определяем тип подключения
		switch(this->server.config->proxy.ipver){
			// Для протокола IPv4
			case 4: host = this->server.config->ipv4.internal; break;
			// Для протокола IPv6
			case 6: host = this->server.config->ipv6.internal; break;
		}
		// Структура определяющая параметры сервера приложений
		struct hostent * server = gethostbyname(host.c_str());
		// Очищаем всю структуру
		memset(&sin, 0, sizeof(sin));
		// Listen on 0.0.0.0
		// sin.sin_addr.s_addr = htonl(0);
		// Устанавливаем тип подключения что это INET адрес
		sin.sin_family = AF_INET;
		// Указываем адрес прокси сервера
		sin.sin_addr.s_addr = * ((unsigned long *) server->h_addr);
		// Указываем локальный порт
		sin.sin_port = htons(this->server.config->proxy.port);
		// Вешаем приложение на порт
		listener = evconnlistener_new_bind(
			base, &HttpProxy::accept_connect, this,
			LEV_OPT_REUSEABLE |
			// LEV_OPT_THREADSAFE |
			LEV_OPT_CLOSE_ON_FREE,// |
			// LEV_OPT_LEAVE_SOCKETS_BLOCKING,
			this->server.config->proxy.allcon, (struct sockaddr *) &sin, sizeof(sin)
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