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
 * add Метод добавления нового подключения в объект клиента
 * @param ctx передаваемый указатель на объект
 */
void ConnectClients::Client::add(void * ctx){
	// Захватываем поток
	this->mtx.lock();
	// Получаем данные подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	// Если подключение существует
	if(http){
		// Запоминаем блокиратор лишнего потока
		http->frze = &this->freeze;
		// Устанавливаем функцию удаления клиента
		http->remove = [this](){
			// Выполняем удаление текущего подключения
			rm();
		};
		// Устанавливаем функцию проверки доступных коннектов
		http->isFull = [this](){
			// Выводим результат проверки
			return (this->connects >= this->max);
		};
		// Устанавливаем функцию проверяющую активные подключения
		http->activeConnects = [this](){
			// Определяем количество подключений
			return (this->connects < this->max ? this->connects : this->max);
		};
		// Запоминаем ключ клиента
		this->key = (http->proxy->config->connects.key ? http->client.mac : http->client.ip);
		// Запоминаем максимально-возможное количество подключений
		this->max = http->proxy->config->connects.connect;
		// Создаем поток
		std::thread thr(&HttpProxy::connection, http);
		// Выполняем активацию потока
		thr.detach();
		// Если количество подключений еще не достигло максимума
		this->connects++;
	}
	// Освобождаем поток
	this->mtx.unlock();
}
/**
 * rm Метод удаления подключения из объекта клиента
 */
void ConnectClients::Client::rm(){
	// Захватываем поток
	this->mtx.lock();
	// Уменьшаем количество подключений
	this->connects--;
	// Освобождаем поток
	this->mtx.unlock();
	// Если подключения еще есть то отправляем сигнал
	if(this->connects && (this->connects < this->max))
		// Сообщаем что подключения еще есть
		this->freeze.cond.notify_one();
	// Если это было последнее подключение то удаляем объект клиента
	else if(!this->connects) this->remove(this->key);
}
/**
 * add Метод добавления нового подключения в объект пользователя
 * @param ctx передаваемый указатель на объект
 */
void ConnectClients::add(void * ctx){
	// Получаем данные подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	// Если подключение существует
	if(http){
		// Определяем тип ключа который будет использован для определения клиента
		string key = (http->proxy->config->connects.key ? http->client.mac : http->client.ip);
		// Если клиент не найден
		if(this->clients.count(key) < 1){
			// Создаем новый объект клиента
			unique_ptr <Client> client(new Client);
			// Добавляем в список нового клиента
			this->clients.insert(pair <string, unique_ptr <Client>> (key, move(client)));
		}
		// Получаем данные клиента
		Client * client = (this->clients.find(key)->second).get();
		// Устанавливаем функцию удаления клиента
		client->remove = [this](const string key){
			// Выполняем удаление клиента
			rm(key);
		};
		// Передаем клиенту данные объекта
		client->add(http);
	}
}
/**
 * rm Метод удаления объекта подключившихся клиентов
 * @param key ключ клиента
 */
void ConnectClients::rm(const string key){
	// Захватываем поток
	this->mtx.lock();
	// Если такой клиент найден
	if(this->clients.count(key) > 0){
		// Получаем итератор
		auto it = this->clients.find(key);
		// Если клиент найден то удаляем его
		this->clients.erase(it);
	}
	// Освобождаем поток
	this->mtx.unlock();
}
/**
 * free_socket Метод отключения сокета
 * @param fd ссылка на файловый дескриптор (сокет)
 */
void BufferHttpProxy::free_socket(evutil_socket_t * fd){
	// Если сокет существует
	if(* fd > -1){
		// Отключаем подключение для сокета
		shutdown(* fd, SHUT_RDWR);
		// Закрываем сокет
		::close(* fd);
		// Сообщаем что сокет закрыт
		* fd = -1;
	}
}
/**
 * free_event Метод удаления буфера события
 * @param event указатель на объект буфера события
 */
void BufferHttpProxy::free_event(struct bufferevent ** event){
	// Удаляем событие
	if(* event != NULL){
		// Удаляем буфер события
		bufferevent_free(*event);
		// Устанавливаем что событие удалено
		* event = NULL;
	}
}
/**
 * closeClient Метод закрытия соединения клиента
 */
void BufferHttpProxy::closeClient(){
	// Захватываем поток
	this->mtx.lock();
	// Закрываем сокет
	free_socket(&this->sockets.client);
	// Закрываем буфер события
	free_event(&this->events.client);
	// Освобождаем поток
	this->mtx.unlock();
}
/**
 * closeServer Метод закрытия соединения сервера
 */
void BufferHttpProxy::closeServer(){
	// Захватываем поток
	this->mtx.lock();
	// Закрываем сокет
	free_socket(&this->sockets.server);
	// Закрываем буфер события
	free_event(&this->events.server);
	// Освобождаем поток
	this->mtx.unlock();
}
/**
 * close Метод закрытия подключения
 */
void BufferHttpProxy::close(){
	// Закрываем подключение клиента
	closeClient();
	// Закрываем подключение сервера
	closeServer();
	// Захватываем поток
	this->mtx.lock();
	// Удаляем базу событий
	event_base_loopbreak(this->base);
	// Освобождаем поток
	this->mtx.unlock();
}
/**
 * freeze Метод заморозки потока
 */
void BufferHttpProxy::freeze(){
	// Если все коннекты исчерпаны
	if(this->isFull()){
		// Получаем блокиратор потока
		ConnectClients::Freeze * frze = reinterpret_cast <ConnectClients::Freeze *> (this->frze);
		// Лочим мютекс
		unique_lock <mutex> locker(frze->mtx);
		// Блокируем поток
		frze->cond.wait(locker);
	}
}
/**
 * setCompress Метод проверки активации режима сжатия данных на уровне прокси сервера
 */
