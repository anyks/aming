/* ГРУППЫ ПОЛЬЗОВАТЕЛЕЙ ПРОКСИ AMING */
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
 * setProxyOptions Функция добавления опций прокси
 * @param option       опция для добавления
 * @param proxyOptions список существующих опций
 * @param flag         флаг добавления или удаления опции
 */
void Groups::setProxyOptions(const u_short option, u_short &proxyOptions, const bool flag){
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
 * setDataGroupFromLdap Метод заполнения данных группы из LDAP
 * @param group объект группы
 */
void Groups::setDataGroupFromLdap(Groups::Data &group){
	// Параметр для поиска
	const char * key = "%g";
	// Выполняем поиск ключа
	size_t pos = this->ldap.filterConfig.find(key);
	// Если ключ найден
	if(pos != string::npos){
		// Создаем объект подключения LDAP
		ALDAP ldap(this->config, this->log);
		// Формируем наш фильтр
		string filter = this->ldap.filterConfig;
		// Заменяем ключ на логин пользователя
		filter = filter.replace(pos, strlen(key), to_string(group.id));
		// Создаем dn
		const string dn = (string("ac=") + this->config->proxy.name + string(",") + this->ldap.dnConfig);
		// Формируем параметры запроса
		const string params =	"amingConfigsConnectsConnect,amingConfigsConnectsSize,amingConfigsGzipChunk,"
								"amingConfigsGzipLength,amingConfigsGzipLevel,amingConfigsGzipProxied,"
								"amingConfigsGzipRegex,amingConfigsGzipResponse,amingConfigsGzipTransfer,"
								"amingConfigsGzipTypes,amingConfigsGzipVary,amingConfigsGzipVhttp,amingConfigsAuth,"
								"amingConfigsIdntIp4,amingConfigsIdntIp6,amingConfigsIdntMac,amingConfigsIdntNetwork4,"
								"amingConfigsIdntNetwork6,amingConfigsIpExternal4,amingConfigsIpExternal6,"
								"amingConfigsIpResolver4,amingConfigsIpResolver6,amingConfigsKeepAliveCnt,"
								"amingConfigsKeepAliveEnabled,amingConfigsKeepAliveIdle,amingConfigsKeepAliveIntvl,"
								"amingConfigsProxyAgent,amingConfigsProxyConnect,amingConfigsProxyDeblock,"
								"amingConfigsProxyForward,amingConfigsProxyPipelining,amingConfigsProxyReverse,"
								"amingConfigsProxySkill,amingConfigsProxySubnet,amingConfigsProxyTransfer,"
								"amingConfigsProxyUpgrade,amingConfigsSpeedInput,amingConfigsSpeedOutput,"
								"amingConfigsTimeoutsRead,amingConfigsTimeoutsUpgrade,amingConfigsTimeoutsWrite";
		// Запрашиваем данные параметров
		auto groups = ldap.data(dn, params, this->ldap.scopeConfig, filter);
		// Если параметры получены
		if(!groups.empty()){
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
			for(auto it = groups.cbegin(); it != groups.cend(); ++it){
				// Списки данных идентификации
				vector <string> idnt_ip4, idnt_ip6, idnt_mac, idnt_network4, idnt_network6;
				// Переходим по всему массиву полученных объектов
				for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
					// Если список значений существует
					if(!dt->second.empty()){
						// Если это идентификатор IPv4
						if(dt->first.compare("amingConfigsIdntIp4") == 0) idnt_ip4 = dt->second;
						// Если это идентификатор IPv6
						else if(dt->first.compare("amingConfigsIdntIp6") == 0) idnt_ip6 = dt->second;
						// Если это идентификатор MAC
						else if(dt->first.compare("amingConfigsIdntMac") == 0) idnt_mac = dt->second;
						// Если это идентификатор Network IPv4
						else if(dt->first.compare("amingConfigsIdntNetwork4") == 0) idnt_network4 = dt->second;
						// Если это идентификатор Network IPv6
						else if(dt->first.compare("amingConfigsIdntNetwork6") == 0) idnt_network6 = dt->second;
						// Если это External IPv4
						else if(dt->first.compare("amingConfigsIpExternal4") == 0) group.ipv4.ip = dt->second;
						// Если это External IPv6
						else if(dt->first.compare("amingConfigsIpExternal6") == 0) group.ipv6.ip = dt->second;
						// Если это Resolver IPv4
						else if(dt->first.compare("amingConfigsIpResolver4") == 0) group.ipv4.resolver = dt->second;
						// Если это Resolver IPv6
						else if(dt->first.compare("amingConfigsIpResolver6") == 0) group.ipv6.resolver = dt->second;
						// Если это тип авторизации клиента
						else if(dt->first.compare("amingConfigsAuth") == 0) group.auth = Anyks::toCase(dt->second[0]);
						// Если это параметры прокси Connect
						else if(dt->first.compare("amingConfigsProxyConnect") == 0){
							// Устанавливаем или убираем опцию
							setProxyOptions(OPT_CONNECT, group.options, getBoolean(dt->second[0]));
						// Если это параметры прокси Upgrade
						} else if(dt->first.compare("amingConfigsProxyUpgrade") == 0){
							// Устанавливаем или убираем опцию
							setProxyOptions(OPT_UPGRADE, group.options, getBoolean(dt->second[0]));
						// Если это параметры прокси Agent
						} else if(dt->first.compare("amingConfigsProxyAgent") == 0){
							// Устанавливаем или убираем опцию
							setProxyOptions(OPT_AGENT, group.options, getBoolean(dt->second[0]));
						// Если это параметры прокси Deblock
						} else if(dt->first.compare("amingConfigsProxyDeblock") == 0){
							// Устанавливаем или убираем опцию
							setProxyOptions(OPT_DEBLOCK, group.options, getBoolean(dt->second[0]));
						// Если это параметры gzip Transfer
						} else if(dt->first.compare("amingConfigsGzipTransfer") == 0){
							// Устанавливаем или убираем опцию
							setProxyOptions(OPT_GZIP, group.options, getBoolean(dt->second[0]));
						// Если это параметры gzip Response
						} else if(dt->first.compare("amingConfigsGzipResponse") == 0){
							// Устанавливаем или убираем опцию
							setProxyOptions(OPT_PGZIP, group.options, getBoolean(dt->second[0]));
						// Если это параметры keepalive Enabled
						} else if(dt->first.compare("amingConfigsKeepAliveEnabled") == 0){
							// Устанавливаем или убираем опцию
							setProxyOptions(OPT_KEEPALIVE, group.options, getBoolean(dt->second[0]));
						// Если это параметры прокси Skill
						} else if(dt->first.compare("amingConfigsProxySkill") == 0){
							// Выполняем проверку на доступность опции
							const bool check = (dt->second[0].compare("smart") == 0);
							// Устанавливаем или убираем опцию
							setProxyOptions(OPT_SMART, group.options, check);
						// Если это параметры таймауты read
						} else if(dt->first.compare("amingConfigsTimeoutsRead") == 0){
							// Устанавливаем параметры
							group.timeouts.read = (size_t) Anyks::getSeconds(dt->second[0]);
						// Если это параметры таймауты write
						} else if(dt->first.compare("amingConfigsTimeoutsWrite") == 0){
							// Устанавливаем параметры
							group.timeouts.write = (size_t) Anyks::getSeconds(dt->second[0]);
						// Если это параметры таймауты upgrade
						} else if(dt->first.compare("amingConfigsTimeoutsUpgrade") == 0){
							// Устанавливаем параметры
							group.timeouts.upgrade = (size_t) Anyks::getSeconds(dt->second[0]);
						// Если это параметры speed input
						} else if(dt->first.compare("amingConfigsSpeedInput") == 0){
							// Устанавливаем параметры
							group.buffers.read = Anyks::getSizeBuffer(dt->second[0]);
						// Если это параметры speed output
						} else if(dt->first.compare("amingConfigsSpeedOutput") == 0){
							// Устанавливаем параметры
							group.buffers.write = Anyks::getSizeBuffer(dt->second[0]);
						// Если это параметры keepalive keepcnt
						} else if(dt->first.compare("amingConfigsKeepAliveCnt") == 0){
							// Устанавливаем параметры
							group.keepalive.keepcnt = (int) ::atoi(dt->second[0].c_str());
						// Если это параметры keepalive keepidle
						} else if(dt->first.compare("amingConfigsKeepAliveIdle") == 0){
							// Устанавливаем параметры
							group.keepalive.keepidle = (int) ::atoi(dt->second[0].c_str());
						// Если это параметры keepalive keepintvl
						} else if(dt->first.compare("amingConfigsKeepAliveIntvl") == 0){
							// Устанавливаем параметры
							group.keepalive.keepintvl = (int) ::atoi(dt->second[0].c_str());
						// Если это параметры connects size
						} else if(dt->first.compare("amingConfigsConnectsSize") == 0){
							// Устанавливаем параметры
							group.connects.size = Anyks::getBytes(dt->second[0]);
						// Если это параметры connects connect
						} else if(dt->first.compare("amingConfigsConnectsConnect") == 0){
							// Устанавливаем параметры
							group.connects.connect = (u_int) ::atoi(dt->second[0].c_str());
						// Если это параметры proxy subnet
						} else if(dt->first.compare("amingConfigsProxySubnet") == 0){
							// Устанавливаем параметры
							group.proxy.subnet = getBoolean(dt->second[0]);
						// Если это параметры proxy reverse
						} else if(dt->first.compare("amingConfigsProxyReverse") == 0){
							// Устанавливаем параметры
							group.proxy.reverse = getBoolean(dt->second[0]);
						// Если это параметры proxy forward
						} else if(dt->first.compare("amingConfigsProxyForward") == 0){
							// Устанавливаем параметры
							group.proxy.forward = getBoolean(dt->second[0]);
						// Если это параметры proxy transfer
						} else if(dt->first.compare("amingConfigsProxyTransfer") == 0){
							// Устанавливаем параметры
							group.proxy.transfer = getBoolean(dt->second[0]);
						// Если это параметры proxy pipelining
						} else if(dt->first.compare("amingConfigsProxyPipelining") == 0){
							// Устанавливаем параметры
							group.proxy.pipelining = getBoolean(dt->second[0]);
						// Если это параметры gzip regex
						} else if(dt->first.compare("amingConfigsGzipRegex") == 0){
							// Устанавливаем параметры
							group.gzip.regex = dt->second[0];
						// Если это параметры gzip vary
						} else if(dt->first.compare("amingConfigsGzipVary") == 0){
							// Устанавливаем параметры
							group.gzip.vary = getBoolean(dt->second[0]);
						// Если это параметры gzip length
						} else if(dt->first.compare("amingConfigsGzipLength") == 0){
							// Устанавливаем параметры
							group.gzip.length = (int) ::atoi(dt->second[0].c_str());
						// Если это параметры gzip chunk
						} else if(dt->first.compare("amingConfigsGzipChunk") == 0){
							// Устанавливаем параметры
							group.gzip.chunk = Anyks::getBytes(dt->second[0]);
						// Если это параметры gzip vhttp
						} else if(dt->first.compare("amingConfigsGzipVhttp") == 0){
							// Устанавливаем параметры
							group.gzip.vhttp = dt->second;
						// Если это параметры gzip types
						} else if(dt->first.compare("amingConfigsGzipTypes") == 0){
							// Устанавливаем параметры
							group.gzip.types = dt->second;
						// Если это параметры gzip proxied
						} else if(dt->first.compare("amingConfigsGzipProxied") == 0){
							// Устанавливаем параметры
							group.gzip.proxied = dt->second;
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
							if(level != OPT_NULL) group.gzip.level = level;
						}
					}
				}
				// Создаем список идентификаторов группы
				group.idnt = {idnt_ip4, idnt_ip6, idnt_network4, idnt_network6, idnt_mac};
			}
		}
	}
}
/**
 * setDataGroupFromFile Метод заполнения данных группы из конфигурационного файла
 * @param group объект группы
 * @param ini   указатель на объект конфигурации
 */
