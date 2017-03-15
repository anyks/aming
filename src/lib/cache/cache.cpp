/* МОДУЛЬ УПРАВЛЕНИЯ КЭШЕМ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#include "cache/cache.h"

// Устанавливаем область видимости
using namespace std;

/**
 * getPathDomain Метод создания пути из доменного имени
 * @param  domain название домена
 * @return        путь к файлу кэша
 */
const string Cache::getPathDomain(const string domain){
	// Результирующий адрес
	string result;
	// Если параметры переданы
	if(!domain.empty()){
		// Результат работы регулярного выражения
		smatch match;
		// Устанавливаем правило регулярного выражения
		regex e("[\\w\\-\\.]+\\.[\\w\\-]+", regex::ECMAScript | regex::icase);
		// Выполняем проверку
		regex_search(domain, match, e);
		// Если домен существует
		if(!match.empty()){
			// Формируем регулярное выражение
			regex e("\\.");
			// Запоминаем результат
			result = match[0].str();
			// Формируем результирующий адрес
			result = regex_replace(result, e, "/");
			// Выполняем реверс строки
			reverse(result.begin(), result.end());
		}
	}
	// Выводим результат
	return result;
}
/**
 * toCase Функция перевода в указанный регистр
 * @param  str  строка для перевода в указанных регистр
 * @param  flag флаг указания типа регистра
 * @return      результирующая строка
 */
const string Cache::toCase(string str, bool flag){
	// Переводим в указанный регистр
	transform(str.begin(), str.end(), str.begin(), (flag ? ::toupper : ::tolower));
	// Выводим результат
	return str;
}
/**
 * rtrim Функция усечения указанных символов с правой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & Cache::rtrim(string &str, const char * t){
	str.erase(str.find_last_not_of(t) + 1);
	return str;
}
/**
 * ltrim Функция усечения указанных символов с левой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & Cache::ltrim(string &str, const char * t){
	str.erase(0, str.find_first_not_of(t));
	return str;
}
/**
 * trim Функция усечения указанных символов с правой и левой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & Cache::trim(string &str, const char * t){
	return ltrim(rtrim(str, t), t);
}
/**
 * split Метод разбива строки на составляющие
 * @param  str   исходная строка
 * @param  delim разделитель
 * @return       массив составляющих строки
 */
vector <string> Cache::split(const string str, const string delim){
	// Результат данных
	vector <string> result;
	// Создаем новую строку
	string value = str;
	// Убираем пробелы в строке
	value = trim(value);
	// Если строка передана
	if(!value.empty()){
		string data;
		string::size_type i = 0;
		string::size_type j = value.find(delim);
		u_int len = delim.length();
		// Выполняем разбиение строк
		while(j != string::npos){
			data = value.substr(i, j - i);
			result.push_back(trim(data));
			i = ++j + (len - 1);
			j = value.find(delim, j);
			if(j == string::npos){
				data = value.substr(i, value.length());
				result.push_back(trim(data));
			}
		}
		// Если данные не существуют то устанавливаем строку по умолчанию
		if(result.empty()) result.push_back(value);
	}
	// Выводим результат
	return result;
}
/**
 * addToPath Метод формирования адреса из пути и названия файла
 * @param  path путь где хранится файл
 * @param  file название файла
 * @return      сформированный путь
 */
const string Cache::addToPath(const string path, const string file){
	// Результирующий адрес
	string result;
	// Если параметры переданы
	if(!path.empty() && !file.empty()){
		// Формируем регулярное выражение
		regex pe("\\/+$"), fe("^[\\/\\.\\~]+");
		// Формируем результирующий адрес
		result = (regex_replace(path, pe, "") + string("/") + regex_replace(file, fe, ""));
	}
	// Выводим результат
	return result;
}
/**
 * getUid Функция вывода идентификатора пользователя
 * @param  name имя пользователя
 * @return      полученный идентификатор пользователя
 */