void BufferHttpProxy::setCompress(){
	// Если контент пришел не сжатым а сжатие требуется
	if((this->proxy->config->options & OPT_PGZIP)
	&& !this->client.connect
	&& this->httpResponse.isEndHeaders()
	&& this->httpResponse.getHeader("content-encoding").empty()){
		// Если это режим сжатия, тогда отправляем завершающие данные
		if(!this->httpResponse.isIntGzip()){
			// Получаем размер тела
			string clength = this->httpResponse.getHeader("content-length");
			// Получаем тип данных чанкованием
			string chunked = this->httpResponse.getHeader("transfer-encoding");
			// Получаем размер контента
			size_t contentLength = (!clength.empty() ? ::atoi(clength.c_str()) : 0);
			// Если размер контента не найден тогда проверяем на чанкование
			if(!contentLength && (!chunked.empty() && (chunked.find("chunked") != string::npos))) contentLength = 2;
			// Если размер контента не найден но это закрытие подключения
			else if(!contentLength && this->httpResponse.isClose()) contentLength = 1;
			// Если размер контента не найден тогда выходим
			if(!contentLength) return;
			// Флаг установки режима сжатия
			bool gzip = true;
			// Результат работы регулярного выражения
			smatch match;
			// Получаем наличие заголовка Via
			string via = this->httpResponse.getHeader("via");
			// Получаем тип файла
			string cmime = this->httpResponse.getHeader("content-type");
			// Устанавливаем правило регулярного выражения
			regex e(this->proxy->config->gzip.regex, regex::ECMAScript | regex::icase);
			// Получаем версию протокола
			for(u_int i = 0; i < this->proxy->config->gzip.vhttp.size(); i++){
				// Поверяем на соответствие версии
				if(this->httpResponse.getVersion() == float(::atof(this->proxy->config->gzip.vhttp[i].c_str()))){
					// Запоминаем что протокол проверку прошел
					gzip = true;
					// Выходим
					break;
				// Запрещаем сжатие
				} else gzip = false;
			}
			// Если сжатие разрешено
			if(gzip){
				// Проверяем на тип данных
				for(u_int i = 0; i < this->proxy->config->gzip.types.size(); i++){
					// Выполняем проверку на тип данных
					if((this->proxy->config->gzip.types[i].compare("*") == 0)
					|| (cmime.find(this->proxy->config->gzip.types[i]) != string::npos)){
						// Запоминаем что протокол проверку прошел
						gzip = true;
						// Выходим
						break;
					// Запрещаем сжатие
					} else gzip = false;
				}
				// Если сжатие разрешено
				if(gzip && !via.empty()){
					// Readme: http://nginx.org/ru/docs/http/ngx_http_gzip_module.html
					// Запрещаем сжатие
					gzip = false;
					// Переходим по всем параметрам
					for(u_int i = 0; i < this->proxy->config->gzip.proxied.size(); i++){
						// Получаем параметр
						const string param = this->proxy->config->gzip.proxied[i];
						// Запрещаем сжатие для всех проксированных запросов, игнорируя остальные параметры;
						if(param.compare("off") == 0){
							// Запрещаем сжатие
							gzip = false;
							// Выходим
							break;
						// Разрешаем сжатие для всех проксированных запросов;
						} else if(param.compare("any") == 0) {
							// Разрешаем сжатие
							gzip = true;
							// Выходим
							break;
						// Разрешаем сжатие, если в заголовке ответа есть поле “Expires” со значением, запрещающим кэширование;
						} else if(param.compare("expired") == 0) {
							// Получаем наличие заголовка Expires
							if(!this->httpResponse.getHeader("expires").empty()) gzip = true;
							// Если проверка не пройдена
							else {
								// Запрещаем сжатие
								gzip = false;
								// Выходим
								break;
							}
						// Разрешаем сжатие, если в заголовке ответа есть поле “Cache-Control” с параметром “no-cache”;
						} else if(param.compare("no-cache") == 0) {
							// Получаем наличие заголовка Cache-Control
							string cc = this->httpResponse.getHeader("cache-control");
							// Если заголовок существует
							if(!cc.empty() && (toCase(cc).find(param) != string::npos)) gzip = true;
							// Если проверка не пройдена
							else {
								// Запрещаем сжатие
								gzip = false;
								// Выходим
								break;
							}
						// Разрешаем сжатие, если в заголовке ответа есть поле “Cache-Control” с параметром “no-store”;
						} else if(param.compare("no-store") == 0) {
							// Получаем наличие заголовка Cache-Control
							string cc = this->httpResponse.getHeader("cache-control");
							// Если заголовок существует
							if(!cc.empty() && (toCase(cc).find(param) != string::npos)) gzip = true;
							// Если проверка не пройдена
							else {
								// Запрещаем сжатие
								gzip = false;
								// Выходим
								break;
							}
						// Разрешаем сжатие, если в заголовке ответа есть поле “Cache-Control” с параметром “private”;
						} else if(param.compare("private") == 0) {
							// Получаем наличие заголовка Cache-Control
							string cc = this->httpResponse.getHeader("cache-control");
							// Если заголовок существует
							if(!cc.empty() && (toCase(cc).find(param) != string::npos)) gzip = true;
							// Если проверка не пройдена
							else {
								// Запрещаем сжатие
								gzip = false;
								// Выходим
								break;
							}
						// Разрешаем сжатие, если в заголовке ответа нет поля “Last-Modified”;
						} else if(param.compare("no_last_modified") == 0) {
							// Получаем наличие заголовка Last-Modified
							if(this->httpResponse.getHeader("last-modified").empty()) gzip = true;
							// Если проверка не пройдена
							else {
								// Запрещаем сжатие
								gzip = false;
								// Выходим
								break;
							}
						// Разрешаем сжатие, если в заголовке ответа нет поля “ETag”;
						} else if(param.compare("no_etag") == 0) {
							// Получаем наличие заголовка ETag
							if(this->httpResponse.getHeader("etag").empty()) gzip = true;
							// Если проверка не пройдена
							else {
								// Запрещаем сжатие
								gzip = false;
								// Выходим
								break;
							}
						// Разрешаем сжатие, если в заголовке запроса есть поле “Authorization”;
						} else if(param.compare("auth") == 0) {
							// Получаем наличие заголовка Authorization
							if(!this->httpResponse.getHeader("authorization").empty()) gzip = true;
							// Если проверка не пройдена
							else {
								// Запрещаем сжатие
								gzip = false;
								// Выходим
								break;
							}
						}
					}
				}
				// Проверяем есть ли размер
				if(gzip && contentLength){
					// Проверяем соответствует ли размер
					if(this->proxy->config->gzip.length > contentLength) gzip = false;
				}
				// Если сжатие разрешено
				if(gzip){
					// Выполняем проверку
					regex_search(this->client.useragent, match, e);
					// Если проверка не пройдена тогда запрещаем сжатие
					if(!match.empty()) gzip = false;
				}
				// Если запрещено выводить заголовок Vary
				if(gzip && this->proxy->config->gzip.vary){
					// Считываем заголовок
					string vary = this->httpResponse.getHeader("vary");
					// Проверяем наличие
					if(!vary.empty() && (toCase(vary)
					.find("accept-encoding") != string::npos)) this->httpResponse.rmHeader("vary");
				}
			}
			// Устанавливаем что идет сжатие
			if(gzip) this->httpResponse.setGzip();
		}
	}
}
/**
 * checkUpgrade Метод проверки на желание смены протокола
 */
void BufferHttpProxy::checkUpgrade(){
	// Если сервер переключил версию протокола с HTTP1.0 на HTTP2
	if(!this->client.connect
	&& this->httpResponse.isEndHeaders()
	&& this->httpResponse.isUpgrade()
	&& (this->httpResponse.getStatus() == 101)){
		// Устанавливаем что это соединение CONNECT,
		// для будущего обмена данными, так как они будут приходить бинарные
		this->client.connect = true;
	}
}
/**
 * checkClose Метод проверки на отключение от сервера
 */