void Groups::setDataGroupFromFile(Groups::Data &group, INI * ini){
	// Флаг удаления созданного объекта ini конфигурации
	bool rmINI = false;
	// Если объект ini не передан то создаем его
	if(!ini){
		// Создаем адрес для хранения файла
		const string filename = Anyks::addToPath(this->config->proxy.dir, "groups.ini");
		// Проверяем на существование адреса
		if(!filename.empty() && Anyks::isFileExist(filename.c_str())){
			// Инициализируем парсер ini файла
			ini = new INI(filename);
			// Запоминаем что нужно удалить объект конфигурации
			rmINI = true;
		// Если объект конфигурации не существует тогда выходим
		} else return;
	}
	// Создаем список идентификаторов группы
	group.idnt	= {
		Anyks::split(ini->getString(group.name + "_idnt", "ip4"), "|"),
		Anyks::split(ini->getString(group.name + "_idnt", "ip6"), "|"),
		Anyks::split(ini->getString(group.name + "_idnt", "network4"), "|"),
		Anyks::split(ini->getString(group.name + "_idnt", "network6"), "|"),
		Anyks::split(ini->getString(group.name + "_idnt", "mac"), "|")
	};
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_proxy", "connect")){
		// Выполняем проверку на доступность опции
		const bool check = ini->getBoolean(group.name + "_proxy", "connect");
		// Устанавливаем или убираем опцию
		setProxyOptions(OPT_CONNECT, group.options, check);
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_proxy", "upgrade")){
		// Выполняем проверку на доступность опции
		const bool check = ini->getBoolean(group.name + "_proxy", "upgrade");
		// Устанавливаем или убираем опцию
		setProxyOptions(OPT_UPGRADE, group.options, check);
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_proxy", "agent")){
		// Выполняем проверку на доступность опции
		const bool check = ini->getBoolean(group.name + "_proxy", "agent");
		// Устанавливаем или убираем опцию
		setProxyOptions(OPT_AGENT, group.options, check);
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_proxy", "deblock")){
		// Выполняем проверку на доступность опции
		const bool check = ini->getBoolean(group.name + "_proxy", "deblock");
		// Устанавливаем или убираем опцию
		setProxyOptions(OPT_DEBLOCK, group.options, check);
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_gzip", "transfer")){
		// Выполняем проверку на доступность опции
		const bool check = ini->getBoolean(group.name + "_gzip", "transfer");
		// Устанавливаем или убираем опцию
		setProxyOptions(OPT_GZIP, group.options, check);
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_gzip", "response")){
		// Выполняем проверку на доступность опции
		const bool check = ini->getBoolean(group.name + "_gzip", "response");
		// Устанавливаем или убираем опцию
		setProxyOptions(OPT_PGZIP, group.options, check);
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_proxy", "skill")){
		// Выполняем проверку на доступность опции
		const bool check = (ini->getString(group.name + "_proxy", "skill", "dumb").compare("smart") == 0);
		// Устанавливаем или убираем опцию
		setProxyOptions(OPT_SMART, group.options, check);
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_keepalive", "enabled")){
		// Выполняем проверку на доступность опции
		const bool check = ini->getBoolean(group.name + "_keepalive", "enabled");
		// Устанавливаем или убираем опцию
		setProxyOptions(OPT_KEEPALIVE, group.options, check);
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_ipv4", "external")){
		// Устанавливаем список ip адресов
		group.ipv4.ip = Anyks::split(ini->getString(group.name + "_ipv4", "external"), "|");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_ipv6", "external")){
		// Устанавливаем список ip адресов
		group.ipv6.ip = Anyks::split(ini->getString(group.name + "_ipv6", "external"), "|");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_ipv4", "resolver")){
		// Устанавливаем список резолверов
		group.ipv4.resolver = Anyks::split(ini->getString(group.name + "_ipv4", "resolver"), "|");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_ipv6", "resolver")){
		// Устанавливаем список резолверов
		group.ipv6.resolver = Anyks::split(ini->getString(group.name + "_ipv6", "resolver"), "|");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_timeouts", "read")){
		// Устанавливаем параметры
		group.timeouts.read = (size_t) Anyks::getSeconds(ini->getString(group.name + "_timeouts", "read"));
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_timeouts", "write")){
		// Устанавливаем параметры
		group.timeouts.write = (size_t) Anyks::getSeconds(ini->getString(group.name + "_timeouts", "write"));
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_timeouts", "upgrade")){
		// Устанавливаем параметры
		group.timeouts.upgrade = (size_t) Anyks::getSeconds(ini->getString(group.name + "_timeouts", "upgrade"));
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_speed", "input")){
		// Устанавливаем параметры
		group.buffers.read = Anyks::getSizeBuffer(ini->getString(group.name + "_speed", "input"));
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_speed", "output")){
		// Устанавливаем параметры
		group.buffers.write = Anyks::getSizeBuffer(ini->getString(group.name + "_speed", "output"));
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_keepalive", "keepcnt")){
		// Устанавливаем параметры
		group.keepalive.keepcnt = (int) ini->getNumber(group.name + "_keepalive", "keepcnt");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_keepalive", "keepidle")){
		// Устанавливаем параметры
		group.keepalive.keepidle = (int) ini->getNumber(group.name + "_keepalive", "keepidle");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_keepalive", "keepintvl")){
		// Устанавливаем параметры
		group.keepalive.keepintvl = (int) ini->getNumber(group.name + "_keepalive", "keepintvl");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_connects", "size")){
		// Устанавливаем параметры
		group.connects.size = Anyks::getBytes(ini->getString(group.name + "_connects", "size"));
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_connects", "connect")){
		// Устанавливаем параметры
		group.connects.connect = (u_int) ini->getUNumber(group.name + "_connects", "connect");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_proxy", "subnet")){
		// Устанавливаем параметры
		group.proxy.subnet = ini->getBoolean(group.name + "_proxy", "subnet");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_proxy", "reverse")){
		// Устанавливаем параметры
		group.proxy.reverse = ini->getBoolean(group.name + "_proxy", "reverse");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_proxy", "forward")){
		// Устанавливаем параметры
		group.proxy.forward = ini->getBoolean(group.name + "_proxy", "forward");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_proxy", "transfer")){
		// Устанавливаем параметры
		group.proxy.transfer = ini->getBoolean(group.name + "_proxy", "transfer");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_proxy", "pipelining")){
		// Устанавливаем параметры
		group.proxy.pipelining = ini->getBoolean(group.name + "_proxy", "pipelining");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_gzip", "regex")){
		// Устанавливаем параметры
		group.gzip.regex = ini->getString(group.name + "_gzip", "regex");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_gzip", "vary")){
		// Устанавливаем параметры
		group.gzip.vary = ini->getBoolean(group.name + "_gzip", "vary");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_gzip", "length")){
		// Устанавливаем параметры
		group.gzip.length = ini->getNumber(group.name + "_gzip", "length");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_gzip", "chunk")){
		// Устанавливаем параметры
		group.gzip.chunk = Anyks::getBytes(ini->getString(group.name + "_gzip", "chunk"));
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_gzip", "vhttp")){
		// Устанавливаем параметры
		group.gzip.vhttp = Anyks::split(ini->getString(group.name + "_gzip", "vhttp"), "|");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_gzip", "types")){
		// Устанавливаем параметры
		group.gzip.types = Anyks::split(ini->getString(group.name + "_gzip", "types"), "|");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_gzip", "proxied")){
		// Устанавливаем параметры
		group.gzip.proxied = Anyks::split(ini->getString(group.name + "_gzip", "proxied"), "|");
	}
	// Выполняем проверку на существование записи в конфигурационном файле
	if(ini->checkParam(group.name + "_gzip", "level")){
		// Уровень сжатия gzip
		u_int level = OPT_NULL;
		// Получаем уровень сжатия
		const string gzipLevel = ini->getString(group.name + "_gzip", "level");
		// Если размер указан
		if(!gzipLevel.empty()){
			// Определяем тип сжатия
			if(gzipLevel.compare("default") == 0)		level = Z_DEFAULT_COMPRESSION;
			else if(gzipLevel.compare("best") == 0)		level = Z_BEST_COMPRESSION;
			else if(gzipLevel.compare("speed") == 0)	level = Z_BEST_SPEED;
			else if(gzipLevel.compare("no") == 0)		level = Z_NO_COMPRESSION;
		}
		if(level != OPT_NULL) group.gzip.level = level;
	}
	// Удаляем объект конфигурации если это требуется
	if(rmINI) delete ini;
}
/**
 * setDataGroup Метод заполнения данных группы
 * @param group объект группы
 * @param ini   указатель на объект конфигурации
 */
