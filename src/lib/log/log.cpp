/* МОДУЛЬ ЛОГОВ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#include "log/log.h"

// Устанавливаем область видимости
using namespace std;

/**
 * is_number Функция проверки является ли строка числом
 * @param  str строка для проверки
 * @return     результат проверки
 */
bool LogApp::isNumber(const string &str){
	return !str.empty() && find_if(str.begin(), str.end(), [](char c){
		return !isdigit(c);
	}) == str.end();
}
/**
 * getUid Функция вывода идентификатора пользователя
 * @param  name имя пользователя
 * @return      полученный идентификатор пользователя
 */
uid_t LogApp::getUid(const char * name){
	// Получаем идентификатор имени пользователя
	struct passwd * pwd = getpwnam(name);
	// Если идентификатор пользователя не найден
	if(pwd == NULL){
		// Выводим сообщение об ошибке
		printf("failed to get userId from username [%s]\r\n", name);
		// Выходим из приложения
		exit(EXIT_FAILURE);
	}
	// Выводим идентификатор пользователя
	return pwd->pw_uid;
}
/**
 * getGid Функция вывода идентификатора группы пользователя
 * @param  name название группы пользователя
 * @return      полученный идентификатор группы пользователя
 */
gid_t LogApp::getGid(const char * name){
	// Получаем идентификатор группы пользователя
	struct group * grp = getgrnam(name);
	// Если идентификатор группы не найден
	if(grp == NULL){
		// Выводим сообщение об ошибке
		printf("failed to get groupId from groupname [%s]\r\n", name);
		// Выходим из приложения
		exit(EXIT_FAILURE);
	}
	// Выводим идентификатор группы пользователя
	return grp->gr_gid;
}
/**
 * setOwner Функция установки владельца на каталог
 * @param path путь к файлу или каталогу для установки владельца
 */
void LogApp::setOwner(const char * path){
	uid_t uid;	// Идентификатор пользователя
	gid_t gid;	// Идентификатор группы
	// Размер строкового типа данных
	string::size_type sz;
	// Если идентификатор пользователя пришел в виде числа
	if(isNumber((* config)->proxy.user))
		// Получаем идентификатор пользователя
		uid = stoi((* config)->proxy.user, &sz);
	// Если идентификатор пользователя пришел в виде названия
	else uid = getUid((* config)->proxy.user.c_str());
	// Если идентификатор группы пришел в виде числа
	if(isNumber((* config)->proxy.group))
		// Получаем идентификатор группы пользователя
		gid = stoi((* config)->proxy.group, &sz);
	// Если идентификатор группы пришел в виде названия
	else gid = getGid((* config)->proxy.group.c_str());
	// Устанавливаем права на каталог
	chown(path, uid, gid);
}
/**
 * makePath Функция создания каталога для хранения логов
 * @param  path адрес для каталога
 * @return      результат создания каталога
 */
bool LogApp::makePath(const char * path){
	// Проверяем существует ли нужный нам каталог
	if(!isDirExist(path)){
		// Устанавливаем параметры каталога
		mode_t mode = 0755;
		// Создаем каталог
		if(mkdir(path, mode) == 0){
			// Устанавливаем права на каталог
			setOwner(path);
			// Сообщаем что все удачно
			return true;
		// Если каталог не создан тогда сообщаем об ошибке
		} else return false;
	}
	// Сообщаем что все создано удачно
	return true;
}
/**
 * isDirExist Функция проверки существования каталога
 * @param  path адрес каталога
 * @return      результат проверки
 */
bool LogApp::isDirExist(const char * path){
	// Структура проверка статистики
	struct stat info;
	// Проверяем переданный нам адрес
	if(stat(path, &info) != 0) return false;
	// Если это каталог
	return (info.st_mode & S_IFDIR) != 0;
}
/**
 * isFileExist Функция проверки существования файла
 * @param  path адрес каталога
 * @return      результат проверки
 */
bool LogApp::isFileExist(const char * path){
	// Структура проверка статистики
	struct stat info;
	// Проверяем переданный нам адрес
	if(stat(path, &info) != 0) return false;
	// Если это файл
	return (info.st_mode & S_IFMT) != 0;
}
/**
 * write_to_file Функция записи лога в файл
 * @param type    тип лога (1 - Ошибка, 2 - Доступ, 3 - Сообщение)
 * @param message сообщение для записи
 */
