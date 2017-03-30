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
 * size Метод получения размеров сырых данных
 * @return размер сырых данных
 */
const size_t Cache::DataDNS::size(){
	// Если размер данные не существует, выполняем генерацию данных
	if(this->raw.empty()) data();
	// Выводим результат
	return this->raw.size();
}
/**
 * data Метод получения сырых данных
 * @return сырые данные
 */
const u_char * Cache::DataDNS::data(){
	// Если данные заполнены
	if(this->ttl || !this->ipv4.empty() || !this->ipv6.empty()){
		// Объект размерности данных
		Map sizes = {sizeof(this->ttl), this->ipv4.size(), this->ipv6.size()};
		// Получаем размер структуры
		const size_t size = sizeof(sizes);
		// Получаем данные карты размеров
		const u_char * map = reinterpret_cast <const u_char *> (&sizes);
		// Выполняем копирование карты размеров
		copy(map, map + size, back_inserter(this->raw));
		// Получаем данные времени жизни
		const u_char * ttl = reinterpret_cast <const u_char *> (&this->ttl);
		// Выполняем копирование времени жизни
		copy(ttl, ttl + sizes.ttl, back_inserter(this->raw));
		// Выполняем копирование данных адреса ipv4
		copy(this->ipv4.begin(), this->ipv4.end(), back_inserter(this->raw));
		// Выполняем копирование данных адреса ipv6
		copy(this->ipv6.begin(), this->ipv6.end(), back_inserter(this->raw));
	}
	// Выводим сформированные данные
	return this->raw.data();
}
/**
 * set Метод установки сырых данных
 * @param data сырые данные
 * @param size размер сырых данных
 */
void Cache::DataDNS::set(const u_char * data, size_t size){
	// Если данные существуют
	if(size){
		// Получаем размер структуры
		size_t size_map = sizeof(Map);
		// Если размер карты меньше общего размера
		if(size_map < size){
			// Размеры полученных данных
			size_t size_it = size_map;
			// Извлекаем данные карты размеров
			for(size_t i = 0, j = 0; i < size_map; i += sizeof(size_t), j++){
				// Размер полученных данных
				size_t size_data;
				// Извлекаем размер данных
				memcpy(&size_data, data + i, sizeof(size_t));
				// Если данные верные
				if(size_data && ((size_data + size_it) <= size)){
					// Определяем тип извлекаемых данных
					switch(j){
						// Если это время жизни
						case 0: cpydata(data, size_data, size_it, &this->ttl); break;
						// Если это адрес ipv4
						case 1: cpydata(data, size_data, size_it, this->ipv4); break;
						// Если это адрес ipv6
						case 2: cpydata(data, size_data, size_it, this->ipv6); break;
					}
				}
			}
		}
	}
}
/**
 * ~DataDNS Деструктор
 */
Cache::DataDNS::~DataDNS(){
	// Очищаем полученные данные
	this->raw.clear();
	// Очищаем память выделенную для вектора
	vector <u_char> ().swap(this->raw);
}
/**
 * size Метод получения размеров сырых данных
 * @return размер сырых данных
 */
const size_t Cache::DataCache::size(){
	// Если размер данные не существует, выполняем генерацию данных
	if(this->raw.empty()) data();
	// Выводим результат
	return this->raw.size();
}
/**
 * data Метод получения сырых данных
 * @return сырые данные
 */
