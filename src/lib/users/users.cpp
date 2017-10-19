/* ПОЛЬЗОВАТЕЛИ ПРОКСИ AMING */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/

#include "ausers/ausers.h"

// Устанавливаем область видимости
using namespace std;

/**
 * setProxyOptions Функция добавления опций прокси
 * @param option       опция для добавления
 * @param proxyOptions список существующих опций
 * @param flag         флаг добавления или удаления опции
 */
void AUsers::Users::setProxyOptions(const u_short option, u_short &proxyOptions, const bool flag){
	// Формируем параметры
	u_short options = proxyOptions;
	// Устанавливаем параметры прокси сервера
	if(flag) options = options | option;
	// Если нужно убрать настройку
	else {
		// Убираем настройку
		options = options ^ option;
		// Если параметры больше стали чем были значит ошибка
		if(options > proxyOptions) options = proxyOptions;
	}
	// Устанавливаем новые параметры
	proxyOptions = options;
}
/**
 * setDataUserFromLdap Метод заполнения данных пользователя из LDAP
 * @param user объект пользователя
 */
void AUsers::Users::setDataUserFromLdap(AUsers::DataUser &user){
	// Параметр для поиска
	const char * key = "%u";
	// Выполняем поиск ключа
	size_t pos = this->ldap.filterConfig.find(key);
	// Если ключ найден
	if(pos != string::npos){
		// Создаем объект подключения LDAP
		ALDAP ldap(this->config, this->log);
		// Формируем наш фильтр
		string filter = this->ldap.filterConfig;
		// Заменяем ключ на логин пользователя
		filter = filter.replace(pos, strlen(key), to_string(user.id));
		// Создаем dn
		const string dn = (string("ac=") + this->config->proxy.name + string(",") + this->ldap.dnConfig);
		// Формируем параметры запроса
		const string params =	"amingConfigsConnectsConnect,amingConfigsConnectsSize,amingConfigsGzipChunk,"
								"amingConfigsGzipLength,amingConfigsGzipLevel,amingConfigsGzipProxied,"
								"amingConfigsGzipRegex,amingConfigsGzipResponse,amingConfigsGzipTransfer,"
								"amingConfigsGzipTypes,amingConfigsGzipVary,amingConfigsGzipVhttp,amingConfigsAuth,"
								"amingConfigsIdnt,amingConfigsIpExternal4,amingConfigsIpExternal6,"
								"amingConfigsIpResolver4,amingConfigsIpResolver6,amingConfigsKeepAliveCnt,"
								"amingConfigsKeepAliveEnabled,amingConfigsKeepAliveIdle,amingConfigsKeepAliveIntvl,"
								"amingConfigsProxyAgent,amingConfigsProxyConnect,amingConfigsProxyDeblock,"
								"amingConfigsProxyForward,amingConfigsProxyPipelining,amingConfigsProxyReverse,"
								"amingConfigsProxySkill,amingConfigsProxySubnet,amingConfigsProxyTransfer,"
								"amingConfigsProxyUpgrade,amingConfigsSpeedInput,amingConfigsSpeedOutput,"
								"amingConfigsTimeoutsRead,amingConfigsTimeoutsUpgrade,amingConfigsTimeoutsWrite";
		// Запрашиваем данные параметров
		auto users = ldap.data(dn, params, this->ldap.scopeConfig, filter);
		// Если параметры получены
		if(!users.empty()){
			/**
			* getBoolean Метод получения булевых данных из строки
			* @param value строковое значение булевых данных
			* @return      булевое значение
			*/
			auto getBoolean = [](const string value){
				// Выполняем проверку на доступность опции
				bool check = false;
				// Получаем значение параметра
				string param = value;
				// Приводим к нижнему регистру
				param = Anyks::toCase(param);
				// Определяем тип параметра
				if(param.compare("true") == 0) check = true;
				// Выводим результат
				return check;
			};
			// Переходим по всему объекту параметров
			for(auto it = users.cbegin(); it != users.cend(); ++it){
				// Переходим по всему массиву полученных объектов
				for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
					// Если список значений существует
					if(!dt->second.empty()){
						// Если это идентификатор
						if(dt->first.compare("amingConfigsIdnt") == 0) user.idnt = dt->second;
						// Если это External IPv4
						else if(dt->first.compare("amingConfigsIpExternal4") == 0) user.ipv4.ip = dt->second;
						// Если это External IPv6
						else if(dt->first.compare("amingConfigsIpExternal6") == 0) user.ipv6.ip = dt->second;
						// Если это Resolver IPv4
						else if(dt->first.compare("amingConfigsIpResolver4") == 0) user.ipv4.resolver = dt->second;
						// Если это Resolver IPv6
						else if(dt->first.compare("amingConfigsIpResolver6") == 0) user.ipv6.resolver = dt->second;
						// Если это тип авторизации клиента
						else if(dt->first.compare("amingConfigsAuth") == 0) user.auth = Anyks::toCase(dt->second[0]);
						// Если это параметры прокси Connect
						else if(dt->first.compare("amingConfigsProxyConnect") == 0){
							// Устанавливаем или убираем опцию
							setProxyOptions(OPT_CONNECT, user.options, getBoolean(dt->second[0]));
						// Если это параметры прокси Upgrade
						} else if(dt->first.compare("amingConfigsProxyUpgrade") == 0){
							// Устанавливаем или убираем опцию
							setProxyOptions(OPT_UPGRADE, user.options, getBoolean(dt->second[0]));
						// Если это параметры прокси Agent
						} else if(dt->first.compare("amingConfigsProxyAgent") == 0){
							// Устанавливаем или убираем опцию
							setProxyOptions(OPT_AGENT, user.options, getBoolean(dt->second[0]));
						// Если это параметры прокси Deblock
						} else if(dt->first.compare("amingConfigsProxyDeblock") == 0){
							// Устанавливаем или убираем опцию
							setProxyOptions(OPT_DEBLOCK, user.options, getBoolean(dt->second[0]));
						// Если это параметры gzip Transfer
						} else if(dt->first.compare("amingConfigsGzipTransfer") == 0){
							// Устанавливаем или убираем опцию
							setProxyOptions(OPT_GZIP, user.options, getBoolean(dt->second[0]));
						// Если это параметры gzip Response
						} else if(dt->first.compare("amingConfigsGzipResponse") == 0){
							// Устанавливаем или убираем опцию
							setProxyOptions(OPT_PGZIP, user.options, getBoolean(dt->second[0]));
						// Если это параметры keepalive Enabled
						} else if(dt->first.compare("amingConfigsKeepAliveEnabled") == 0){
							// Устанавливаем или убираем опцию
							setProxyOptions(OPT_KEEPALIVE, user.options, getBoolean(dt->second[0]));
						// Если это параметры прокси Skill
						} else if(dt->first.compare("amingConfigsProxySkill") == 0){
							// Выполняем проверку на доступность опции
							const bool check = (dt->second[0].compare("smart") == 0);
							// Устанавливаем или убираем опцию
							setProxyOptions(OPT_SMART, user.options, check);
						// Если это параметры таймауты read
						} else if(dt->first.compare("amingConfigsTimeoutsRead") == 0){
							// Устанавливаем параметры
							user.timeouts.read = (size_t) Anyks::getSeconds(dt->second[0]);
						// Если это параметры таймауты write
						} else if(dt->first.compare("amingConfigsTimeoutsWrite") == 0){
							// Устанавливаем параметры
							user.timeouts.write = (size_t) Anyks::getSeconds(dt->second[0]);
						// Если это параметры таймауты upgrade
						} else if(dt->first.compare("amingConfigsTimeoutsUpgrade") == 0){
							// Устанавливаем параметры
							user.timeouts.upgrade = (size_t) Anyks::getSeconds(dt->second[0]);
						// Если это параметры speed input
						} else if(dt->first.compare("amingConfigsSpeedInput") == 0){
							// Устанавливаем параметры
							user.buffers.read = Anyks::getSizeBuffer(dt->second[0]);
						// Если это параметры speed output
						} else if(dt->first.compare("amingConfigsSpeedOutput") == 0){
							// Устанавливаем параметры
							user.buffers.write = Anyks::getSizeBuffer(dt->second[0]);
						// Если это параметры keepalive keepcnt
						} else if(dt->first.compare("amingConfigsKeepAliveCnt") == 0){
							// Устанавливаем параметры
							user.keepalive.keepcnt = (int) ::atoi(dt->second[0].c_str());
						// Если это параметры keepalive keepidle
						} else if(dt->first.compare("amingConfigsKeepAliveIdle") == 0){
							// Устанавливаем параметры
							user.keepalive.keepidle = (int) ::atoi(dt->second[0].c_str());
						// Если это параметры keepalive keepintvl
						} else if(dt->first.compare("amingConfigsKeepAliveIntvl") == 0){
							// Устанавливаем параметры
							user.keepalive.keepintvl = (int) ::atoi(dt->second[0].c_str());
						// Если это параметры connects size
						} else if(dt->first.compare("amingConfigsConnectsSize") == 0){
							// Устанавливаем параметры
							user.connects.size = Anyks::getBytes(dt->second[0]);
						// Если это параметры connects connect
						} else if(dt->first.compare("amingConfigsConnectsConnect") == 0){
							// Устанавливаем параметры
							user.connects.connect = (u_int) ::atoi(dt->second[0].c_str());
						// Если это параметры proxy subnet
						} else if(dt->first.compare("amingConfigsProxySubnet") == 0){
							// Устанавливаем параметры
							user.proxy.subnet = getBoolean(dt->second[0]);
						// Если это параметры proxy reverse
						} else if(dt->first.compare("amingConfigsProxyReverse") == 0){
							// Устанавливаем параметры
							user.proxy.reverse = getBoolean(dt->second[0]);
						// Если это параметры proxy forward
						} else if(dt->first.compare("amingConfigsProxyForward") == 0){
							// Устанавливаем параметры
							user.proxy.forward = getBoolean(dt->second[0]);
						// Если это параметры proxy transfer
						} else if(dt->first.compare("amingConfigsProxyTransfer") == 0){
							// Устанавливаем параметры
							user.proxy.transfer = getBoolean(dt->second[0]);
						// Если это параметры proxy pipelining
						} else if(dt->first.compare("amingConfigsProxyPipelining") == 0){
							// Устанавливаем параметры
							user.proxy.pipelining = getBoolean(dt->second[0]);
						// Если это параметры gzip regex
						} else if(dt->first.compare("amingConfigsGzipRegex") == 0){
							// Устанавливаем параметры
							user.gzip.regex = dt->second[0];
						// Если это параметры gzip vary
						} else if(dt->first.compare("amingConfigsGzipVary") == 0){
							// Устанавливаем параметры
							user.gzip.vary = getBoolean(dt->second[0]);
						// Если это параметры gzip length
						} else if(dt->first.compare("amingConfigsGzipLength") == 0){
							// Устанавливаем параметры
							user.gzip.length = (int) ::atoi(dt->second[0].c_str());
						// Если это параметры gzip chunk
						} else if(dt->first.compare("amingConfigsGzipChunk") == 0){
							// Устанавливаем параметры
							user.gzip.chunk = Anyks::getBytes(dt->second[0]);
						// Если это параметры gzip vhttp
						} else if(dt->first.compare("amingConfigsGzipVhttp") == 0){
							// Устанавливаем параметры
							user.gzip.vhttp = dt->second;
						// Если это параметры gzip types
						} else if(dt->first.compare("amingConfigsGzipTypes") == 0){
							// Устанавливаем параметры
							user.gzip.types = dt->second;
						// Если это параметры gzip proxied
						} else if(dt->first.compare("amingConfigsGzipProxied") == 0){
							// Устанавливаем параметры
							user.gzip.proxied = dt->second;
						// Если это параметры gzip level
						} else if(dt->first.compare("amingConfigsGzipLevel") == 0){
							// Уровень сжатия gzip
							u_int level = OPT_NULL;
							// Получаем уровень сжатия
							const string gzipLevel = dt->second[0];
							// Если размер указан
							if(!gzipLevel.empty()){
								// Определяем тип сжатия
								if(gzipLevel.compare("default") == 0)		level = Z_DEFAULT_COMPRESSION;
								else if(gzipLevel.compare("best") == 0)		level = Z_BEST_COMPRESSION;
								else if(gzipLevel.compare("speed") == 0)	level = Z_BEST_SPEED;
								else if(gzipLevel.compare("no") == 0)		level = Z_NO_COMPRESSION;
							}
							if(level != OPT_NULL) user.gzip.level = level;
						}
					}
				}
			}
		}
	}
}
/**
 * setDataUserFromFile Метод заполнения данных пользователя из конфигурационного файла
 * @param user объект пользователя
 * @param ini  указатель на объект конфигурации
 */