void LogApp::write_to_file(u_short type, const char * message){
	// Адрес каталога для хранения логов
	string path = ((* config)->logs.dir + string("/") + (* config)->proxy.name);
	// Проверяем существует ли нужный нам каталог
	if(!makePath(path.c_str())){
		// Сообщаем что каталог не может быть создан
		perror("Unable to create directory for log files");
		// Выходим из приложения
		exit(EXIT_FAILURE);
	}
	// Стартовая строка
	string filename;
	// Определяем тип сообщения
	switch(type){
		// Если это ошибка
		case 1: filename = (path + string("/") + "error.log"); break;
		// Если это доступ
		case 2: filename = (path + string("/") + "access.log"); break;
		// Если это сообщение
		case 3: filename = (path + string("/") + "message.log"); break;
	}
	// Файловый дескриптор
	FILE * file = NULL;
	// Проверяем существует ли файл лога
	if(isFileExist(filename.c_str())){
		// Устанавливаем права на файл лога
		setOwner(filename.c_str());
		// Открываем файл на чтение в бинарном виде
		file = fopen(filename.c_str(), "rb");
		// Если файл открыт
		if(file){
			// Перемещаемся в конец файла
			fseek(file, 0, SEEK_END);
			// Определяем размер файла
			size_t size = ftell(file);
			// Закрываем файл
			fclose(file);
			// Если размер файла больше максимального
			if(size >= this->size){
				// Создаем буфер для хранения даты
				char datefile[80];
				// Определяем количество секунд
				time_t seconds = time(NULL);
				// Получаем структуру локального времени
				struct tm * timeinfo = localtime(&seconds);
				// Создаем формат полученного времени
				string dateformat = "_%m-%d-%Y_%H-%M-%S";
				// Копируем в буфер полученную дату и время
				int datelen = strftime(datefile, sizeof(datefile), dateformat.c_str(), timeinfo);
				// Устанавливаем конец строки
				datefile[datelen] = '\0';
				// Строка чтения из файла
				string filedata;
				// Открываем файл на чтение
				ifstream logfile(filename.c_str());
				// Если файл открыт
				if(logfile.is_open()){
					// Создаем адрес сжатого файла
					string gzlogfile = filename;
					// Заменяем название файла
					gzlogfile.replace(gzlogfile.length() - 4, 4, string(datefile) + ".log.gz");
					// Открываем файл на сжатие
					gzFile gz = gzopen(gzlogfile.c_str(), "w6h");
					// Считываем до тех пор пока все удачно
					while(logfile.good()){
						// Считываем строку из файла
						getline(logfile, filedata);
						// Добавляем конец строки
						filedata += "\r\n";
						// Выполняем сжатие файла
						gzwrite(gz, filedata.c_str(), filedata.size());
					}
					// Закрываем сжатый файл
					gzclose(gz);
					// Закрываем файл
					logfile.close();
				}
				// Удаляем файл
				remove(filename.c_str());
			}
		// Выводим в консоль что файл не может быть прочитан
		} else write_to_console(LOG_ERROR, (string("cannot read log file, ") + filename).c_str());
	}
	// Открываем файл на запись
	file = fopen(filename.c_str(), "a");
	// Если файл открыт
	if(file){
		// Записываем в файл pid процесса
		fprintf(file, "%s\n", message);
		// Закрываем файл
		fclose(file);
	}
}
/**
 * write_to_console Функция записи лога в консоль
 * @param type    тип лога (1 - Ошибка, 2 - Доступ, 3 - Сообщение)
 * @param message сообщение для записи
 */
void LogApp::write_to_console(u_short type, const char * message){
	// Стартовая строка
	string str;
	// Определяем тип сообщения
	switch(type){
		// Если это ошибка
		case 1: str = "\x1B[31m\x1B[1mError\x1B[0m "; break;
		// Если это доступ
		case 2: str = "\x1B[32m\x1B[1mAccess\x1B[0m "; break;
		// Если это сообщение
		case 3: str = "\x1B[34m\x1B[1mMessage\x1B[0m "; break;
	}
	// Добавляем заголовок к сообщению
	str += message;
	// Выводим сообщение лога
	printf("\r\n%s\r\n\r\n%s\r\n\r\n%s\r\n", "*************** START ***************", str.c_str(), "---------------- END ----------------");
}
/**
 * write_to_db Функция записи лога в базу данных
 * @param type    тип лога (1 - Ошибка, 2 - Доступ, 3 - Сообщение)
 * @param message сообщение для записи
 */
void LogApp::write_to_db(u_short type, const char * message){
	cout << " DATABASE = " << message << endl;
}
/**
 * enable Метод активации модуля
 */
void LogApp::enable(){
	// Запоминаем что модуль активирован
	this->enabled = true;
}
/**
 * enable Метод деактивации модуля
 */
