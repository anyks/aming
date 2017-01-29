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
 * signal Метод отправки сигнала первому блокированному потоку
 */
inline void Connects::signal(){
	// Отправляем сигнал
	this->condition.notify_one();
}
/**
 * broadcastSignal Метод отправки сигналов всем блокированным потокам
 */
inline void Connects::broadcastSignal(){
	// Выполняем вещание
	this->condition.notify_all();
}
/**
 * wait Метод блокировки потока
 */
inline void Connects::wait(recursive_mutex &mutx){
	// Лочим мютекс
	unique_lock <recursive_mutex> locker(mutx);
	// Блокируем поток
	this->condition.wait(locker);
	// Блокируем поток и избавляемся от случайных пробуждений
	// this->condition.wait(locker, [&](){return !g_codes.empty();});
}
/**
 * Connects Конструктор
 */
Connects::Connects(){
	// Устанавливаем первоначальное значение коннекта
	this->connects = 1;
}
/**
 * lock Метод блокировки мютекса
 */
inline void BufferHttpProxy::lock(){
	// Лочим мютекс
	this->lock_thread.lock();
}
/**
 * unlock Метод разблокировки мютекса
 */
inline void BufferHttpProxy::unlock(){
	// Разлочим мютекс
	this->lock_thread.unlock();
}
/**
 * appconn Функция которая добавляет или удаляет в список склиента
 * @param flag флаг подключения или отключения клиента
 */