void AUsers::Users::setDataUserFromFile(AUsers::DataUser &user, INI * ini){
	// Флаг удаления созданного объекта ini конфигурации
	bool rmINI = false;
	// Если объект ini не передан то создаем его
	if(!ini){
		// Создаем адрес для хранения файла
		const string filename = Anyks::addToPath(this->config->proxy.dir, "users.ini");
		// Проверяем на существование адреса
		if(!filename.empty() && Anyks::isFileExist(filename.c_str())){
			// Инициализируем парсер ini файла
			ini = new INI(filename);
			// Запоминаем что нужно удалить объект конфигурации
			rmINI = true;
		// Если объект конфигурации не существует тогда выходим
		} else return;
	}
	// Создаем список идентификаторов пользователя
	user.idnt = Anyks::split(ini->getString("identificators", user.name), "|");
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_proxy", "connect")){
		// Выполняем проверку на доступность опции
		const bool check = ini->getBoolean(user.name + "_proxy", "connect");
		// Устанавливаем или убираем опцию
		setProxyOptions(OPT_CONNECT, user.options, check);
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_proxy", "upgrade")){
		// Выполняем проверку на доступность опции
		const bool check = ini->getBoolean(user.name + "_proxy", "upgrade");
		// Устанавливаем или убираем опцию
		setProxyOptions(OPT_UPGRADE, user.options, check);
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_proxy", "agent")){
		// Выполняем проверку на доступность опции
		const bool check = ini->getBoolean(user.name + "_proxy", "agent");
		// Устанавливаем или убираем опцию
		setProxyOptions(OPT_AGENT, user.options, check);
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_proxy", "deblock")){
		// Выполняем проверку на доступность опции
		const bool check = ini->getBoolean(user.name + "_proxy", "deblock");
		// Устанавливаем или убираем опцию
		setProxyOptions(OPT_DEBLOCK, user.options, check);
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_gzip", "transfer")){
		// Выполняем проверку на доступность опции
		const bool check = ini->getBoolean(user.name + "_gzip", "transfer");
		// Устанавливаем или убираем опцию
		setProxyOptions(OPT_GZIP, user.options, check);
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_gzip", "response")){
		// Выполняем проверку на доступность опции
		const bool check = ini->getBoolean(user.name + "_gzip", "response");
		// Устанавливаем или убираем опцию
		setProxyOptions(OPT_PGZIP, user.options, check);
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_proxy", "skill")){
		// Выполняем проверку на доступность опции
		const bool check = (ini->getString(user.name + "_proxy", "skill", "dumb").compare("smart") == 0);
		// Устанавливаем или убираем опцию
		setProxyOptions(OPT_SMART, user.options, check);
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_keepalive", "enabled")){
		// Выполняем проверку на доступность опции
		const bool check = ini->getBoolean(user.name + "_keepalive", "enabled");
		// Устанавливаем или убираем опцию
		setProxyOptions(OPT_KEEPALIVE, user.options, check);
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_ipv4", "external")){
		// Устанавливаем список ip адресов
		user.ipv4.ip = Anyks::split(ini->getString(user.name + "_ipv4", "external"), "|");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_ipv6", "external")){
		// Устанавливаем список ip адресов
		user.ipv6.ip = Anyks::split(ini->getString(user.name + "_ipv6", "external"), "|");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_ipv4", "resolver")){
		// Устанавливаем список резолверов
		user.ipv4.resolver = Anyks::split(ini->getString(user.name + "_ipv4", "resolver"), "|");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_ipv6", "resolver")){
		// Устанавливаем список резолверов
		user.ipv6.resolver = Anyks::split(ini->getString(user.name + "_ipv6", "resolver"), "|");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_timeouts", "read")){
		// Устанавливаем параметры
		user.timeouts.read = (size_t) Anyks::getSeconds(ini->getString(user.name + "_timeouts", "read"));
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_timeouts", "write")){
		// Устанавливаем параметры
		user.timeouts.write = (size_t) Anyks::getSeconds(ini->getString(user.name + "_timeouts", "write"));
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_timeouts", "upgrade")){
		// Устанавливаем параметры
		user.timeouts.upgrade = (size_t) Anyks::getSeconds(ini->getString(user.name + "_timeouts", "upgrade"));
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_speed", "input")){
		// Устанавливаем параметры
		user.buffers.read = Anyks::getSizeBuffer(ini->getString(user.name + "_speed", "input"));
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_speed", "output")){
		// Устанавливаем параметры
		user.buffers.write = Anyks::getSizeBuffer(ini->getString(user.name + "_speed", "output"));
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_keepalive", "keepcnt")){
		// Устанавливаем параметры
		user.keepalive.keepcnt = (int) ini->getNumber(user.name + "_keepalive", "keepcnt");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_keepalive", "keepidle")){
		// Устанавливаем параметры
		user.keepalive.keepidle = (int) ini->getNumber(user.name + "_keepalive", "keepidle");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_keepalive", "keepintvl")){
		// Устанавливаем параметры
		user.keepalive.keepintvl = (int) ini->getNumber(user.name + "_keepalive", "keepintvl");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_connects", "size")){
		// Устанавливаем параметры
		user.connects.size = Anyks::getBytes(ini->getString(user.name + "_connects", "size"));
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_connects", "connect")){
		// Устанавливаем параметры
		user.connects.connect = (u_int) ini->getUNumber(user.name + "_connects", "connect");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_proxy", "subnet")){
		// Устанавливаем параметры
		user.proxy.subnet = ini->getBoolean(user.name + "_proxy", "subnet");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_proxy", "reverse")){
		// Устанавливаем параметры
		user.proxy.reverse = ini->getBoolean(user.name + "_proxy", "reverse");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_proxy", "forward")){
		// Устанавливаем параметры
		user.proxy.forward = ini->getBoolean(user.name + "_proxy", "forward");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_proxy", "transfer")){
		// Устанавливаем параметры
		user.proxy.transfer = ini->getBoolean(user.name + "_proxy", "transfer");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_proxy", "pipelining")){
		// Устанавливаем параметры
		user.proxy.pipelining = ini->getBoolean(user.name + "_proxy", "pipelining");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_gzip", "regex")){
		// Устанавливаем параметры
		user.gzip.regex = ini->getString(user.name + "_gzip", "regex");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_gzip", "vary")){
		// Устанавливаем параметры
		user.gzip.vary = ini->getBoolean(user.name + "_gzip", "vary");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_gzip", "length")){
		// Устанавливаем параметры
		user.gzip.length = ini->getNumber(user.name + "_gzip", "length");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_gzip", "chunk")){
		// Устанавливаем параметры
		user.gzip.chunk = Anyks::getBytes(ini->getString(user.name + "_gzip", "chunk"));
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_gzip", "vhttp")){
		// Устанавливаем параметры
		user.gzip.vhttp = Anyks::split(ini->getString(user.name + "_gzip", "vhttp"), "|");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_gzip", "types")){
		// Устанавливаем параметры
		user.gzip.types = Anyks::split(ini->getString(user.name + "_gzip", "types"), "|");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_gzip", "proxied")){
		// Устанавливаем параметры
		user.gzip.proxied = Anyks::split(ini->getString(user.name + "_gzip", "proxied"), "|");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(user.name + "_gzip", "level")){
		// Уровень сжатия gzip
		u_int level = OPT_NULL;
		// Получаем уровень сжатия
		const string gzipLevel = ini->getString(user.name + "_gzip", "level");
		// Если размер указан
		if(!gzipLevel.empty()){
			// Определяем тип сжатия
			if(gzipLevel.compare("default") == 0)		level = Z_DEFAULT_COMPRESSION;
			else if(gzipLevel.compare("best") == 0)		level = Z_BEST_COMPRESSION;
			else if(gzipLevel.compare("speed") == 0)	level = Z_BEST_SPEED;
			else if(gzipLevel.compare("no") == 0)		level = Z_NO_COMPRESSION;
		}
		if(level != OPT_NULL) user.gzip.level = level;
	}
	// Удаляем объект конфигурации если это требуется
	if(rmINI) delete ini;
}
/**
 * setDataUser Метод заполнения данных пользователей
 * @param user объект пользователя
 * @param ini  указатель на объект конфигурации
 */
 void AUsers::Users::setDataUser(AUsers::DataUser &user, INI * ini){
	// Определяем тип системы откуда нужно получить конфигурационные файлы
	switch(this->typeConfigs){
		// Переопределяем дефолтные данные из файла конфигурации
		case 0: setDataUserFromFile(user, ini); break;
		// Переопределяем дефолтные данные из LDAP
		case 1: setDataUserFromLdap(user); break;
	}
}
/**
 * createDefaultData Метод создания пользователя с параметрами по умолчанию
 * @param  id   идентификатор пользователя
 * @param  name название пользователя
 * @return      созданный пользователь
 */
 const AUsers::DataUser AUsers::Users::createDefaultData(const uid_t id, const string name){
	// Создаем блок с данными пользователя
	DataUser user;
	// Если входные параметры верные
	if(id && !name.empty() && (this->groups != nullptr)){
		// Получаем объект групп
		Groups * groups = reinterpret_cast <Groups *> (this->groups);
		// Получаем список групп которым принадлежит пользователь
		auto gids = groups->getGroupIdByUser(id);
		// Если список получен
		if(!gids.empty()){
			// Запоминаем название пользователя
			string userName = name;
			// Заполняем данные пользователя
			user.id		= id;
			user.name	= Anyks::toCase(userName);
			// Идентификатор группы
			gid_t gid = 1000000;
			// Переходим по всему списку групп
			for(auto gt = gids.cbegin(); gt != gids.cend(); ++gt){
				// Чем ниже идентификатор группы тем выше приоритет группы
				if((* gt) < gid){
					// Запоминаем идентификатор группы
					gid = (* gt);
					// Получаем данные группы
					auto group = groups->getDataById(gid);
					// Запоминаем тип авторизации
					user.auth 		= group->auth;
					user.options	= group->options;
					user.ipv4		= {group->ipv4.ip, group->ipv4.resolver};
					user.ipv6		= {group->ipv6.ip, group->ipv6.resolver};
					// Устанавливаем сжатие по умолчанию из общего конфига
					user.gzip = {
						group->gzip.vary,
						group->gzip.level,
						group->gzip.length,
						group->gzip.chunk,
						group->gzip.regex,
						group->gzip.vhttp,
						group->gzip.proxied,
						group->gzip.types
					};
					// Устанавливаем параметры прокси-сервера
					user.proxy = {
						group->proxy.reverse,
						group->proxy.transfer,
						group->proxy.forward,
						group->proxy.subnet,
						group->proxy.pipelining
					};
					// Инициализируем модуль управления заголовками
					user.headers = group->headers;
					// Устанавливаем параметры контроля подключений клиента к серверу
					user.connects = {
						group->connects.size,
						group->connects.connect
					};
					// Устанавливаем параметры таймаутов
					user.timeouts = {
						group->timeouts.read,
						group->timeouts.write,
						group->timeouts.upgrade
					};
					// Устанавливаем буферы передачи данных
					user.buffers = {
						group->buffers.read,
						group->buffers.write
					};
					// Устанавливаем параметры постоянного подключения
					user.keepalive = {
						group->keepalive.keepcnt,
						group->keepalive.keepidle,
						group->keepalive.keepintvl
					};
				}
			}
		}
	}
	// Выводим результат
	return user;
}
/**
 * readUsersFromLdap Метод чтения данных пользователей из LDAP сервера
 * @return результат операции
 */