void LogApp::disable(){
	// Запоминаем что модуль деактивирован
	this->enabled = false;
}
/**
 * write Метод записи данных в лог
 * @param type    тип лога (1 - Ошибка, 2 - Доступ, 3 - Сообщение)
 * @param message текст сообщения
 * @param ...     дополнительные параметры
 */
void LogApp::write(u_short type, const char * message, ...){
	// Если модуль активирован
	if(this->enabled){
		// Создаем буфер
		char buffer[1024 * 16];
		// Получаем текущее время
		time_t seconds = time(NULL);
		// Получаем структуру времени
		struct tm * timeinfo = localtime(&seconds);
		// Получаем данные времени
		char * uk = asctime(timeinfo);
		// Перемещаем конец строки и убираем ненужный нам перенос на новую строку
		*(uk + 24) = 0;
		// Добавляем название системы
		string str = ((* config)->proxy.name + string(" [") + string(uk) + string("]: "));
		// Записываем сообщение в буфер
		int len = sprintf(buffer, "%s", str.c_str());
		// Создаем список аргументов
		va_list args;
		// Устанавливаем начальный список аргументов
		va_start(args, message);
		// Выполняем запись в буфер
		if((len = vsnprintf(buffer + len, sizeof(buffer), message, args)) > 0){
			// Устанавливаем конец строки
			buffer[strlen(buffer)] = '\0';
			// Выполняем запись в файл
			if(this->type & TOLOG_FILES) write_to_file(type, buffer);
			// Выполняем запись в консоль
			if(this->type & TOLOG_CONSOLE) write_to_console(type, buffer);
			// Выполняем запись в базу данных
			if(this->type & TOLOG_DATABASE) write_to_db(type, buffer);
		}
		// Завершаем список аргументов
		va_end(args);
	}
}
/**
 * welcome Функция выводящая приглашение
 */