void BufferHttpProxy::checkClose(){
	// Если это закрытие коннекта на стороне сервера
	if(!this->client.connect
	&& this->httpResponse.isEndHeaders()
	&& this->httpResponse.isClose()){
		// Указываем что запрос завершен
		this->httpResponse.setBodyEnd();
		// Отсылаем данные клиенту
		this->sendClient();
		// Очищаем объект http данных
		this->httpRequest.clear();
		// Формируем лог данные
		string log = (this->client.request + "\r\n\r\n");
		// Дополняем лог данные, данными ответа
		log.append(this->httpResponse.getRawResponseData());
		// Выполняем запись данные запроса в лог
		this->proxy->log->write_data(this->client.ip, log);
	// Выполняем отключение
	} else close();
}
/**
 * sleep Метод установки сна потока на время необходимое для соблюдения скоростного ограничения сети
 * @param  size размер передаваемых данных
 * @param  type тип передаваемого сообщения (true - чтение, false - запись)
 * @return      время в секундах на которое следует задержать поток
 */
void BufferHttpProxy::sleep(const size_t size, const bool type){
	// Расчитанное время задержки
	int seconds = 0;
	// Получаем размер максимально-возможных передачи данных для всех подключений
	float max = float(type ? this->proxy->config->buffers.read : this->proxy->config->buffers.write);
	// Если буфер существует
	if(max > 0){
		// Высчитываем размер максимально-возможных передачи данных для одного подключения
		max = (max / float(this->activeConnects()));
		// Если размер больше нуля то продолжаем
		if((max > 0) && (size > max)) seconds = (size / max);
		// Погружаем поток в сон на указанное время, чтобы соблюсти предел скорости
		this_thread::sleep_for(chrono::seconds(seconds));
	}
}
/**
 * setTimeout Метод установки таймаутов
 * @param type  тип подключения (клиент или сервер)
 * @param read  таймаут на чтение
 * @param write таймаут на запись
 */
void BufferHttpProxy::setTimeout(const u_short type, const bool read, const bool write){
	// Устанавливаем таймаут ожидания результата на чтение с сервера
	struct timeval _read = {this->proxy->config->timeouts.read, 0};
	// Устанавливаем таймаут записи результата на запись
	struct timeval _write = {this->proxy->config->timeouts.write, 0};
	// Получаем параметры флагов
	bool readVal	= read;
	bool writeVal	= write;
	// Если время постоянного подключения меньше 1 секунды значит таймер ставить не надо
	if(this->proxy->config->timeouts.read < 1)	readVal		= false;
	if(this->proxy->config->timeouts.write < 1)	writeVal	= false;
	// Устанавливаем таймауты для сервера
	if((type & TM_SERVER) && this->events.server)
		// Устанавливаем таймауты
		bufferevent_set_timeouts(this->events.server, (readVal ? &_read : NULL), (writeVal ? &_write : NULL));
	// Устанавливаем таймауты для клиента
	if((type & TM_CLIENT) && this->events.client)
		// Устанавливаем таймауты
		bufferevent_set_timeouts(this->events.client, (readVal ? &_read : NULL), (writeVal ? &_write : NULL));
}
/**
 * sendClient Метод отправки данных на клиент
 */
void BufferHttpProxy::sendClient(){
	// Если это код разрешающий коннект, устанавливаем таймер для клиента
	if(!this->httpResponse.isClose()) this->setTimeout(TM_CLIENT, true, true);
	// Если это завершение работы то устанавливаем таймер только на запись
	else this->setTimeout(TM_CLIENT, false, true);
	/** Данные установки необходимы для модификации заголовков НАЧАЛО **/
	// Запоминаем метод который был в запросе
	this->httpResponse.setMethod(this->httpRequest.getMethod());
	// Запоминаем путь который был в запросе
	this->httpResponse.setPath(this->httpRequest.getPath());
	// Запоминаем домен который был в запросе
	this->httpResponse.setHost(this->httpRequest.getHost());
	// Выполняем модификацию заголовков
	this->proxy->headers->modify(this->client.ip, this->client.mac, this->server.ip, this->httpResponse);
	/** Данные установки необходимы для модификации заголовков КОНЕЦ **/
	// Запоминаем данные запроса
	string response = this->httpResponse.getResponseData(!this->httpResponse.isClose() && (this->httpResponse.getVersion() > 1));
	// Погружаем поток в сон на указанное время, чтобы соблюсти предел скорости
	if(!this->httpResponse.isClose()) this->sleep(response.size(), false);
	// Устанавливаем водяной знак на количество байт необходимое для идентификации переданных данных
	bufferevent_setwatermark(this->events.client, EV_WRITE, response.size(), 0);
	// Активируем ватермарк
	bufferevent_enable(this->events.client, EV_WRITE);
	// Отправляем клиенту сообщение
	bufferevent_write(this->events.client, response.data(), response.size());
	// Добавляем данные в кэш
	this->proxy->cache->setCache(this->httpResponse);
}
/**
 * sendServer Метод отправки данных на сервер
 */
void BufferHttpProxy::sendServer(){
	// Если это не постоянное подключение
	if(!this->client.alive)
		// Устанавливаем таймаут на чтение и запись
		this->setTimeout(TM_SERVER, true, true);
	// Устанавливаем таймаут только на запись
	else this->setTimeout(TM_SERVER, false, true);
	// Выполняем модификацию заголовков
	this->proxy->headers->modify(this->client.ip, this->client.mac, this->server.ip, this->httpRequest);
	// Формируем запрос на сервер
	this->client.request = this->httpRequest.getRequestData();
	// Погружаем поток в сон на указанное время, чтобы соблюсти предел скорости
	this->sleep(this->client.request.size(), false);
	// Отправляем серверу сообщение
	bufferevent_write(this->events.server, this->client.request.data(), this->client.request.size());
	// Удаляем объект подключения
	if(!this->parser.httpData.empty()){
		// Удаляем из массива объект запроса
		this->parser.httpData.erase(this->parser.httpData.begin());
	}
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
	this->parser.create(this->proxy->config->proxy.name, this->proxy->config->options);
	// Определяем тип подключения
	switch(this->proxy->config->proxy.extIPv){
		// Для протокола IPv4
		case 4: this->dns = new DNSResolver(
			this->proxy->log,
			this->proxy->cache,
			this->base,
			AF_INET,
			this->proxy->config->ipv4.resolver
		); break;
		// Для протокола IPv6
		case 6: this->dns = new DNSResolver(
			this->proxy->log,
			this->proxy->cache,
			this->base,
			AF_INET6,
			this->proxy->config->ipv6.resolver
		); break;
	}
}
/**
 * ~BufferHttpProxy Деструктор
 */