void BufferHttpProxy::appconn(const bool flag){
	// Если такое подключение найдено
	if((*this->connects).count(this->client.ip) > 0){
		// Получаем объект текущего коннекта
		Connects * connect = (*this->connects).find(this->client.ip)->second;
		// Выполняем захват мютекса
		this->lock_thread.lock();
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
		this->lock_thread.unlock();
		// Проверяем есть ли еще подключения
		if(connect->end()){
			// Удаляем объект подключения
			delete connect;
			// Удаляем само подключение из списка подключений
			(*this->connects).erase(this->client.ip);
		}
	// Если нужно добавить подключение
	} else if(flag){
		// Создаем объект подключения
		Connects * connect = new Connects;
		// Добавляем в список новое подключение
		(*this->connects).insert(pair <string, Connects *> (this->client.ip, connect));
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
		::close(*fd);
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
 * blockconnect Метод блокировки лишних коннектов
 */
void BufferHttpProxy::blockconnect(){
	// Если такое подключение найдено
	if((*this->connects).count(this->client.ip) > 0){
		// Получаем объект текущего коннекта
		Connects * connect = (*this->connects).find(this->client.ip)->second;
		// Запоминаем количество подключений пользователя
		this->myconns = connect->get();
		// Если количество подключений достигло предела
		if(this->myconns >= this->proxy->config->proxy.maxcon){
			// Усыпляем поток до момента пока не освободятся коннекты
			connect->wait(this->lock_connect);
		}
	}
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
 * close Метод закрытия подключения
 */
void BufferHttpProxy::close(){
	// Захватываем поток
	this->lock();
	// Закрываем подключение клиента
	close_client();
	// Закрываем подключение сервера
	close_server();
	// Удаляем из списока подключений
	appconn(false);
	// Удаляем базу событий
	event_base_loopexit(this->base, NULL);
	// Отпускаем поток
	this->unlock();
}
/**
 * set_timeout Метод установки таймаутов
 * @param type  тип подключения (клиент или сервер)
 * @param read  таймаут на чтение
 * @param write таймаут на запись
 */
void BufferHttpProxy::set_timeout(const u_short type, bool read, bool write){
	// Устанавливаем таймаут ожидания результата на чтение с сервера
	struct timeval _read = {this->proxy->config->timeouts.read, 0};
	// Устанавливаем таймаут записи результата на запись
	struct timeval _write = {this->proxy->config->timeouts.write, 0};
	// Если время постоянного подключения меньше 1 секунды значит таймер ставить не надо
	if(this->proxy->config->timeouts.read < 1)	read	= false;
	if(this->proxy->config->timeouts.write < 1)	write	= false;
	// Устанавливаем таймауты для сервера
	if((type & TM_SERVER) && (this->events.server != NULL))
		// Устанавливаем таймауты
		bufferevent_set_timeouts(this->events.server, (read ? &_read : NULL), (write ? &_write : NULL));
	// Устанавливаем таймауты для клиента
	if((type & TM_CLIENT) && (this->events.client != NULL))
		// Устанавливаем таймауты
		bufferevent_set_timeouts(this->events.client, (read ? &_read : NULL), (write ? &_write : NULL));
}
/**
 * sleep Метод усыпления потока на время необходимое для соблюдения скоростного ограничения сети
 * @param  size размер передаваемых данных
 * @param  type тип передаваемого сообщения (true - чтение, false - запись)
 * @return      время в секундах на которое следует задержать поток
 */
void BufferHttpProxy::sleep(size_t size, bool type){
	// Расчитанное время задержки
	int seconds = 0;
	// Получаем размер максимально-возможных передачи данных для всех подключений
	float max = float(type ? this->proxy->config->buffers.read : this->proxy->config->buffers.write);
	// Если буфер существует
	if(max > 0){
		// Высчитываем размер максимально-возможных передачи данных для одного подключения
		max = (max / float(this->myconns));
		// Если размер больше нуля то продолжаем
		if((max > 0) && (size > max)) seconds = (size / max);
	}
	// Усыпляем поток на указанное время, чтобы соблюсти предел скорости
	this_thread::sleep_for(chrono::seconds(seconds));
}
/**
 * BufferHttpProxy Конструктор
 * @param proxy объект данных прокси сервера
 */
BufferHttpProxy::BufferHttpProxy(System * proxy){
	// Запоминаем данные прокси сервера
	this->proxy = proxy;
	// Создаем новую базу событий
	this->base = event_base_new();
	// Создаем объект для работы с http заголовками
	this->parser = new Http(this->proxy->config->proxy.name, this->proxy->config->options);
	// Определяем тип подключения
	switch(this->proxy->config->proxy.ipver){
		// Для протокола IPv4
		case 4: this->dns = new DNSResolver(this->proxy->log, this->base, AF_INET, this->proxy->config->proxy.resolver);	break;
		// Для протокола IPv6
		case 6: this->dns = new DNSResolver(this->proxy->log, this->base, AF_INET6, this->proxy->config->proxy.resolver);	break;
	}
}
/**
 * ~BufferHttpProxy Деструктор
 */
BufferHttpProxy::~BufferHttpProxy(){
	// Закрываем подключение
	close();
	// Удаляем dns сервер
	delete this->dns;
	// Удаляем парсер
	delete this->parser;
	// Очищаем объект базы событий
	event_base_free(this->base);
}
/**
 * get_mac Метод определения мак адреса клиента
 * @param  ctx указатель на объект подключения
 * @return     данные мак адреса
 */
string HttpProxy::get_mac(void * ctx){
	// Буфер для копирования мак адреса
	char buff[256];
	// Получаем данные адреса
	struct sockaddr * s = reinterpret_cast <struct sockaddr *> (ctx);
	// Получаем указатель на мак адрес
	unsigned char * ptr = (unsigned char *) s->sa_data;
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
 * get_ip Функция получения данных ip адреса
 * @param  family тип интернет протокола
 * @param  ctx    указатель на объект подключения
 * @return        данные ip адреса
 */
string HttpProxy::get_ip(int family, void * ctx){
	// Определяем тип интернет протокола
	switch(family){
		// Если это IPv4
		case AF_INET: {
			// Создаем буфер для получения ip адреса
			char ipstr[INET_ADDRSTRLEN];
			// Получаем данные адреса
			struct sockaddr_in * s = reinterpret_cast <struct sockaddr_in *> (ctx);
			// Копируем полученные данные
			inet_ntop(family, &s->sin_addr, ipstr, sizeof(ipstr));
			// Выводим результат
			return ipstr;
		}
		// Если это IPv6
		case AF_INET6: {
			// Создаем буфер для получения ip адреса
			char ipstr[INET6_ADDRSTRLEN];
			// Получаем данные адреса
			struct sockaddr_in6 * s = reinterpret_cast <struct sockaddr_in6 *> (ctx);
			// Копируем полученные данные
			inet_ntop(family, &s->sin6_addr, ipstr, sizeof(ipstr));
			// Выводим результат
			return ipstr;
		}
	}
	// Сообщаем что ничего не найдено
	return "";
}
/**
 * socket_nonblocking Функция установки неблокирующего сокета
 * @param  fd   файловый дескриптор (сокет)
 * @param  log  указатель на объект ведения логов
 * @return      результат работы функции
 */
int HttpProxy::socket_nonblocking(evutil_socket_t fd, LogApp * log){
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
 * socket_tcpnodelay Функция отключения алгоритма Нейгла
 * @param  fd   файловый дескриптор (сокет)
 * @param  log  указатель на объект ведения логов
 * @return      результат работы функции
 */
int HttpProxy::socket_tcpnodelay(evutil_socket_t fd, LogApp * log){
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
 * socket_reuseable Функция разрешающая повторно использовать сокет после его удаления
 * @param  fd   файловый дескриптор (сокет)
 * @param  log  указатель на объект ведения логов
 * @return      результат работы функции
 */
int HttpProxy::socket_reuseable(evutil_socket_t fd, LogApp * log){
	// Устанавливаем параметр
	int reuseaddr = 1;
	// Разрешаем повторно использовать тот же host:port после отключения
	if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)) < 0){
		// Выводим в лог информацию
		log->write(LOG_ERROR, "cannot set SO_REUSEADDR option on socket %d", fd);
		// Выходим
		return -1;
	}
	// Все удачно
	return 0;
}
/**
 * socket_keepalive Функция устанавливает постоянное подключение на сокет
 * @param  fd      файловый дескриптор (сокет)
 * @param  log     указатель на объект ведения логов
 * @param  cnt     максимальное количество попыток
 * @param  idle    время через которое происходит проверка подключения
 * @param  intvl   время между попытками
 * @return         результат работы функции
 */
int HttpProxy::socket_keepalive(evutil_socket_t fd, LogApp * log, int cnt, int idle, int intvl){
	// Устанавливаем параметр
	int keepalive = 1;
	// Активация постоянного подключения
	if(setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(int))){
		// Выводим в лог информацию
		log->write(LOG_ERROR, "cannot set SO_KEEPALIVE option on socket %d", fd);
		// Выходим
		return -1;
	}
	// Максимальное количество попыток
	if(setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &cnt, sizeof(int))){
		// Выводим в лог информацию
		log->write(LOG_ERROR, "cannot set TCP_KEEPCNT option on socket %d", fd);
		// Выходим
		return -1;
	}
// Если это Linux
#ifdef __linux__
	// Время через которое происходит проверка подключения
	if(setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(int))){
		// Выводим в лог информацию
		log->write(LOG_ERROR, "cannot set TCP_KEEPIDLE option on socket %d", fd);
		// Выходим
		return -1;
	}
// Если это FreeBSD или MacOS X
#elif __FreeBSD__ || __APPLE__ || __MACH__
	// Время через которое происходит проверка подключения
	if(setsockopt(fd, IPPROTO_TCP, TCP_KEEPALIVE, &idle, sizeof(int))){
		// Выводим в лог информацию
		log->write(LOG_ERROR, "cannot set TCP_KEEPALIVE option on socket %d", fd);
		// Выходим
		return -1;
	}
#endif
	// Время между попытками
	if(setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &intvl, sizeof(int))){
		// Выводим в лог информацию
		log->write(LOG_ERROR, "cannot set TCP_KEEPINTVL option on socket %d", fd);
		// Выходим
		return -1;
	}
	// Все удачно
	return 0;
}
/**
 * socket_buffersize Функция установки размеров буфера
 * @param  fd         файловый дескриптор (сокет)
 * @param  read_size  размер буфера на чтение
 * @param  write_size размер буфера на запись
 * @param  maxcon     максимальное количество подключений
 * @param  log        указатель на объект ведения логов
 * @return            результат работы функции
 */