uid_t Cache::getUid(const char * name){
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
gid_t Cache::getGid(const char * name){
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
void Cache::setOwner(const char * path){
	uid_t uid;	// Идентификатор пользователя
	gid_t gid;	// Идентификатор группы
	// Размер строкового типа данных
	string::size_type sz;
	// Если идентификатор пользователя пришел в виде числа
	if(isNumber((* this->config)->proxy.user))
		// Получаем идентификатор пользователя
		uid = stoi((* this->config)->proxy.user, &sz);
	// Если идентификатор пользователя пришел в виде названия
	else uid = getUid((* this->config)->proxy.user.c_str());
	// Если идентификатор группы пришел в виде числа
	if(isNumber((* this->config)->proxy.group))
		// Получаем идентификатор группы пользователя
		gid = stoi((* this->config)->proxy.group, &sz);
	// Если идентификатор группы пришел в виде названия
	else gid = getGid((* this->config)->proxy.group.c_str());
	// Устанавливаем права на каталог
	chown(path, uid, gid);
}
/**
 * readDomain Метод чтения данных домена из файла
 * @param domain название домена
 * @param data   указатель на данные домена
 */
void Cache::readDomain(const string domain, DataDNS * data){
	// Получаем данные каталога где хранится кэш
	string dir = (* this->config)->cache.dir;
	// Получаем имя файла
	dir = addToPath(dir, "dns");
	// Добавляем основной путь
	dir = addToPath(dir, getPathDomain(domain));
	// Создаем адрес для хранения файла
	const string filename = addToPath(dir, "domain");
	// Проверяем на существование адреса
	if(!filename.empty() && isFileExist(filename.c_str())){
		// Открываем файл на чтение
		FILE * file = fopen(filename.c_str(), "rb");
		// Если файл открыт
		if(file){
			// Считываем из файла данные домена
			fread(data, sizeof(DataDNS), 1, file);
			// Закрываем файл
			fclose(file);
		// Выводим сообщение в лог
		} else this->log->write(LOG_ERROR, 0, "cannot read dns cache file %s for domain %s", filename.c_str(), domain.c_str());
	}
}
/**
 * writeDomain Метод записи данных домена в файл
 * @param domain название домена
 * @param data   данные домена
 */
void Cache::writeDomain(const string domain, DataDNS data){
	// Получаем данные каталога где хранится кэш
	string dir = (* this->config)->cache.dir;
	// Получаем имя файла
	dir = addToPath(dir, "dns");
	// Добавляем основной путь
	dir = addToPath(dir, getPathDomain(domain));
	// Создаем адрес для хранения файла
	const string filename = addToPath(dir, "domain");
	// Проверяем на существование адреса
	if(!filename.empty()){
		// Проверяем существует ли нужный нам каталог
		if(!makePath(dir.c_str())){
			// Выводим в лог информацию
			this->log->write(LOG_ERROR, 0, "unable to create directory for dns cache file %s for domain %s", dir.c_str(), domain.c_str());
			// Выходим
			return;
		}
		// Открываем файл на запись
		FILE * file = fopen(filename.c_str(), "wb");
		// Если файл открыт
		if(file){
			// Записываем в файл данные домена
			fwrite(&data, sizeof(DataDNS), 1, file);
			// Закрываем файл
			fclose(file);
		// Выводим сообщение в лог
		} else this->log->write(LOG_ERROR, 0, "cannot write dns cache file %s for domain %s", filename.c_str(), domain.c_str());
	}
}
/**
 * mkdir Метод рекурсивного создания каталогов
 * @param path адрес каталогов
 */
void Cache::mkdir(const char * path){
	// Буфер с названием каталога
	char tmp[256];
	// Указатель на сепаратор
	char * p = NULL;
	// Копируем переданный адрес в буфер
	snprintf(tmp, sizeof(tmp), "%s", path);
	// Определяем размер адреса
	size_t len = strlen(tmp);
	// Если последний символ является сепаратором тогда удаляем его
	if(tmp[len - 1] == '/') tmp[len - 1] = 0;
	// Переходим по всем символам
	for(p = tmp + 1; * p; p++){
		// Если найден сепаратор
		if(* p == '/'){
			// Сбрасываем указатель
			* p = 0;
			// Создаем каталог
			::mkdir(tmp, S_IRWXU);
			// Запоминаем сепаратор
			* p = '/';
		}
	}
	// Создаем последний каталог
	::mkdir(tmp, S_IRWXU);
}
/**
 * rmdir Метод удаления каталога и всего содержимого
 * @param  path путь до каталога
 * @return      количество дочерних элементов
 */
int Cache::rmdir(const char * path){
	// Открываем указанный каталог
	DIR * d = opendir(path);
	// Получаем длину адреса
	size_t path_len = strlen(path);
	// Количество дочерних элементов
	int r = -1;
	// Если каталог открыт
	if(d){
		// Создаем указатель на содержимое каталога
		struct dirent * p;
		// Устанавливаем количество дочерних элементов
		r = 0;
		// Выполняем чтение содержимого каталога
		while(!r && (p = readdir(d))){
			// Количество найденных элементов
			int r2 = -1;
			// Пропускаем названия текущие "." и внешние "..", так как идет рекурсия
			if(!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) continue;
			// Получаем размер дочернего каталога
			size_t len = path_len + strlen(p->d_name) + 2;
			// Создаем буфер данных
			char * buf = new char [(const size_t) len];
			// Если память выделена
			if(buf){
				// Создаем структуру буфера статистики
				struct stat statbuf;
				// Копируем в буфер название дочернего элемента
				snprintf(buf, len, "%s/%s", path, p->d_name);
				// Если статистика извлечена
				if(!stat(buf, &statbuf)){
					// Если дочерний элемент является дирректорией
					if(S_ISDIR(statbuf.st_mode)) r2 = rmdir(buf);
					// Если дочерний элемент является файлом то удаляем его
					else r2 = ::unlink(buf);
				}
				// Освобождаем выделенную ранее память
				delete [] buf;
			}
			// Запоминаем количество дочерних элементов
			r = r2;
		}
		// Закрываем открытый каталог
		closedir(d);
	}
	// Удаляем последний каталог
	if(!r) r = ::rmdir(path);
	// Выводим результат
	return r;
}
/**
 * makePath Функция создания каталога для хранения логов
 * @param  path адрес для каталога
 * @return      результат создания каталога
 */
bool Cache::makePath(const char * path){
	// Проверяем существует ли нужный нам каталог
	if(!isDirExist(path)){
		// Создаем каталог
		mkdir(path);
		// Устанавливаем права на каталог
		setOwner(path);
		// Сообщаем что все удачно
		return true;
	}
	// Сообщаем что все создано удачно
	return true;
}
/**
 * is_number Функция проверки является ли строка числом
 * @param  str строка для проверки
 * @return     результат проверки
 */
bool Cache::isNumber(const string &str){
	return !str.empty() && find_if(str.begin(), str.end(), [](char c){
		return !isdigit(c);
	}) == str.end();
}
/**
 * isDirExist Функция проверки существования каталога
 * @param  path адрес каталога
 * @return      результат проверки
 */
bool Cache::isDirExist(const char * path){
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
bool Cache::isFileExist(const char * path){
	// Структура проверка статистики
	struct stat info;
	// Проверяем переданный нам адрес
	if(stat(path, &info) != 0) return false;
	// Если это файл
	return (info.st_mode & S_IFMT) != 0;
}
/**
 * isDomain Метод проверки на доменное имя
 * @param  domain строка названия домена для проверки
 * @return        результат проверки
 */
bool Cache::isDomain(const string domain){
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e("[\\w\\-\\.]+\\.[\\w\\-]+", regex::ECMAScript | regex::icase);
	// Выполняем проверку
	regex_search(domain, match, e);
	// Выводим результат
	return !match.empty();
}
/**
 * isIpV4 Метод проверки на ip адрес, интернет протокола версии 4
 * @param  ip строка ip адреса для проверки
 * @return    результат проверки
 */
bool Cache::isIpV4(const string ip){
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e("\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}", regex::ECMAScript | regex::icase);
	// Выполняем проверку
	regex_search(ip, match, e);
	// Выводим результат
	return !match.empty();
}
/**
 * isIpV6 Метод проверки на ip адрес, интернет протокола версии 6
 * @param  ip строка ip адреса для проверки
 * @return    результат проверки
 */
bool Cache::isIpV6(const string ip){
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e("[A-Fa-f\\d]{4}\\:[A-Fa-f\\d]{4}\\:[A-Fa-f\\d]{4}\\:[A-Fa-f\\d]{4}\\:[A-Fa-f\\d]{4}\\:[A-Fa-f\\d]{4}\\:[A-Fa-f\\d]{4}\\:[A-Fa-f\\d]{4}", regex::ECMAScript | regex::icase);
	// Выполняем проверку
	regex_search(ip, match, e);
	// Выводим результат
	return !match.empty();
}
/**
 * getDomain Метод получения ip адреса домена
 * @param  domain название домена
 * @return        ip адрес домена
 */
const string Cache::getDomain(const string domain){
	// Результат полученных данных
	string result;
	// Если данные домена переданы
	if(!domain.empty() && isDomain(domain)){
		// Создаем объект данных
		DataDNS data;
		// Считываем данные домена
		readDomain(domain, &data);
		// Получаем текущее количество секунд
		time_t seconds = time(NULL);
		// Если время жизни не истекло тогда отдаем результат
		if((data.ttl + (* this->config)->cache.dttl) > seconds){
			// Создаем объект сети
			Network nwk;
			// Определяем тип подключения
			switch((* this->config)->proxy.extIPv){
				// Для протокола IPv4
				case 4: result = nwk.getLowIp(data.ipv4);	break;
				// Для протокола IPv6
				case 6: result = nwk.getLowIp6(data.ipv6);	break;
			}
		}
	}
	// Выводим результат
	return result;
}
/**
 * setDomain Метод записи домена в кэш
 * @param  domain название домена
 * @param  ip     ip адрес домена
 */
void Cache::setDomain(const string domain, const string ip){
	// Если данные домена и ip адреса переданы
	if(!domain.empty() && !ip.empty() && isDomain(domain)){
		// Создаем объект сети
		Network nwk;
		// Создаем объект данных
		DataDNS data;
		// Считываем данные домена
		readDomain(domain, &data);
		// Определяем тип подключения
		switch((* this->config)->proxy.extIPv){
			// Для протокола IPv4
			case 4: {
				// Создаем ip адрес
				const string ipv4 = nwk.setLowIp(ip);
				// Проверяем является ли адрес IPv4
				if(isIpV4(ipv4)) data.ipv4 = ipv4;
			} break;
			// Для протокола IPv6
			case 6: {
				// Создаем ip адрес
				const string ipv6 = nwk.setLowIp6(ip);
				// Проверяем является ли адрес IPv6
				if(isIpV6(ipv6)) data.ipv6 = ipv6;
			} break;
		}
		// Определяем количество секунд
		data.ttl = time(NULL);
		// Выполняем запись домена в кэш
		writeDomain(domain, data);
	}
}
/**
 * rmDomain Метод удаления домена из кэша
 * @param domain название домена
 */
void Cache::rmDomain(const string domain){
	// Получаем данные каталога где хранится кэш
	string dir = (* this->config)->cache.dir;
	// Получаем имя файла
	dir = addToPath(dir, "dns");
	// Добавляем основной путь
	dir = addToPath(dir, getPathDomain(domain));
	// Проверяем на существование адреса, если существует то удаляем
	if(!dir.empty() && isDirExist(dir.c_str())) rmdir(dir.c_str());
}
/**
 * rmAddDomains Метод удаления всех доменов из кэша
 */
void Cache::rmAddDomains(){
	// Получаем данные каталога где хранится кэш
	string dir = (* this->config)->cache.dir;
	// Получаем имя файла
	dir = addToPath(dir, "dns");
	// Проверяем на существование адреса, если существует то удаляем
	if(!dir.empty() && isDirExist(dir.c_str())) rmdir(dir.c_str());
}
/**
 * Cache Конструктор
 * @param log    объект лога для вывода информации
 * @param config конфигурационные данные
 */
Cache::Cache(LogApp * log, Config ** config){
	// Запоминаем объект логов
	this->log = log;
	// Запоминаем параметры конфига
	this->config = config;
}