BufferHttpProxy::~BufferHttpProxy(){
	// Захватываем мютекс
	this->mtx.lock();
	// Удаляем dns сервер
	delete this->dns;
	// Очищаем объект базы событий
	event_base_free(this->base);
	// Освобождаем мютекс
	this->mtx.unlock();
}
/**
 * create_client Метод создания нового клиента
 * @param ip  адрес интернет протокола клиента
 * @param mac аппаратный адрес сетевого интерфейса клиента
 * @param fd  файловый дескриптор (сокет) подключившегося клиента
 */
void HttpProxy::create_client(const string ip, const string mac, const evutil_socket_t fd){
	// Устанавливаем неблокирующий режим для сокета
	socket_nonblocking(fd, this->server->log);
	// Устанавливаем разрешение на повторное использование сокета
	socket_reuseable(fd, this->server->log);
	// Отключаем сигнал записи в оборванное подключение
	socket_nosigpipe(fd, this->server->log);
	// Выводим в лог сообщение
	this->server->log->write(LOG_ACCESS, 0, "client connect to proxy server, host = %s, mac = %s, socket = %d", ip.c_str(), mac.c_str(), fd);
	// Создаем новый объект подключения
	BufferHttpProxy * http = new BufferHttpProxy(this->server);
	// Запоминаем файловый дескриптор текущего подключения
	http->sockets.client = fd;
	// Запоминаем данные мак адреса
	http->client.mac = mac;
	// Запоминаем данные клиента
	http->client.ip = ip;
	// Добавляем в список клиентов объект подключения
	this->clients.add(http);
}
/**
 * get_mac Метод определения мак адреса клиента
 * @param  ctx указатель на объект подключения
 * @return     данные мак адреса
 */
const string HttpProxy::get_mac(void * ctx){
	// Буфер для копирования мак адреса
	char buff[256];
	// Получаем данные адреса
	struct sockaddr * s = reinterpret_cast <struct sockaddr *> (ctx);
	// Получаем указатель на мак адрес
	u_char * ptr = (u_char *) s->sa_data;
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
const string HttpProxy::get_ip(const int family, void * ctx){
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
 * socket_nosigpipe Функция установки отключения сигнала записи в оборванное подключение
 * @param  fd   файловый дескриптор (сокет)
 * @param  log  указатель на объект ведения логов
 * @return      результат работы функции
 */
const int HttpProxy::socket_nosigpipe(const evutil_socket_t fd, LogApp * log){
	// Устанавливаем параметр
	int nosigpipe = 1;
	// Устанавливаем SO_NOSIGPIPE
	if(setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &nosigpipe, sizeof(nosigpipe)) < 0){
		// Выводим в лог информацию
		log->write(LOG_ERROR, 0, "cannot set SO_NOSIGPIPE option on socket %d", fd);
		// Выходим
		return -1;
	}
	// Все удачно
	return 0;
}
/**
 * socket_nonblocking Функция установки неблокирующего сокета
 * @param  fd   файловый дескриптор (сокет)
 * @param  log  указатель на объект ведения логов
 * @return      результат работы функции
 */
const int HttpProxy::socket_nonblocking(const evutil_socket_t fd, LogApp * log){
	int flags;
	flags = fcntl(fd, F_GETFL);
	if(flags < 0) return flags;
	flags |= O_NONBLOCK;
	// Устанавливаем неблокирующий режим
	if(fcntl(fd, F_SETFL, flags) < 0){
		// Выводим в лог информацию
		log->write(LOG_ERROR, 0, "cannot set NON_BLOCK option on socket %d", fd);
		// Выходим
		return -1;
	}
	// Все удачно
	return 0;
}
/**
 * socket_tcpcork Функция активации tcp_cork
 * @param  fd   файловый дескриптор (сокет)
 * @param  log  указатель на объект ведения логов
 * @return      результат работы функции
 */
const int HttpProxy::socket_tcpcork(const evutil_socket_t fd, LogApp * log){
	// Устанавливаем параметр
	int tcpcork = 1;
// Если это Linux
#ifdef __linux__
	// Устанавливаем TCP_CORK
	if(setsockopt(fd, IPPROTO_TCP, TCP_CORK, &tcpcork, sizeof(tcpcork)) < 0){
		// Выводим в лог информацию
		log->write(LOG_ERROR, 0, "cannot set TCP_CORK option on socket %d", fd);
		// Выходим
		return -1;
	}
// Если это FreeBSD или MacOS X
#elif __APPLE__ || __FreeBSD__
	// Устанавливаем TCP_NOPUSH
	if(setsockopt(fd, IPPROTO_TCP, TCP_NOPUSH, &tcpcork, sizeof(tcpcork)) < 0){
		// Выводим в лог информацию
		log->write(LOG_ERROR, 0, "cannot set TCP_NOPUSH option on socket %d", fd);
		// Выходим
		return -1;
	}
#endif
	// Все удачно
	return 0;
}
/**
 * socket_tcpnodelay Функция отключения алгоритма Нейгла
 * @param  fd   файловый дескриптор (сокет)
 * @param  log  указатель на объект ведения логов
 * @return      результат работы функции
 */
const int HttpProxy::socket_tcpnodelay(const evutil_socket_t fd, LogApp * log){
	// Устанавливаем параметр
	int tcpnodelay = 1;
	// Устанавливаем TCP_NODELAY
	if(setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &tcpnodelay, sizeof(tcpnodelay)) < 0){
		// Выводим в лог информацию
		log->write(LOG_ERROR, 0, "cannot set TCP_NODELAY option on socket %d", fd);
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
const int HttpProxy::socket_reuseable(const evutil_socket_t fd, LogApp * log){
	// Устанавливаем параметр
	int reuseaddr = 1;
	// Разрешаем повторно использовать тот же host:port после отключения
	if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)) < 0){
		// Выводим в лог информацию
		log->write(LOG_ERROR, 0, "cannot set SO_REUSEADDR option on socket %d", fd);
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
const int HttpProxy::socket_keepalive(const evutil_socket_t fd, LogApp * log, const int cnt, const int idle, const int intvl){
	// Устанавливаем параметр
	int keepalive = 1;
	// Активация постоянного подключения
	if(setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(int))){
		// Выводим в лог информацию
		log->write(LOG_ERROR, 0, "cannot set SO_KEEPALIVE option on socket %d", fd);
		// Выходим
		return -1;
	}
	// Максимальное количество попыток
	if(setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &cnt, sizeof(int))){
		// Выводим в лог информацию
		log->write(LOG_ERROR, 0, "cannot set TCP_KEEPCNT option on socket %d", fd);
		// Выходим
		return -1;
	}
// Если это MacOS X
#ifdef __APPLE__
	// Время через которое происходит проверка подключения
	if(setsockopt(fd, IPPROTO_TCP, TCP_KEEPALIVE, &idle, sizeof(int))){
		// Выводим в лог информацию
		log->write(LOG_ERROR, 0, "cannot set TCP_KEEPALIVE option on socket %d", fd);
		// Выходим
		return -1;
	}