const bool AUsers::Users::readUsersFromLdap(){
	// Результат работы функции
	bool result = false;
	// Создаем объект подключения LDAP
	ALDAP ldap(this->config, this->log);
	// Запрашиваем данные пользователей
	auto users = ldap.data(this->ldap.dnUser, "uid,sn,cn,givenName,initials,userPassword,uidNumber", this->ldap.scopeUser, this->ldap.filterUser);
	// Если пользователи получены
	if(!users.empty()){
		// Переходим по всему объекту пользователей
		for(auto it = users.cbegin(); it != users.cend(); ++it){
			// Идентификатор пользователя
			uid_t uid;
			// Название пользователя
			string name;
			// Имя пользователя
			string firstName;
			// Фамилия пользователя
			string lastName;
			// Отчество пользователя
			string secondName;
			// Описание пользователя
			string description;
			// Пароль пользователя
			string password;
			// Переходим по всему массиву полученных объектов
			for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
				// Если список значений существует
				if(!dt->second.empty()){
					// Если это название пользователя
					if(dt->first.compare("uid") == 0) name = dt->second[0];
					// Если это идентификатор пользователя
					else if(dt->first.compare("uidNumber") == 0) uid = ::atoi(dt->second[0].c_str());
					// Если это описание пользователя
					else if(dt->first.compare("cn") == 0) description = dt->second[0];
					// Если это пароль пользователя
					else if(dt->first.compare("userPassword") == 0) password = dt->second[0];
					// Если это имя пользователя
					else if(dt->first.compare("givenName") == 0) firstName = dt->second[0];
					// Если это фамилия пользователя
					else if(dt->first.compare("sn") == 0) lastName = dt->second[0];
					// Если это отчество пользователя
					else if(dt->first.compare("initials") == 0) secondName = dt->second[0];
				}
			}
			// Создаем блок с данными пользователя
			DataUser user = createDefaultData(uid, name);
			// Устанавливаем тип пользователя
			user.type = 2;
			// Добавляем пароль пользователя
			user.pass = password;
			// Добавляем описание пользователя
			user.desc = (!firstName.empty() ? (!lastName.empty() ? (!secondName.empty() ? firstName + string(" ") + lastName + string(" ") + secondName : firstName + string(" ") + lastName) : firstName) : !description.empty() ? description : name);
			// Переопределяем дефолтные данные из файла конфигурации
			setDataUser(user);
			// Устанавливаем параметры http парсера
			user.headers.setOptions(user.options);
			// Добавляем пользователя в список
			this->data.insert(pair <uid_t, DataUser>(user.id, user));
		}
		// Сообщаем что все удачно
		result = true;
	}
	// Выводим результат
	return result;
}
/**
 * readUsersFromPam Метод чтения данных пользователей из операционной системы
 * @return результат операции
 */
