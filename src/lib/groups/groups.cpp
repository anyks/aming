/* ГРУППЫ ПОЛЬЗОВАТЕЛЕЙ ПРОКСИ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/

#include "groups/groups.h"

// Устанавливаем область видимости
using namespace std;

/**
 * createDefaultData Метод создания группы с параметрами по умолчанию
 * @param  id   идентификатор групыы
 * @param  name название группы
 * @return      созданная группа
 */
const Groups::Data Groups::createDefaultData(const u_int id, const string name){
	// Создаем блок с данными группы
	Data group;
	// Если входные параметры верные
	if(id && !name.empty()){
		// Заполняем данные группы
		group.id		= id;
		group.name		= name;
		group.options	= this->config->options;
		group.ipv4		= {this->config->ipv4.external, this->config->ipv4.resolver};
		group.ipv6		= {this->config->ipv6.external, this->config->ipv6.resolver};
		// Устанавливаем сжатие по умолчанию из общего конфига
		group.gzip = {
			this->config->gzip.vary,
			this->config->gzip.level,
			this->config->gzip.length,
			this->config->gzip.chunk,
			this->config->gzip.regex,
			this->config->gzip.vhttp,
			this->config->gzip.proxied,
			this->config->gzip.types
		};
		// Устанавливаем параметры прокси-сервера
		group.proxy = {
			this->config->proxy.reverse,
			this->config->proxy.transfer,
			this->config->proxy.forward,
			this->config->proxy.subnet,
			this->config->proxy.pipelining
		};
		// Инициализируем модуль управления заголовками
		group.headers = Headers(this->config, this->log, group.options, group.name);
		// Устанавливаем параметры контроля подключений клиента к серверу
		group.connects = {
			this->config->connects.size,
			this->config->connects.connect
		};
		// Устанавливаем параметры таймаутов
		group.timeouts = {
			this->config->timeouts.read,
			this->config->timeouts.write,
			this->config->timeouts.upgrade
		};
		// Устанавливаем буферы передачи данных
		group.buffers = {
			this->config->buffers.read,
			this->config->buffers.write
		};
		// Устанавливаем параметры постоянного подключения
		group.keepalive = {
			this->config->keepalive.keepcnt,
			this->config->keepalive.keepidle,
			this->config->keepalive.keepintvl
		};
	}
	// Выводим результат
	return group;
}
/**
 * readGroupsFromFile Метод чтения данных групп из операционной системы
 * @return результат операции
 */
const bool Groups::readGroupsFromPam(){
	// Результат работы функции
	bool result = false;
	// Выводим результат
	return result;
}
/**
 * readGroupsFromFile Метод чтения данных групп из LDAP сервера
 * @return результат операции
 */
const bool Groups::readGroupsFromLdap(){
	// Результат работы функции
	bool result = false;
	// Выводим результат
	return result;
}
/**
 * readGroupsFromFile Метод чтения данных групп из файла
 * @return результат операции
 */
const bool Groups::readGroupsFromFile(){
	// Результат работы функции
	bool result = false;
	// Создаем адрес для хранения файла
	const string filename = addToPath(this->config->proxy.dir, "groups.ini");
	// Проверяем на существование адреса
	if(!filename.empty() && isFileExist(filename.c_str())){
		// Инициализируем парсер ini файла
		INI ini(filename);
		// Если во время чтения файла ошибок не возникло
		if(!ini.isError()){
			// Получаем список групп
			auto groups = ini.getParamsInSection("groups");
			// Получаем список пользователей
			auto users = ini.getParamsInSection("users");
			// Переходим по списку групп
			for(auto it = groups.cbegin(); it != groups.cend(); ++it){
				// Создаем блок с данными группы
				Data group = createDefaultData(::atoi(it->key.c_str()), it->value);
				// Переходим по списку пользователей
				for(auto ut = users.cbegin(); ut != users.cend(); ++ut){
					// Если группа соответствует текущей
					if((::isNumber(ut->value)
					&& (u_int(::atoi(ut->value.c_str())) == group.id))
					|| (ut->value.compare(group.name) == 0)){
						// Создаем идентификатор пользователя
						u_int uid = 0;
						// Проверяем является ли название пользователя идентификатором
						if(::isNumber(ut->key)) uid = ::atoi(ut->key.c_str());
						// Если это не идентификатор то запрашиваем идентификатор пользователя
						// else uid = getUidByName(ut->key);
						// Добавляем пользователя в список
						group.users.push_back(uid);
					}
				}
				// Создаем список идентификаторов группы
				group.idnt	= {
					split(ini.getString(group.name + "_idnt", "ip"), "|"),
					split(ini.getString(group.name + "_idnt", "mac"), "|")
				};



				// Добавляем группу в список групп
				this->data.insert(pair <u_int, Data>(group.id, group));
			}
		}
	}
	// Выводим результат
	return result;
}
/**
 * Groups Конструктор
 * @param config конфигурационные данные
 * @param log    объект лога для вывода информации
 */
Groups::Groups(Config * config, LogApp * log){
	// Если конфигурационные данные переданы
	if(config != NULL){
		// Запоминаем данные логов
		this->log = log;
		// Запоминаем конфигурационные данные
		this->config = config;
		// Запоминаем тип поиска групп пользователя
		this->typeSearch = 0;
		// Считываем данные групп из файлов
		readGroupsFromFile();
		// Определяем тип поиска группы
		switch(this->typeSearch){
			// Считываем данные групп из системы
			case 1: readGroupsFromPam(); break;
			// Считываем данные групп из LDAP
			case 2: readGroupsFromLdap(); break;
		}
	}
}
