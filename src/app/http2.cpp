/* СЕРВЕР HTTP ПРОКСИ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#include "http2.h"

// Устанавливаем пространство имен
using namespace std;

/**
 * isfull Метод проверки заполненности максимального количества коннектов
 * @return результат проверки
 */
bool ConnectClients::Client::isfull(){
	// Если количество подключений не достигло предела
	if(this->active < this->max) return false;
	// Сообщаем что подключений больше нет
	return true;
}
/**
 * add Метод добавления нового подключения в объект клиента
 * @param client родительский объект клиента
 * @param ctx    передаваемый указатель на объект
 */
void ConnectClients::Client::add(ConnectClients * client, void * ctx){
	// Захватываем поток
	this->mtx.lock();
	// Получаем данные подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	// Если подключение существует
	if(http != NULL){
		// Запоминаем объект подключения
		http->connect = this;
		// Запоминаем кондишин переменную
		http->cond = &this->cond;
		// Запоминаем индекс текущего подключения
		http->index = this->connects.size();
		// Копируем указатель на родителький объект
		this->client = client;
		// Запоминаем ip адрес клиента
		this->id = http->client.ip;
		// Запоминаем максимально-возможное количество подключений
		this->max = http->proxy->config->proxy.maxcon;
		// Создаем новый поток
		std::thread thr(&HttpProxy::connection, http);
		// Добавляем в массив созданный нами поток
		this->connects.push_back(thr);
		// Если количество подключений еще не достигло максимума
		if(!isfull()) this->active++;
		// Выполняем активацию потока
		(* this->connects.end()).detach();
	}
	// Освобождаем поток
	this->mtx.unlock();
}
/**
 * rm Метод удаления подключения из объекта клиента
 * @param index индекс подключения
 */
void ConnectClients::Client::rm(size_t index){
	// Захватываем поток
	this->mtx.lock();
	// Выполняем удаление подключения
	this->connects.erase(this->connects.begin() + index);
	// Уменьшаем количество подключений
	this->active--;
	// Если подключения еще есть то отправляем сигнал
	if(this->active && !isfull())
		// Сообщаем что подключения еще есть
		this->cond.notify_one();
	// Если это было последнее подключение то удаляем объект клиента
	else if(!this->active){
		// Освобождаем поток
		this->mtx.unlock();
		// Выполняем удаление
		this->client->rm(this->id);
		// Выходим
		return;
	}
	// Освобождаем поток
	this->mtx.unlock();
}
/**
 * add Метод добавления нового подключения в объект пользователя
 * @param id  идентификатор клиента
 * @param ctx передаваемый указатель на объект
 */
void ConnectClients::add(void * ctx){
	this->mtx.lock();
	// Получаем данные подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	// Если подключение существует
	if(http != NULL){
		// Если клиент не найден
		if(this->clients.count(http->client.ip) < 1){
			// Создаем новый объект клиента
			unique_ptr <Client> client(new Client);
			// Добавляем в список нового клиента
			this->clients.insert(pair <string, unique_ptr <Client>> (http->client.ip, move(client)));
		}
		// Получаем данные клиента
		Client * client = (this->clients.find(http->client.ip)).get();
		// Передаем клиенту данные объекта
		client.add(this, http);
	}
	// Освобождаем поток
	this->mtx.unlock();
}
/**
 * rm Метод удаления объекта подключившихся клиентов
 * @param id идентификатор клиента
 */
void ConnectClients::rm(const string id){
	// Захватываем поток
	this->mtx.lock();
	// Если такой клиент найден
	if(this->clients.count(id) > 0){
		// Получаем итератор
		auto it = this->clients.find(id);
		// Если клиент найден то удаляем его
		this->clients.erase(it);
	}
	// Освобождаем поток
	this->mtx.unlock();
}


/**
 * close Метод закрытия подключения
 */
void BufferHttpProxy::close(){
	// Захватываем поток
	this->mtx.lock();
	// Закрываем подключение клиента
	close_client();
	// Закрываем подключение сервера
	close_server();
	// Удаляем базу событий
	event_base_loopexit(this->base, NULL);
	// Освобождаем поток
	this->mtx.unlock();
}

/**
 * ~BufferHttpProxy Деструктор
 */
BufferHttpProxy::~BufferHttpProxy(){
	// Закрываем подключение
	close();
	// Захватываем поток
	this->mtx.lock();
	// Удаляем dns сервер
	delete this->dns;
	// Очищаем объект базы событий
	event_base_free(this->base);
	// Получаем данные подключения
	ConnectClients::Client * connect = reinterpret_cast <ConnectClients::Client *> (this->connect);
	// Освобождаем поток
	this->mtx.unlock();
	// Удаляем себя из списока подключений
	if(connect != NULL) connect->rm(this->index);
}

/**
 * freeze Метод заморозки потока
 */
void BufferHttpProxy::freeze(){
	// Получаем данные подключения
	ConnectClients::Client * connect = reinterpret_cast <ConnectClients::Client *> (this->connect);
	// Если подключение найдено
	if(connect != NULL){
		// Если все коннекты исчерпаны
		if(connect->isfull()){
			// Лочим мютекс
			unique_lock <mutex> locker(this->mutx);
			// Блокируем поток
			(* this->cond).wait(locker);
		}
	}
}



/**
 * create_client Метод создания нового клиента
 * @param ip  адрес интернет протокола клиента
 * @param mac аппаратный адрес сетевого интерфейса клиента
 * @param fd  файловый дескриптор (сокет) подключившегося клиента
 */
void HttpProxy::create_client(const string ip, const string mac, evutil_socket_t fd){
	// Захватываем поток
	this->mtx.lock();
	// Устанавливаем неблокирующий режим для сокета
	socket_nonblocking(fd, this->server->log);
	// Устанавливаем разрешение на повторное использование сокета
	socket_reuseable(fd, this->server->log);
	// Отключаем сигнал записи в оборванное подключение
	socket_nosigpipe(fd, this->server->log);
	// Выводим в лог сообщение
	this->server->log->write(LOG_ACCESS, "client connect to proxy server, host = %s, mac = %s, socket = %d", ip.c_str(), mac.c_str(), fd);
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
	// Освобождаем поток
	this->mtx.unlock();
}