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
 * timeToStr Метод преобразования timestamp в строку
 * @param  date дата в timestamp
 * @return      строка содержащая дату
 */
const string Cache::timeToStr(const time_t date){
	// Создаем структуру времени
	struct tm * tm = localtime(&date);
	// Буфер с данными
	char buf[255];
	// Зануляем структуру
	memset(tm, 0, sizeof(struct tm));
	// Выполняем парсинг даты
	strftime(buf, sizeof(buf), "%A, %d %b %Y %H:%M:%S %Z", tm);
	// Выводим результат
	return string(buf);
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
const uid_t Cache::getUid(const char * name){
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
const gid_t Cache::getGid(const char * name){
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
	if(isNumber(this->config->proxy.user))
		// Получаем идентификатор пользователя
		uid = stoi(this->config->proxy.user, &sz);
	// Если идентификатор пользователя пришел в виде названия
	else uid = getUid(this->config->proxy.user.c_str());
	// Если идентификатор группы пришел в виде числа
	if(isNumber(this->config->proxy.group))
		// Получаем идентификатор группы пользователя
		gid = stoi(this->config->proxy.group, &sz);
	// Если идентификатор группы пришел в виде названия
	else gid = getGid(this->config->proxy.group.c_str());
	// Устанавливаем права на каталог
	chown(path, uid, gid);
}
/**
 * readDomain Метод чтения данных домена из файла
 * @param domain название домена
 * @param data   указатель на данные домена
 */
void Cache::readDomain(const string domain, DataDNS * data){
	// Если кэширование разрешено
	if(this->config->cache.dns && data){
		// Получаем данные каталога где хранится кэш
		string dir = this->config->cache.dir;
		// Получаем имя файла
		dir = addToPath(dir, "dns");
		// Добавляем основной путь
		dir = addToPath(dir, getPathDomain(domain));
		// Создаем адрес для хранения файла
		const string filename = addToPath(dir, "data");
		// Проверяем на существование адреса
		if(!filename.empty() && isFileExist(filename.c_str())){
			// Открываем файл на чтение
			FILE * file = fopen(filename.c_str(), "rb");
			// Если файл открыт
			if(file){
				// Создаем объект карты данных
				CacheDNSMap map;
				// Время жизни кэша
				size_t ttl = 0;
				// Данные ip адресов
				char ipv4[256], ipv6[256];
				// Зануляем буферы
				memset(ipv4, 0, sizeof(ipv4));
				memset(ipv6, 0, sizeof(ipv6));
				// Смещаем на начальную позицию
				fseek(file, 0L, SEEK_SET);
				// Считываем значение размеров
				size_t size = fread(&map, sizeof(map), 1, file);
				// Если данные прочитаны
				if(size){
					// Смещаем на следующую позицию
					fseek(file, size - 1, SEEK_CUR);
					// Считываем время жизни кэша ttl
					size = fread(&ttl, map.ttl, 1, file);
					// Если размер данных существует
					if(map.ipv4){
						// Смещаем на следующую позицию
						fseek(file, size - 1, SEEK_CUR);
						// Считываем адрес IPv4
						size = fread(ipv4, map.ipv4, 1, file);
					}
					// Если размер данных существует
					if(map.ipv6){
						// Смещаем на следующую позицию
						fseek(file, size - 1, SEEK_CUR);
						// Считываем адрес IPv4
						size = fread(ipv6, map.ipv6, 1, file);
					}
				}
				// Закрываем файл
				fclose(file);
				// Запоминаем полученные данные
				data->ttl = ttl;
				// Запоминаем данные IP адресов
				data->ipv4.assign(ipv4, map.ipv4);
				data->ipv6.assign(ipv6, map.ipv6);
			// Выводим сообщение в лог
			} else this->log->write(LOG_ERROR, 0, "cannot read dns cache file %s for domain %s", filename.c_str(), domain.c_str());
		}
	}
}
/**
 * readCache Метод чтения данных из файла кэша
 * @param domain название домена
 * @param name   название запроса
 * @param data   данные запроса
 */
void Cache::readCache(const string domain, const string name, DataCache * data){
	// Если кэширование разрешено
	if(this->config->cache.response){
		// Получаем данные каталога где хранится кэш
		string dir = this->config->cache.dir;
		// Получаем имя файла
		dir = addToPath(dir, "cache");
		// Добавляем основной путь
		dir = addToPath(dir, getPathDomain(domain));
		// Создаем адрес для хранения файла
		const string filename = addToPath(dir, md5(name));
		// Проверяем на существование адреса
		if(!filename.empty() && isFileExist(filename.c_str())){
			// Открываем файл на чтение
			FILE * file = fopen(filename.c_str(), "rb");
			// Если файл открыт
			if(file){
				// Считываем из файла данные домена
				fread(data, sizeof(DataCache), 1, file);
				// Закрываем файл
				fclose(file);
			// Выводим сообщение в лог
			} else this->log->write(LOG_ERROR, 0, "cannot read dns cache file %s for domain %s", filename.c_str(), domain.c_str());
		}
	}
}
/**
 * writeDomain Метод записи данных домена в файл
 * @param domain название домена
 * @param data   данные домена
 */
void Cache::writeDomain(const string domain, DataDNS data){
	// Если кэширование разрешено
	if(this->config->cache.dns){
		// Получаем данные каталога где хранится кэш
		string dir = this->config->cache.dir;
		// Получаем имя файла
		dir = addToPath(dir, "dns");
		// Добавляем основной путь
		dir = addToPath(dir, getPathDomain(domain));
		// Создаем адрес для хранения файла
		const string filename = addToPath(dir, "data");
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
				// Создаем объект карты данных
				CacheDNSMap map = {sizeof(data.ttl), data.ipv4.size(), data.ipv6.size()};
				// Смещаем на начальную позицию
				fseek(file, 0L, SEEK_SET);
				// Выполняем запись карты размеров
				if(fwrite(&map, sizeof(map), 1, file)){
					// Смещаем значение записи на конец файла
					fseek(file, 0L, SEEK_END);
					// Выполняем запись времени жизни кэша
					fwrite(&data.ttl, map.ttl, 1, file);
					// Если данные ip адреса получены
					if(!data.ipv4.empty()){
						// Смещаем значение записи на конец файла
						fseek(file, 0L, SEEK_END);
						// Выполняем запись ip адреса
						fwrite(data.ipv4.data(), map.ipv4, 1, file);
					}
					// Выполняем запись ip адреса
					if(!data.ipv6.empty()){
						// Смещаем значение записи на конец файла
						fseek(file, 0L, SEEK_END);
						// Выполняем запись ip адреса
						fwrite(data.ipv6.data(), map.ipv6, 1, file);
					}
				}
				// Закрываем файл
				fclose(file);
			// Выводим сообщение в лог
			} else this->log->write(LOG_ERROR, 0, "cannot write dns cache file %s for domain %s", filename.c_str(), domain.c_str());
		}
	}
}
/**
 * writeCache Метод записи данных кэша
 * @param domain название домена
 * @param name   название запроса
 * @param data   данные запроса
 */
