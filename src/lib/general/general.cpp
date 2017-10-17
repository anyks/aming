/* МОДУЛЬ ОБЩИХ ФУНКЙИЙ AMING */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#include "general/general.h"

// Устанавливаем область видимости
using namespace std;

/**
 * cpydata Метод копирования из сырых данных в рабочие
 * @param data   буфер сырых данных
 * @param size   размер сырых данных
 * @param it     смещение
 * @param result результат куда копируются данные
 */
void Anyks::cpydata(const u_char * data, size_t size, size_t &it, void * result){
	// Извлекаем данные
	memcpy(result, data + it, size);
	// Определяем смещение
	it += size;
}
/**
 * cpydata Метод копирования из сырых данных в рабочие
 * @param data   буфер сырых данных
 * @param size   размер сырых данных
 * @param it     смещение
 * @param result результат куда копируются данные
 */
void Anyks::cpydata(const u_char * data, size_t size, size_t &it, string &result){
	// Выделяем динамически память
	char * buffer = new char [size];
	// Извлекаем данные адреса
	memcpy(buffer, data + it, size);
	// Запоминаем результат
	result.assign(buffer, size);
	// Определяем смещение
	it += size;
	// Удаляем полученные данные
	delete [] buffer;
}
/**
 * toCase Функция перевода в указанный регистр
 * @param  str  строка для перевода в указанных регистр
 * @param  flag флаг указания типа регистра
 * @return      результирующая строка
 */
const string Anyks::toCase(string str, bool flag){
	// Переводим в указанный регистр
	transform(str.begin(), str.end(), str.begin(), (flag ? ::toupper : ::tolower));
	// Выводим результат
	return str;
}
/**
 * isNumber Функция проверки является ли строка числом
 * @param  str строка для проверки
 * @return     результат проверки
 */
const bool Anyks::isNumber(const string &str){
	return !str.empty() && find_if(str.begin(), str.end(), [](char c){
		return !isdigit(c);
	}) == str.end();
}
/**
 * rtrim Функция усечения указанных символов с правой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & Anyks::rtrim(string &str, const char * t){
	str.erase(str.find_last_not_of(t) + 1);
	return str;
}
/**
 * ltrim Функция усечения указанных символов с левой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & Anyks::ltrim(string &str, const char * t){
	str.erase(0, str.find_first_not_of(t));
	return str;
}
/**
 * trim Функция усечения указанных символов с правой и левой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & Anyks::trim(string &str, const char * t){
	return Anyks::ltrim(Anyks::rtrim(str, t), t);
}
/**
 * checkPort Функция проверки на качество порта
 * @param  port входная строка якобы содержащая порт
 * @return      результат проверки
 */
const bool Anyks::checkPort(string str){
	// Если строка существует
	if(!str.empty()){
		// Преобразуем строку в цифры
		if(Anyks::isNumber(str)){
			// Получаем порт
			u_int port = ::atoi(str.c_str());
			// Проверяем диапазон портов
			if((port > 0) && (port < 65536)) return true;
		}
	}
	// Сообщаем что ничего не нашли
	return false;
}
/**
 * replace Функция замены указанных фраз в строке
 * @param s строка в которой происходит замена
 * @param f искомая строка
 * @param r строка на замену
 */
void Anyks::replace(string &s, const string f, const string r){
	// Переходим по всем найденным элементам и заменяем в них искомые фразы
	for(string::size_type n = 0; (n = s.find(f, n)) != string::npos; ++n){
		// Заменяем искомую фразу указанной
		s.replace(n, f.length(), r);
	}
}
/**
 * split Функция разделения строк на составляющие
 * @param str   строка для поиска
 * @param delim разделитель
 * @param v     результирующий вектор
 */
void Anyks::split(const string &str, const string delim, vector <string> &v){
	string::size_type i = 0;
	string::size_type j = str.find(delim);
	size_t len = delim.length();
	// Выполняем разбиение строк
	while(j != string::npos){
		v.push_back(str.substr(i, j - i));
		i = ++j + (len - 1);
		j = str.find(delim, j);
		if(j == string::npos) v.push_back(str.substr(i, str.length()));
	}
}
/**
 * split Метод разбива строки на составляющие
 * @param  str   исходная строка
 * @param  delim разделитель
 * @return       массив составляющих строки
 */