void Groups::setDataGroup(Groups::Data &group, INI * ini){
	// Определяем тип системы откуда нужно получить конфигурационные файлы
	switch(this->typeConfigs){
		// Переопределяем дефолтные данные из файла конфигурации
		case 0: setDataGroupFromFile(group, ini); break;
		// Переопределяем дефолтные данные из LDAP
		case 1: setDataGroupFromLdap(group); break;
	}
}
/**
 * createDefaultData Метод создания группы с параметрами по умолчанию
 * @param  id   идентификатор группы
 * @param  name название группы
 * @return      созданная группа
 */
const Groups::Data Groups::createDefaultData(const gid_t id, const string name){
	// Создаем блок с данными группы
	Data group;
	// Если входные параметры верные
	if(id && !name.empty()){
		// Запоминаем название группы
		string groupName = name;
		// Заполняем данные группы
		group.id		= id;
		group.name		= Anyks::toCase(groupName);
		group.auth		= this->config->auth.auth;
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
 * readGroupsFromLdap Метод чтения данных групп из LDAP сервера
 * @return результат операции
 */
const bool Groups::readGroupsFromLdap(){
	// Результат работы функции
	bool result = false;
	// Создаем объект подключения LDAP
	ALDAP ldap(this->config, this->log);
	// Запрашиваем данные пользователей
	auto users = ldap.data(this->ldap.dnUser, "gidNumber,uidNumber", this->ldap.scopeUser, this->ldap.filterUser);
	// Запрашиваем данные групп
	auto groups = ldap.data(this->ldap.dnGroup, "cn,description,gidNumber,Password,memberUid", this->ldap.scopeGroup, this->ldap.filterGroup);
	// Если группы получены
	if(!groups.empty()){
		// Переходим по всему объекту групп
		for(auto it = groups.cbegin(); it != groups.cend(); ++it){
			// Идентификатор группы
			gid_t gid;
			// Название группы
			string name;
			// Описание группы
			string description;
			// Пароль группы
			string password;
			// Список пользователей
			vector <uid_t> users;
			// Переходим по всему массиву полученных объектов
			for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
				// Если список значений существует
				if(!dt->second.empty()){
					// Если это идентификатор группы
					if(dt->first.compare("cn") == 0) name = dt->second[0];
					// Если это идентификатор группы
					else if(dt->first.compare("gidNumber") == 0) gid = ::atoi(dt->second[0].c_str());
					// Если это описание группы
					else if(dt->first.compare("description") == 0) description = dt->second[0];
					// Если это пароль группы
					else if(dt->first.compare("Password") == 0) password = dt->second[0];
					// Если это список пользователей
					else if(dt->first.compare("memberUid") == 0){
						// Переходим по списку пользователей
						for(auto ut = dt->second.cbegin(); ut != dt->second.cend(); ++ut){
							// Получаем идентификатор пользвоателя
							uid_t uid = getUidByName(* ut);
							// Добавляем в список идентификатор пользователя
							if(uid > -1) users.push_back(uid);
						}
					}
				}
			}
			// Создаем блок с данными группы
			Data group = createDefaultData(gid, name);
			// Устанавливаем тип группы
			group.type = 2;
			// Добавляем пользователя в список
			group.users = users;
			// Добавляем пароль группы
			group.pass = password;
			// Добавляем описание группы
			group.desc = description;
			// Переопределяем дефолтные данные из файла конфигурации
			setDataGroup(group);
			// Устанавливаем параметры http парсера
			group.headers.setOptions(group.options);
			// Добавляем группу в список групп
			this->data.insert(pair <gid_t, Data>(group.id, group));
		}
		// Если пользователи существуют
		if(!users.empty()){
			// Переходим по всему объекту пользователей
			for(auto it = users.cbegin(); it != users.cend(); ++it){
				// Идентификатор группы
				gid_t gid;
				// Идентификатор пользователя
				uid_t uid;
				// Переходим по всему массиву полученных объектов
				for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
					// Если список значений существует
					if(!dt->second.empty()){
						// Если это идентификатор группы
						if(dt->first.compare("gidNumber") == 0){
							// Получаем идентификатор группы
							gid = ::atoi(dt->second[0].c_str());
						} else if(dt->first.compare("uidNumber") == 0) {
							// Получаем идентификатор пользователя
							uid = ::atoi(dt->second[0].c_str());
						}
					}
				}
				// Если группа найдена
				if(this->data.count(gid)){
					// Пользователель существует в данной группе
					bool userExist = false;
					// Получаем список пользователей
					vector <uid_t> * users = &this->data.find(gid)->second.users;
					// Переходим по списку пользователей
					for(auto it = users->cbegin(); it != users->cend(); ++it){
						// Если идентификатор пользователя найден
						if(uid == *it){
							// Запоминаем что пользователь существует
							userExist = true;
							// Выходим из цикла
							break;
						}
					}
					// Если пользователь не существует то добавляем его в список
					if(!userExist) users->push_back(uid);
				}
			}
		}
		// Сообщаем что все удачно
		result = true;
	}
	// Выводим результат
	return result;
}
/**
 * readGroupsFromPam Метод чтения данных групп из операционной системы
 * @return результат операции
 */