int HttpProxy::socket_buffersize(evutil_socket_t fd, int read_size, int write_size, u_int maxcon, LogApp * log){
	// Определяем размер массива опции
	socklen_t read_optlen	= sizeof(read_size);
	socklen_t write_optlen	= sizeof(write_size);
	// Устанавливаем размер буфера для сокета на чтение
	if(read_size > 0){
		// Выполняем перерасчет размера буфера
		read_size = read_size / maxcon;
		// Устанавливаем размер буфера
		setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *) &read_size, read_optlen);
	}
	// Устанавливаем размер буфера для сокета на запись
	if(write_size > 0){
		// Выполняем перерасчет размера буфера
		write_size = write_size / maxcon;
		// Устанавливаем размер буфера
		setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *) &write_size, write_optlen);
	}
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
		http->proxy->log->write(LOG_MESSAGE, "auth client [%s] to proxy wrong!", http->client.ip.c_str());
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
		// Если сервер еще не подключен
		if(http->events.server == NULL){
			// Адрес сервера для биндинга
			string bindhost;
			// Размер структуры подключения
			socklen_t sinlen = 0, sotlen = 0;
			// Структура сервера для биндинга
			struct sockaddr * sin = NULL, * sot = NULL;
			// Структуры серверного и локального подключений
			struct sockaddr_in server4_addr, client4_addr;
			// Структуры серверного и локального подключений
			struct sockaddr_in6 server6_addr, client6_addr;
			// Определяем тип подключения
			switch(http->proxy->config->proxy.ipver){
				// Для протокола IPv4
				case 4: {
					// Запоминаем адрес сервера для биндинга
					bindhost = http->proxy->config->ipv4.external;
					// Получаем данные хоста удаленного сервера по его названию
					struct hostent * client = gethostbyname2(bindhost.c_str(), AF_INET);
					// Очищаем всю структуру для клиента
					memset(&client4_addr, 0, sizeof(client4_addr));
					// Очищаем всю структуру для сервера
					memset(&server4_addr, 0, sizeof(server4_addr));
					// Устанавливаем протокол интернета
					client4_addr.sin_family = AF_INET;
					server4_addr.sin_family = AF_INET;
					// Устанавливаем произвольный порт для локального подключения
					client4_addr.sin_port = htons(0);
					// Устанавливаем порт для локального подключения
					server4_addr.sin_port = htons(http->server.port);
					// Устанавливаем адрес для локальго подключения
					client4_addr.sin_addr.s_addr = * ((unsigned long *) client->h_addr);
					// Устанавливаем адрес для удаленного подключения
					server4_addr.sin_addr.s_addr = inet_addr(http->server.host.c_str());
					// Обнуляем серверную структуру
					bzero(&server4_addr.sin_zero, sizeof(server4_addr.sin_zero));
					// Запоминаем размер структуры
					sinlen = sizeof(client4_addr);
					sotlen = sizeof(server4_addr);
					// Запоминаем полученную структуру
					sin	= reinterpret_cast <struct sockaddr *> (&client4_addr);
					sot	= reinterpret_cast <struct sockaddr *> (&server4_addr);
					// Создаем сокет подключения
					http->sockets.server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				} break;
				// Для протокола IPv6
				case 6: {
					// Буфер содержащий адрес IPv6
					char host_client[50], host_server[50];
					// Запоминаем адрес сервера для биндинга
					bindhost = http->proxy->config->ipv6.external;
					// Получаем данные хоста удаленного сервера по его названию
					struct hostent * client	= gethostbyname2(bindhost.c_str(), AF_INET6);
					struct hostent * server	= gethostbyname2(http->server.host.c_str(), AF_INET6);
					// Указываем адрес прокси сервера
					inet_ntop(AF_INET6, (struct in_addr *) client->h_addr, host_client, sizeof(host_client));
					inet_ntop(AF_INET6, (struct in_addr *) server->h_addr, host_server, sizeof(host_server));
					// Очищаем всю структуру для клиента
					memset(&client6_addr, 0, sizeof(client6_addr));
					// Очищаем всю структуру для сервера
					memset(&server6_addr, 0, sizeof(server6_addr));
					// Неважно, IPv4 или IPv6
					client6_addr.sin6_family = AF_INET6;
					server6_addr.sin6_family = AF_INET6;
					// Устанавливаем произвольный порт для локального подключения
					client6_addr.sin6_port = htons(0);
					// Устанавливаем порт для локального подключения
					server6_addr.sin6_port = htons(http->server.port);
					// Копируем полученный ip адрес
					memcpy(client6_addr.sin6_addr.s6_addr, host_client, sizeof(host_client));
					memcpy(server6_addr.sin6_addr.s6_addr, host_server, sizeof(host_server));
					// Запоминаем размер структуры
					sinlen = sizeof(client6_addr);
					sotlen = sizeof(server6_addr);
					// Запоминаем полученную структуру
					sin	= reinterpret_cast <struct sockaddr *> (&client6_addr);
					sot	= reinterpret_cast <struct sockaddr *> (&server6_addr);
					// Создаем сокет подключения
					http->sockets.server = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
				} break;
			}
			// Получаем данные мак адреса клиента
			http->server.mac = get_mac(sot);
			// Если сокет не создан то выходим
			if(http->sockets.server < 0){
				// Выводим в лог сообщение
				http->proxy->log->write(
					LOG_ERROR,
					"creating socket to server = %s, port = %d, client = %s",
					http->server.host.c_str(),
					http->server.port,
					http->client.ip.c_str()
				);
				// Выходим
				return 0;
			}
			// Разблокируем сокет
			socket_nonblocking(http->sockets.server, http->proxy->log);
			// Устанавливаем разрешение на повторное использование сокета
			socket_reuseable(http->sockets.server, http->proxy->log);
			// Если подключение постоянное
			if(http->client.alive){
				// Отключаем алгоритм Нейгла для сервера и клиента
				socket_tcpnodelay(http->sockets.server, http->proxy->log);
				socket_tcpnodelay(http->sockets.client, http->proxy->log);
				// Активируем keepalive
				socket_keepalive(
					http->sockets.server,
					http->proxy->log,
					http->proxy->config->keepalive.keepcnt,
					http->proxy->config->keepalive.keepidle,
					http->proxy->config->keepalive.keepintvl
				);
			}
			// Выполняем бинд на сокет
			if(::bind(http->sockets.server, sin, sinlen) < 0){
				// Выводим в лог сообщение
				http->proxy->log->write(LOG_ERROR, "bind local network [%s] error", bindhost.c_str());
				// Выходим
				return 0;
			}
			// Создаем буфер событий для сервера
			http->events.server = bufferevent_socket_new(http->base, http->sockets.server, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
			// Устанавливаем водяной знак на 1 байт (чтобы считывать данные когда они действительно приходят)
			bufferevent_setwatermark(http->events.server, EV_READ | EV_WRITE, 1, 0);
			// Устанавливаем коллбеки
			bufferevent_setcb(http->events.server, &HttpProxy::read_server_cb, NULL, &HttpProxy::event_cb, http);
			// Активируем буферы событий на чтение и запись
			bufferevent_enable(http->events.server, EV_READ | EV_WRITE);
			// Очищаем буферы событий при завершении работы
			bufferevent_flush(http->events.server, EV_READ | EV_WRITE, BEV_FINISHED);
			// Выполняем подключение к удаленному серверу, если подключение не выполненно то сообщаем об этом
			if(bufferevent_socket_connect(http->events.server, sot, sotlen) < 0){
				// Выводим в лог сообщение
				http->proxy->log->write(
					LOG_ERROR,
					"connecting to server = %s, port = %d, client = %s",
					http->server.host.c_str(),
					http->server.port,
					http->client.ip.c_str()
				);
				// Выходим
				return -1;
			}
			// Выводим в лог сообщение о новом коннекте
			http->proxy->log->write(
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
			http->proxy->log->write(
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
	// Выходим
	return -1;
}
/**
 * event_cb Функция обработка входящих событий
 * @param bev    буфер события
 * @param events произошедшее событие
 * @param ctx    объект входящих данных
 */
void HttpProxy::event_cb(struct bufferevent * bev, short events, void * ctx){
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
				http->proxy->log->write(
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
				if(err) http->proxy->log->write(LOG_ERROR, "DNS error: %s", evutil_gai_strerror(err));
			}
			// Если отключился клиент
			if(subject.compare("client") == 0){
				// Выводим в лог сообщение
				http->proxy->log->write(
					LOG_ACCESS,
					"closing client [%s] from server [%s:%d], socket = %d",
					http->client.ip.c_str(),
					http->server.host.c_str(),
					http->server.port,
					current_fd
				);
			// Если отключился сервер
			} else {
				// Выводим в лог сообщение
				http->proxy->log->write(
					LOG_ACCESS,
					"closing server [%s:%d] from client [%s], socket = %d",
					http->server.host.c_str(),
					http->server.port,
					http->client.ip.c_str(),
					current_fd
				);
			}
			// Закрываем подключение
			http->close();
		}
	}
	// Выходим
	return;
}
/**
 * read_server_cb Функция чтения данных с сокета сервера
 * @param bev буфер события
 * @param ctx передаваемый объект
 */
void HttpProxy::read_server_cb(struct bufferevent * bev, void * ctx){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	// Если подключение не передано
	if(http != NULL){
		// Получаем буферы входящих данных и исходящих
		struct evbuffer * input		= bufferevent_get_input(http->events.server);
		struct evbuffer * output	= bufferevent_get_output(http->events.client);
		// Ставим таймер только на чтение и запись
		http->set_timeout(TM_CLIENT, true, true);
		// Получаем размер входящих данных
		size_t len = evbuffer_get_length(input);
		// Усыпляем поток на указанное время, чтобы соблюсти предел скорости
		http->sleep(len, true);
		
		evbuffer_add_buffer(output, input);

		/*
		// Если заголовки менять не надо тогда просто обмениваемся данными
		if(http->client.connect) evbuffer_add_buffer(output, input);
		// Иначе изменяем заголовки
		else if(len){
			// Создаем буфер данных
			char * buffer = new char[len];
			// Копируем в буфер полученные данные
			evbuffer_copyout(input, buffer, len);
			// Если заголовки запроса не получены
			if(http->headers.response.empty()){
				// Добавляем полученный буфер в вектор
				vector <char> headers(buffer, buffer + len);
				// Если буферы созданы
				if(http->headers.response.create(headers.data())){
					// Если сервер переключил версию протокола с HTTP1.0 на HTTP2
					if((strstr(headers.data(), "101 Switching Protocols") != NULL)
					&& !http->headers.response.getHeader("upgrade").value.empty()
					&& (http->headers.response.getHeader("connection").value.find("Upgrade") != string::npos)){
						// Устанавливаем что это соединение CONNECT,
						// для будущего обмена данными, так как они будут приходить бинарные
						http->client.connect = true;
					}
					// Создаем буфер для исходящих данных
					struct evbuffer * tmp = evbuffer_new();
					// Выполняем модификацию заголовков
					http->parser->modify(headers);
					// Добавляем в новый буфер модифицированные заголовки
					evbuffer_add(tmp, headers.data(), headers.size());
					// Отправляем данные клиенту
					evbuffer_add_buffer(output, tmp);
					// Удаляем данные из буфера
					evbuffer_drain(input, len);
					// Удаляем временный буфер
					evbuffer_free(tmp);
				}
			// Если заголовки получены, выводим так как есть
			} else evbuffer_add_buffer(output, input);
			// Удаляем буфер данных
			delete [] buffer;
		// Закрываем соединение
		} else http->close();
		*/
	}
	// Выходим
	return;
}
/**
 * resolve_cb Функция выполняющая ресолвинг домена
 * @param ip  IP адрес сервера
 * @param ctx передаваемый объект
 */
void HttpProxy::resolve_cb(const string ip, void * ctx){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	// Если подключение не передано
	if(http != NULL){
		// Получаем первый элемент из массива
		auto httpData = http->parser->httpData.begin();
		// Если дарес домена найден
		if(!ip.empty()){
			// Определяем connect прокси разрешен
			bool conn_enabled = (http->proxy->config->options & OPT_CONNECT);
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
				// Получаем порт сервера
				u_int port = http->httpData.getPort();
				// Если хост и порт сервера не совпадают тогда очищаем данные
				if((http->events.server != NULL)
				&& ((http->server.host.compare(ip) != 0)
				|| (http->server.port != port))) http->close_server();
				// Запоминаем хост и порт сервера
				http->server.host = ip;
				http->server.port = port;
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
						// Если это не постоянное подключение
						if(!http->client.alive)
							// Устанавливаем таймаут на чтение и запись
							http->set_timeout(TM_SERVER, true, true);
						// Устанавливаем таймаут только на запись
						else http->set_timeout(TM_SERVER, false, true);
						// Усыпляем поток на указанное время, чтобы соблюсти предел скорости
						http->sleep(http->response.size(), false);
						// Отправляем серверу сообщение
						bufferevent_write(http->events.server, http->response.data(), http->response.size());
						// Удаляем объект подключения
						http->parser->httpData.erase(httpData);
						// Если данные в массиве существуют тогда продолжаем загрузку
						if(!http->parser->httpData.empty()) do_request(http, true);
						// Очищаем объект http данных если запросов больше нет
						else http->httpData.clear();
						// Выходим
						return;
					}
				// Если подключение не удачное то сообщаем об этом
				} else if(connect == 0) http->response = http->httpData.faultConnect();
				// Если подключение удалено, выходим
				else {
					// Закрываем подключение
					http->close();
					// Выходим
					return;
				}
			}
		// Если домен не найден
		} else {
			// Выводим в лог сообщение
			http->proxy->log->write(
				LOG_ERROR,
				"host server = %s not found, port = %d, client = %s, socket = %d",
				http->httpData.getHost().c_str(),
				http->httpData.getPort(),
				http->client.ip.c_str(),
				http->sockets.client
			);
			// Формируем ответ клиенту, что домен не найден
			http->response = http->httpData.faultConnect();
		}
		// Ответ готов
		if(!http->response.empty()){
			// Если это код разрешающий коннект, устанавливаем таймер для клиента
			if(http->response.code == 200) http->set_timeout(TM_CLIENT, true, true);
			// Если это завершение работы то устанавливаем таймер только на запись
			else http->set_timeout(TM_CLIENT, false, true);
			// Усыпляем поток на указанное время, чтобы соблюсти предел скорости
			if(http->response.code == 200) http->sleep(http->response.size(), false);
			// Устанавливаем водяной знак на количество байт необходимое для идентификации переданных данных
			bufferevent_setwatermark(http->events.client, EV_WRITE, http->response.size(), 0);
			// Отправляем клиенту сообщение
			bufferevent_write(http->events.client, http->response.data(), http->response.size());
			// Удаляем объект подключения
			http->parser->httpData.erase(httpData);
		}
	}
	// Выходим
	return;
}
/**
 * do_request Функция запроса данных у сервера
 * @param ctx  передаваемый объект
 * @param flag флаг разрешающий новый запрос данных
 */