const bool AUsers::Users::readUsersFromPam(){
	// Результат работы функции
	bool result = false;
	// Блок данных пользователей
	struct passwd * pw = nullptr;
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения для проверки оболочки пользователя
	regex e("\\/(?:sh|bash)$", regex::ECMAScript | regex::icase);
	// Извлекаем всех пользователей что есть в системе
	while((pw = getpwent()) != nullptr){
		// Если это не root
		if(pw->pw_uid > 0){
			// Получаем оболочку пользователя
			string shell = pw->pw_shell;
			// Выполняем проверку оболочки пользователя
			regex_search(shell, match, e);
			// Если оболочка пользователя актуальная
			if(!match.empty()){
				// Создаем блок с данными пользователя
				DataUser user = createDefaultData(pw->pw_uid, pw->pw_name);
				// Устанавливаем тип пользователя
				user.type = 1;
				// Добавляем пароль пользователя
				user.pass = pw->pw_passwd;
				// Добавляем описание пользователя
				user.desc = pw->pw_name;
				// Переопределяем дефолтные данные из файла конфигурации
				setDataUser(user);
				// Устанавливаем параметры http парсера
				user.headers.setOptions(user.options);
				// Добавляем пользователя в список
				this->data.insert(pair <uid_t, DataUser>(user.id, user));
				// Сообщаем что все удачно
				result = true;
			}
		}
	}
	// Выводим результат
	return result;
}
/**
 * readUsersFromFile Метод чтения данных пользователей из файла
 * @return результат операции
 */