const bool Groups::readGroupsFromPam(){
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
				// Максимальное количество групп пользователя
				int maxGroupsUser = this->maxPamGroupsUser;
				// Создаем список групп пользователя
				int * userGroups = new int[(const int) maxGroupsUser];
				// Данные группы
				struct group * gr = nullptr;
				// Получаем список групп пользователя
				if(getgrouplist(pw->pw_name, pw->pw_gid, userGroups, &maxGroupsUser) == -1){
					// Выводим сообщение что группы не созданы
					this->log->write(LOG_ERROR, 0, "groups from user = %s [%s] not found", pw->pw_name, pw->pw_gecos);
				// Если группы получены удачно
				} else {
					// Переходим по всем группам пользователя
					for(int i = 0; i < maxGroupsUser; i++){
						// Извлекаем данные группы
						gr = getgrgid(userGroups[i]);
						// Если группа получена
						if(gr != nullptr){
							// Если группа найдена
							if(this->data.count(gr->gr_gid)){
								// Пользователель существует в данной группе
								bool userExist = false;
								// Получаем список пользователей
								vector <uid_t> * users = &this->data.find(gr->gr_gid)->second.users;
								// Переходим по списку пользователей
								for(auto it = users->cbegin(); it != users->cend(); ++it){
									// Если идентификатор пользователя найден
									if(pw->pw_uid == *it){
										// Запоминаем что пользователь существует
										userExist = true;
										// Выходим из цикла
										break;
									}
								}
								// Если пользователь не существует то добавляем его в список
								if(!userExist) users->push_back(pw->pw_uid);
							// Если группа не найдена, то создаем её
							} else {
								// Создаем блок с данными группы
								Data group = createDefaultData(gr->gr_gid, gr->gr_name);
								// Устанавливаем тип группы
								group.type = 1;
								// Добавляем пароль группы
								group.pass = gr->gr_passwd;
								// Добавляем пользователя в список
								group.users.push_back(pw->pw_uid);
								// Переопределяем дефолтные данные из файла конфигурации
								setDataGroup(group);
								// Устанавливаем параметры http парсера
								group.headers.setOptions(group.options);
								// Добавляем группу в список групп
								this->data.insert(pair <gid_t, Data>(group.id, group));
							}
						// Выводим сообщение что данная группа не найдена
						} else this->log->write(LOG_ERROR, 0, "group [%i] from user = %s [%s] not found", userGroups[i], pw->pw_name, pw->pw_gecos);
					}
					// Сообщаем что все удачно
					result = true;
				}
				// Удаляем выделенную память для групп
				delete [] userGroups;
			}
		}
	}
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
	const string filename = Anyks::addToPath(this->config->proxy.dir, "groups.ini");
	// Проверяем на существование адреса
	if(!filename.empty() && Anyks::isFileExist(filename.c_str())){
		// Инициализируем парсер ini файла
		INI ini(filename);
		// Если во время чтения файла ошибок не возникло
		if(!ini.isError()){
			// Получаем список пользователей
			auto users = ini.getParamsInSection("users");
			// Получаем список групп
			auto groups = ini.getParamsInSection("groups");
			// Получаем список паролей
			auto passwords = ini.getParamsInSection("passwords");
			// Получаем список описаний
			auto descriptions = ini.getParamsInSection("descriptions");
			// Переходим по списку групп
			for(auto it = groups.cbegin(); it != groups.cend(); ++it){
				// Если идентификатор группы существует
				if(Anyks::isNumber(it->key) || Anyks::isNumber(it->value)){
					// Получаем идентификатор группы
					const gid_t gid = (Anyks::isNumber(it->key) ? ::atoi(it->key.c_str()) : ::atoi(it->value.c_str()));
					// Получаем название группы
					const string name = (Anyks::isNumber(it->key) ? it->value : it->key);
					// Создаем блок с данными группы
					Data group = createDefaultData(gid, name);
					// Устанавливаем тип группы
					group.type = 0;
					// Если список пользователей существует
					if(!users.empty()){
						// Переходим по списку пользователей
						for(auto ut = users.cbegin(); ut != users.cend(); ++ut){
							// Если группа соответствует текущей
							if((Anyks::isNumber(ut->value)
							&& (gid_t(::atoi(ut->value.c_str())) == group.id))
							|| (Anyks::toCase(ut->value).compare(group.name) == 0)){
								// Создаем идентификатор пользователя
								uid_t uid = -1;
								// Проверяем является ли название пользователя идентификатором
								if(Anyks::isNumber(ut->key)) uid = ::atoi(ut->key.c_str());
								// Если это не идентификатор то запрашиваем идентификатор пользователя
								else uid = getUidByName(ut->key);
								// Добавляем пользователя в список
								if(uid > -1) group.users.push_back(uid);
							}
						}
					}
					// Если пароли групп существуют
					if(!passwords.empty()){
						// Переходим по списку паролей
						for(auto gp = passwords.cbegin(); gp != passwords.cend(); ++gp){
							// Если группа соответствует текущей, устанавливаем пароль
							if((Anyks::isNumber(gp->key)
							&& (gid_t(::atoi(gp->key.c_str())) == group.id))
							|| (Anyks::toCase(gp->key).compare(group.name) == 0)) group.pass = gp->value;
						}
					}
					// Если описания групп существуют
					if(!descriptions.empty()){
						// Переходим по списку описаний
						for(auto gd = descriptions.cbegin(); gd != descriptions.cend(); ++gd){
							// Если группа соответствует текущей, устанавливаем описание
							if((Anyks::isNumber(gd->key)
							&& (gid_t(::atoi(gd->key.c_str())) == group.id))
							|| (Anyks::toCase(gd->key).compare(group.name) == 0)) group.desc = gd->value;
						}
					}
					// Переопределяем дефолтные данные из файла конфигурации
					setDataGroup(group, &ini);
					// Устанавливаем параметры http парсера
					group.headers.setOptions(group.options);
					// Добавляем группу в список групп
					this->data.insert(pair <gid_t, Data>(group.id, group));
				}
			}
		}
	}
	// Выводим результат
	return result;
}
/**
 * update Метод обновления групп
 */