// Если это FreeBSD или Linux
#elif __linux__ || __FreeBSD__
	// Время через которое происходит проверка подключения
	if(setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(int))){
		// Выводим в лог информацию
		log->write(LOG_ERROR, 0, "cannot set TCP_KEEPIDLE option on socket %d", fd);
		// Выходим
		return -1;
	}
#endif
	// Время между попытками
	if(setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &intvl, sizeof(int))){
		// Выводим в лог информацию
		log->write(LOG_ERROR, 0, "cannot set TCP_KEEPINTVL option on socket %d", fd);
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
const int HttpProxy::socket_buffersize(const evutil_socket_t fd, const int read_size, const int write_size, const u_int maxcon, LogApp * log){
	// Получаем переданные размеры
	int readSize	= read_size;
	int writeSize	= write_size;
	// Определяем размер массива опции
	socklen_t read_optlen	= sizeof(readSize);
	socklen_t write_optlen	= sizeof(write_size);
	// Устанавливаем размер буфера для сокета на чтение
	if(readSize > 0){
		// Выполняем перерасчет размера буфера
		readSize = (readSize / maxcon);
		// Устанавливаем размер буфера
		setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *) &readSize, read_optlen);
	}
	// Устанавливаем размер буфера для сокета на запись
	if(writeSize > 0){
		// Выполняем перерасчет размера буфера
		writeSize = (writeSize / maxcon);
		// Устанавливаем размер буфера
		setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *) &writeSize, write_optlen);
	}
	// Считываем установленный размер буфера
	if((getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &readSize, &read_optlen) < 0)
	|| (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &writeSize, &write_optlen) < 0)){
		// Выводим в лог информацию
		log->write(LOG_ERROR, 0, "get buffer wrong on socket %d", fd);
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
const bool HttpProxy::check_auth(void * ctx){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	// Если подключение не передано
	if(http){
		// Логин
		const char * username = "zdD786KeuS";
		// Проль
		const char * password = "k.frolovv@gmail.com";
		// Проверяем логин и пароль
		if(!strcmp(http->httpRequest.getLogin().c_str(), username)
		&& !strcmp(http->httpRequest.getPassword().c_str(), password)) return true;
		// Выводим в лог информацию о неудачном подключении
		http->proxy->log->write(LOG_MESSAGE, 0, "auth client [%s] to proxy wrong!", http->client.ip.c_str());
	}
	// Сообщаем что проверка не прошла
	return false;
}
/**
 * isallow_remote_connect Функция проверяет разрешено ли подключение к удаленному серверу
 * @param  ip  ip адрес удаленного сервера
 * @param  ctx объект с данными подключения
 * @return     результат проверки
 */
const bool HttpProxy::isallow_remote_connect(const string ip, void * ctx){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	// Если подключение не передано
	if(http){
		// Создаем объект сети
		Network nwk;
		// Результат проверки
		int check = -1;
		// Определяем тип подключения
		switch(http->proxy->config->proxy.extIPv){
			// Для протокола IPv4
			case 4: check = nwk.isLocal(ip);	break;
			// Для протокола IPv6
			case 6: check = nwk.isLocal6(ip);	break;
		}
		// Проверяем ip адрес
		switch(check){
			// Если это запрещенный ip адрес
			case -1: return false;
			// Если это локальный адрес
			case 0: return http->proxy->config->proxy.reverse;
			// Если это доступ во внешнюю сеть
			case 1: return http->proxy->config->proxy.forward;
		}
	}
	// Запрещаем подключение
	return false;
}
/**
 * connect_server Функция создания сокета для подключения к удаленному серверу
 * @param ctx объект входящих данных
 * @return    результат подключения
 */
const int HttpProxy::connect_server(void * ctx){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	// Если подключение не передано
	if(http){
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
			switch(http->proxy->config->proxy.extIPv){
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
					server4_addr.sin_addr.s_addr = inet_addr(http->server.ip.c_str());
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
					struct hostent * server	= gethostbyname2(http->server.ip.c_str(), AF_INET6);
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
					LOG_ERROR, 0,
					"creating socket to server = %s, port = %d, client = %s",
					http->server.ip.c_str(),
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
			// Отключаем сигнал записи в оборванное подключение
			socket_nosigpipe(http->sockets.server, http->proxy->log);
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
				http->proxy->log->write(LOG_ERROR, 0, "bind local network [%s] error", bindhost.c_str());
				// Выходим
				return 0;
			}
			// Создаем буфер событий для сервера
			http->events.server = bufferevent_socket_new(http->base, http->sockets.server, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
			// Устанавливаем водяной знак на 1 байт (чтобы считывать данные когда они действительно приходят)
			// bufferevent_setwatermark(http->events.server, EV_READ | EV_WRITE, 1, 0);
			// Устанавливаем коллбеки
			bufferevent_setcb(http->events.server, &HttpProxy::read_server_cb, NULL, &HttpProxy::event_cb, http);
			// Очищаем буферы событий при завершении работы
			bufferevent_flush(http->events.server, EV_READ | EV_WRITE, BEV_FINISHED);
			// Активируем буферы событий на чтение и запись
			bufferevent_enable(http->events.server, EV_READ | EV_WRITE);
			// Выполняем подключение к удаленному серверу, если подключение не выполненно то сообщаем об этом
			if(bufferevent_socket_connect(http->events.server, sot, sotlen) < 0){
				// Выводим в лог сообщение
				http->proxy->log->write(
					LOG_ERROR, 0,
					"connecting to server = %s, port = %d, client = %s",
					http->server.ip.c_str(),
					http->server.port,
					http->client.ip.c_str()
				);
				// Выходим
				return -1;
			}
			// Выводим в лог сообщение о новом коннекте
			http->proxy->log->write(
				LOG_MESSAGE, 0,
				"connect client [%s] to host = %s [%s:%d], mac = %s, method = %s, path = %s, useragent = %s, socket = %d",
				http->client.ip.c_str(),
				http->httpRequest.getHost().c_str(),
				http->server.ip.c_str(),
				http->server.port,
				http->server.mac.c_str(),
				http->httpRequest.getMethod().c_str(),
				http->httpRequest.getPath().c_str(),
				http->httpRequest.getUseragent().c_str(),
				http->sockets.client
			);
			// Сообщаем что все удачно
			return 1;
		// Если сервер уже подключен, сообщаем что все удачно
		} else {
			// Выводим в лог сообщение о новом коннекте
			http->proxy->log->write(
				LOG_MESSAGE, 0,
				"last connect client [%s] to host = %s [%s:%d], mac = %s, method = %s, path = %s, useragent = %s, socket = %d",
				http->client.ip.c_str(),
				http->httpRequest.getHost().c_str(),
				http->server.ip.c_str(),
				http->server.port,
				http->server.mac.c_str(),
				http->httpRequest.getMethod().c_str(),
				http->httpRequest.getPath().c_str(),
				http->httpRequest.getUseragent().c_str(),
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
void HttpProxy::event_cb(struct bufferevent * bev, const short events, void * ctx){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	// Если подключение не передано
	if(http){
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
					LOG_ACCESS, 0,
					"connect client [%s], useragent = %s, socket = %d to server [%s:%d]",
					http->client.ip.c_str(),
					http->client.useragent.c_str(),
					current_fd,
					http->server.ip.c_str(),
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
				if(err) http->proxy->log->write(LOG_ERROR, 0, "DNS error: %s", evutil_gai_strerror(err));
			}
			// Если отключился клиент
			if(subject.compare("client") == 0){
				// Выводим в лог сообщение
				http->proxy->log->write(
					LOG_ACCESS, 0,
					"closing client [%s] from server [%s:%d], socket = %d",
					http->client.ip.c_str(),
					http->server.ip.c_str(),
					http->server.port,
					current_fd
				);
				// Закрываем подключение
				http->close();
			// Если отключился сервер
			} else {
				// Выводим в лог сообщение
				http->proxy->log->write(
					LOG_ACCESS, 0,
					"closing server [%s:%d] from client [%s], socket = %d",
					http->server.ip.c_str(),
					http->server.port,
					http->client.ip.c_str(),
					current_fd
				);
				// Закрываем подключение
				http->checkClose();
			}
		}
	}
	// Выходим
	return;
}
/**
 * send_http_data Функция отправки незашифрованных данных клиенту
 * @param ctx передаваемый объект
 */
void HttpProxy::send_http_data(void * ctx){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	// Если подключение не передано
	if(http){
		// Получаем буферы входящих данных и исходящих
		struct evbuffer * input		= bufferevent_get_input(http->events.server);
		struct evbuffer * output	= bufferevent_get_output(http->events.client);
		// Получаем размер входящих данных
		const size_t len = evbuffer_get_length(input);
		// Если данные существуют
		if(len){
			// Создаем буфер данных
			char * buffer = new char[len];
			// Копируем в буфер полученные данные
			evbuffer_copyout(input, buffer, len);
			// Добавляем данные тела
			size_t size = http->httpResponse.setEntitybody(buffer, len);
			// Получаем размер данных превысил разрешенный предел
			if(http->httpResponse.getRawBodySize() > http->proxy->config->connects.size){
				// Закрываем подключение сервера
				http->closeServer();
				// Активируем отдачу буферов целиком одним разом
				socket_tcpcork(http->sockets.client, http->proxy->log);
				// Формируем ответ клиенту, что домен не найден
				http->httpResponse.brokenRequest();
				// Отправляем ответ клиенту
				http->sendClient();
			// Если данные соответствуют разрешенным размерам
			} else {
				// И если это не автоотключение от сервера, так как эти данные будут отправлены при событии отключения
				if(!http->httpResponse.isClose()){
					// Если это не режим сжатия, тогда отправляем заголовок
					if(!http->httpResponse.isIntGzip()){
						// Создаем буфер для исходящих данных
						struct evbuffer * tmp = evbuffer_new();
						// Добавляем в буфер оставшиеся данные
						evbuffer_add(tmp, buffer, size);
						// Отправляем данные клиенту
						evbuffer_add_buffer(output, tmp);
						// Удаляем временный буфер
						evbuffer_free(tmp);
					// Если это удачно завершенный запрос и тело получено, отправляем данные клиенту
					} else if(http->httpResponse.isEndBody()) http->sendClient();
					// Если тело собрано то получаем данные тела для логов
					if(http->httpResponse.isEndBody()){
						// Формируем лог данные
						string log = (http->client.request + "\r\n\r\n");
						// Дополняем лог данные, данными ответа
						log.append(http->httpResponse.getRawResponseData());
						// Выполняем запись данные запроса в лог
						http->proxy->log->write_data(http->client.ip, log);
						// Очищаем объект http данных
						http->httpRequest.clear();
						// Если данные в массиве существуют тогда продолжаем загрузку
						if(!http->parser.httpData.empty()
						&& !http->httpResponse.isClose()) do_request(http);
					}
				}
				// Удаляем данные из буфера
				evbuffer_drain(input, size);
			}
			// Удаляем буфер данных
			delete [] buffer;
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
	if(http){
		// Получаем буферы входящих данных и исходящих
		struct evbuffer * input		= bufferevent_get_input(http->events.server);
		struct evbuffer * output	= bufferevent_get_output(http->events.client);
		// Ставим таймер только на чтение и запись
		http->setTimeout(TM_CLIENT, true, true);
		// Получаем размер входящих данных
		size_t len = evbuffer_get_length(input);
		// Погружаем поток в сон на указанное время, чтобы соблюсти предел скорости
		http->sleep(len, true);
		// Если заголовки менять не надо тогда просто обмениваемся данными
		if(http->client.connect) evbuffer_add_buffer(output, input);
		// Если это обычные данные, выполняем отправку данных
		else if(http->httpResponse.isEndHeaders()) send_http_data(http);
		// Иначе изменяем заголовки
		else if(len){
			// Считываем данные из буфера до тех пор пока можешь считать
			while(!http->httpResponse.isEndHeaders()){
				// Считываем строки из буфера
				const char * line = evbuffer_readln(input, &len, EVBUFFER_EOL_CRLF_STRICT);
				// Если данные не найдены тогда выходим
				if(!line) break;
				// Добавляем заголовки в запрос
				http->httpResponse.addHeader(line);
			}
			// Если все данные получены
			if(http->httpResponse.isEndHeaders()){
				// Получаем статус запроса
				const u_int status = http->httpResponse.getStatus();
				// Если статус утвердительный
				if(((status > 99) && (status < 200))
				|| ((status == 204) || (status == 205))
				|| (status > 300)){
					// Активируем отдачу буферов целиком одним разом
					socket_tcpcork(http->sockets.server, http->proxy->log);
					socket_tcpcork(http->sockets.client, http->proxy->log);
				}
				// Проверяем есть ли размер вложений
				string cl = http->httpResponse.getHeader("content-length");
				// Получаем размер данных превысил разрешенный предел
				if(!cl.empty() && (::atoi(cl.c_str()) > http->proxy->config->connects.size)){
					// Закрываем подключение сервера
					http->closeServer();
					// Активируем отдачу буферов целиком одним разом
					socket_tcpcork(http->sockets.client, http->proxy->log);
					// Формируем ответ клиенту, что домен не найден
					http->httpResponse.brokenRequest();
					// Отправляем ответ клиенту
					http->sendClient();
					// Выходим из обработки данных
					return;
				}
				// Проверяем является ли это переключение на другой протокол
				http->checkUpgrade();
				// Активируем сжатие данных, на стороне прокси сервера
				http->setCompress();
				// Если данных нет или это не сжатие на стороне прокси, отправляем заголовки
				if(!http->httpResponse.isIntGzip()
				&& !http->httpResponse.isClose()) http->sendClient();
				// Выполняем инициализацию тела данных
				http->httpResponse.initBody(
					http->proxy->config->gzip.chunk,
					http->proxy->config->gzip.level
				);
				// Если данные есть тогда продолжаем обработку данных
				if(evbuffer_get_length(input)) send_http_data(http);
			}
		// Закрываем соединение
		} else http->close();
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
	if(http){
		// Если дарес домена найден
		if(!ip.empty()){
			// Если подключение к указанному серверу разрешено
			if(isallow_remote_connect(ip, http)){
				// Определяем connect прокси разрешен
				bool conn_enabled = (http->proxy->config->options & OPT_CONNECT);
				// Если авторизация не прошла
				if(!http->auth) http->auth = check_auth(http);
				// Если нужно запросить пароль
				if(!http->auth && (http->httpRequest.getLogin().empty()
				|| http->httpRequest.getPassword().empty())){
					// Формируем ответ клиенту
					http->httpResponse.requiredAuth();
				// Сообщаем что авторизация не удачная
				} else if(!http->auth) {
					// Формируем ответ клиенту
					http->httpResponse.faultAuth();
				// Если авторизация прошла
				} else {
					// Получаем порт сервера
					u_int port = http->httpRequest.getPort();
					// Если хост и порт сервера не совпадают тогда очищаем данные
					if(http->events.server
					&& ((http->server.ip.compare(ip) != 0)
					|| (http->server.port != port))) http->closeServer();
					// Запоминаем хост и порт сервера
					http->server.ip		= ip;
					http->server.port	= port;
					// Заполняем структуру клиента
					http->client.alive		= http->httpRequest.isAlive();
					http->client.https		= http->httpRequest.isHttps();
					http->client.connect	= http->httpRequest.isConnect();
					http->client.useragent	= http->httpRequest.getUseragent();
					

					// Добавляем данные в кэш
					// auto tt = http->proxy->cache->getCache(http->httpRequest);

					// if(tt.load) cout << " --------------------- " << tt.http.getResponseData() << endl;

					// Выполняем подключение к удаленному серверу
					int connect = connect_server(http);
					// Если сокет существует
					if(connect > 0){
						// Определяем порт, если это метод connect
						if(http->client.connect && (conn_enabled || http->client.https))
							// Формируем ответ клиенту
							http->httpResponse.authSuccess();
						// Если connect разрешен только для https подключений
						else if(!conn_enabled && http->client.connect)
							// Сообращем что подключение запрещено
							http->httpResponse.faultConnect();
						// Иначе делаем запрос на получение данных
						else {
							// Указываем что нужно отключится сразу после отправки запроса
							if(!http->client.alive) http->httpRequest.setClose();
							// Отправляем данные на сервер
							http->sendServer();
							// Выходим
							return;
						}
					// Если подключение не удачное то сообщаем об этом
					} else if(connect == 0) http->httpResponse.faultConnect();
					// Если подключение удалено, выходим
					else {
						// Закрываем подключение
						http->close();
						// Выходим
						return;
					}
				}
			// Если подключение к указанному серверу запрещено
			} else http->httpResponse.faultAuth();
		// Если домен не найден
		} else {
			// Выводим в лог сообщение
			http->proxy->log->write(
				LOG_ERROR, 0,
				"host server = %s not found, port = %d, client = %s, socket = %d",
				http->httpRequest.getHost().c_str(),
				http->httpRequest.getPort(),
				http->client.ip.c_str(),
				http->sockets.client
			);
			// Формируем ответ клиенту, что домен не найден
			http->httpResponse.pageNotFound();
		}
		// Активируем отдачу буферов целиком одним разом
		if(http->httpResponse.isClose()) socket_tcpcork(http->sockets.client, http->proxy->log);
		// Отправляем ответ клиенту
		http->sendClient();
	}
	// Выходим
	return;
}
/**
 * do_request Функция запроса данных у сервера
 * @param ctx передаваемый объект
 */
void HttpProxy::do_request(void * ctx){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	// Если подключение не передано
	if(http){
		// Если данные еще не заполнены, но они есть в массиве
		if(!http->parser.httpData.empty()
		&& http->httpRequest.isEmpty()){
			// Очищаем таймеры для клиента
			http->setTimeout(TM_CLIENT);
			// Очищаем объект ответа
			http->httpResponse.clear();
			// Создаем объект http данных
			http->httpResponse.create(
				http->proxy->config->proxy.name,
				http->proxy->config->options
			);
			// Получаем первый элемент из массива
			auto httpData = http->parser.httpData.begin();
			// Запоминаем данные объекта http запроса
			http->httpRequest = * httpData;
			// Выполняем ресолв домена
			http->dns->resolve(http->httpRequest.getHost(), &HttpProxy::resolve_cb, http);
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
	// Если подключение передано
	if(http){
		// Получаем буферы входящих данных и исходящих
		struct evbuffer * output = bufferevent_get_output(http->events.client);
		// Получаем размер входящих данных
		const size_t len = evbuffer_get_length(output);
		// Отключаем клиента если требуется
		if(!len && http->httpResponse.isClose()) http->close();
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
	if(http){
		// Удаляем таймер для клиента
		http->setTimeout(TM_CLIENT | TM_SERVER);
		// Определяем connect прокси разрешен
		bool conn_enabled = (http->proxy->config->options & OPT_CONNECT);
		// Получаем буферы входящих данных и исходящих
		struct evbuffer * input = bufferevent_get_input(bev);
		// Получаем размер входящих данных
		size_t len = evbuffer_get_length(input);
		// Погружаем поток в сон на указанное время, чтобы соблюсти предел скорости
		http->sleep(len, true);
		// Если это метод connect
		if(http->client.connect && (conn_enabled || http->client.https)){
			// Получаем буферы входящих данных и исходящих
			struct evbuffer * output = bufferevent_get_output(http->events.server);
			// Если это не постоянное подключение
			if(!http->client.alive)
				// Устанавливаем таймаут на чтение и запись
				http->setTimeout(TM_SERVER, true, true);
			// Устанавливаем таймаут только на запись
			else http->setTimeout(TM_SERVER, false, true);
			// Выводим ответ сервера
			evbuffer_add_buffer(output, input);
		// Если это обычный запрос
		} else if(!http->client.connect) {
			// Создаем буфер данных
			char * buffer = new char[len];
			// Копируем в буфер полученные данные
			evbuffer_copyout(input, buffer, len);
			// Удаляем данные из буфера
			evbuffer_drain(input, http->parser.parse(buffer, len));
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
	if(http){
		// Коллбек для удаления текущего подключения
		function <void (void)> remove = http->remove;
		// Выполняем заморозку потока
		http->freeze();
		// Создаем буфер событий
		http->events.client = bufferevent_socket_new(http->base, http->sockets.client, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
		// Устанавливаем таймер для клиента
		http->setTimeout(TM_CLIENT, true);
		// Устанавливаем водяной знак на 5 байт (чтобы считывать данные когда они действительно приходят)
		// bufferevent_setwatermark(http->events.client, EV_READ | EV_WRITE, 5, 0);
		// Устанавливаем коллбеки
		bufferevent_setcb(http->events.client, &HttpProxy::read_client_cb, &HttpProxy::write_client_cb, &HttpProxy::event_cb, http);
		// Очищаем буферы событий при завершении работы
		bufferevent_flush(http->events.client, EV_READ | EV_WRITE, BEV_FINISHED);
		// Активируем буферы событий на чтение и запись
		bufferevent_enable(http->events.client, EV_READ | EV_WRITE);
		// Активируем перебор базы событий
		event_base_dispatch(http->base);
		// Удаляем объект подключения
		delete http;
		// Выполняем удаление подключения из списка
		remove();
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
void HttpProxy::accept_cb(const evutil_socket_t fd, const short event, void * ctx){
	// Получаем объект прокси сервера
	HttpProxy * proxy = reinterpret_cast <HttpProxy *> (ctx);
	// Если прокси существует
	if(proxy){
		// IP и MAC адрес подключения
		string ip, mac;
		// Сокет подключившегося клиента
		evutil_socket_t socket = -1;
		// Определяем тип подключения
		switch(proxy->server->config->proxy.intIPv){
			// Для протокола IPv4
			case 4: {
				// Структура получения
				struct sockaddr_in client;
				// Размер структуры подключения
				socklen_t len = sizeof(client);
				// Определяем разрешено ли подключение к прокси серверу
				socket = accept(fd, reinterpret_cast <struct sockaddr *> (&client), &len);
				// Если сокет не создан тогда выходим
				if(socket < 0) return;
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
				if(socket < 0) return;
				// Получаем данные подключившегося клиента
				ip = get_ip(AF_INET6, &client);
				// Получаем данные мак адреса клиента
				mac = get_mac(&client);
			} break;
		}
		// Создаем новое подключение
		proxy->create_client(ip, mac, socket);
	}
}
/**
 * create_server Функция создания прокси сервера
 * @return сокет прокси сервера
 */
const evutil_socket_t HttpProxy::create_server(){
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
	switch(this->server->config->proxy.intIPv){
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
		this->server->log->write(LOG_ERROR, 0, "[-] could not create socket");
		// Выходим
		return -1;
	}
	// Устанавливаем неблокирующий режим для сокета
	socket_nonblocking(sock, this->server->log);
	// Устанавливаем разрешение на повторное использование сокета
	socket_reuseable(sock, this->server->log);
	// Устанавливаем неблокирующий режим
	socket_tcpnodelay(sock, this->server->log);
	// Отключаем сигнал записи в оборванное подключение
	socket_nosigpipe(sock, this->server->log);
	// Выполняем биндинг сокета
	if(::bind(sock, sin, sinlen) < 0){
		// Выводим в консоль информацию
		this->server->log->write(LOG_ERROR, 0, "[-] bind error");
		// Выходим
		return -1;
	}
	// Выполняем чтение сокета
	if(listen(sock, this->server->config->connects.total) < 0){
		// Выводим в консоль информацию
		this->server->log->write(LOG_ERROR, 0, "[-] listen error");
		// Выходим
		return -1;
	}
	// Выходим
	return sock;
}
/**
 * HttpProxy::run_server Метод запуска прокси сервера
 * @param fd  сокет (файловый дескриптор) который слушает прокси сервер
 * @param ctx объект прокси сервера
 */
void HttpProxy::run_server(const evutil_socket_t fd, void * ctx){
	// Получаем объект прокси сервера
	HttpProxy * proxy = reinterpret_cast <HttpProxy *> (ctx);
	// Если объект прокси сервера существует
	if(proxy){
		// Погружаем поток в сон на 500 милисекунд
		this_thread::sleep_for(chrono::milliseconds(500));
		// Выводим в консоль информацию
		proxy->server->log->write(LOG_MESSAGE, 0, "[+] start service: pid = %i, socket = %i", getpid(), fd);
		// Создаем новую базу
		proxy->base = event_base_new();
		// Добавляем событие в базу
		struct event * evnt = event_new(proxy->base, fd, EV_READ | EV_PERSIST, &HttpProxy::accept_cb, proxy);
		// Активируем событие
		event_add(evnt, NULL);
		// Активируем перебор базы событий
		event_base_loop(proxy->base, EVLOOP_NO_EXIT_ON_EMPTY);
		// Отключаем подключение для сокета
		shutdown(fd, SHUT_RDWR);
		// Закрываем сокет
		close(fd);
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
 * @param pids указатель на массив пидов процессов
 * @param fd   сокет (файловый дескриптор) прокси сервера
 * @param cur  текущее значение пида процесса
 * @param max  максимальное значение пидов процессов
 * @param ctx  объект прокси сервера
 */
void HttpProxy::run_works(pid_t * pids, const evutil_socket_t fd, const size_t cur, const size_t max, void * ctx){
	// Получаем объект прокси сервера
	HttpProxy * proxy = reinterpret_cast <HttpProxy *> (ctx);
	// Если массив пидов существует
	if(pids && proxy && (fd > 0) && max){
		// Если не все форки созданы
		if(cur < max){
			// Выполняем форк процесса
			switch(pids[cur] = fork()){
				// Если поток не создан
				case -1: {
					// Выводим в консоль информацию
					proxy->server->log->write(LOG_ERROR, 0, "[-] create fork error");
					// Выходим из приложения
					exit(SIGHUP);
				}
				// Если это дочерний поток значит все нормально, запускаем прокси сервер
				case 0: run_server(fd, proxy); break;
				// Если это родительский процесс
				default: run_works(pids, fd, cur + 1, max, proxy);
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
				proxy->server->log->write(LOG_ERROR, 0, "[-] end service: pid = %i, status = %i", pids[i], WTERMSIG(status));
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
	if(proxy){
		// Запоминаем параметры прокси сервера
		this->server = proxy;
		// Создаем прокси сервер
		evutil_socket_t socket = create_server();
		// Если сокет существует
		if(socket > -1){
			// Очищаем весь кэш данных
			this->server->cache->rmAllCache();
			// Очищаем кэш dns запросов
			this->server->cache->rmAllDomains();
			// Если режим отладки не включен
			if(!this->server->config->proxy.debug){
				// Определяем максимальное количество потоков
				u_int max_works = (
					this->server->config->proxy.workers
					? this->server->config->proxy.workers
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
	if(pids) delete [] pids;
}