const u_char * Cache::DataCache::data(){
	// Если данные заполнены
	if(!this->http.empty()){
		// Объект размерности данных
		Map sizes = {
			sizeof(this->age),
			sizeof(this->date),
			sizeof(this->expires),
			sizeof(this->modified),
			sizeof(this->valid),
			this->etag.size(),
			this->http.size()
		};
		// Получаем размер структуры
		const size_t size = sizeof(sizes);
		// Получаем данные карты размеров
		const u_char * map = reinterpret_cast <const u_char *> (&sizes);
		// Выполняем копирование карты размеров
		copy(map, map + size, back_inserter(this->raw));
		// Получаем данные времени жизни
		const u_char * age = reinterpret_cast <const u_char *> (&this->age);
		// Выполняем копирование времени жизни
		copy(age, age + sizes.age, back_inserter(this->raw));
		// Получаем данные даты создания кэша
		const u_char * date = reinterpret_cast <const u_char *> (&this->date);
		// Выполняем копирование данных даты создания кэша
		copy(date, date + sizes.date, back_inserter(this->raw));
		// Получаем данные периода жизни кэша
		const u_char * expires = reinterpret_cast <const u_char *> (&this->expires);
		// Выполняем копирование данных периода жизни кэша
		copy(expires, expires + sizes.expires, back_inserter(this->raw));
		// Получаем данные периода даты модификации кэша
		const u_char * modified = reinterpret_cast <const u_char *> (&this->modified);
		// Выполняем копирование данных даты модификации кэша
		copy(modified, modified + sizes.modified, back_inserter(this->raw));
		// Получаем данные ревалидации кэша
		const u_char * valid = reinterpret_cast <const u_char *> (&this->valid);
		// Выполняем копирование данных ревалидации кэша
		copy(valid, valid + sizes.valid, back_inserter(this->raw));
		// Выполняем копирование данных тегда ETag
		copy(this->etag.begin(), this->etag.end(), back_inserter(this->raw));
		// Получаем данные кэша
		const u_char * cache = this->http.data();
		// Выполняем копирование данных кэша
		copy(cache, cache + sizes.cache, back_inserter(this->raw));
	}
	// Выводим сформированные данные
	return this->raw.data();
}
/**
 * set Метод установки сырых данных
 * @param data сырые данные
 * @param size размер сырых данных
 */
void Cache::DataCache::set(const u_char * data, size_t size){
	// Если данные существуют
	if(size){
		// Получаем размер структуры
		size_t size_map = sizeof(Map);
		// Если размер карты меньше общего размера
		if(size_map < size){
			// Размеры полученных данных
			size_t size_it = size_map;
			// Извлекаем данные карты размеров
			for(size_t i = 0, j = 0; i < size_map; i += sizeof(size_t), j++){
				// Размер полученных данных
				size_t size_data;
				// Извлекаем размер данных
				memcpy(&size_data, data + i, sizeof(size_t));
				// Если данные верные
				if(size_data && ((size_data + size_it) <= size)){
					// Определяем тип извлекаемых данных
					switch(j){
						// Если это время жизни
						case 0: cpydata(data, size_data, size_it, &this->age); break;
						// Если это дата записи кэша прокси сервером
						case 1: cpydata(data, size_data, size_it, &this->date); break;
						// Если это дата смерти кэша
						case 2: cpydata(data, size_data, size_it, &this->expires); break;
						// Если это дата последней модификации
						case 3: cpydata(data, size_data, size_it, &this->modified); break;
						// Если это обязательная ревалидация
						case 4: cpydata(data, size_data, size_it, &this->valid); break;
						// Если это идентификатор ETag
						case 5: cpydata(data, size_data, size_it, this->etag); break;
						// Если это данные кэша
						case 6: {
							// Выделяем динамически память
							u_char * buffer = new u_char [size_data];
							// Извлекаем данные адреса
							memcpy(buffer, data + size_it, size_data);
							// Запоминаем результат
							this->http.assign(buffer, buffer + size_data);
							// Определяем смещение
							size_it += size_data;
							// Удаляем полученные данные
							delete [] buffer;
						} break;
					}
				}
			}
		}
	}
}
/**
 * ~DataCache Деструктор
 */
Cache::DataCache::~DataCache(){
	// Очищаем полученные данные
	this->raw.clear();
	// Очищаем память выделенную для вектора
	vector <u_char> ().swap(this->raw);
}
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
	struct tm * tm = gmtime(&date);
	// Буфер с данными
	char buf[255];
	// Зануляем буфер
	memset(buf, 0, sizeof(buf));
	// Выполняем парсинг даты
	strftime(buf, sizeof(buf), "%a, %d %b %Y %X %Z", tm);
	// Выводим результат
	return string(buf);
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
			ifstream file(filename.c_str(), ios::binary);
			// Если файл открыт
			if(file.is_open()){
				// Перемещаемся в конец файла
				file.seekg(0, file.end);
				// Определяем размер файла
				const size_t size = file.tellg();
				// Перемещаемся в начало файла
				file.seekg(0, file.beg);
				// Создаем буфер данных
				u_char * buffer = new u_char [size];
				// Считываем до тех пор пока все удачно
				while(file.good()) file.read((char *) buffer + file.tellg(), 60);
				// Устанавливаем полученные данные
				data->set(buffer, size);
				// Удаляем выделенную память
				delete [] buffer;
				// Закрываем файл
				file.close();
			// Выводим сообщение в лог
			} else this->log->write(LOG_ERROR, 0, "cannot read dns cache file %s for domain %s", filename.c_str(), domain.c_str());
		}
	}
}
/**
 * readCache Метод чтения данных из файла кэша
 * @param http блок с данными запроса или ответа
 * @param data данные запроса
 */