const bool Groups::update(){
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
		// Определяем тип поиска группы
		switch(this->typeSearch){
			// Считываем данные групп из файлов
			case 0: readGroupsFromFile(); break;
			// Считываем данные групп из системы
			case 1: readGroupsFromPam(); break;
			// Считываем данные групп из LDAP
			case 2: readGroupsFromLdap(); break;
			// Считываем данные групп из файлов и системы
			case 3: {
				readGroupsFromFile();
				readGroupsFromPam();
			} break;
			// Считываем данные групп из файлов и LDAP
			case 4: {
				readGroupsFromFile();
				readGroupsFromLdap();
			} break;
			// Считываем данные групп из системы и LDAP
			case 5: {
				readGroupsFromPam();
				readGroupsFromLdap();
			} break;
			// Считываем данные групп из файлов, системы и LDAP
			case 6: {
				readGroupsFromFile();
				readGroupsFromPam();
				readGroupsFromLdap();
			} break;
		}
		// Сообщаем что все удачно
		result = true;
	}
	// Выводим результат
	return result;
}
/**
 * getAllGroups Метод получения данных всех групп
 * @return      список данных всех групп
 */
const vector <const Groups::Data *> Groups::getAllGroups(){
	// Список данных по умолчанию
	vector <const Data *> result;
	// Если данные групп существуют
	if(!this->data.empty()){
		// Переходим по всем данным групп
		for(auto it = this->data.cbegin(); it != this->data.cend(); ++it){
			// Добавляем в список данные групп
			result.push_back(&(it->second));
		}
	}
	// Выводим результат
	return result;
}
/**
 * getDataById Метод получения данные группы по идентификатору группы
 * @param  gid идентификатор группы
 * @return     данные группы
 */