const bool AUsers::Users::readUsersFromFile(){
	// Результат работы функции
	bool result = false;
	// Создаем адрес для хранения файла
	const string filename = Anyks::addToPath(this->config->proxy.dir, "users.ini");
	// Проверяем на существование адреса
	if(!filename.empty() && Anyks::isFileExist(filename.c_str())){
		// Инициализируем парсер ini файла
		INI ini(filename);
		// Если во время чтения файла ошибок не возникло
		if(!ini.isError()){
			// Пароли хранятся для всех пользователей в одном файле
			string filepass;
			// Получаем список пользователей
			auto users = ini.getParamsInSection("users");
			// Получаем список паролей
			auto passwords = ini.getParamsInSection("passwords");
			// Получаем список описаний
			auto descriptions = ini.getParamsInSection("descriptions");
			// Если список пользователей существует
			if(!users.empty()){
				// Переходим по списку пользователей
				for(auto ut = users.cbegin(); ut != users.cend(); ++ut){
					// Если идентификатор пользователя существует
					if(Anyks::isNumber(ut->key) || Anyks::isNumber(ut->value)){
						// Получаем идентификатор пользователя
						const uid_t uid = (Anyks::isNumber(ut->key) ? ::atoi(ut->key.c_str()) : ::atoi(ut->value.c_str()));
						// Получаем название пользователя
						const string name = (Anyks::isNumber(ut->key) ? ut->value : ut->key);
						// Создаем блок с данными пользователя
						DataUser user = createDefaultData(uid, name);
						// Устанавливаем тип пользователя
						user.type = 0;
						// Если пароли пользователей существуют
						if(!passwords.empty()){
							// Переходим по списку паролей
							for(auto up = passwords.cbegin(); up != passwords.cend(); ++up){
								// Создаем ключ пользователя
								const string key = Anyks::toCase(up->key);
								// Если это параметр не для всех пользователей
								if(key.compare("all") != 0){
									// Если пользователь соответствует текущей, устанавливаем пароль
									if((Anyks::isNumber(key)
									&& (uid_t(::atoi(key.c_str())) == user.id))
									|| (key.compare(user.name) == 0)) user.pass = getPasswordFromFile(up->value, this->log, user.id, user.name);
								// Запоминаем что пароли хранятся для всех пользователей в одном файле
								} else if(filepass.empty()) filepass = up->value;
							}
						}
						// Если описания пользователей существуют
						if(!descriptions.empty()){
							// Переходим по списку описаний
							for(auto ud = descriptions.cbegin(); ud != descriptions.cend(); ++ud){
								// Если пользователь соответствует текущей, устанавливаем описание
								if((Anyks::isNumber(ud->key)
								&& (uid_t(::atoi(ud->key.c_str())) == user.id))
								|| (Anyks::toCase(ud->key).compare(user.name) == 0)) user.desc = ud->value;
							}
						}
						// Переопределяем дефолтные данные из файла конфигурации
						setDataUser(user, &ini);
						// Устанавливаем параметры http парсера
						user.headers.setOptions(user.options);
						// Добавляем пользователя в список пользователей
						this->data.insert(pair <uid_t, DataUser>(user.id, user));
						// Сообщаем что все удачно
						result = true;
					}
				}
				// Если файл паролей существует то устанавливаем пароли из него
				if(!filepass.empty()) getPasswordsFromFile(filepass, this->log, this, AMING_USER);
			}
		}
	}
	// Выводим результат
	return result;
}
/**
 * update Метод обновления пользователей
 */
