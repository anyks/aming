/**
 * Config Конструктор модуля конфигурационного файла
 * @param filename адрес конфигурационного файла
 */
Config::Config(const string & filename){
	// Запоминаем адрес файла с конфигурационным файлом
	this->filename = filename;
	// Инициализируем парсер ini файла
	INI ini(this->filename);
	// Если во время чтения файла возникли ошибки
	if(ini.ParseError() < 0){
		// Выводим сообщение об ошибке
		printf("Can't load '%s' file\n", this->filename.c_str());
		// Выходим из приложения
		exit(0);
	// Если все нормально то выполняем извлечение данных из конфигурационного файла
	} else {
		// Активируем разрешение connect прокси сервера
		this->options = (ini.GetBoolean("proxy", "connect", true) ? OPT_CONNECT : OPT_NULL);
		// Активируем вывод названия прокси сервера
		this->options = (this->options | (ini.GetBoolean("proxy", "headname", true) ? OPT_AGENT : OPT_NULL));
		// Активируем разрешение сжатия данных методом gzip
		this->options = (this->options | (ini.GetBoolean("gzip", "transfer", true) ? OPT_GZIP : OPT_NULL));
		// Активируем сжатие не сжатых данных
		this->options = (this->options | (ini.GetBoolean("gzip", "response", true) ? OPT_PGZIP : OPT_NULL));
		// Активируем разрешающий smart прокси сервер
		this->options = (this->options | ((ini.Get("proxy", "skill", "dumb").compare("smart") == 0) ? OPT_SMART : OPT_NULL));
		// Активируем постоянное соединение
		this->options = (this->options | (ini.GetBoolean("proxy", "keepalive", true) ? OPT_KEEPALIVE : OPT_NULL));
		// Активируем логирование данных
		this->options = (this->options | (ini.GetBoolean("logs", "enabled", true) ? OPT_LOG : OPT_NULL));
		// Тип прокси сервера
		u_int proxy_type, proxy_port, proxy_ipv;
		// Получаем версию протокола интернета
		proxy_ipv = ini.GetInteger("proxy", "ipver", PROXY_IPV);
		// Если версия меньше 4 или больше 6 то устанавливаем версию по умолчанию
		proxy_ipv = ((proxy_ipv < 4) || (proxy_ipv > 6) ? PROXY_IPV : proxy_ipv);
		// Получаем тип прокси сервера
		string type = ini.Get("proxy", "type", PROXY_TYPE);
		// Если прокси сервер это http
		if(type.compare("http") == 0) proxy_type = 1;
		else if(type.compare("socks5") == 0) proxy_type = 2;
		else if(type.compare("redirect") == 0) proxy_type = 3;
		// Определяем порт прокси сервера
		switch(proxy_type){
			// Порт http прокси
			case 1: proxy_port = PROXY_HTTP_PORT;		break;
			// Порт socks5 прокси
			case 2: proxy_port = PROXY_SOCKS5_PORT;		break;
			// Порт redirect прокси
			case 3: proxy_port = PROXY_REDIRECT_PORT;	break;
		}
		// Заполняем структуру proxy
		this->proxy = {
			// Версия протокола интернета (4, 6)
			proxy_ipv,
			// Тип прокси сервера (http, socks5, redirect)
			proxy_type,
			// Устанавливаем порт прокси сервера
			ini.GetInteger("proxy", "port", proxy_port),
			// Максимальное количество подключений (одного клиента к прокси серверу)
			ini.GetInteger("proxy", "maxcon", MAX_CONNECTS),
			// Максимальное количество файловых дескрипторов
			ini.GetInteger("proxy", "maxfds", MAX_FDS),
			// Активация режима отладки
			ini.GetBoolean("proxy", "debug", PROXY_DEBUG),
			// Запусть в виде демона
			ini.GetBoolean("proxy", "daemon", PROXY_DAEMON),
			// Обратный прокси (доступ из сети в локальную сеть)
			ini.GetBoolean("proxy", "reverse", PROXY_REVERSE),
			// Активация поддержки прокси листа
			ini.GetBoolean("proxy", "transfer", PROXY_TRANSFER),
			// Прямой прокси (доступ во внешнюю сеть)
			ini.GetBoolean("proxy", "forward", PROXY_FORWARD),
			// Попробовать обойти блокировки сайтов на уровне прокси (многие сайты могут работать не правильно)
			ini.GetBoolean("proxy", "deblock", PROXY_DEDLOCK),
			// Оптимизировать настройки операционной системы (нужен root доступ)
			ini.GetBoolean("proxy", "optimos", PROXY_OPTIMOS),
			// Активировать ограничение трафика пользователей
			ini.GetBoolean("proxy", "bandlimin", PROXY_BANDLIMIN),
			// Активировать кеширование часто-запрашиваемых страниц
			ini.GetBoolean("proxy", "cache", PROXY_CACHE),
			// Идентификатор группы пользователя под которым запускается прокси
			ini.Get("proxy", "user", PROXY_USER),
			// Идентификатор пользователя под которым запускается прокси
			ini.Get("proxy", "group", PROXY_GROUP),
			// Название сервиса
			ini.Get("proxy", "name", PROXY_NAME),
			// Адрес хранения pid файла
			ini.Get("proxy", "piddir", PID_DIR),
			// Адрес хранения конфигурационных файлов
			ini.Get("proxy", "confdir", CONFIG_DIR),
		};
		// Заполняем структуру bloking
		this->bloking = {
			// Количество неудачных попыток авторизации
			ini.GetInteger("blocking", "maxtryauth", BLOCK_MAX_TRYAUTH),
			// Разрешить блокирование пользователя после неудачных попыток авторизации
			ini.GetBoolean("blocking", "blockauth", BLOCK_AUTH),
			// Разрешить блокирование зацикливающих запросов
			ini.GetBoolean("blocking", "blockloop", BLOCK_LOOP),
			// Время блокирования зацикливающих запросов (s - секунды, m - минуты, h - часы, d - дни, M - месяцы, y - годы)
			ini.Get("blocking", "timeblockloop", BLOCK_TIME_LOOP)
		};
		// Заполняем структуру listsites
		this->listsites = {
			// Активировать черный список сайтов или портов
			ini.GetBoolean("listsites", "blacklist", SITES_BLACK_LIST),
			// Активировать белый список сайтов или портов
			ini.GetBoolean("listsites", "whitelist", SITES_WHITE_LIST)
		};
		// Заполняем структуру authorization
		this->authorization = {
			// Активировать авторизацию через пользователей в операционной системе
			ini.GetBoolean("authorization", "osusers", AUTH_OS_USERS),
			// Активировать авторизацию через файл со списком пользователей и ip/mac адресами
			ini.GetBoolean("authorization", "listusers", AUTH_FILE_USERS),
			// Без авторизации
			ini.GetBoolean("authorization", "noauth", AUTH_NO)
		};
		// Заполняем структуру logs
		this->logs = {
			// Разрешить хранить логи в файлах
			ini.GetBoolean("logs", "files", LOGS_FILES),
			// Разрешить ведение логов
			ini.GetBoolean("logs", "enabled", LOGS_ENABLED),
			// Адрес хранения логов
			ini.Get("logs", "dir", LOGS_DIR)
		};
		// Заполняем структуру ipv4
		this->ipv4 = {
			// Внешний интерфейс, через который будут уходить запросы от сервера
			ini.Get("ipv4", "external", IPV4_EXTERNAL),
			// IP адрес интерфейса на котором будут приниматься запросы от клиентов
			ini.Get("ipv4", "internal", IPV4_INTERNAL)
		};
		// Заполняем структуру ipv6
		this->ipv4 = {
			// Внешний интерфейс, через который будут уходить запросы от сервера
			ini.Get("ipv6", "external", IPV6_EXTERNAL),
			// IP адрес интерфейса на котором будут приниматься запросы от клиентов
			ini.Get("ipv6", "internal", IPV6_INTERNAL)
		};
	}
}