const Groups::Data * Groups::getDataById(const gid_t gid){
	// Если данные групп существуют
	if(gid && !this->data.empty()){
		// Выполняем поиск данных групп
		if(this->data.count(gid)){
			// Получаем название группы
			return &(this->data.find(gid)->second);
		// Если группа не найдена
		} else if(update()) {
			// Получаем имя группы
			return getDataById(gid);
		}
	}
	// Выводим результат
	return nullptr;
}
/**
 * getDataByName Метод получения данные группы по имени группы
 * @param  groupName название группы
 * @return           данные группы
 */
const Groups::Data * Groups::getDataByName(const string groupName){
	// Если данные групп существуют
	if(!groupName.empty() && !this->data.empty()){
		// Приводим имя группы к нужному виду
		string name = Anyks::toCase(groupName);
		// Переходим по всем данным групп
		for(auto it = this->data.cbegin(); it != this->data.cend(); ++it){
			// Если нашли имя группы
			if(it->second.name.compare(name) == 0){
				// Запоминаем идентификатор группы
				return &(it->second);
			}
		}
		// Если группа не найдена
		if(update()) return getDataByName(groupName);
	}
	// Выводим результат
	return nullptr;
}
/**
 * getGroupIdByUser Метод получения идентификатор группы по идентификатору пользователя
 * @param  uid идентификатор пользователя
 * @return     идентификатор группы
 */
const vector <gid_t> Groups::getGroupIdByUser(const uid_t uid){
	// Результат работы функции
	vector <gid_t> result;
	// Если идентификатор пользователя передан
	if(uid){
		// Переходим по списку групп
		for(auto it = this->data.cbegin(); it != this->data.cend(); ++it){
			// Получаем список пользователей
			auto users = it->second.users;
			// Переходим по списку пользователей
			for(auto ut = users.cbegin(); ut != users.cend(); ++ut){
				// Если идентификатор пользователя соответствует
				if(uid == *ut){
					// Добавляем идентификатор группы в список
					result.push_back(it->first);
					// Выходим из цикла
					break;
				}
			}
		}
	}
	// Выводим результат
	return result;
}
/**
 * getGroupIdByUser Метод получения идентификатор группы по имени пользователя
 * @param  userName название пользователя
 * @return          идентификатор группы
 */
