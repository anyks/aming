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
		// Запоминаем название группы
		string groupName = name;
		// Заполняем данные группы
		group.id		= id;
		group.name		= ::toCase(groupName);
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
		group.headers = Headers(this->config, this->log);
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
			// Получаем список пользователей
			auto users = ini.getParamsInSection("users");
			// Получаем список групп
			auto groups = ini.getParamsInSection("groups");
			// Переходим по списку групп
			for(auto it = groups.cbegin(); it != groups.cend(); ++it){
				// Формируем параметры
				u_short options = 0x00;
				// Создаем блок с данными группы
				Data group = createDefaultData(::atoi(it->key.c_str()), it->value);
				// Если список пользователей существует
				if(!users.empty()){
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
							else uid = getUidByName(ut->key);
							// Добавляем пользователя в список
							group.users.push_back(uid);
						}
					}
				}
				// Создаем список идентификаторов группы
				group.idnt	= {
					split(ini.getString(group.name + "_idnt", "ip"), "|"),
					split(ini.getString(group.name + "_idnt", "mac"), "|")
				};
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_proxy", "connect")){
					// Устанавливаем параметры прокси сервера
					options = (options | (ini.getBoolean(group.name + "_proxy", "connect") ? OPT_CONNECT : OPT_NULL));
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_proxy", "upgrade")){
					// Устанавливаем параметры прокси сервера
					options = (options | (ini.getBoolean(group.name + "_proxy", "upgrade") ? OPT_UPGRADE : OPT_NULL));
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_proxy", "agent")){
					// Устанавливаем параметры прокси сервера
					options = (options | (ini.getBoolean(group.name + "_proxy", "agent") ? OPT_AGENT : OPT_NULL));
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_proxy", "deblock")){
					// Устанавливаем параметры прокси сервера
					options = (options | (ini.getBoolean(group.name + "_proxy", "deblock") ? OPT_DEBLOCK : OPT_NULL));
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_gzip", "transfer")){
					// Устанавливаем параметры прокси сервера
					options = (options | (ini.getBoolean(group.name + "_gzip", "transfer") ? OPT_GZIP : OPT_NULL));
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_gzip", "response")){
					// Устанавливаем параметры прокси сервера
					options = (options | (ini.getBoolean(group.name + "_gzip", "response") ? OPT_PGZIP : OPT_NULL));
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_proxy", "skill")){
					// Устанавливаем параметры прокси сервера
					options = (options | ((ini.getString(group.name + "_proxy", "skill", "dumb").compare("smart") == 0) ? OPT_SMART : OPT_NULL));
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_keepalive", "enabled")){
					// Устанавливаем параметры прокси сервера
					options = (options | (ini.getBoolean(group.name + "_keepalive", "enabled") ? OPT_KEEPALIVE : OPT_NULL));
				}
				// Перекрываем параметры по умолчанию
				if(options != 0x00) group.options = options;
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_ipv4", "external")){
					// Устанавливаем список ip адресов
					group.ipv4.ip = split(ini.getString(group.name + "_ipv4", "external"), "|");
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_ipv6", "external")){
					// Устанавливаем список ip адресов
					group.ipv6.ip = split(ini.getString(group.name + "_ipv6", "external"), "|");
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_ipv4", "resolver")){
					// Устанавливаем список резолверов
					group.ipv4.resolver = split(ini.getString(group.name + "_ipv4", "resolver"), "|");
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_ipv6", "resolver")){
					// Устанавливаем список резолверов
					group.ipv6.resolver = split(ini.getString(group.name + "_ipv6", "resolver"), "|");
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_timeouts", "read")){
					// Устанавливаем параметры
					group.timeouts.read = (size_t) getSeconds(ini.getString(group.name + "_timeouts", "read"));
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_timeouts", "write")){
					// Устанавливаем параметры
					group.timeouts.write = (size_t) getSeconds(ini.getString(group.name + "_timeouts", "write"));
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_timeouts", "upgrade")){
					// Устанавливаем параметры
					group.timeouts.upgrade = (size_t) getSeconds(ini.getString(group.name + "_timeouts", "upgrade"));
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_speed", "input")){
					// Устанавливаем параметры
					group.buffers.read = getSizeBuffer(ini.getString(group.name + "_speed", "input"));
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_speed", "output")){
					// Устанавливаем параметры
					group.buffers.write = getSizeBuffer(ini.getString(group.name + "_speed", "output"));
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_keepalive", "keepcnt")){
					// Устанавливаем параметры
					group.keepalive.keepcnt = (int) ini.getNumber(group.name + "_keepalive", "keepcnt");
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_keepalive", "keepidle")){
					// Устанавливаем параметры
					group.keepalive.keepidle = (int) ini.getNumber(group.name + "_keepalive", "keepidle");
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_keepalive", "keepintvl")){
					// Устанавливаем параметры
					group.keepalive.keepintvl = (int) ini.getNumber(group.name + "_keepalive", "keepintvl");
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_connects", "size")){
					// Устанавливаем параметры
					group.connects.size = getBytes(ini.getString(group.name + "_connects", "size"));
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_connects", "connect")){
					// Устанавливаем параметры
					group.connects.connect = (u_int) ini.getUNumber(group.name + "_connects", "connect");
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_proxy", "subnet")){
					// Устанавливаем параметры
					group.proxy.subnet = ini.getBoolean(group.name + "_proxy", "subnet");
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_proxy", "reverse")){
					// Устанавливаем параметры
					group.proxy.reverse = ini.getBoolean(group.name + "_proxy", "reverse");
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_proxy", "forward")){
					// Устанавливаем параметры
					group.proxy.forward = ini.getBoolean(group.name + "_proxy", "forward");
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_proxy", "transfer")){
					// Устанавливаем параметры
					group.proxy.transfer = ini.getBoolean(group.name + "_proxy", "transfer");
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_proxy", "pipelining")){
					// Устанавливаем параметры
					group.proxy.pipelining = ini.getBoolean(group.name + "_proxy", "pipelining");
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_gzip", "regex")){
					// Устанавливаем параметры
					group.gzip.regex = ini.getString(group.name + "_gzip", "regex");
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_gzip", "vary")){
					// Устанавливаем параметры
					group.gzip.vary = ini.getBoolean(group.name + "_gzip", "vary");
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_gzip", "length")){
					// Устанавливаем параметры
					group.gzip.length = ini.getNumber(group.name + "_gzip", "length");
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_gzip", "chunk")){
					// Устанавливаем параметры
					group.gzip.chunk = getBytes(ini.getString(group.name + "_gzip", "chunk"));
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_gzip", "vhttp")){
					// Устанавливаем параметры
					group.gzip.vhttp = split(ini.getString(group.name + "_gzip", "vhttp"), "|");
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_gzip", "types")){
					// Устанавливаем параметры
					group.gzip.types = split(ini.getString(group.name + "_gzip", "types"), "|");
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_gzip", "proxied")){
					// Устанавливаем параметры
					group.gzip.proxied = split(ini.getString(group.name + "_gzip", "proxied"), "|");
				}
				// Выполняем проверку на существование записи в конфигурационном файле
				if(ini.checkParam(group.name + "_gzip", "level")){
					// Уровень сжатия gzip
					u_int level = 0x00;
					// Получаем уровень сжатия
					const string gzipLevel = ini.getString(group.name + "_gzip", "level");
					// Если размер указан
					if(!gzipLevel.empty()){
						// Определяем тип сжатия
						if(gzipLevel.compare("default") == 0)		level = Z_DEFAULT_COMPRESSION;
						else if(gzipLevel.compare("best") == 0)		level = Z_BEST_COMPRESSION;
						else if(gzipLevel.compare("speed") == 0)	level = Z_BEST_SPEED;
						else if(gzipLevel.compare("no") == 0)		level = Z_NO_COMPRESSION;
					}
					if(level != 0x00) group.gzip.level = level;
				}
				// Инициализируем модуль управления заголовками
				if(group.headers.checkAvailable(group.name)){
					// Присваиваем новый файл конфигурации заголовков
					group.headers = Headers(this->config, this->log, group.options, group.name);
				}
				// Добавляем группу в список групп
				this->data.insert(pair <u_int, Data>(group.id, group));
			}
		}
	}
	// Выводим результат
	return result;
}
/**
 * getUidByName Метод извлечения идентификатора пользователя по его имени
 * @param  userName название пользователя
 * @return          идентификатор пользователя
 */
