/* УТИЛИТА ЧИСТКИ УСТАРЕВШИХ ФЙЛОВ КЭША AMING */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2016 - 2017
*/
#include "ccache.h"

// Устанавливаем пространство имен
using namespace std;

/**
 * size Метод получения размеров сырых данных
 * @return размер сырых данных
 */
const size_t CCache::DataDNS::size(){
	// Если размер данные не существует, выполняем генерацию данных
	if(this->raw.empty()) data();
	// Выводим результат
	return this->raw.size();
}
/**
 * data Метод получения сырых данных
 * @return сырые данные
 */
const u_char * CCache::DataDNS::data(){
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
void CCache::DataDNS::set(const u_char * data, size_t size){
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
						case 0: Anyks::(data, size_data, size_it, &this->ttl); break;
						// Если это адрес ipv4
						case 1: Anyks::cpydata(data, size_data, size_it, this->ipv4); break;
						// Если это адрес ipv6
						case 2: Anyks::cpydata(data, size_data, size_it, this->ipv6); break;
					}
				}
			}
		}
	}
}
/**
 * ~DataDNS Деструктор
 */
CCache::DataDNS::~DataDNS(){
	// Очищаем полученные данные
	this->raw.clear();
	// Очищаем память выделенную для вектора
	vector <u_char> ().swap(this->raw);
}
/**
 * size Метод получения размеров сырых данных
 * @return размер сырых данных
 */
const size_t CCache::DataCache::size(){
	// Если размер данные не существует, выполняем генерацию данных
	if(this->raw.empty()) data();
	// Выводим результат
	return this->raw.size();
}
/**
 * data Метод получения сырых данных
 * @return сырые данные
 */