vector <string> Anyks::split(const string str, const string delim){
	// Результат данных
	vector <string> result;
	// Создаем новую строку
	string value = str;
	// Убираем пробелы в строке
	value = Anyks::trim(value);
	// Если строка передана
	if(!value.empty()){
		string data;
		string::size_type i = 0;
		string::size_type j = value.find(delim);
		u_int len = delim.length();
		// Выполняем разбиение строк
		while(j != string::npos){
			data = value.substr(i, j - i);
			result.push_back(Anyks::trim(data));
			i = ++j + (len - 1);
			j = value.find(delim, j);
			if(j == string::npos){
				data = value.substr(i, value.length());
				result.push_back(Anyks::trim(data));
			}
		}
		// Если данные не существуют то устанавливаем строку по умолчанию
		if(result.empty()) result.push_back(value);
	}
	// Выводим результат
	return result;
}
/**
 * getUid Функция вывода идентификатора пользователя
 * @param  name имя пользователя
 * @return      полученный идентификатор пользователя
 */
uid_t Anyks::getUid(const char * name){
	// Получаем идентификатор имени пользователя
	struct passwd * pwd = getpwnam(name);
	// Если идентификатор пользователя не найден
	if(pwd == nullptr){
		// Выводим сообщение об ошибке
		printf("failed to get userId from username [%s]\r\n", name);
		// Сообщаем что ничего не найдено
		return 0;
	}
	// Выводим идентификатор пользователя
	return pwd->pw_uid;
}
/**
 * getGid Функция вывода идентификатора группы пользователя
 * @param  name название группы пользователя
 * @return      полученный идентификатор группы пользователя
 */
gid_t Anyks::getGid(const char * name){
	// Получаем идентификатор группы пользователя
	struct group * grp = getgrnam(name);
	// Если идентификатор группы не найден
	if(grp == nullptr){
		// Выводим сообщение об ошибке
		printf("failed to get groupId from groupname [%s]\r\n", name);
		// Сообщаем что ничего не найдено
		return 0;
	}
	// Выводим идентификатор группы пользователя
	return grp->gr_gid;
}
/**
 * setOwner Функция установки владельца на каталог
 * @param path  путь к файлу или каталогу для установки владельца
 * @param user  данные пользователя
 * @param group идентификатор группы
 */
void Anyks::setOwner(const char * path, const string user, const string group){
	uid_t uid;	// Идентификатор пользователя
	gid_t gid;	// Идентификатор группы
	// Размер строкового типа данных
	string::size_type sz;
	// Если идентификатор пользователя пришел в виде числа
	if(Anyks::isNumber(user)) uid = stoi(user, &sz);
	// Если идентификатор пользователя пришел в виде названия
	else uid = Anyks::getUid(user.c_str());
	// Если идентификатор группы пришел в виде числа
	if(Anyks::isNumber(group)) gid = stoi(group, &sz);
	// Если идентификатор группы пришел в виде названия
	else gid = Anyks::getGid(group.c_str());
	// Устанавливаем права на каталог
	if(uid && gid) chown(path, uid, gid);
}
/**
 * mkDir Метод рекурсивного создания каталогов
 * @param path адрес каталогов
 */