const bool AUsers::Users::update(){
	// Результат проверки
	bool result = false;
	// Создаем текущее время генерации
	time_t curUpdate = time(nullptr);
	// Если время ожидания уже вышло, выполняем обновление данных
	if((this->lastUpdate + this->maxUpdate) < curUpdate){
		// Запоминаем текущее время
		this->lastUpdate = curUpdate;
		// Очищаем блок данных
		this->data.clear();
		// Определяем тип поиска пользователей
		switch(this->typeSearch){
			// Считываем данные пользователей из файлов
			case 0: readUsersFromFile(); break;
			// Считываем данные пользователей из системы
			case 1: readUsersFromPam(); break;
			// Считываем данные пользователей из LDAP
			case 2: readUsersFromLdap(); break;
			// Считываем данные пользователей из файлов и системы
			case 3: {
				readUsersFromFile();
				readUsersFromPam();
			} break;
			// Считываем данные пользователей из файлов и LDAP
			case 4: {
				readUsersFromFile();
				readUsersFromLdap();
			} break;
			// Считываем данные пользователей из системы и LDAP
			case 5: {
				readUsersFromPam();
				readUsersFromLdap();
			} break;
			// Считываем данные пользователей из файлов, системы и LDAP
			case 6: {
				readUsersFromFile();
				readUsersFromPam();
				readUsersFromLdap();
			} break;
		}
		// Сообщаем что все удачно
		result = true;
	}
	// Выводим результат
	return result;
}
/**
 * getAllUsers Метод получения данных всех пользователей
 * @return     список данных всех пользователей
 */
