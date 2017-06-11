/* МОДУЛЬ ОБЩИХ ФУНКЙИЙ ANYKS */
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
void cpydata(const u_char * data, size_t size, size_t &it, void * result){
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
void cpydata(const u_char * data, size_t size, size_t &it, string &result){
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
const string toCase(string str, bool flag){
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
const bool isNumber(const string &str){
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
string & rtrim(string &str, const char * t){
	str.erase(str.find_last_not_of(t) + 1);
	return str;
}
/**
 * ltrim Функция усечения указанных символов с левой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & ltrim(string &str, const char * t){
	str.erase(0, str.find_first_not_of(t));
	return str;
}
/**
 * trim Функция усечения указанных символов с правой и левой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & trim(string &str, const char * t){
	return ltrim(rtrim(str, t), t);
}
/**
 * checkPort Функция проверки на качество порта
 * @param  port входная строка якобы содержащая порт
 * @return      результат проверки
 */
const bool checkPort(string str){
	// Если строка существует
	if(!str.empty()){
		// Преобразуем строку в цифры
		if(isNumber(str)){
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
void replace(string &s, const string f, const string r){
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
void split(const string &str, const string delim, vector <string> &v){
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
vector <string> split(const string str, const string delim){
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
 * getUid Функция вывода идентификатора пользователя
 * @param  name имя пользователя
 * @return      полученный идентификатор пользователя
 */
uid_t getUid(const char * name){
	// Получаем идентификатор имени пользователя
	struct passwd * pwd = getpwnam(name);
	// Если идентификатор пользователя не найден
	if(pwd == NULL){
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
gid_t getGid(const char * name){
	// Получаем идентификатор группы пользователя
	struct group * grp = getgrnam(name);
	// Если идентификатор группы не найден
	if(grp == NULL){
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
void setOwner(const char * path, const string user, const string group){
	uid_t uid;	// Идентификатор пользователя
	gid_t gid;	// Идентификатор группы
	// Размер строкового типа данных
	string::size_type sz;
	// Если идентификатор пользователя пришел в виде числа
	if(isNumber(user)) uid = stoi(user, &sz);
	// Если идентификатор пользователя пришел в виде названия
	else uid = getUid(user.c_str());
	// Если идентификатор группы пришел в виде числа
	if(isNumber(group)) gid = stoi(group, &sz);
	// Если идентификатор группы пришел в виде названия
	else gid = getGid(group.c_str());
	// Устанавливаем права на каталог
	if(uid && gid) chown(path, uid, gid);
}
/**
 * mkDir Метод рекурсивного создания каталогов
 * @param path адрес каталогов
 */
void mkDir(const char * path){
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
 * rmDir Метод удаления каталога и всего содержимого
 * @param  path путь до каталога
 * @return      количество дочерних элементов
 */
const int rmDir(const char * path){
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
					if(S_ISDIR(statbuf.st_mode)) r2 = rmDir(buf);
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
bool makePath(const char * path, const string user, const string group){
	// Проверяем существует ли нужный нам каталог
	if(!isDirExist(path)){
		// Создаем каталог
		mkDir(path);
		// Устанавливаем права на каталог
		setOwner(path, user, group);
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
bool isDirExist(const char * path){
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
bool isFileExist(const char * path){
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
const string addToPath(const string path, const string file){
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
 * strToTime Метод перевода строки в timestamp
 * @param  date строка даты
 * @return      timestamp
 */
const time_t strToTime(const char * date){
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
const string timeToStr(const time_t date){
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
const long getSizeBuffer(const string str){
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
const size_t getBytes(const string str){
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
const size_t getSeconds(const string str){
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