void HttpProxy::do_request(void * ctx, bool flag){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	// Если подключение не передано
	if(http != NULL){
		// Если данные еще не заполнены, но они есть в массиве
		if(!http->parser->httpData.empty() && (!http->httpData.size() || flag)){
			// Очищаем таймеры для клиента
			http->set_timeout(TM_CLIENT);
			// Очищаем объект ответа
			http->response.clear();
			// Очищаем заголовки прошлых ответов
			http->headers.response.clear();
			// Получаем первый элемент из массива
			auto httpData = http->parser->httpData.begin();
			// Запоминаем данные объекта http запроса
			http->httpData = * httpData;
			// Выполняем ресолв домена
			http->dns->resolve(http->httpData.getHost(), &HttpProxy::resolve_cb, http);
		}
	}
	// Выходим
	return;
}
/**
 * write_client_cb Функция записи данных в сокет клиента
 * @param bev буфер события
 * @param ctx передаваемый объект
 */
void HttpProxy::write_client_cb(struct bufferevent * bev, void * ctx){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	// Если подключение не передано
	if(http != NULL){
		// Это закрывающее соединение
		if(!http->response.empty()
		&& (http->response.code != 200)) http->close();
	}
	// Выходим
	return;
}
/**
 * read_client_cb Функция чтения данных с сокета клиента
 * @param bev буфер события
 * @param ctx передаваемый объект
 */