const vector <const AUsers::DataUser *> AUsers::Users::getAllUsers(){
	// Список данных по умолчанию
	vector <const DataUser *> result;
	// Если данные пользователей существуют
	if(!this->data.empty()){
		// Переходим по всем данным пользователей
		for(auto it = this->data.cbegin(); it != this->data.cend(); ++it){
			// Добавляем в список данные пользователей
			result.push_back(&(it->second));
		}
	}
	// Выводим результат
	return result;
}
/**
 * getUserByConnect Метод поиска данных пользователя по данным коннекта
 * @param ip  адрес интернет протокола клиента
 * @param mac аппаратный адрес сетевого интерфейса клиента
 * @return    данные пользователя
 */
const AUsers::DataUser * AUsers::Users::getUserByConnect(const string ip, const string mac){
	// Если данные существуют
	if(!ip.empty() || !mac.empty()){
		// Если данные существуют
		if(!this->data.empty()){
			// Получаем mac адрес
			string cmac = mac;
			// Данные пользователя
			const DataUser * user = nullptr;
			// Создаем объект сети
			Network nwk;
			// Определяем ip адрес
			u_int nettype = (!ip.empty() ? nwk.checkNetworkByIp(ip) : 0);
			// Переходим по всему списку пользователей
			for(auto it = this->data.cbegin(); it != this->data.cend(); ++it){
				// Прорверка на ip адрес
				bool _ip = false;
				// Проверка на mac адрес
				bool _mac = false;
				// Переходим по всему списку идентификаторов
				for(auto iit = it->second.idnt.cbegin(); iit != it->second.idnt.cend(); ++iit){
					// Определяем тип адреса
					const u_int idnt = Anyks::getTypeAmingByString(* iit);
					// Если это нужный нам адрес
					switch(idnt){
						case AMING_IPV4: {
							// Если нам нужен протокол версии 4
							if((nettype == 4) && (ip.compare(* iit) == 0)) _ip = true;
						} break;
						case AMING_IPV6: {
							// Если нам нужен протокол версии 4
							if((nettype == 6) && nwk.compareIP6(ip, * iit)) _ip = true;
						} break;
						case AMING_MAC: {
							// Если mac адреса соответствует
							if(!cmac.empty() && (Anyks::toCase(* iit).compare(Anyks::toCase(cmac)) == 0)) _mac = true;
						} break;
					}
				}
				// Если ip адрес и mac адрес совпали
				if(_ip && _mac) return &(it->second);
				// Если хоть что-то совпало, то выводим сампого первого найденного пользователя в списке
				else if((_ip || _mac) && (user == nullptr)) user = &(it->second);
			}
			// Если хотя бы один пользователь найден
			return user;
		}
	}
	// Выводим результат
	return nullptr;
}
/**
 * getDataById Метод получения данные пользователя по идентификатору
 * @param  uid идентификатор пользователя
 * @return     данные пользователя
 */
const AUsers::DataUser * AUsers::Users::getDataById(const uid_t uid){
	// Если данные пользователей существуют
	if(uid && !this->data.empty()){
		// Выполняем поиск данных пользователя
		if(this->data.count(uid)){
			// Получаем название пользователя
			return &(this->data.find(uid)->second);
		// Если пользователь не найден
		} else if(update()) {
			// Получаем имя пользователя
			return getDataById(uid);
		}
	}
	// Выводим результат
	return nullptr;
}
/**
 * getDataByName Метод получения данные пользователя по имени
 * @param  groupName название пользователя
 * @return           данные пользователя
 */