const vector <gid_t> Groups::getGroupIdByUser(const string userName){
	// Результат работы функции
	vector <gid_t> result;
	// Если название пользователя передано
	if(!userName.empty()){
		// Получаем идентификатор пользователя
		const uid_t uid = getUidByName(userName);
		// Получаем идентификаторы группы
		result = getGroupIdByUser(uid);
	}
	// Выводим результат
	return result;
}
/**
 * getGroupNameByUser Метод получения название группы по идентификатору пользователя
 * @param  uid идентификатор пользователя
 * @return     название группы
 */
const vector <string> Groups::getGroupNameByUser(const uid_t uid){
	// Результат работы функции
	vector <string> result;
	// Если идентификатор пользователя передан
	if(uid){
		// Переходим по списку групп
		for(auto it = this->data.cbegin(); it != this->data.cend(); ++it){
			// Получаем список пользователей
			auto users = it->second.users;
			// Переходим по списку пользователей
			for(auto ut = users.cbegin(); ut != users.cend(); ++ut){
				// Если идентификатор пользователя соответствует
				if(uid == *ut){
					// Добавляем название группы в список
					result.push_back(it->second.name);
					// Выходим из цикла
					break;
				}
			}
		}
	}
	// Выводим результат
	return result;
}
/**
 * getGroupNameByUser Метод получения название группы по имени пользователя
 * @param  userName название пользователя
 * @return          название группы
 */
const vector <string> Groups::getGroupNameByUser(const string userName){
	// Результат работы функции
	vector <string> result;
	// Если название пользователя передано
	if(!userName.empty()){
		// Получаем идентификатор пользователя
		const uid_t uid = getUidByName(userName);
		// Получаем название группы
		result = getGroupNameByUser(uid);
	}
	// Выводим результат
	return result;
}
/**
 * checkUser Метод проверки принадлежности пользователя к группе
 * @param  gid идентификатор группы
 * @param  uid идентификатор пользователя
 * @return     результат проверки
 */
const bool Groups::checkUser(const gid_t gid, const uid_t uid){
	// Результат проверки
	bool result = false;
	// Если идентификаторы пользователя и группы переданы
	if(gid && uid && this->data.count(gid)){
		// Получаем список пользователей
		auto users = this->data.find(gid)->second.users;
		// Переходим по списку пользователей
		for(auto it = users.cbegin(); it != users.cend(); ++it){
			// Если идентификатор пользователя найден
			if(uid == *it){
				// Запоминаем результат
				result = true;
				// Выходим из цикла
				break;
			}
		}
	// Если группа не найдена
	} else if(gid && uid && update()) {
		// Выполняем проверку пользователя
		result = checkUser(gid, uid);
	}
	// Выводим результат
	return result;
}
/**
 * checkUser Метод проверки принадлежности пользователя к группе
 * @param  gid      идентификатор группы
 * @param  userName название пользователя
 * @return          результат проверки
 */
const bool Groups::checkUser(const gid_t gid, const string userName){
	// Результат проверки
	bool result = false;
	// Если данные для проверки переданы
	if(gid && !userName.empty()){
		// Получаем идентификатор пользователя
		const uid_t uid = getUidByName(userName);
		// Проверяем принадлежность пользователя
		result = checkUser(gid, uid);
	}
	// Выводим результат
	return result;
}
/**
 * checkUser Метод проверки принадлежности пользователя к группе
 * @param  groupName название группы
 * @param  uid       идентификатор пользователя
 * @return           результат проверки
 */
const bool Groups::checkUser(const string groupName, const uid_t uid){
	// Результат проверки
	bool result = false;
	// Если данные для проверки переданы
	if(uid && !groupName.empty()){
		// Получаем идентификатор группы
		const gid_t gid = getIdByName(groupName);
		// Проверяем принадлежность пользователя
		result = checkUser(gid, uid);
	}
	// Выводим результат
	return result;
}
/**
 * checkUser Метод проверки принадлежности пользователя к группе
 * @param  groupName название группы
 * @param  userName  название пользователя
 * @return           результат проверки
 */
const bool Groups::checkUser(const string groupName, const string userName){
	// Результат проверки
	bool result = false;
	// Если названия пользователя и группы переданы
	if(!groupName.empty() && !userName.empty()){
		// Получаем идентификатор группы
		const gid_t gid = getIdByName(groupName);
		// Получаем идентификатор пользователя
		const uid_t uid = getUidByName(userName);
		// Проверяем принадлежность пользователя
		result = checkUser(gid, uid);
	}
	// Выводим результат
	return result;
}
/**
 * checkGroupById Метод проверки на существование группы
 * @param  gid идентификатор группы
 * @return     данные группы
 */
const bool Groups::checkGroupById(const gid_t gid){
	// Результат проверки
	bool result = false;
	// Если идентификатор группы передан
	if(gid && this->data.count(gid)){
		// Сообщаем что группа существует
		result = true;
	// Если группа не найдена
	} else if(gid && update()) {
		// Проверяем снова на существование группы
		result = checkGroupById(gid);
	}
	// Выводим результат
	return result;
}
/**
 * checkGroupByName Метод проверки на существование группы
 * @param  groupName название группы
 * @return           результат проверки
 */
const bool Groups::checkGroupByName(const string groupName){
	// Результат проверки
	bool result = false;
	// Если название группы передано
	if(!groupName.empty()){
		// Получаем идентификатор группы
		const gid_t gid = getIdByName(groupName);
		// Выводим результат
		result = checkGroupById(gid);
	}
	// Выводим результат
	return result;
}
/**
 * getUidByName Метод извлечения идентификатора пользователя по его имени
 * @param  userName название пользователя
 * @return          идентификатор пользователя
 */
const uid_t Groups::getUidByName(const string userName){
	// Результат работы функции
	uid_t result = -1;
	// Если идентификатор пользователя передан
	if(!userName.empty()){
		result = -1;
	}
	// Выводим результат
	return result;
}
/**
 * getIdByName Метод извлечения идентификатора группы по ее имени
 * @param  groupName название группы
 * @return           идентификатор группы
 */
