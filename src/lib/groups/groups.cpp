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
				// Получаем список исходящих адресов
				auto IPv4Ext = split(ini.getString(group.name + "_ipv4", "external"), "|");
				auto IPv6Ext = split(ini.getString(group.name + "_ipv6", "external"), "|");
				// Получаем список резолверов
				auto IPv4Res = split(ini.getString(group.name + "_ipv4", "resolver"), "|");
				auto IPv6Res = split(ini.getString(group.name + "_ipv6", "resolver"), "|");
				// Получаем данные таймаутов
				size_t tRead	= (size_t) getSeconds(ini.getString(group.name + "_timeouts", "read"));
				size_t tWrite	= (size_t) getSeconds(ini.getString(group.name + "_timeouts", "write"));
				size_t tUpgrade	= (size_t) getSeconds(ini.getString(group.name + "_timeouts", "upgrade"));
				// Получаем данные буферов
				long buffIn		= getSizeBuffer(ini.getString(group.name + "_speed", "input"));
				long buffOut	= getSizeBuffer(ini.getString(group.name + "_speed", "output"));
				// Получаем параметры постоянного подключения
				int kPcnt	= (int) ini.getNumber(group.name + "_keepalive", "keepcnt");
				int kPidle	= (int) ini.getNumber(group.name + "_keepalive", "keepidle");
				int kIntvl	= (int) ini.getNumber(group.name + "_keepalive", "keepintvl");
				/* Извлекаем данные GZIP НАЧАЛО */
				string gRegex	= ini.getString(group.name + "_gzip", "regex");
				bool gVary		= ini.getBoolean(group.name + "_gzip", "vary");
				long gLength	= ini.getNumber(group.name + "_gzip", "length");
				size_t gChunk	= getBytes(ini.getString(group.name + "_gzip", "chunk"));
				auto gVhttp		= split(ini.getString(group.name + "_gzip", "vhttp"), "|");
				auto gTypes		= split(ini.getString(group.name + "_gzip", "types"), "|");
				auto gProxied	= split(ini.getString(group.name + "_gzip", "proxied"), "|");
				// Уровень сжатия gzip
				int gLevel = 0x0;
				// Получаем уровень сжатия
				const string gzipLevel = ini.getString(group.name + "_gzip", "level");
				// Если размер указан
				if(!gzipLevel.empty()){
					// Определяем тип сжатия
					if(gzipLevel.compare("default") == 0)		gLevel = Z_DEFAULT_COMPRESSION;
					else if(gzipLevel.compare("best") == 0)		gLevel = Z_BEST_COMPRESSION;
					else if(gzipLevel.compare("speed") == 0)	gLevel = Z_BEST_SPEED;
					else if(gzipLevel.compare("no") == 0)		gLevel = Z_NO_COMPRESSION;
				}
				/* Извлекаем данные GZIP КОНЕЦ */
				// Получаем параметры коннектов
				size_t conSize		= getBytes(ini.getString(group.name + "_connects", "size"));
				u_int conConnect	= (u_int) ini.getUNumber(group.name + "_connects", "connect");
				// Получаем параметры прокси
				bool pSubnet		= ini.getBoolean(group.name + "_proxy", "subnet");
				bool pReverse		= ini.getBoolean(group.name + "_proxy", "reverse");
				bool pForward		= ini.getBoolean(group.name + "_proxy", "forward");
				bool pTransfer		= ini.getBoolean(group.name + "_proxy", "transfer");
				bool pPipelining	= ini.getBoolean(group.name + "_proxy", "pipelining");
				// Формируем параметры
				u_short options = (ini.getBoolean(group.name + "_proxy", "connect") ? OPT_CONNECT : OPT_NULL);
				options = (options | (ini.getBoolean(group.name + "_proxy", "upgrade") ? OPT_UPGRADE : OPT_NULL));
				options = (options | (ini.getBoolean(group.name + "_proxy", "agent") ? OPT_AGENT : OPT_NULL));
				options = (options | (ini.getBoolean(group.name + "_proxy", "deblock") ? OPT_DEBLOCK : OPT_NULL));
				options = (options | (ini.getBoolean(group.name + "_gzip", "transfer") ? OPT_GZIP : OPT_NULL));
				options = (options | (ini.getBoolean(group.name + "_gzip", "response") ? OPT_PGZIP : OPT_NULL));
				options = (options | ((ini.getString(group.name + "_proxy", "skill", "dumb").compare("smart") == 0) ? OPT_SMART : OPT_NULL));
				options = (options | (ini.getBoolean(group.name + "_keepalive", "enabled") ? OPT_KEEPALIVE : OPT_NULL));
				// Перекрываем параметры по умолчанию
				if(options != 0x00) group.options = options;
				// Заполняем параметры ip адресов
				if(!IPv4Ext.empty()) group.ipv4.ip = IPv4Ext;
				if(!IPv6Ext.empty()) group.ipv6.ip = IPv6Ext;
				if(!IPv4Res.empty()) group.ipv4.resolver = IPv4Res;
				if(!IPv6Res.empty()) group.ipv6.resolver = IPv6Res;
				// Заполняем параметры сжатия
				if(gVary) group.gzip.vary = gVary;
				if(gLevel) group.gzip.level = gLevel;
				if(gLength) group.gzip.length = gLength;
				if(gChunk) group.gzip.chunk = gChunk;
				if(!gRegex.empty()) group.gzip.regex = gRegex;
				if(!gVhttp.empty()) group.gzip.vhttp = gVhttp;
				if(!gProxied.empty()) group.gzip.proxied = gProxied;
				if(!gTypes.empty()) group.gzip.types = gTypes;
				// Заполняем параметры прокси
				if(pSubnet) group.proxy.subnet = pSubnet;
				if(pReverse) group.proxy.reverse = pReverse;
				if(pForward) group.proxy.forward = pForward;
				if(pTransfer) group.proxy.transfer = pTransfer;
				if(pPipelining) group.proxy.pipelining = pPipelining;
				// Инициализируем модуль управления заголовками
				if(group.headers.checkAvailable(group.name)){
					// Присваиваем новый файл конфигурации заголовков
					group.headers = Headers(this->config, this->log, group.options, group.name);
				}
				// Присваиваем параметры подключения
				if(conSize)		group.connects.size = conSize;
				if(conConnect)	group.connects.connect = conConnect;
				// Присваиваем параметры таймаутов
				if(tRead)		group.timeouts.read = tRead;
				if(tWrite)		group.timeouts.write = tWrite;
				if(tUpgrade)	group.timeouts.upgrade = tUpgrade;
				// Присваиваем параметры буферов
				if(buffIn)	group.buffers.read = buffIn;
				if(buffOut)	group.buffers.write = buffOut;
				// Присваиваем параметры постоянного подключения
				if(kPcnt)	group.keepalive.keepcnt = kPcnt;
				if(kPidle)	group.keepalive.keepidle = kPidle;
				if(kIntvl)	group.keepalive.keepintvl = kIntvl;
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