void HttpProxy::read_client_cb(struct bufferevent * bev, void * ctx){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	// Если подключение не передано
	if(http != NULL){
		// Удаляем таймер для клиента
		http->set_timeout(TM_CLIENT | TM_SERVER);
		// Определяем connect прокси разрешен
		bool conn_enabled = (http->proxy->config->options & OPT_CONNECT);
		// Получаем буферы входящих данных и исходящих
		struct evbuffer * input = bufferevent_get_input(bev);
		// Получаем размер входящих данных
		size_t len = evbuffer_get_length(input);
		// Усыпляем поток на указанное время, чтобы соблюсти предел скорости
		http->sleep(len, true);
		// Если это метод connect
		if(http->client.connect && (conn_enabled || http->client.https)){
			// Получаем буферы входящих данных и исходящих
			struct evbuffer * output = bufferevent_get_output(http->events.server);
			// Если это не постоянное подключение
			if(!http->client.alive)
				// Устанавливаем таймаут на чтение и запись
				http->set_timeout(TM_SERVER, true, true);
			// Устанавливаем таймаут только на запись
			else http->set_timeout(TM_SERVER, false, true);
			// Выводим ответ сервера
			evbuffer_add_buffer(output, input);
		// Если это обычный запрос
		} else if(!http->client.connect) {
			// Создаем буфер данных
			char * buffer = new char[len];
			// Копируем в буфер полученные данные
			evbuffer_copyout(input, buffer, len);
			// Выполняем получение заголовков входящего запроса
			http->headers.request.create(buffer);
			// Выполняем парсинг данных
			size_t size = http->parser->parse(buffer, len);
			// Удаляем данные из буфера
			evbuffer_drain(input, size);
			// Удаляем буфер данных
			delete [] buffer;
			// Выполняем разбор данных
			do_request(http);
		// Закрываем подключение
		} else http->close();
	}
	// Выходим
	return;
}
/**
 * connection Функция обработки данных подключения в треде
 * @param ctx передаваемый объект
 */