void Cache::writeCache(const string domain, const string name, DataCache data){
	// Если кэширование разрешено
	if(this->config->cache.response){
		// Получаем данные каталога где хранится кэш
		string dir = this->config->cache.dir;
		// Получаем имя файла
		dir = addToPath(dir, "cache");
		// Добавляем основной путь
		dir = addToPath(dir, getPathDomain(domain));
		// Создаем адрес для хранения файла
		const string filename = addToPath(dir, md5(name));
		// Проверяем на существование адреса
		if(!filename.empty()){
			// Проверяем существует ли нужный нам каталог
			if(!makePath(dir.c_str())){
				// Выводим в лог информацию
				this->log->write(LOG_ERROR, 0, "unable to create directory for cache file %s for domain %s", dir.c_str(), domain.c_str());
				// Выходим
				return;
			}
			// Открываем файл на запись
			FILE * file = fopen(filename.c_str(), "wb");
			// Если файл открыт
			if(file){
				
				/*
				// Получаем размер данных
				size_t size = 0;
				// Считаем общий объем сохраняемых данных
				size += sizeof(data.age);
				size += sizeof(data.date);
				size += sizeof(data.expires);
				size += sizeof(data.modified);
				size += sizeof(data.rvalid);
				size += data.etag.size();
				size += data.http.size();
				// Записываем в файл данные домена
				fwrite(&data, size, 1, file);
				*/
				// Закрываем файл
				fclose(file);
			// Выводим сообщение в лог
			} else this->log->write(LOG_ERROR, 0, "cannot write cache file %s for domain %s", filename.c_str(), domain.c_str());
		}
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
const int Cache::rmdir(const char * path){
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
 * strToTime Метод перевода строки в timestamp
 * @param  date строка даты
 * @return      timestamp
 */
const time_t Cache::strToTime(const char * date){
	// Создаем структуру времени
	struct tm tm;
	// Зануляем структуру
	memset(&tm, 0, sizeof(struct tm));
	// Выполняем парсинг даты
	strptime(date, "%a, %d %b %Y %H:%M:%S %Z", &tm);
	// Выводим результат
	return mktime(&tm);
}
/**
 * makePath Функция создания каталога для хранения логов
 * @param  path адрес для каталога
 * @return      результат создания каталога
 */
const bool Cache::makePath(const char * path){
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
const bool Cache::isNumber(const string &str){
	return !str.empty() && find_if(str.begin(), str.end(), [](char c){
		return !isdigit(c);
	}) == str.end();
}
/**
 * isDirExist Функция проверки существования каталога
 * @param  path адрес каталога
 * @return      результат проверки
 */
const bool Cache::isDirExist(const char * path){
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
const bool Cache::isFileExist(const char * path){
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
const bool Cache::isDomain(const string domain){
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
const bool Cache::isIpV4(const string ip){
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
const bool Cache::isIpV6(const string ip){
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
 * checkEnabledCache Метод проверки, разрешено ли создавать кэш
 * @param  http блок с данными запроса или ответа
 * @return      результат проверки
 */
const bool Cache::checkEnabledCache(HttpData & http){
	// Результат проверки
	bool result = false;
	// Если кэширование разрешено
	if(this->config->cache.response && http.isEndHeaders()){
		// Генерируем текущую дату
		time_t date = time(NULL), expires = 0;
		// Определяем время жизни
		const string ag = http.getHeader("age");
		// Получаем данные etag
		const string et = http.getHeader("etag");
		// Получаем заголовок pragma
		const string pr = http.getHeader("pragma");
		// Получаем дату смерти кэша
		const string ex = http.getHeader("expires");
		// Получаем заголовок контроль кэша
		const string cc = http.getHeader("cache-control");
		// Получаем дату последней модификации
		const string lm = http.getHeader("last-modified");
		// Определяем время жизни кэша
		if(!ex.empty()) expires = strToTime(ex.c_str());
		// Если прагма запрещает кэш то отключаем его
		if(!pr.empty() && (pr.find("no-cache") != string::npos)) result = false;
		// Если время для жизни кэша еще есть то разрешаем кэширование
		if(date < expires) result = true;
		// Запрещаем кэш если время жизни уже истекло
		else result = false;
		// Если установлен etag или дата последней модификации значит разрешаем кэширование
		if(!et.empty() || !lm.empty() || !ag.empty()) result = true;
		// Если управление кэшем существует
		if(!cc.empty()){
			// Получаем параметры кэша
			auto control = split(cc, ",");
			// Переходим по всему массиву
			for(u_int i = 0; i < control.size(); i++){
				// Получаем строку кэша
				const string cache = control[i];
				// Директивы управление кэшем
				bool ccPrivate		= (cache.compare("private") == 0);
				bool ccNoCache		= (cache.compare("no-cache") == 0);
				bool ccNoStore		= (cache.compare("no-store") == 0);
				bool ccMaxAge		= (cache.compare("s-maxage") == 0);
				bool ccRevalidate	= (cache.compare("proxy-revalidate") == 0);
				// Определяем тип заголовка
				if(ccNoCache || ccRevalidate){
					// Если etag существует
					if(!et.empty() || !lm.empty()) result = true;
					else result = false;
				// Если время жизни найдено, то определяем его
				} else if(ccMaxAge){
					// Возраст жизни кэша
					size_t age = (!ag.empty() ? ::atoi(ag.c_str()) : 0);
					// Извлекачем значение времени
					size_t pos = cache.find("s-maxage=");
					// Если позиция найдена тогда извлекаем контент
					if(pos != string::npos) age = ::atoi(cache.substr(pos, cache.length() - pos).c_str());
					// Если возраст больше нуля и это публичное кэширование тогда разрешаем
					if(age && (cc.find("public") != string::npos)) result = true;
					// Если это приватный кэш тогда запрещаем кэширование
					else if(cc.find("private") != string::npos) {
						// Запрещаем кэширование
						result = false;
						// Выходим из цикла
						break;
					}
				// Если кэширование запрещено тогда запрещаем
				} else if(ccNoStore || ccPrivate){
					// Запрещаем кэширование
					result = false;
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
 * getDomain Метод получения ip адреса домена
 * @param  domain название домена
 * @return        ip адрес домена
 */
const string Cache::getDomain(const string domain){
	// Результат полученных данных
	string result;
	// Если кэширование разрешено
	if(this->config->cache.dns){
		// Если данные домена переданы
		if(!domain.empty() && isDomain(domain)){
			// Создаем объект данных
			DataDNS data;
			// Считываем данные домена
			readDomain(domain, &data);
			// Получаем текущее количество секунд
			time_t seconds = time(NULL);
			// Если время жизни не истекло тогда отдаем результат
			if((data.ttl + this->config->cache.dttl) > seconds){
				// Создаем объект сети
				Network nwk;
				// Определяем тип подключения
				switch(this->config->proxy.extIPv){
					// Для протокола IPv4
					case 4: result = nwk.getLowIp(data.ipv4);	break;
					// Для протокола IPv6
					case 6: result = nwk.getLowIp6(data.ipv6);	break;
				}
			}
		}
	}
	// Выводим результат
	return result;
}
/**
 * getCache Метод получения данных кэша
 * @param  http блок с данными запроса или ответа
 * @return      объект с данными кэша
 */
Cache::ResultData Cache::getCache(HttpData & http){
	// Создаем объект результата
	ResultData result;
	// Если кэширование разрешено
	if(this->config->cache.response && http.isEndHeaders()){
		// Получаем данные if-none-match
		const string inm = http.getHeader("if-none-match");
		// Получаем данные if-modified-since
		const string ims = http.getHeader("if-modified-since");
		// Если какой-то из заголовков существует тогда не трогаем кэш, так как эти данные есть у клиента
		if(inm.empty() && ims.empty()){
			// Создаем объект кеша
			DataCache data;
			// Выполняем чтение данных из кэша
			readCache(http.getHost(), http.getPath(), &data);
			// Если заголовки получены
			if(!data.http.headers.empty()){
				// Результат проверки валидности кэша
				bool check = false;
				// Получаем текущую дату
				time_t date = time(NULL);
				// Если дата жизни кэша указана
				if(data.expires){
					// Если дата смерти кэша меньше текущей даты
					if(data.expires < date) check = false;
					else check = true;
				}
				// Если время жизни файла указано
				if(data.age){
					// Дата модификации
					time_t mdate = (data.modified ? data.modified : data.date);
					// Проверяем устарел ли файл
					if((mdate + data.age) < date) check = false;
					else check = true;
				}
				// Если кэш устарел но указан eTag или дата последней модификации, или же кэш не устарел
				if(check || (!check
				&& (!data.etag.empty()
				|| (data.modified < date)))){
					// Помечаем что данные получены
					result.load = true;
					// Запоминаем etag
					result.etag = data.etag;
					// Запоминаем дату последней модификации
					if(data.modified) result.modified = timeToStr(data.modified);
				// Удаляем кэш, если он безнадежно устарел
				} else rmCache(http);
				// Если кэш не устарел, копируем данные кэша
				if(check && !data.rvalid) result.http = data.http;
				// Если данные получены а остальных данных нет тогда удаляем кэш
				if(result.load
				&& result.etag.empty()
				&& result.modified.empty()
				&& !result.http.headers.empty()){
					// Сообщаем что ничего не найдено
					result.load = false;
					// Удаляем кэш
					rmCache(http);
				}
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
	// Если кэширование разрешено
	if(this->config->cache.dns){
		// Если данные домена и ip адреса переданы
		if(!domain.empty() && !ip.empty() && isDomain(domain)){
			// Создаем объект сети
			Network nwk;
			// Создаем объект данных
			DataDNS data;
			// Считываем данные домена
			readDomain(domain, &data);
			// Определяем тип подключения
			switch(this->config->proxy.extIPv){
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
}
/**
 * rmDomain Метод удаления домена из кэша
 * @param domain название домена
 */
void Cache::rmDomain(const string domain){
	// Если кэширование разрешено
	if(this->config->cache.dns){
		// Получаем данные каталога где хранится кэш
		string dir = this->config->cache.dir;
		// Получаем имя файла
		dir = addToPath(dir, "dns");
		// Добавляем основной путь
		dir = addToPath(dir, getPathDomain(domain));
		// Проверяем на существование адреса, если существует то удаляем
		if(!dir.empty() && isDirExist(dir.c_str())) rmdir(dir.c_str());
	}
}
/**
 * rmAllDomains Метод удаления всех доменов из кэша
 */
void Cache::rmAllDomains(){
	// Если кэширование разрешено
	if(this->config->cache.dns){
		// Получаем данные каталога где хранится кэш
		string dir = this->config->cache.dir;
		// Получаем имя файла
		dir = addToPath(dir, "dns");
		// Проверяем на существование адреса, если существует то удаляем
		if(!dir.empty() && isDirExist(dir.c_str())) rmdir(dir.c_str());
	}
}
/**
 * setCache Метод сохранения кэша
 * @param http блок с данными запроса или ответа
 */
void Cache::setCache(HttpData & http){
	// Если кэширование разрешено
	if(this->config->cache.response && http.isEndHeaders()){
		// Определяем метод запроса
		const string method = http.getMethod();
		// Определяем метод запроса, разрешено только GET и POST
		if(((method.compare("get") == 0)
		|| (method.compare("post") == 0))
		// Проверяем разрешено ли выполнять сохранение кэша
		&& checkEnabledCache(http)){
			// Определяем время жизни
			const string ag = http.getHeader("age");
			// Получаем данные etag
			const string et = http.getHeader("etag");
			// Получаем заголовок pragma
			const string pr = http.getHeader("pragma");
			// Получаем дату смерти кэша
			const string ex = http.getHeader("expires");
			// Получаем заголовок контроль кэша
			const string cc = http.getHeader("cache-control");
			// Получаем дату последней модификации
			const string lm = http.getHeader("last-modified");
			// Обязательная валидация данных
			bool rvalid = false;
			// Возраст жизни кэша
			time_t expires = 0, modified = 0, date = time(NULL);
			// Возраст жизни кэша
			time_t age = (!ag.empty() ? ::atoi(ag.c_str()) : 0);
			// Если дата модификации данных указана
			if(!lm.empty()) modified = strToTime(lm.c_str());
			// Если дата смерти кэша указана
			if(!ex.empty()) expires = strToTime(ex.c_str());
			// Если управление кэшем существует
			if(!cc.empty()){
				// Получаем параметры кэша
				auto control = split(cc, ",");
				// Переходим по всему массиву
				for(u_int i = 0; i < control.size(); i++){
					// Получаем строку кэша
					const string cache = control[i];
					// Если нужно проводить обязательную валидацию данных
					if(cache.compare("proxy-rervalid") == 0) rvalid = true;
					// Если время жизни найдено, то определяем его
					else if(cache.compare("s-maxage") == 0){
						// Извлекачем значение времени
						size_t pos = cache.find("s-maxage=");
						// Если позиция найдена тогда извлекаем контент
						if(pos != string::npos) age = ::atoi(cache.substr(pos, cache.length() - pos).c_str());
					}
				}
			}
			// Выполняем запись данных в кэш
			//writeCache(http.getHost(), http.getPath(), {age, date, expires, modified, rvalid, et, http.getDump()});
		}
	}
}
/**
 * rmCache Метод удаления кэша
 * @param http блок с данными запроса или ответа
 */
void Cache::rmCache(HttpData & http){
	// Если кэширование разрешено
	if(this->config->cache.response && http.isEndHeaders()){
		// Получаем данные каталога где хранится кэш
		string dir = this->config->cache.dir;
		// Получаем имя файла
		dir = addToPath(dir, "cache");
		// Добавляем основной путь
		dir = addToPath(dir, getPathDomain(http.getHost()));
		// Проверяем на существование адреса, если существует то удаляем
		if(!dir.empty() && isDirExist(dir.c_str())) rmdir(dir.c_str());
	}
}
/**
 * rmAllCache Метод удаления кэша всех сайтов
 */
void Cache::rmAllCache(){
	// Если кэширование разрешено
	if(this->config->cache.response){
		// Получаем данные каталога где хранится кэш
		string dir = this->config->cache.dir;
		// Получаем имя файла
		dir = addToPath(dir, "cache");
		// Проверяем на существование адреса, если существует то удаляем
		if(!dir.empty() && isDirExist(dir.c_str())) rmdir(dir.c_str());
	}
}
/**
 * Cache Конструктор
 * @param log    объект лога для вывода информации
 * @param config конфигурационные данные
 */
Cache::Cache(LogApp * log, Config * config){
	// Запоминаем объект логов
	this->log = log;
	// Запоминаем параметры конфига
	this->config = config;
}