const u_char * CCache::DataCache::data(){
	// Если данные заполнены
	if(!this->http.empty()){
		// Объект размерности данных
		Map sizes = {
			sizeof(this->ipv),
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
		// Получаем данные интернет протокола
		const u_char * ipv = reinterpret_cast <const u_char *> (&this->ipv);
		// Выполняем копирование времени жизни
		copy(ipv, ipv + sizes.ipv, back_inserter(this->raw));
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
void CCache::DataCache::set(const u_char * data, size_t size){
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
						// Если это версия интернет протокола
						case 0: Anyks::(data, size_data, size_it, &this->ipv); break;
						// Если это время жизни
						case 1: Anyks::cpydata(data, size_data, size_it, &this->age); break;
						// Если это дата записи кэша прокси сервером
						case 2: Anyks::cpydata(data, size_data, size_it, &this->date); break;
						// Если это дата смерти кэша
						case 3: Anyks::cpydata(data, size_data, size_it, &this->expires); break;
						// Если это дата последней модификации
						case 4: Anyks::cpydata(data, size_data, size_it, &this->modified); break;
						// Если это обязательная ревалидация
						case 5: Anyks::cpydata(data, size_data, size_it, &this->valid); break;
						// Если это идентификатор ETag
						case 6: Anyks::cpydata(data, size_data, size_it, this->etag); break;
						// Если это данные кэша
						case 7: {
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
CCache::DataCache::~DataCache(){
	// Очищаем полученные данные
	this->raw.clear();
	// Очищаем память выделенную для вектора
	vector <u_char> ().swap(this->raw);
}
/**
 * readDomain Метод чтения данных домена из файла
 * @param filename адрес файла кэша
 * @param data     указатель на данные домена
 */
void CCache::readDomain(const string filename, DataDNS * data){
	// Если объект DNS существует
	if(data){
		// Проверяем на существование адреса
		if(!filename.empty() && Anyks::isFileExist(filename.c_str())){
			// Открываем файл на чтение
			ifstream file(filename.c_str(), ios::binary);
			// Если файл открыт
			if(file.is_open()){
				// Перемещаемся в конец файла
				file.seekg(0, file.end);
				// Определяем размер файла
				const long size = file.tellg();
				// Перемещаемся в начало файла
				file.seekg(0, file.beg);
				// Если размер файла получен верно
				if(size > 0){
					// Создаем буфер данных
					u_char * buffer = new u_char [size];
					// Считываем до тех пор пока все удачно
					while(file.good()) file.read((char *) buffer + file.tellg(), 60);
					// Устанавливаем полученные данные
					data->set(buffer, size);
					// Удаляем выделенную память
					delete [] buffer;
				}
				// Закрываем файл
				file.close();
			// Выводим сообщение в лог
			} else printf("Cannot read dns cache file %s\r\n", filename.c_str());
		}
	}
}
/**
 * readCache Метод чтения данных из файла кэша
 * @param filename адрес файла кэша
 * @param data     данные запроса
 */
void CCache::readCache(const string filename, DataCache * data){
	// Если блок данных кэша существует
	if(data){
		// Проверяем на существование адреса
		if(!filename.empty() && Anyks::isFileExist(filename.c_str())){
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
			} else printf("Cannot read cache file %s\r\n", filename.c_str());
		}
	}
}
/**
 * checkDomains Функция проверки кэша доменов
 * @param filename адрес файла записи
 * @param ctx      объект приложения
 */
void CCache::checkDomains(const string filename, void * ctx){
	// Получаем объект подключения
	CCache * ccache = reinterpret_cast <CCache *> (ctx);
	// Если данные переданы верные
	if(!filename.empty() && ccache){
		// Результат работы регулярного выражения
		smatch match;
		// Устанавливаем правило регулярного выражения
		regex e("[\\s\\S]+\\/data$", regex::ECMAScript | regex::icase);
		// Выполняем проверку на соответствие верности данных кэша домена
		regex_search(filename, match, e);
		// Если домен существует
		if(!match.empty()){
			// Создаем объект данных
			DataDNS data;
			// Считываем данные домена
			ccache->readDomain(filename, &data);
			// Получаем текущее количество секунд
			time_t seconds = time(nullptr);
			// Если время жизни домена истекло то удаляем его
			if((data.ttl + ccache->config->cache.dttl) <= seconds){
				/* Определяем название домена */
				// Устанавливаем правило регулярного выражения
				regex e("^([\\s\\S]+)\\/data$", regex::ECMAScript | regex::icase);
				// Выполняем поиск название домена
				regex_search(filename, match, e);
				// Если название домена найдено то удаляем его
				if(!match.empty()){
					// Получаем адрес домена для удаления
					const string rmDomain = match[1].str();
					// Выводим сообщение об удаляемом домене в консоль
					printf("Domain was deleted: %s/\r\n", rmDomain.c_str());
					// Выполняем удаление домена
					Anyks::rmDir(rmDomain.c_str());
				}
			}
		}
	}
}
/**
 * checkCache Функция проверки кэша данных
 * @param filename адрес файла записи
 * @param ctx      объект приложения
 */
void CCache::checkCache(const string filename, void * ctx){
	// Получаем объект подключения
	CCache * ccache = reinterpret_cast <CCache *> (ctx);
	// Если данные переданы верные
	if(!filename.empty() && ccache){
		// Результат работы регулярного выражения
		smatch match;
		// Устанавливаем правило регулярного выражения
		regex e("[\\s\\S]+\\/[A-Fa-f\\d]{32}$", regex::ECMAScript | regex::icase);
		// Выполняем проверку на соответствие верности данных кэша домена
		regex_search(filename, match, e);
		// Если домен существует
		if(!match.empty()){
			// Создаем объект кеша
			DataCache cache;
			// Выполняем чтение данных из кэша
			ccache->readCache(filename, &cache);
			// Результат проверки валидности кэша
			bool check = false;
			// Если заголовки получены
			if(!cache.http.empty()){
				// Получаем текущую дату
				time_t date = time(nullptr);
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
				}
				/*
				// Если кэш устарел но указан eTag или дата последней модификации, или же кэш не устарел
				if(check || !cache.etag.empty() || (cache.modified < date))
					check = true;
				else check = false;
				*/
			}
			// Если проверки не пройдены тогда удаляем кэш
			if(!check){
				/* Определяем название домена */
				// Устанавливаем правило регулярного выражения
				regex e("^([\\s\\S]+)\\/\\d+\\/[\\s\\S]+\\/[A-Fa-f\\d]{32}$", regex::ECMAScript | regex::icase);
				// Выполняем поиск название домена
				regex_search(filename, match, e);
				// Если название домена найдено то удаляем его
				if(!match.empty()){
					// Получаем адрес домена для удаления
					const string rmDomain = match[1].str();
					// Выводим сообщение об удаляемом кэше в консоль
					printf("Cache was deleted: %s/\r\n", rmDomain.c_str());
					// Выполняем удаление домена
					Anyks::rmDir(rmDomain.c_str());
				}
			}
		}
	}
}
/**
 * infoPatch Функция определяющая тип адреса
 * @param  pathName  адрес файла или каталога
 * @return           результат проверки (-1 - Не определено, 0 - Файл, 1 - Каталог)
 */
const short CCache::infoPatch(const string pathName){
	// Результат проверки
	int result = -1;
	// Если файл определен
	if(!pathName.empty()){
		// Создаем структуру для информации о содержимом
		struct stat entryInfo;
		// Считываем информацию о содержимом
		if(lstat(pathName.c_str(), &entryInfo) == 0){
			// Если это каталог
			if(S_ISDIR(entryInfo.st_mode)){
				// Выводим название каталога в консоль
				printf("Catalog name: %s/\r\n", pathName.c_str());
				// Сообщаем что это каталог
				return 1;
			// Если это файл
			} else if(S_ISREG(entryInfo.st_mode)) {
				// Выводим название файла в консоль
				printf("\t%s has %lld bytes\r\n", pathName.c_str(), (long long) entryInfo.st_size);
				// Выводим результат
				return 0;
			// Если это символьная ссылка
			} else if(S_ISLNK(entryInfo.st_mode)) {
				// Копируем название файла на который ссылается ссылка
				char targetName[PATH_MAX + 1];
				// Считываем данные ссылки
				if(readlink(pathName.c_str(), targetName, PATH_MAX) != -1){
					// Выводим название файла в консоль
					printf("\t%s -> %s\r\n", pathName.c_str(), targetName);
					// Проверяем содержимое символьной ссылки
					return infoPatch(targetName);
				// Если ссылка битая
				} else printf("\t%s -> (invalid symbolic link!)\r\n", pathName.c_str());
			}
		// Если произошла ошибка чтения файла
		} else printf("Error statting %s: %s\r\n", pathName.c_str(), strerror(errno));
	}
	// Выводим результат
	return result;
}
/**
 * processDirectory Рекурсивная функция обхода содержимого каталога
 * @param  curDir адрес текущего каталога
 * @param  fn     функция обработчик содержимого файла
 * @return        результат работы
 */
const u_long CCache::processDirectory(const string curDir, handler fn){
	// Основные структуры
	struct dirent entry;
	struct dirent * entryPtr = nullptr;
	// Количество найденных файлов
	u_long count = 0;
	// Адрес файла
	char pathName[PATH_MAX + 1];
	// Открываем текущий каталог
	DIR * dir = opendir(curDir.c_str());
	// Если каталог удачно открыт
	if(dir == nullptr){
		// Выводим сообщение об ошибке
		printf("Error opening %s: %s\r\n", curDir.c_str(), strerror(errno));
		// Выходим и говорим что ничего не найдено
		return 0;
	}
	// Считываем содержимое каталога
	int retval = readdir_r(dir, &entry, &entryPtr);
	// Если указатель на каталог существует
	while(entryPtr != nullptr){
		// Определяем внешний или текущий каталог
		if((strncmp(entry.d_name, ".", PATH_MAX) == 0)
		|| (strncmp(entry.d_name, "..", PATH_MAX) == 0)){
			/* Short-circuit the . and .. entries. */
			retval = readdir_r(dir, &entry, &entryPtr);
			continue;
		}
		// Копируем название текущего каталога в буфер
		(void) strncpy(pathName, curDir.c_str(), PATH_MAX);
		// Добавляем экранирование к текущему каталогу
		(void) strncat(pathName, "/", PATH_MAX);
		// Добавляем к пути название внутреннего файла или каталога
		(void) strncat(pathName, entry.d_name, PATH_MAX);
		// Определяем тип адреса
		int info = infoPatch(pathName);
		// Если путь определен
		if(info > -1){
			// Увеличиваем количество найденных файлов
			count++;
			// Проверяем что это за файл
			switch(info){
				// Обрабатываем полученный файл
				case 0: fn(pathName, this);							break;
				// Продолжаем работу, загружаем данные следующего каталога
				case 1: count += processDirectory(pathName, fn);	break;
			}
		}
		// Считываем количество файлов
		retval = readdir_r(dir, &entry, &entryPtr);
	}
	// Закрываем каталог
	(void) closedir(dir);
	// Возвращаем количество найденных файлов
	return count;
}
/**
 * CCache Конструктор
 * @param config адрес конфигурационного файла
 */
CCache::CCache(const string config){
	// Создаем объект конфигурации
	this->config = new Config(config);
	// Получаем каталог с адресом кэша доменов
	const string dns = Anyks::addToPath(this->config->cache.dir, "dns");
	// Получаем каталог с адресом кэша данных
	const string cache = Anyks::addToPath(this->config->cache.dir, "cache");
	// Выполняем проверку доменов
	processDirectory(dns, &CCache::checkDomains);
	// Выполняем проверку кэша
	processDirectory(cache, &CCache::checkCache);
}
/**
 * ~CCache Деструктор
 */
CCache::~CCache(){
	// Удаляем объект конфигурации
	if(this->config) delete this->config;
}
/**
 * main Главная функция приложения
 * @param  argc длина массива параметров
 * @param  argv массив параметров
 * @return      код выхода из приложения
 */
int main(int argc, char * argv[]){
	// Активируем локаль приложения
	// setlocale(LC_ALL, "");
	setlocale(LC_ALL, "en_US.UTF-8");
	// Адрес конфигурационного файла
	string configfile;
	// Определяем параметр запуска
	string param = (argc >= 2 ? argv[1] : "");
	// Если это параметр поиска конфигурационного файла
	if(param.compare("-c") == 0)
		// Ищем адрес конфигурационного файла
		configfile = (argc >= 3 ? argv[2] : "");
	// Если параметр конфигурационного файла найден
	else if(param.find("--config=") != string::npos)
		// Удаляем параметр из адреса файла
		configfile = param.replace(0, 9, "");
	// Создаем объект приложения
	CCache * app = new CCache(configfile);
	// Удаляем объект приложения
	if(app) delete app;
	// Выходим
	return 0;
}
