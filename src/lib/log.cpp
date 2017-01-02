/* МОДУЛЬ ЛОГОВ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2016
*/
#include "log.h"

// Устанавливаем область видимости
using namespace std;
/**
 * getOsName Функция определения операционной системы
 * @return название операционной системы
 */
const char * LogApp::getOsName(){
	#ifdef _WIN32
		return "Windows 32-bit";
	#elif _WIN64
		return "Windows 64-bit";
	#elif __unix || __unix__
		return "Unix";
	#elif __APPLE__ || __MACH__
		return "Mac OSX";
	#elif __linux__
		return "Linux";
	#elif __FreeBSD__
		return "FreeBSD";
	#else
		return "Other";
	#endif
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
		if(mkdir(path, mode) == 0) return true;
		// Если каталог не создан тогда сообщаем об ошибке
		else return false;
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
	string path = (this->dirlog + string("/") + this->name);
	// Проверяем существует ли нужный нам каталог
	if(!makePath(path.c_str())){
		// Сообщаем что каталог не может быть создан
		perror("Unable to create directory for log files");
		// Выходим из приложения
		exit(1);
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
		// Открываем файл на чтение в бинарном виде
		file = fopen(filename.c_str(), "rb");
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
		case 3: str = "\x1B[1mMessage\x1B[0m "; break;
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
		string str = (this->name + string(" [") + string(uk) + string("]: "));
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
 * @param appname     название приложения
 * @param name        пользовательское название
 * @param version     версия приложения
 * @param host        хост на котором поднято приложение
 * @param ipv4        активация IPv4
 * @param ipv6        активация IPv6
 * @param gzip_t      активация обмена сжатыми данными
 * @param gzip_r      сжимать полученные не сжатые данные
 * @param smart       активация умного прокси
 * @param keepalive   активация постоянных подключений
 * @param http        тип поднятого прокси
 * @param socks5      тип поднятого прокси
 * @param connect     активация коннект прокси
 * @param maxcon      максимальное количество подключений
 * @param http_port   порт http прокси
 * @param socks5_port порт socks5 прокси
 * @param copyright   копирайт автора прокси
 * @param site        сайт автора прокси
 * @param email       адрес электронной почты автора
 * @param support     адрес электронной почты службы поддержки
 * @param author      ник или имя автора
 */
void LogApp::welcome(
	const char * appname,
	const char * name,
	const char * version,
	const char * host,
	bool ipv4,
	bool ipv6,
	bool gzip_t,
	bool gzip_r,
	bool smart,
	bool keepalive,
	bool http,
	bool socks5,
	bool connect,
	int maxcon,
	u_int http_port,
	u_int socks5_port,
	const char * copyright,
	const char * site,
	const char * email,
	const char * support,
	const char * author
){
	// Если модуль активирован
	if(this->enabled){
		// Заполняем текстовые данные
		const char * _ipv4 = (ipv4 ? "yes" : "no");
		const char * _ipv6 = (ipv6 ? "yes" : "no");
		const char * _gzip_t = (gzip_t ? "yes" : "no");
		const char * _gzip_r = (gzip_r ? "yes" : "no");
		const char * _smart = (smart ? "yes" : "no");
		const char * _keepalive = (keepalive ? "yes" : "no");
		const char * _http = (http ? "yes" : "no");
		const char * _socks5 = (socks5 ? "yes" : "no");
		const char * _connect = (connect ? "yes" : "no");
		const char * _maxcon = (maxcon > -1 ? to_string(maxcon).c_str() : "auto");
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
		// Название прокси сервера
		string proxyname = appname;
		// Копирайт автора
		string copyauthor = (copyright + string(" - ") + year);
		// Переводим в указанный регистр
		transform(proxyname.begin(), proxyname.end(), proxyname.begin(), ::toupper);
		transform(copyauthor.begin(), copyauthor.end(), copyauthor.begin(), ::toupper);
		// Создаем буфер
		char buffer[1024 * 16];
		// Создаем формат вывода
		const char * format = "\n*\n*   "
		"WELCOME TO %s PROXY\n*\n*   Parameters proxy:\n*   "
		"name:             %s\n*   version:          %s\n"
		"*   IPv4:             %s\n*   IPv6:             %s\n"
		"*   gzip transfer:    %s\n*   gzip response:    %s\n"
		"*   max connect:      %s\n*   smart             %s\n"
		"*   keep-alive:       %s\n*   http:             %s\n"
		"*   socks5:           %s\n*   connect:          %s\n"
		"*   host:             %s\n*   http port:        %d\n"
		"*   socks5 port:      %d\n*   operating system: %s\n"
		"*   date start proxy: %s\n*\n*   Contact Developer:\n"
		"*   copyright:        %s\n*   site:             %s\n"
		"*   e-mail:           %s\n*   support:          %s\n"
		"*   author:           @%s\n*\n";
		// Записываем сообщение в буфер
		len = sprintf(
			buffer, format, proxyname.c_str(),
			name, version, _ipv4, _ipv6, _gzip_t,
			_gzip_r, _maxcon, _smart, _keepalive,
			_http, _socks5, _connect, host,
			http_port, socks5_port, getOsName(),
			date, copyauthor.c_str(), site,
			email, support, author
		);
		// Выводим в консоль сообщение
		cout << buffer << endl;
		// Записываем сообщение в лог
		write_to_file(3, buffer);
	}
}
/**
 * LogApp Конструктор log класса
 * @param type    тип логов (TOLOG_FILES - запись в файл, TOLOG_CONSOLE - запись в коносль, TOLOG_DATABASE - запись в базу данных)
 * @param name    название системы
 * @param dir     адрес куда следует сохранять логи
 * @param size    размер файла лога
 * @param enabled активирован модуль или деактивирован
 */
LogApp::LogApp(u_short type, const char * name, const char * dir, size_t size, bool enabled){
	// Запоминаем тип модуля
	this->type = type;
	// Запоминаем активирован модуль или нет
	this->enabled = enabled;
	// Запоминаем название системы
	this->name = name;
	// Запоминаем адрес хранения логов
	this->dirlog = dir;
	// Запоминаем размер файла лога, максимальный размер не может быть больше 100Мб
	this->size = (size <= 102400 ? size : 102400);
	// Переводим все в киллобайты
	this->size *= 1024;
}