void HttpProxy::connection(void * ctx){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	// Если подключение не передано
	if(http != NULL){
		// Выполняем блокировку подключения
		http->blockconnect();
		// Создаем буфер событий
		http->events.client = bufferevent_socket_new(http->base, http->sockets.client, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
		// Устанавливаем таймер для клиента
		http->set_timeout(TM_CLIENT, true);
		// Устанавливаем водяной знак на 5 байт (чтобы считывать данные когда они действительно приходят)
		bufferevent_setwatermark(http->events.client, EV_READ | EV_WRITE, 5, 0);
		// Устанавливаем коллбеки
		bufferevent_setcb(http->events.client, &HttpProxy::read_client_cb, &HttpProxy::write_client_cb, &HttpProxy::event_cb, http);
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
	return;
}
/**
 * accept_cb Функция подключения к серверу
 * @param fd    файловый дескриптор (сокет)
 * @param event событие на которое сработала функция обратного вызова
 * @param ctx   объект передаваемый как значение
 */
void HttpProxy::accept_cb(evutil_socket_t fd, short event, void * ctx){
	// Получаем объект прокси сервера
	HttpProxy * proxy = reinterpret_cast <HttpProxy *> (ctx);
	// Если прокси существует
	if(proxy != NULL){
		// IP и MAC адрес подключения
		string ip, mac;
		// Сокет подключившегося клиента
		evutil_socket_t socket = -1;
		// Определяем тип подключения
		switch(proxy->server->config->proxy.ipver){
			// Для протокола IPv4
			case 4: {
				// Структура получения
				struct sockaddr_in client;
				// Размер структуры подключения
				socklen_t len = sizeof(client);
				// Определяем разрешено ли подключение к прокси серверу
				socket = accept(fd, reinterpret_cast <struct sockaddr *> (&client), &len);
				// Если сокет не создан тогда выходим
				if(socket < 1) return;
				// Получаем данные подключившегося клиента
				ip = get_ip(AF_INET, &client);
				// Получаем данные мак адреса клиента
				mac = get_mac(&client);
			} break;
			// Для протокола IPv6
			case 6: {
				// Структура получения
				struct sockaddr_in6 client;
				// Размер структуры подключения
				socklen_t len = sizeof(client);
				// Определяем разрешено ли подключение к прокси серверу
				socket = accept(fd, reinterpret_cast <struct sockaddr *> (&client), &len);
				// Если сокет не создан тогда выходим
				if(socket < 1) return;
				// Получаем данные подключившегося клиента
				ip = get_ip(AF_INET6, &client);
				// Получаем данные мак адреса клиента
				mac = get_mac(&client);
			} break;
		}
		// Устанавливаем неблокирующий режим для сокета
		socket_nonblocking(socket, proxy->server->log);
		// Устанавливаем разрешение на повторное использование сокета
		socket_reuseable(socket, proxy->server->log);
		// Выводим в лог сообщение
		proxy->server->log->write(LOG_ACCESS, "client connect to proxy server, host = %s, mac = %s, socket = %d", ip.c_str(), mac.c_str(), socket);
		// Создаем новый объект подключения
		BufferHttpProxy * http = new BufferHttpProxy(proxy->server);
		// Запоминаем список подключений
		http->connects = &proxy->connects;
		// Запоминаем файловый дескриптор текущего подключения
		http->sockets.client = socket;
		// Запоминаем данные мак адреса
		http->client.mac = mac;
		// Запоминаем данные клиента
		http->client.ip = ip;
		// Создаем поток
		thread thr(&HttpProxy::connection, http);
		// Выполняем активацию потока
		thr.detach();
	}
}
/**
 * create_server Функция создания прокси сервера
 * @return сокет прокси сервера
 */
evutil_socket_t HttpProxy::create_server(){
	// Сокет сервера
	evutil_socket_t sock = -1;
	// Размер структуры подключения
	socklen_t sinlen = 0;
	// Структура сервера для биндинга
	struct sockaddr * sin = NULL;
	// Структура для сервера
	struct sockaddr_in server4_addr;
	// Структура для сервера
	struct sockaddr_in6 server6_addr;
	// Определяем тип подключения
	switch(this->server->config->proxy.ipver){
		// Для протокола IPv4
		case 4: {
			// Заполняем структуру сервера нулями
			memset(&server4_addr, 0, sizeof(server4_addr));
			// Получаем данные хоста удаленного сервера по его названию
			struct hostent * bindhost = gethostbyname2(this->server->config->ipv4.internal.c_str(), AF_INET);
			// Указываем версию интернет протокола
			server4_addr.sin_family = AF_INET;
			// Указываем адрес прокси сервера
			server4_addr.sin_addr.s_addr = * ((unsigned long *) bindhost->h_addr);
			// Указываем порт сервера
			server4_addr.sin_port = htons(this->server->config->proxy.port);
			// Запоминаем размер структуры
			sinlen = sizeof(server4_addr);
			// Запоминаем полученную структуру
			sin = reinterpret_cast <struct sockaddr *> (&server4_addr);
			// Обнуляем серверную структуру
			bzero(&server4_addr.sin_zero, sizeof(server4_addr.sin_zero));
			// Получаем сокет сервера
			sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		} break;
		// Для протокола IPv6
		case 6: {
			// Буфер содержащий адрес IPv6
			char straddr[50];
			// Заполняем структуру сервера нулями
			memset(&server6_addr, 0, sizeof(server6_addr));
			// Получаем данные хоста удаленного сервера по его названию
			struct hostent * bindhost = gethostbyname2(this->server->config->ipv6.internal.c_str(), AF_INET6);
			// Указываем адрес прокси сервера
			inet_ntop(AF_INET6, (struct in_addr *) bindhost->h_addr, straddr, sizeof(straddr));
			// Копируем полученный ip адрес
			memcpy(server6_addr.sin6_addr.s6_addr, straddr, sizeof(straddr));
			// Указываем версию интернет протокола
			server6_addr.sin6_family = AF_INET6;
			// Указываем порт сервера
			server6_addr.sin6_port = htons(this->server->config->proxy.port);
			// Запоминаем размер структуры
			sinlen = sizeof(server6_addr);
			// Запоминаем полученную структуру
			sin = reinterpret_cast <struct sockaddr *> (&server6_addr);
			// Получаем сокет сервера
			sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
		} break;
	}
	// Создаем сокет
	if(sock < 0){
		// Выводим в консоль информацию
		this->server->log->write(LOG_ERROR, "[-] could not create socket");
		// Выходим
		return -1;
	}
	// Устанавливаем неблокирующий режим для сокета
	socket_nonblocking(sock, this->server->log);
	// Устанавливаем разрешение на повторное использование сокета
	socket_reuseable(sock, this->server->log);
	// Устанавливаем неблокирующий режим
	socket_tcpnodelay(sock, this->server->log);
	// Выполняем биндинг сокета
	if(::bind(sock, sin, sinlen) < 0){
		// Выводим в консоль информацию
		this->server->log->write(LOG_ERROR, "[-] bind error");
		// Выходим
		return -1;
	}
	// Выполняем чтение сокета
	if(listen(sock, this->server->config->proxy.allcon) < 0){
		// Выводим в консоль информацию
		this->server->log->write(LOG_ERROR, "[-] listen error");
		// Выходим
		return -1;
	}
	// Выходим
	return sock;
}
/**
 * HttpProxy::run_server Метод запуска прокси сервера
 * @param socket сокет который слушает прокси сервер
 * @param ctx    объект прокси сервера
 */
void HttpProxy::run_server(evutil_socket_t socket, void * ctx){
	// Получаем объект прокси сервера
	HttpProxy * proxy = reinterpret_cast <HttpProxy *> (ctx);
	// Если объект прокси сервера существует
	if(proxy != NULL){
		// Выводим в консоль информацию
		proxy->server->log->write(LOG_MESSAGE, "[+] start service: pid = %i, socket = %i", getpid(), socket);
		// Создаем новую базу
		proxy->base = event_base_new();
		// Добавляем событие в базу
		struct event * evnt = event_new(proxy->base, socket, EV_READ | EV_PERSIST, &HttpProxy::accept_cb, proxy);
		// Активируем событие
		event_add(evnt, NULL);
		// Активируем перебор базы событий
		event_base_loop(proxy->base, EVLOOP_NO_EXIT_ON_EMPTY);
		// Отключаем подключение для сокета
		shutdown(socket, SHUT_RDWR);
		// Закрываем сокет
		close(socket);
		// Удаляем событие
		event_del(evnt);
		// Очищаем событие
		event_free(evnt);
		// Удаляем объект базы событий
		event_base_free(proxy->base);
	}
}
/**
 * run_works Метод запуска воркеров
 * @param pids   указатель на массив пидов процессов
 * @param socket сокет прокси сервера
 * @param cur    текущее значение пида процесса
 * @param max    максимальное значение пидов процессов
 * @param ctx    объект прокси сервера
 */
void HttpProxy::run_works(pid_t * pids, evutil_socket_t socket, size_t cur, size_t max, void * ctx){
	// Получаем объект прокси сервера
	HttpProxy * proxy = reinterpret_cast <HttpProxy *> (ctx);
	// Если массив пидов существует
	if((pids != NULL) && (proxy != NULL) && (socket > 0) && max){
		// Если не все форки созданы
		if(cur < max){
			// Выполняем форк процесса
			switch(pids[cur] = fork()){
				// Если поток не создан
				case -1: {
					// Выводим в консоль информацию
					proxy->server->log->write(LOG_ERROR, "[-] create fork error");
					// Выходим из приложения
					exit(SIGHUP);
				}
				// Если это дочерний поток значит все нормально, запускаем прокси сервер
				case 0: run_server(socket, proxy); break;
				// Если это родительский процесс
				default: run_works(pids, socket, cur + 1, max, proxy);
			}
		// Если все процессы созданы и это родительский процесс
		} else {
			// Добавляем свой идентификатор в массив тем самым id балансера всегда будет последним в списке
			pids[max] = getpid();
			// Отправляем идентификаторы созданных пидов управляющему балансером
			proxy->server->sendPids(pids, max + 1);
			// Статус воркера
			int status;
			// Ждем завершение работы потомка (от 1 потому что 0-й это этот же процесс а он не может ждать завершения самого себя)
			for(u_int i = 0; i < max; i++){
				// Ожидаем завершения процесса
				waitpid(pids[i], &status, 0);
				// Выводим в консоль информацию
				proxy->server->log->write(LOG_ERROR, "[-] end service: pid = %i, status = %i", pids[i], WTERMSIG(status));
			}
			// Выходим из приложения
			exit(SIGHUP);
		}
	}
}
/**
 * HttpProxy Конструктор
 * @param proxy объект параметров прокси сервера
 */
HttpProxy::HttpProxy(System * proxy){
	// Если лог существует
	if(proxy != NULL){
		// Запоминаем параметры прокси сервера
		this->server = proxy;
		// Создаем прокси сервер
		evutil_socket_t socket = create_server();
		// Если сокет существует
		if(socket > -1){
			// Если режим отладки не включен
			if(!this->server->config->proxy.debug){
				// Определяем максимальное количество потоков
				u_int max_works = (
					this->server->config->proxy.maxworks
					? this->server->config->proxy.maxworks
					: this->server->config->os.ncpu
				);
				// Если максимальное число пидов больше указанного то запрещаем
				if(max_works > MMAX_WORKERS) max_works = MMAX_WORKERS;
				// Наши ID процесса и сессии
				pids = new pid_t[max_works];
				// Запускаем создание воркеров
				run_works(pids, socket, 0, max_works, this);
			// Если режим отладки включен, тогда просто запускаем прокси сервер
			} else {
				// Добавляем свой идентификатор пида
				pid_t pids[] = {getpid()};
				// Отправляем идентификатор только балансера
				this->server->sendPids(pids, 1);
				// Запускаем сервер
				run_server(socket, this);
			}
		// Иначе выходим окончательно
		} else exit(SIGSTOP);
	}
}
/**
 * ~HttpProxy Деструктор
 */
HttpProxy::~HttpProxy(){
	// Если массив процессов существует то удаляем его
	if(pids != NULL) delete [] pids;
}