void Cache::readCache(HttpData &http, DataCache * data){
	// Если кэширование разрешено
	if(this->config->cache.dat){
		// Получаем данные каталога где хранится кэш
		string dir = this->config->cache.dir;
		// Получаем имя файла
		dir = addToPath(dir, "cache");
		// Добавляем основной путь
		dir = addToPath(dir, getPathDomain(http.getHost()));
		// Добавляем порт
		dir = addToPath(dir, to_string(http.getPort()));
		// Добавляем метод
		dir = addToPath(dir, http.getMethod());
		// Добавляем тип подключения
		dir = addToPath(dir, (http.isAlive() ? "a" : "c"));
		// Создаем адрес для хранения файла
		const string filename = addToPath(dir, md5(http.getPath()));
		// Проверяем на существование адреса
		if(!filename.empty() && isFileExist(filename.c_str())){
			// Открываем файл на чтение
			ifstream file(filename.c_str(), ios::binary);
			// Если файл открыт
			if(file.is_open()){
				// Перемещаемся в конец файла
				file.seekg(0, file.end);
				// Определяем размер файла
				const size_t size = file.tellg();
				// Перемещаемся в начало файла
				file.seekg(0, file.beg);
				// Создаем буфер данных
				u_char * buffer = new u_char [size];
				// Считываем до тех пор пока все удачно
				while(file.good()) file.read((char *) buffer + file.tellg(), 60);
				// Устанавливаем полученные данные
				data->set(buffer, size);
				// Удаляем выделенную память
				delete [] buffer;
				// Закрываем файл
				file.close();
			// Выводим сообщение в лог
			} else this->log->write(LOG_ERROR, 0, "cannot read dns cache file %s for domain %s", filename.c_str(), http.getHost().c_str());
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
			// Открываем файл на чтение
			ofstream file(filename.c_str(), ios::binary);
			// Если файл открыт
			if(file.is_open()){
				// Выполняем запись данных в файл
				file.write((const char *) data.data(), data.size());
				// Закрываем файл
				file.close();
			// Выводим сообщение в лог
			} else this->log->write(LOG_ERROR, 0, "cannot write dns cache file %s for domain %s", filename.c_str(), domain.c_str());
		}
	}
}
/**
 * writeCache Метод записи данных кэша
 * @param http блок с данными запроса или ответа
 * @param data данные запроса
 */