void Anyks::mkDir(const char * path){
	// Буфер с названием каталога
	char tmp[256];
	// Указатель на сепаратор
	char * p = nullptr;
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
 * rmDir Метод удаления каталога и всего содержимого
 * @param  path путь до каталога
 * @return      количество дочерних элементов
 */
const int Anyks::rmDir(const char * path){
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
					if(S_ISDIR(statbuf.st_mode)) r2 = Anyks::rmDir(buf);
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
 * makePath Функция создания каталога для хранения логов
 * @param  path  адрес для каталога
 * @param  user  данные пользователя
 * @param  group идентификатор группы
 * @return       результат создания каталога
 */
bool Anyks::makePath(const char * path, const string user, const string group){
	// Проверяем существует ли нужный нам каталог
	if(!Anyks::isDirExist(path)){
		// Создаем каталог
		Anyks::mkDir(path);
		// Устанавливаем права на каталог
		Anyks::setOwner(path, user, group);
		// Сообщаем что все удачно
		return true;
	}
	// Сообщаем что все создано удачно
	return true;
}
/**
 * isDirExist Функция проверки существования каталога
 * @param  path адрес каталога
 * @return      результат проверки
 */
bool Anyks::isDirExist(const char * path){
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
bool Anyks::isFileExist(const char * path){
	// Структура проверка статистики
	struct stat info;
	// Проверяем переданный нам адрес
	if(stat(path, &info) != 0) return false;
	// Если это файл
	return (info.st_mode & S_IFMT) != 0;
}
/**
 * addToPath Метод формирования адреса из пути и названия файла
 * @param  path путь где хранится файл
 * @param  file название файла
 * @return      сформированный путь
 */
const string Anyks::addToPath(const string path, const string file){
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
 * md5 Метод получения md5 хэша из строки
 * @param text текст для перевода в строку
 */
const string Anyks::md5(const string text){
	// Результат работы функции
	string result;
	// Если текст передан
	if(!text.empty()){
		// Массив полученных значений
		u_char digest[16];
		// Создаем контекст
		MD5_CTX ctx;
		// Выполняем инициализацию контекста
		MD5_Init(&ctx);
		// Выполняем расчет суммы
		MD5_Update(&ctx, text.c_str(), text.length());
		// Копируем полученные данные
		MD5_Final(digest, &ctx);
		// Строка md5
		char mdString[33];
		// Заполняем массив нулями
		memset(mdString, 0, 33);
		// Заполняем строку данными md5
		for(u_short i = 0; i < 16; i++) sprintf(&mdString[i * 2], "%02x", u_int(digest[i]));
		// Выводим результат
		result = mdString;
	}
	// Выводим результат
	return result;
}
/**
 * sha1 Метод получения sha1 хэша из строки
 * @param text текст для перевода в строку
 */
const string Anyks::sha1(const string text){
	// Результат работы функции
	string result;
	// Если текст передан
	if(!text.empty()){
		// Массив полученных значений
		u_char digest[20];
		// Создаем контекст
		SHA_CTX ctx;
		// Выполняем инициализацию контекста
		SHA1_Init(&ctx);
		// Выполняем расчет суммы
		SHA1_Update(&ctx, text.c_str(), text.length());
		// Копируем полученные данные
		SHA1_Final(digest, &ctx);
		// Строка sha1
		char mdString[41];
		// Заполняем массив нулями
		memset(mdString, 0, 41);
		// Заполняем строку данными sha1
		for(u_short i = 0; i < 20; i++) sprintf(&mdString[i * 2], "%02x", u_int(digest[i]));
		// Выводим результат
		result = mdString;
	}
	// Выводим результат
	return result;
}
/**
 * sha256 Метод получения sha256 хэша из строки
 * @param text текст для перевода в строку
 */
const string Anyks::sha256(const string text){
	// Результат работы функции
	string result;
	// Если текст передан
	if(!text.empty()){
		// Массив полученных значений
		u_char digest[32];
		// Создаем контекст
		SHA256_CTX ctx;
		// Выполняем инициализацию контекста
		SHA256_Init(&ctx);
		// Выполняем расчет суммы
		SHA256_Update(&ctx, text.c_str(), text.length());
		// Копируем полученные данные
		SHA256_Final(digest, &ctx);
		// Строка sha256
		char mdString[65];
		// Заполняем массив нулями
		memset(mdString, 0, 65);
		// Заполняем строку данными sha256
		for(u_short i = 0; i < 32; i++) sprintf(&mdString[i * 2], "%02x", u_int(digest[i]));
		// Выводим результат
		result = mdString;
	}
	// Выводим результат
	return result;
}
/**
 * sha512 Метод получения sha512 хэша из строки
 * @param text текст для перевода в строку
 */
const string Anyks::sha512(const string text){
	// Результат работы функции
	string result;
	// Если текст передан
	if(!text.empty()){
		// Массив полученных значений
		u_char digest[64];
		// Создаем контекст
		SHA512_CTX ctx;
		// Выполняем инициализацию контекста
		SHA512_Init(&ctx);
		// Выполняем расчет суммы
		SHA512_Update(&ctx, text.c_str(), text.length());
		// Копируем полученные данные
		SHA512_Final(digest, &ctx);
		// Строка sha512
		char mdString[129];
		// Заполняем массив нулями
		memset(mdString, 0, 129);
		// Заполняем строку данными sha512
		for(u_short i = 0; i < 64; i++) sprintf(&mdString[i * 2], "%02x", u_int(digest[i]));
		// Выводим результат
		result = mdString;
	}
	// Выводим результат
	return result;
}
/**
 * strToTime Метод перевода строки в timestamp
 * @param  date строка даты
 * @return      timestamp
 */
const time_t Anyks::strToTime(const char * date){
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
 * timeToStr Метод преобразования timestamp в строку
 * @param  date дата в timestamp
 * @return      строка содержащая дату
 */
const string Anyks::timeToStr(const time_t date){
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
 * getSizeBuffer Функция получения размера буфера в байтах
 * @param  str пропускная способность сети (bps, kbps, Mbps, Gbps)
 * @return     размер буфера в байтах
 */
const long Anyks::getSizeBuffer(const string str){
	/*
	* Help - http://www.securitylab.ru/analytics/243414.php
	*
	* 0.04 - Пропускная способность сети 40 милисекунд
	* 100 - Скорость в мегабитах (Мб) на пользователя
	* 8 - Количество бит в байте
	* 1024000 - количество байт в мегабайте
	* (2 * 0.04) * ((100 * 1024000) / 8)  = 1000 байт
	*
	*/
	// Размер буфера в байтах
	long size = -1;
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e("\\b([\\d\\.\\,]+)(bps|kbps|Mbps|Gbps)", regex::ECMAScript);
	// Выполняем поиск скорости
	regex_search(str, match, e);
	// Если данные найдены
	if(!match.empty()){
		// Запоминаем параметры
		string param = match[2].str();
		// Размерность скорости
		double dimension = 1;
		// Получаем значение скорости
		double speed = ::atof(match[1].str().c_str());
		// Проверяем являются ли переданные данные байтами (8, 16, 32, 64, 128, 256, 512, 1024 ...)
		bool isbite = !fmod(speed / 8, 2);
		// Если это байты
		if(param.compare("bps") == 0) dimension = 1;
		// Если это размерность в киллобитах
		else if(param.compare("kbps") == 0) dimension = (isbite ? 1000 : 1024);
		// Если это размерность в мегабитах
		else if(param.compare("Mbps") == 0) dimension = (isbite ? 1000000 : 1048576);
		// Если это размерность в гигабитах
		else if(param.compare("Gbps") == 0) dimension = (isbite ? 1000000000 : 1073741824);
		// Размер буфера по умолчанию
		size = (long) speed;
		// Если скорость установлена тогда расчитываем размер буфера
		if(speed > -1) size = (2 * 0.04) * ((speed * dimension) / 8);
	}
	// Выводим результат
	return size;
}
/**
 * getBytes Функция получения размера в байтах из строки
 * @param  str строка обозначения размерности
 * @return     размер в байтах
 */
const size_t Anyks::getBytes(const string str){
	// Размер количество байт
	size_t size = 0;
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e("\\b([\\d\\.\\,]+)(B|KB|MB|GB)", regex::ECMAScript);
	// Выполняем размерности данных
	regex_search(str, match, e);
	// Если данные найдены
	if(!match.empty()){
		// Запоминаем параметры
		string param = match[2].str();
		// Размерность скорости
		double dimension = 1;
		// Получаем значение размерности
		double value = ::atof(match[1].str().c_str());
		// Проверяем являются ли переданные данные байтами (8, 16, 32, 64, 128, 256, 512, 1024 ...)
		bool isbite = !fmod(value / 8, 2);
		// Если это байты
		if(param.compare("B") == 0) dimension = 1;
		// Если это размерность в киллобитах
		else if(param.compare("KB") == 0) dimension = (isbite ? 1000 : 1024);
		// Если это размерность в мегабитах
		else if(param.compare("MB") == 0) dimension = (isbite ? 1000000 : 1048576);
		// Если это размерность в гигабитах
		else if(param.compare("GB") == 0) dimension = (isbite ? 1000000000 : 1073741824);
		// Размер буфера по умолчанию
		size = (long) value;
		// Если размерность установлена тогда расчитываем количество байт
		if(value > -1) size = (value * dimension);
	}
	// Выводим результат
	return size;
}
/**
 * getSeconds Функция получения размера в секундах из строки
 * @param  str строка обозначения размерности
 * @return     размер в секундах
 */
const size_t Anyks::getSeconds(const string str){
	// Количество секунд
	size_t seconds = 0;
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e("\\b([\\d\\.\\,]+)(s|m|h|d|M|y)", regex::ECMAScript);
	// Выполняем поиск времени
	regex_search(str, match, e);
	// Если данные найдены
	if(!match.empty()){
		// Запоминаем параметры
		string param = match[2].str();
		// Размерность времени
		double dimension = 1;
		// Получаем значение размерности
		double value = ::atof(match[1].str().c_str());
		// Если это секунды
		if(param.compare("s") == 0) dimension = 1;
		// Если это размерность в минутах
		else if(param.compare("m") == 0) dimension = 60;
		// Если это размерность в часах
		else if(param.compare("h") == 0) dimension = 3600;
		// Если это размерность в днях
		else if(param.compare("d") == 0) dimension = 86400;
		// Если это размерность в месяцах
		else if(param.compare("М") == 0) dimension = 2592000;
		// Если это размерность в годах
		else if(param.compare("y") == 0) dimension = 31104000;
		// Размер буфера по умолчанию
		seconds = (long) value;
		// Если время установлено тогда расчитываем количество секунд
		if(value > -1) seconds = (value * dimension);
	}
	// Выводим результат
	return seconds;
}
/**
 * isAddress Метод проверки на то является ли строка адресом
 * @param  address строка адреса для проверки
 * @return         результат проверки
 */
const bool Anyks::isAddress(const string address){
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e(
		// Определение домена
		"(?:[\\w\\-\\.]+\\.[\\w\\-]+|"
		// Определение мак адреса
		"[A-Fa-f\\d]{2}(?:\\:[A-Fa-f\\d]{2}){5}|"
		// Определение ip4 адреса
		"\\d{1,3}(?:\\.\\d{1,3}){3}|"
		// Определение ip6 адреса
		"(?:\\:\\:ffff\\:\\d{1,3}(?:\\.\\d{1,3}){3}|(?:[A-Fa-f\\d]{1,4}(?:(?:\\:[A-Fa-f\\d]{1,4}){7}|(?:\\:[A-Fa-f\\d]{1,4}){1,6}\\:\\:|\\:\\:)|\\:\\:)))",
		regex::ECMAScript | regex::icase
	);
	// Выполняем проверку
	regex_search(address, match, e);
	// Выводим результат
	return !match.empty();
}
/**
 * getTypeAmingByString Метод определения типа данных из строки
 * @param  str строка с данными
 * @return     определенный тип данных
 */
const u_int Anyks::getTypeAmingByString(const string str){
	// Результат полученных данных
	u_int result = AMING_NULL;
	// Если строка передана
	if(!str.empty()){
		// Результат работы регулярного выражения
		smatch match;
		// Устанавливаем правило регулярного выражения
		regex e(
			// Определение домена
			"^(?:([\\w\\-\\.]+\\.[\\w\\-]+)|"
			// Определение мак адреса
			"([A-Fa-f\\d]{2}(?:\\:[A-Fa-f\\d]{2}){5})|"
			// Определение ip4 адреса
			"(\\d{1,3}(?:\\.\\d{1,3}){3})|"
			// Определение ip6 адреса
			"(\\:\\:ffff\\:\\d{1,3}(?:\\.\\d{1,3}){3}|(?:[A-Fa-f\\d]{1,4}(?:(?:\\:[A-Fa-f\\d]{1,4}){7}|(?:\\:[A-Fa-f\\d]{1,4}){1,6}\\:\\:|\\:\\:)|\\:\\:))|"
			// Если это сеть
			"((?:\\d{1,3}(?:\\.\\d{1,3}){3}|(?:[A-Fa-f\\d]{1,4}(?:(?:\\:[A-Fa-f\\d]{1,4}){7}|(?:\\:[A-Fa-f\\d]{1,4}){1,6}\\:\\:|\\:\\:)|\\:\\:))\\/(?:\\d{1,3}(?:\\.\\d{1,3}){3}|\\d+))|"
			// Если это экшен
			"(add|rm)|"
			// Если это метод
			"(" PROXY_HTTP_METHODS ")|"
			// Если это направление трафика
			"(in|out))$",
			regex::ECMAScript | regex::icase
		);
		// Выполняем проверку
		regex_search(str, match, e);
		// Если результат найден
		if(!match.empty()){
			// Извлекаем полученные данные
			const string domain = match[1].str();
			const string mac = match[2].str();
			const string ip4 = match[3].str();
			const string ip6 = match[4].str();
			const string network = match[5].str();
			const string action = match[6].str();
			const string method = match[7].str();
			const string traffic = match[8].str();
			// Определяем тип данных
			if(!domain.empty())			result = AMING_DOMAIN;
			else if(!mac.empty())		result = AMING_MAC;
			else if(!ip4.empty())		result = AMING_IPV4;
			else if(!ip6.empty())		result = AMING_IPV6;
			else if(!network.empty())	result = AMING_NETWORK;
			else if(!action.empty())	result = AMING_HTTP_ACTION;
			else if(!method.empty())	result = AMING_HTTP_METHOD;
			else if(!traffic.empty())	result = AMING_HTTP_TRAFFIC;
		}
	}
	// Выводим результат
	return result;
}
/**
 * checkDomain Метод определения определения соответствия домена маски
 * @param  domain название домена
 * @param  mask   маска домена для проверки
 * @return        результат проверки
 */
const bool Anyks::checkDomainByMask(const string domain, const string mask){
	// Результат проверки домена
	bool result = false;
	// Если и домен и маска переданы
	if(!domain.empty() && !mask.empty()){
		// Итератор
		string itm = "";
		// Выполняем разбиение домена на составляющие
		vector <string> dom = Anyks::split(Anyks::toCase(domain), ".");
		// Выполняем разбиение маски
		vector <string> msk = Anyks::split(Anyks::toCase(mask), ".");
		// Выполняем реверс данных
		reverse(begin(dom), end(dom));
		reverse(begin(msk), end(msk));
		// Запоминаем размеры массивов
		const size_t dl = dom.size(), ml = msk.size();
		// Запоминаем максимальный размер массива
		const size_t max = (dl > ml ? dl : ml);
		// Запоминаем минимальный размер массива
		const size_t min = (dl < ml ? dl : ml);
		// Переходим по максимальному массиву
		for(u_int i = 0; i < max; i++){
			// Если мы не вышли за границу массива
			if(i < min){
				// Запоминаем значение маски
				itm = msk[i];
				// Сравниваем поэлементно
				if((itm.compare("*") == 0)
				|| (itm.compare(dom[i]) == 0)) result = true;
				// Если сравнение не сработало тогда выходим
				else result = false;
			// Если сравнение получилось
			} else if(itm.compare("*") == 0) result = true;
			// Если сравнение не сработало тогда выходим
			else result = false;
			// Если сравнение не удачно то выходим
			if(!result) break;
		}
	}
	// Выводим результат
	return result;
}