void LogApp::welcome(){
	// Если модуль активирован
	if(this->enabled){
		// Заполняем текстовые данные
		const char * _gzipt = (OPT_GZIP & (* config)->options ? "yes" : "no");
		const char * _gzipr = (OPT_PGZIP & (* config)->options ? "yes" : "no");
		const char * _keepalive = (OPT_KEEPALIVE & (* config)->options ? "yes" : "no");
		const char * _connect = (OPT_CONNECT & (* config)->options ? "yes" : "no");
		const char * _headname = (OPT_AGENT & (* config)->options ? "yes" : "no");
		const char * _debug = ((* config)->proxy.debug ? "yes" : "no");
		const char * _daemon = ((* config)->proxy.daemon ? "yes" : "no");
		const char * _reverse = ((* config)->proxy.reverse ? "yes" : "no");
		const char * _forward = ((* config)->proxy.forward ? "yes" : "no");
		const char * _transfer = ((* config)->proxy.transfer ? "yes" : "no");
		const char * _deblock = ((* config)->proxy.deblock ? "yes" : "no");
		const char * _optimos = ((* config)->proxy.optimos ? "yes" : "no");
		const char * _bandlimin = ((* config)->firewall.bandlimin ? "yes" : "no");
		const char * _cache = ((* config)->cache.response ? "yes" : "no");
		const char * _allcon = ((* config)->connects.all > 0 ? to_string((* config)->connects.all).c_str() : "auto");
		// Данные прокси сервера
		string proxyname = APP_NAME, proxyver = APP_VERSION, proxytype, proxyskill;
		// Внешний и внутренний адрес сервера
		string internal, external;
		// Данные автора
		string	author = APP_AUTHOR,
				site = APP_SITE,
				email = APP_EMAIL,
				support = APP_SUPPORT,
				copyright = APP_COPYRIGHT;
		// Определяем тип прокси сервера
		switch((* config)->proxy.type){
			case 1: proxytype = "http";		break;
			case 2: proxytype = "socks5";	break;
			case 3: proxytype = "redirect";	break;
		}
		// Определяем скил прокси
		if(OPT_SMART & (* config)->options)
			proxyskill = "smart";
		else proxyskill = "dumb";
		// Определяем внешний и внутренний ip адрес
		switch((* config)->proxy.ipver){
			// Версия протокола IPv4
			case 4: {
				internal = (* config)->ipv4.internal;
				external = (* config)->ipv4.external;
			} break;
			// Версия протокола IPv6
			case 6: {
				internal = (* config)->ipv6.internal;
				external = (* config)->ipv6.external;
			} break;
		}
		// Создаем буфер для хранения даты
		char year[5], date[80];
		// Определяем количество секунд
		time_t seconds = time(NULL);
		// Получаем структуру локального времени
		struct tm * timeinfo = localtime(&seconds);
		// Создаем формат года
		string yearformat = "%Y";
		// Создаем формат полученного времени
		string dateformat = "%m/%d/%Y %H:%M:%S";
		// Копируем в буфер полученную дату и время
		int len = strftime(year, sizeof(year), yearformat.c_str(), timeinfo);
		// Устанавливаем конец строки
		year[len] = '\0';
		// Копируем в буфер полученную дату и время
		len = strftime(date, sizeof(date), dateformat.c_str(), timeinfo);
		// Устанавливаем конец строки
		date[len] = '\0';
		// Создаем период копирайта
		string cpyear = (::atoi(APP_YEAR) < ::atoi(year) ? string(APP_YEAR) + string(" - ") + year : string("- ") + year);
		// Заменяем копирайт
		copyright = (copyright + string(" ") + cpyear);
		// Переводим в указанный регистр
		transform(proxyname.begin(), proxyname.end(), proxyname.begin(), ::toupper);
		transform(copyright.begin(), copyright.end(), copyright.begin(), ::toupper);
		// Создаем буфер
		char buffer[1024 * 16];
		// Создаем формат вывода
		const char * format = "\n*\n"
		"*   WELCOME TO %s PROXY\n*\n*   Parameters proxy:\n"
		"*   name:                 %s\n*   version:              %s\n"
		"*   user:                 %s\n*   group:                %s\n"
		"*   daemon:               %s\n*   debug:                %s\n"
		"*   gzip transfer:        %s\n*   gzip response:        %s\n"
		"*   all connects:         %s\n*   max connect:          %i\n"
		"*   max sockets:          %i\n*   cache:                %s\n"
		"*   headname:             %s\n*   bandlimin:            %s\n"
		"*   deblock:              %s\n*   optimos:              %s\n"
		"*   keep-alive:           %s\n*   reverse:              %s\n"
		"*   forward:              %s\n*   transfer:             %s\n"
		"*   connect:              %s\n*   type:                 %s\n"
		"*   internet protocol:    IPv%i\n*   skill:                %s\n"
		"*   internal ip:          %s\n*   external ip:          %s\n"
		"*   port:                 %i\n*   cpu cores:            %i\n"
		"*   cpu name:             %s\n*   operating system:     %s\n"
		"*   date start proxy:     %s\n*\n*   Contact Developer:\n"
		"*   copyright:            %s\n*   site:                 %s\n"
		"*   e-mail:               %s\n*   support:              %s\n"
		"*   author:               @%s\n*\n";
		// Записываем сообщение в буфер
		len = sprintf(
			buffer, format, proxyname.c_str(),
			(* config)->proxy.name.c_str(),
			proxyver.c_str(),
			(* config)->proxy.user.c_str(),
			(* config)->proxy.group.c_str(),
			_daemon, _debug, _gzipt, _gzipr,
			_allcon, (* config)->connects.max,
			(* config)->connects.fds, _cache,
			_headname, _bandlimin, _deblock,
			_optimos, _keepalive, _reverse,
			_forward, _transfer, _connect,
			proxytype.c_str(), (* config)->proxy.ipver,
			proxyskill.c_str(), internal.c_str(),
			external.c_str(), (* config)->proxy.port,
			(* config)->os.ncpu, (* config)->os.cpu.c_str(),
			(* config)->os.name.c_str(), date, copyright.c_str(),
			site.c_str(), email.c_str(), support.c_str(), author.c_str()
		);
		// Выводим в консоль сообщение
		cout << buffer << endl;
		// Записываем сообщение в лог
		write_to_file(3, buffer);
	}
}
/**
 * LogApp Конструктор log класса
 * @param config  конфигурационные данные
 * @param type    тип логов (TOLOG_FILES - запись в файл, TOLOG_CONSOLE - запись в коносль, TOLOG_DATABASE - запись в базу данных)
 */
LogApp::LogApp(Config ** config, u_short type){
	// Если конфигурационные данные существуют
	if(*config != NULL){
		// Запоминаем конфигурационные данные
		this->config = config;
		// Запоминаем тип модуля
		this->type = type;
		// Запоминаем активирован или деактивирован модуль
		this->enabled = (* config)->logs.enabled;
		// Запоминаем размер файла лога, максимальный размер не может быть больше 100Мб
		this->size = ((* config)->logs.size <= 102400 ? (* config)->logs.size : 102400);
		// Переводим все в киллобайты
		this->size *= 1024;
	}
}