const AUsers::DataUser * AUsers::Users::getDataByName(const string userName){
	// Если данные пользователей существуют
	if(!userName.empty() && !this->data.empty()){
		// Приводим имя пользователя к нужному виду
		string name = Anyks::toCase(userName);
		// Переходим по всем данным пользователей
		for(auto it = this->data.cbegin(); it != this->data.cend(); ++it){
			// Если нашли имя пользователя
			if(it->second.name.compare(name) == 0){
				// Запоминаем идентификатор пользователя
				return &(it->second);
			}
		}
		// Если пользователь не найден
		if(update()) return getDataByName(userName);
	}
	// Выводим результат
	return nullptr;
}
/**
 * checkUserById Метод проверки на существование пользователя
 * @param  uid идентификатор пользователя
 * @return     результат проверки
 */
const bool AUsers::Users::checkUserById(const uid_t uid){
	// Если данные пользователей существуют
	if(uid && !this->data.empty()){
		// Выполняем поиск данных пользователя
		if(this->data.count(uid)){
			// Сообщаем что пользователь существует
			return true;
		// Если пользователи не найдены
		} else if(update()) {
			// Проверяем снова на существование пользователя
			return checkUserById(uid);
		}
	}
	// Выводим результат
	return false;
}
/**
 * checkGroupByName Метод проверки на существование пользователя
 * @param  userName название пользователя
 * @return          результат проверки
 */
const bool AUsers::Users::checkUserByName(const string userName){
	// Выполняем проверку на существование пользователя
	return (getIdByName(userName) > -1 ? true : false);
}
/**
 * getIdByName Метод извлечения идентификатора пользователя по его имени
 * @param  userName название пользователя
 * @return          идентификатор пользователя
 */
const uid_t AUsers::Users::getIdByName(const string userName){
	// Если данные пользователей существуют
	if(!userName.empty() && !this->data.empty()){
		// Приводим имя пользователя к нужному виду
		string name = Anyks::toCase(userName);
		// Переходим по всем данным пользователей
		for(auto it = this->data.cbegin(); it != this->data.cend(); ++it){
			// Если нашли имя пользователя
			if(it->second.name.compare(name) == 0){
				// Запоминаем идентификатор пользователя
				return it->first;
			}
		}
		// Если пользователь не найден
		if(update()) return getIdByName(userName);
	}
	// Выводим результат
	return -1;
}
/**
 * getNameById Метод извлечения имени пользователя по его идентификатору
 * @param  uid идентификатор пользователя
 * @return     название пользователя
 */
const string AUsers::Users::getNameById(const uid_t uid){
	// Если данные пользователей существуют
	if(uid && !this->data.empty()){
		// Выполняем поиск данных пользователя
		if(this->data.count(uid)){
			// Получаем название пользователя
			return this->data.find(uid)->second.name;
		// Если пользователь не найден
		} else if(update()) {
			// Получаем имя пользователя
			return getNameById(uid);
		}
	}
	// Выводим результат
	return string();
}
/**
 * getIdAllUsers Метод получения списка всех пользователей
 * @return список идентификаторов пользователей
 */
const vector <uid_t> AUsers::Users::getIdAllUsers(){
	// Результат работы функции
	vector <uid_t> result;
	// Получаем список пользователей
	auto users = getAllUsers();
	// Если пользователи существуют то переходим по ним
	if(!users.empty()){
		// Переходим по всем пользователям
		for(auto it = users.cbegin(); it != users.cend(); ++it){
			// Копируем весь список пользователей
			result.push_back((* it)->id);
		}
		// Сортируем
		sort(result.begin(), result.end());
		// Удаляем дубликаты
		result.resize(unique(result.begin(), result.end()) - result.begin());
	}
	// Выводим результат
	return result;
}
/**
 * setPassword Метод установки пароля пользователя
 * @param gid      идентификатор пользователя
 * @param password пароль пользователя
 */
void AUsers::Users::setPassword(const uid_t uid, const string password){
	// Если идентификатор и название переданы
	if((uid > -1) && !password.empty() && !this->data.empty()){
		// Выполняем поиск данных пользователя
		if(this->data.count(uid)){
			// Устанавливаем пароль пользователя
			(this->data.find(uid)->second).pass = password;
		// Если пользователь не найден
		} else if(update()) {
			// Устанавливаем пароль пользователя
			setPassword(uid, password);
		}
	}
}
/**
 * setGroups Метод добавления объекта групп
 * @param groups объект групп
 */
void AUsers::Users::setGroups(void * groups){
	// Если объект групп существует то добавляем его
	this->groups = groups;
	// Считываем данные групп
	update();
}
/**
 * Users Конструктор
 * @param config конфигурационные данные
 * @param log    объект лога для вывода информации
 */
AUsers::Users::Users(Config * config, LogApp * log){
	// Если конфигурационные данные переданы
	if(config){
		// Запоминаем данные логов
		this->log = log;
		// Запоминаем конфигурационные данные
		this->config = config;
		// Запоминаем тип поиска групп пользователя
		this->typeSearch = 0;
		// Заполняем тип извлечения конфигурационных данных
		this->typeConfigs = 0;
		// Запоминаем время в течение которого запрещено обновлять данные
		this->maxUpdate = 600;
		// Устанавливаем параметры подключения LDAP
		this->ldap = {
			"ou=users,dc=agro24,dc=dev",
			"ou=configs,ou=aming,dc=agro24,dc=dev",
			"one",
			"one",
			"(&(!(agro24CoJpDismissed=TRUE))(objectClass=inetOrgPerson))",
			"(&(amingConfigsUserId=%u)(amingConfigsType=users)(objectClass=amingConfigs))"
		};
	}
}