const u_int Groups::getUidByName(const string userName){
	// Результат работы функции
	u_int result = 0;
	// Если идентификатор пользователя передан
	if(!userName.empty()){
		result = 0;
	}
	// Выводим результат
	return result;
}
/**
 * getIdByName Метод извлечения идентификатора группы по ее имени
 * @param  groupName название группы
 * @return           идентификатор группы
 */
const u_int Groups::getIdByName(const string groupName){
	// Результат работы функции
	u_int result = 0;
	// Если идентификатор группы передан
	if(!groupName.empty()){
		// Переходим по всему списку групп
		for(auto it = this->data.cbegin(); it != this->data.cend(); ++it){
			// Запоминаем полученное название группы
			string name = groupName;
			// Сравниваем группы в базе
			if(it->second.name.compare(::toCase(name)) == 0){
				// Запоминаем полученный идентификатор
				result = it->first;
				// Выходим из цикла
				break;
			}
		}
	}
	// Выводим результат
	return result;
}
/**
 * getUserNameByUid Метод извлечения имени пользователя по его идентификатору
 * @param  uid идентификатор пользователя
 * @return     название пользователя
 */
const string Groups::getUserNameByUid(const u_int uid){
	// Результат работы функции
	string result;
	// Выводим результат
	return result;
}
/**
 * getNameById Метод извлечения имени группы по ее идентификатору
 * @param  gid идентификатор группы
 * @return     название группы
 */
const string Groups::getNameById(const u_int gid){
	// Результат работы функции
	string result;
	// Если идентификатор группы передан
	if(gid && this->data.count(gid)){
		// Получаем название группы
		result = this->data.find(gid)->second.name;
	}
	// Выводим результат
	return result;
}
/**
 * getNameUsers Метод получения списка пользователей в группе
 * @param  gid идентификатор группы
 * @return     список имен пользователей
 */