void Cache::writeCache(HttpData &http, DataCache data){
	// Если кэширование разрешено
	if(this->config->cache.dat){
		// Получаем данные каталога где хранится кэш
		string dir = this->config->cache.dir;
		// Получаем имя файла
		dir = addToPath(dir, "cache");
		// Добавляем основной путь
		dir = addToPath(dir, getPathDomain(http.getHost()));
		// Добавляем порт
		dir = addToPath(dir, to_string(http.getPort()));
		// Добавляем метод
		dir = addToPath(dir, http.getMethod());
		// Добавляем тип подключения
		dir = addToPath(dir, (http.isAlive() ? "a" : "c"));
		// Создаем адрес для хранения файла
		const string filename = addToPath(dir, md5(http.getPath()));
		// Проверяем на существование адреса
		if(!filename.empty()){
			// Проверяем существует ли нужный нам каталог
			if(!makePath(dir.c_str())){
				// Выводим в лог информацию
				this->log->write(LOG_ERROR, 0, "unable to create directory for cache file %s for domain %s", dir.c_str(), http.getHost().c_str());
				// Выходим
				return;
			}
			// Открываем файл на чтение
			ofstream file(filename.c_str(), ios::binary);
			// Если файл открыт
			if(file.is_open()){
				// Выполняем запись данных в файл
				file.write((const char *) data.data(), data.size());
				// Закрываем файл
				file.close();
			// Выводим сообщение в лог
			} else this->log->write(LOG_ERROR, 0, "cannot write cache file %s for domain %s", filename.c_str(), http.getHost().c_str());
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
			const size_t len = path_len + strlen(p->d_name) + 2;
			// Создаем буфер данных
			char * buf = new char [len];
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
			}
			// Освобождаем выделенную ранее память
			delete [] buf;
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
	strptime(date, "%a, %d %b %Y %X %Z", &tm);
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
const bool Cache::checkEnabledCache(HttpData &http){
	// Результат проверки
	bool result = false;
	// Если кэширование разрешено
	if(this->config->cache.dat && http.isEndHeaders()){
		// Генерируем текущую дату
		time_t cdate = time(NULL), sdate = 0, expires = 0;
		// Определяем дату сервера
		const string dt = http.getHeader("date");
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
		// Определяем дату сервера
		if(!dt.empty()) sdate = strToTime(dt.c_str());
		// Определяем время жизни кэша
		if(!ex.empty()) expires = strToTime(ex.c_str());
		// Если прагма запрещает кэш то отключаем его
		if(!pr.empty() && (pr.find("no-cache") != string::npos)) result = false;
		// Если время для жизни кэша еще есть то разрешаем кэширование
		if(cdate < expires) result = true;
		// Запрещаем кэш если время жизни уже истекло
		else result = false;
		// Если управление кэшем существует
		if(!cc.empty()){
			// Получаем параметры кэша
			auto control = split(cc, ",");
			// Переходим по всему массиву
			for(auto it = control.begin(); it != control.end(); it++){
				// Получаем строку кэша
				const string cache = * it;
				// Директивы управление кэшем
				bool ccPrivate		= (cache.find("private") != string::npos);
				bool ccNoCache		= (cache.find("no-cache") != string::npos);
				bool ccNoStore		= (cache.find("no-store") != string::npos);
				bool ccMaxAge		= (cache.find("max-age") != string::npos);
				bool ccsMaxAge		= (cache.find("s-maxage") != string::npos);
				bool ccRevalidate	= (cache.find("proxy-revalidate") != string::npos);
				// Если кэширование запрещено тогда запрещаем
				if(ccNoStore || ccPrivate) result = false;
				// Определяем тип заголовка
				else if(ccNoCache || ccRevalidate){
					// Если etag существует
					if(!et.empty() || !lm.empty()) result = true;
					// Запрещаем кэширование
					else result = false;
				// Если время жизни найдено, то определяем его
				} else if(ccMaxAge || ccsMaxAge){
					// Возраст жизни кэша
					size_t age = 0;
					// Извлекачем значение времени
					const size_t pos = cache.find("=");
					// Если позиция найдена тогда извлекаем контент
					if(pos != string::npos) age = ::atoi(cache.substr(pos + 1, cache.length() - (pos + 1)).c_str());
					// Если возраст больше нуля и это публичное кэширование тогда разрешаем
					if(!age || (sdate && ((sdate + age) < cdate))) result = false;
					else result = true;
					// Если кэш на сервере еще не устарел тогда разрешаем кэширование
					if(age && (!ag.empty() && (::atoi(ag.c_str()) < age))) result = true;
					// Если кэш просто устарел тогда запрещаем кэширование
					else if(age && !ag.empty()) result = false;
				}
				// Если установлен etag или дата последней модификации значит разрешаем кэширование
				if(!et.empty() || !lm.empty()) result = true;
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
Cache::ResultData Cache::getCache(HttpData &http){
	// Создаем объект результата
	ResultData result;
	// Если кэширование разрешено
	if(this->config->cache.dat && http.isEndHeaders()){
		// Получаем данные if-none-match
		const string inm = http.getHeader("if-none-match");
		// Получаем данные if-modified-since
		const string ims = http.getHeader("if-modified-since");
		// Если какой-то из заголовков существует тогда не трогаем кэш, так как эти данные есть у клиента
		if(inm.empty() && ims.empty()){
			// Создаем объект кеша
			DataCache cache;
			// Выполняем чтение данных из кэша
			readCache(http, &cache);
			// Если заголовки получены
			if(!cache.http.empty()){
				// Результат проверки валидности кэша
				bool check = false;
				// Получаем текущую дату
				time_t date = time(NULL);
				// Если дата жизни кэша указана
				if(cache.expires){
					// Если дата смерти кэша меньше текущей даты
					if(cache.expires < date) check = false;
					else check = true;
				}
				// Если время жизни файла указано
				if(cache.age){
					// Дата модификации
					time_t mdate = (cache.modified ? cache.modified : cache.date);
					// Проверяем устарел ли файл
					if((mdate + cache.age) < date) check = false;
					else check = true;
					// Запоминаем время жизни
					result.age = (date - mdate);
				}
				// Если кэш устарел но указан eTag или дата последней модификации, или же кэш не устарел
				if(check || !cache.etag.empty() || (cache.modified < date)){
					// Запоминаем etag
					if(!cache.etag.empty()) result.etag = cache.etag;
					// Запоминаем дату последней модификации
					if(cache.modified) result.modified = timeToStr(cache.modified);
					// Указываем что данные нужно ревалидировать
					result.valid = cache.valid;
					// Если ревалидация не указана и проверку не прошли
					if(!check && !result.valid) result.valid = true;
					// Запоминаем данные из кэша
					result.http.assign(cache.http.begin(), cache.http.end());
				// Если данные получены а остальных данных нет тогда удаляем кэш
				} else rmCache(http);
			// Удаляем файл если данных в нем нет
			}// else rmCache(http);
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
void Cache::setCache(HttpData &http){
	// Если кэширование разрешено
	if(this->config->cache.dat && http.isEndHeaders()){
		// Определяем метод запроса
		const string method = http.getMethod();
		// Определяем метод запроса, разрешено только GET и POST
		if(((method.compare("get") == 0)
		|| (method.compare("post") == 0))
		// Проверяем разрешено ли выполнять сохранение кэша
		&& checkEnabledCache(http)){
			// Извлекаем значение даты сервера
			const string dt = http.getHeader("date");
			// Определяем время жизни
			const string ag = http.getHeader("age");
			// Получаем данные etag
			const string et = http.getHeader("etag");
			// Получаем дату смерти кэша
			const string ex = http.getHeader("expires");
			// Получаем заголовок контроль кэша
			const string cc = http.getHeader("cache-control");
			// Получаем дату последней модификации
			const string lm = http.getHeader("last-modified");
			// Обязательная валидация данных
			bool valid = false;
			// Возраст жизни кэша
			time_t age = 0, expires = 0, modified = 0, date = time(NULL);
			// Если дата сервера установлена
			if(!dt.empty()) date = strToTime(dt.c_str());
			// Если дата модификации данных указана
			if(!lm.empty()) modified = strToTime(lm.c_str());
			// Если дата смерти кэша указана
			if(!ex.empty()) expires = strToTime(ex.c_str());
			// Если управление кэшем существует
			if(!cc.empty()){
				// Получаем параметры кэша
				auto control = split(cc, ",");
				// Переходим по всему массиву
				for(auto it = control.begin(); it != control.end(); it++){
					// Получаем строку кэша
					const string cache = * it;
					// Если нужно проводить обязательную валидацию данных
					if((cache.compare("no-cache") == 0)
					|| (cache.compare("proxy-revalidate") == 0)) valid = true;
					// Проверяем время жизни
					else if((cache.find("s-maxage") != string::npos)
					|| ((cache.find("max-age") != string::npos)
					&& (cc.find("s-maxage") == string::npos))){
						// Извлекачем значение времени
						size_t pos = cache.find("=");
						// Если позиция найдена тогда извлекаем контент
						if(pos != string::npos) age = ::atoi(cache.substr(pos + 1, cache.length() - (pos + 1)).c_str());
						// Если время жизни указано то вычитаем из него время которое кэш уже прожил
						if(age && !ag.empty()) age -= ::atoi(ag.c_str());
					}
				}
			}
			// Создаем объект кэша
			DataCache cache;
			// Получаем дамп данных
			const u_char * dump = http.data();
			// Заполняем данные кэша
			cache.age		= age;
			cache.etag		= et;
			cache.date		= date;
			cache.valid		= valid;
			cache.expires	= expires;
			cache.modified	= modified;
			cache.http.assign(dump, dump + http.size());
			// Выполняем запись данных в кэш
			if(!cache.http.empty()) writeCache(http, cache);
		}
	}
}
/**
 * rmCache Метод удаления кэша
 * @param http блок с данными запроса или ответа
 */
void Cache::rmCache(HttpData &http){
	// Если кэширование разрешено
	if(this->config->cache.dat && http.isEndHeaders()){
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
	if(this->config->cache.dat){
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