const gid_t Groups::getIdByName(const string groupName){
	// Если идентификатор группы передан
	if(!groupName.empty() && !this->data.empty()){
		// Запоминаем полученное название группы
		string name = Anyks::toCase(groupName);
		// Переходим по всему списку групп
		for(auto it = this->data.cbegin(); it != this->data.cend(); ++it){
			// Сравниваем группы в базе
			if(it->second.name.compare(name) == 0){
				// Запоминаем полученный идентификатор
				return it->first;
			}
		}
	}
	// Выводим результат
	return -1;
}
/**
 * getUserNameByUid Метод извлечения имени пользователя по его идентификатору
 * @param  uid идентификатор пользователя
 * @return     название пользователя
 */
const string Groups::getUserNameByUid(const uid_t uid){
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
const string Groups::getNameById(const gid_t gid){
	// Если данные пользователей существуют
	if(gid && !this->data.empty()){
		// Если идентификатор группы передан
		if(this->data.count(gid)){
			// Получаем название группы
			return this->data.find(gid)->second.name;
		// Если группа не найдена
		} else if(update()) {
			// Получаем имя группы
			return getNameById(gid);
		}
	}
	// Выводим результат
	return string();
}
/**
 * getNameUsers Метод получения списка пользователей в группе
 * @param  gid идентификатор группы
 * @return     список имен пользователей
 */
const vector <string> Groups::getNameUsers(const gid_t gid){
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
	// Если группа не найдена
	} else if(gid && update()) {
		// Получаем список имен пользователей
		result = getNameUsers(gid);
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
		const gid_t gid = getIdByName(groupName);
		// Получаем список имен пользователей
		result = getNameUsers(gid);
	}
	// Выводим результат
	return result;
}
/**
 * getIdAllUsers Метод получения списка всех пользователей
 * @return список идентификаторов пользователей
 */
const vector <uid_t> Groups::getIdAllUsers(){
	// Результат работы функции
	vector <uid_t> result;
	// Получаем список групп
	auto groups = getAllGroups();
	// Если группы существуют то переходим по ним
	if(!groups.empty()){
		// Переходим по всем группам
		for(auto it = groups.cbegin(); it != groups.cend(); ++it){
			// Копируем весь список пользователей
			copy((* it)->users.cbegin(), (* it)->users.cend(), back_inserter(result));
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
 * getIdUsers Метод получения списка пользователей в группе
 * @param  gid идентификатор группы
 * @return     список идентификаторов пользователей
 */
const vector <uid_t> Groups::getIdUsers(const gid_t gid){
	// Результат работы функции
	vector <uid_t> result;
	// Если идентификатор группы передан
	if(gid && this->data.count(gid)){
		// Получаем список пользователей
		result = this->data.find(gid)->second.users;
	// Если группа не найдена
	} else if(gid && update()) {
		// Получаем список пользователей
		result = getIdUsers(gid);
	}
	// Выводим результат
	return result;
}
/**
 * getIdUsers Метод получения списка пользователей в группе
 * @param  groupName название группы
 * @return           список идентификаторов пользователей
 */
const vector <uid_t> Groups::getIdUsers(const string groupName){
	// Результат работы функции
	vector <uid_t> result;
	// Если идентификатор группы передан
	if(!groupName.empty()){
		// Получаем идентификатор группы
		const gid_t gid = getIdByName(groupName);
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
const bool Groups::addUser(const gid_t gid, const uid_t uid){
	// Результат работы функции
	bool result = false;
	// Если входящие параметры верные
	if(gid && uid && this->data.count(gid)){
		// Получаем данные группы
		this->data.find(gid)->second.users.push_back(uid);
		// Запоминаем что все удачно
		result = true;
	// Если группа не найдена
	} else if(gid && uid && update()) {
		// Пробуем еще раз
		result = addUser(gid, uid);
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
const bool Groups::addUser(const gid_t gid, const string userName){
	// Результат работы функции
	bool result = false;
	// Если входящие параметры верные
	if(gid && !userName.empty()){
		// Получаем идентификатор пользователя
		const uid_t uid = getUidByName(userName);
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
const bool Groups::addUser(const string groupName, const uid_t uid){
	// Результат работы функции
	bool result = false;
	// Если входящие параметры верные
	if(uid && !groupName.empty()){
		// Получаем идентификатор группы
		const gid_t gid = getIdByName(groupName);
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
		const gid_t gid = getIdByName(groupName);
		// Получаем идентификатор пользователя
		const uid_t uid = getUidByName(userName);
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
const bool Groups::addGroup(const gid_t id, const string name){
	// Результат работы функции
	bool result = false;
	// Если идентификатор и название переданы
	if(id && !name.empty()){
		// Создаем блок с данными группы
		Data group = createDefaultData(id, name);
		// Добавляем группу в список групп
		this->data.insert(pair <gid_t, Data>(group.id, group));
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
	if(config){
		// Запоминаем данные логов
		this->log = log;
		// Запоминаем конфигурационные данные
		this->config = config;
		// Запоминаем тип поиска групп пользователя
		this->typeSearch = 6;
		// Заполняем тип извлечения конфигурационных данных
		this->typeConfigs = 1;
		// Запоминаем время в течение которого запрещено обновлять данные
		this->maxUpdate = 600;
		// Максимальное количество групп пользователя для PAM
		this->maxPamGroupsUser = 100;
		// Устанавливаем параметры подключения LDAP
		this->ldap = {
			"ou=groups,dc=agro24,dc=dev",
			"ou=users,dc=agro24,dc=dev",
			"ou=configs,ou=aming,dc=agro24,dc=dev",
			"one",
			"one",
			"one",
			"(objectClass=posixGroup)",
			"(&(!(agro24CoJpDismissed=TRUE))(objectClass=inetOrgPerson))",
			"(&(amingConfigsGroupId=%g)(amingConfigsType=groups)(objectClass=amingConfigs))"
		};
		// Считываем данные групп
		update();
	}
}