const vector <string> Groups::getNameUsers(const u_int gid){
	// Результат работы функции
	vector <string> result;
	// Если идентификатор группы передан
	if(gid && this->data.count(gid)){
		// Получаем список пользователей
		auto users = this->data.find(gid)->second.users;
		// Переходим по списку пользователей
		for(auto it = users.cbegin(); it != users.cend(); ++it){
			// Получаем имя пользователя
			const string userName = getUserNameByUid(*it);
			// Добавляем имя пользователя в список
			result.push_back(userName);
		}
	}
	// Выводим результат
	return result;
}
/**
 * getNameUsers Метод получения списка пользователей в группе
 * @param  groupName название группы
 * @return           список имен пользователей
 */
const vector <string> Groups::getNameUsers(const string groupName){
	// Результат работы функции
	vector <string> result;
	// Если идентификатор группы передан
	if(!groupName.empty()){
		// Получаем идентификатор группы
		const u_int gid = getIdByName(groupName);
		// Получаем список имен пользователей
		result = getNameUsers(gid);
	}
	// Выводим результат
	return result;
}
/**
 * getIdUsers Метод получения списка пользователей в группе
 * @param  gid идентификатор группы
 * @return     список идентификаторов пользователей
 */
const vector <u_int> Groups::getIdUsers(const u_int gid){
	// Результат работы функции
	vector <u_int> result;
	// Если идентификатор группы передан
	if(gid && this->data.count(gid)){
		// Получаем список пользователей
		result = this->data.find(gid)->second.users;
	}
	// Выводим результат
	return result;
}
/**
 * getIdUsers Метод получения списка пользователей в группе
 * @param  groupName название группы
 * @return           список идентификаторов пользователей
 */
const vector <u_int> Groups::getIdUsers(const string groupName){
	// Результат работы функции
	vector <u_int> result;
	// Если идентификатор группы передан
	if(!groupName.empty()){
		// Получаем идентификатор группы
		const u_int gid = getIdByName(groupName);
		// Получаем список идентификаторов пользователей
		result = getIdUsers(gid);
	}
	// Выводим результат
	return result;
}
/**
 * addUser Метод добавления пользователя
 * @param  gid идентификатор группы
 * @param  uid идентификатор пользователя
 * @return     результат добавления
 */
const bool Groups::addUser(const u_int gid, const u_int uid){
	// Результат работы функции
	bool result = false;
	// Если входящие параметры верные
	if(gid && uid && this->data.count(gid)){
		// Получаем данные группы
		this->data.find(gid)->second.users.push_back(uid);
		// Запоминаем что все удачно
		result = true;
	}
	// Выводим результат
	return result;
}
/**
 * addUser Метод добавления пользователя
 * @param  gid       идентификатор группы
 * @param  userName  название пользователя
 * @return           результат добавления
 */
const bool Groups::addUser(const u_int gid, const string userName){
	// Результат работы функции
	bool result = false;
	// Если входящие параметры верные
	if(gid && !userName.empty()){
		// Получаем идентификатор пользователя
		const u_int uid = getUidByName(userName);
		// Добавляем нового пользователя
		result = addUser(gid, uid);
	}
	// Выводим результат
	return result;
}
/**
 * addUser Метод добавления пользователя
 * @param  groupName название группы
 * @param  uid       идентификатор пользователя
 * @return           результат добавления
 */
const bool Groups::addUser(const string groupName, const u_int uid){
	// Результат работы функции
	bool result = false;
	// Если входящие параметры верные
	if(uid && !groupName.empty()){
		// Получаем идентификатор группы
		const u_int gid = getIdByName(groupName);
		// Добавляем нового пользователя
		result = addUser(gid, uid);
	}
	// Выводим результат
	return result;
}
/**
 * addUser Метод добавления пользователя
 * @param  groupName название группы
 * @param  userName  название пользователя
 * @return           результат добавления
 */
const bool Groups::addUser(const string groupName, const string userName){
	// Результат работы функции
	bool result = false;
	// Если входящие параметры верные
	if(!groupName.empty() && !userName.empty()){
		// Получаем идентификатор группы
		const u_int gid = getIdByName(groupName);
		// Получаем идентификатор пользователя
		const u_int uid = getUidByName(userName);
		// Добавляем нового пользователя
		result = addUser(gid, uid);
	}
	// Выводим результат
	return result;
}
/**
 * addGroup Метод добавления группы
 * @param  id   идентификатор группы
 * @param  name название группы
 * @return      результат добавления
 */
const bool Groups::addGroup(const u_int id, const string name){
	// Результат работы функции
	bool result = false;
	// Если идентификатор и название переданы
	if(id && !name.empty()){
		// Создаем блок с данными группы
		Data group = createDefaultData(id, name);
		// Добавляем группу в список групп
		this->data.insert(pair <u_int, Data>(group.id, group));
		// Выводим сообщение что все удачно
		result = true;
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
