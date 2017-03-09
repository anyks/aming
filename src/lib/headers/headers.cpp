/* МОДУЛЬ УПРАВЛЕНИЯ ЗАГОЛОВКАМИ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#include "headers/headers.h"

// Устанавливаем область видимости
using namespace std;

/**
 * read Метод чтения из файла параметров
 */
void Headers::read(){
	// Если конфигурационный файл существует
	if(this->config){
		// Получаем данные каталога где хранится файл с правилами
		const string dir = (* this->config)->proxy.dir;
		// Получаем имя файла
		const string filename = addToPath(dir, "headers");
		// Проверяем на существование адреса
		if(!filename.empty()
		// Проверяем существует ли такой каталог
		&& isDirExist(dir.c_str())
		// Проверяем существует ли такой файл
		&& isFileExist(filename.c_str())){
			// Устанавливаем права на файл лога
			setOwner(filename.c_str());
			// Строка чтения из файла
			string filedata;
			// Открываем файл на чтение
			ifstream file(filename.c_str());
			// Если файл открыт
			if(file.is_open()){
				// Считываем до тех пор пока все удачно
				while(file.good()){
					// Считываем строку из файла
					getline(file, filedata);
					// Ищем комментарий
					size_t pos = filedata.find("#");
					// Если комментарий найден, вырезаем его из строки
					if(pos != string::npos) filedata = filedata.replace(pos, filedata.length() - pos, "");
					// Если строка существует
					if(!filedata.empty()){
						// Результат работы регулярного выражения
						smatch match;
						// Создаем регулярное выражение
						regex e(
							"(ADD|RM|\\*)(?:\\s+|\\t+)(IN|OUT|\\*)(?:\\s+|\\t+)([\\w\\.\\-\\@\\:]+|\\*)(?:\\s+|\\t+)"
							"(OPTIONS|GET|HEAD|POST|PUT|PATCH|DELETE|TRACE|CONNECT|\\*)(?:\\s+|\\t+)"
							"([\\w\\.\\-\\@\\:]+|\\*)(?:\\s+|\\t+)([^\\s\\r\\n\\t]+|\\*)(?:\\s+|\\t+)([^\\r\\n\\t]+)",
							regex::ECMAScript | regex::icase
						);
						// Выполняем извлечение данных
						regex_search(filedata, match, e);
						// Если данные найдены
						if(!match.empty()){
							// Создаем объект сети
							Network nwk;
							// Получаем идентификатор пользователя
							string userId = match[3].str();
							// Получаем идентификатор сервера
							string serverId = match[5].str();
							// Получаем типы идентификаторов
							u_short utype = checkTypeId(userId);
							u_short stype = checkTypeId(serverId);
							// Если это ip адреса то преобразуем их
							// Для клиента
							if(utype == 1)		userId		= nwk.setLowIp(userId);		// Если это IPv4
							else if(utype == 2)	userId		= nwk.setLowIp6(userId);	// Если это IPv6
							// Для сервера
							if(stype == 1)		serverId	= nwk.setLowIp(serverId);	// Если это IPv4
							else if(stype == 2)	serverId	= nwk.setLowIp6(serverId);	// Если это IPv6
							// Формируем объект
							Params params = {
								utype, stype, toCase(match[1].str()),
								toCase(match[2].str()), toCase(match[4].str()),
								toCase(serverId), toCase(match[6].str()),
								split(match[7].str(), "|")
							};
							// Добавляем полученные параметры в список
							// add(userId, params);
						}
					}
				}
				// Закрываем файл
				file.close();
			}
		// Выводим сообщение в лог
		} else if(!filename.empty() && this->log){
			// Выводим сообщение в лог, что файл не найден
			this->log->write(LOG_WARNING, 0, "headers file (%s) is not found", filename.c_str());
		}
	}
}
/**
 * toCase Функция перевода в указанный регистр
 * @param  str  строка для перевода в указанных регистр
 * @param  flag флаг указания типа регистра
 * @return      результирующая строка
 */
const string Headers::toCase(string str, bool flag){
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
string & Headers::rtrim(string &str, const char * t){
	str.erase(str.find_last_not_of(t) + 1);
	return str;
}
/**
 * ltrim Функция усечения указанных символов с левой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & Headers::ltrim(string &str, const char * t){
	str.erase(0, str.find_first_not_of(t));
	return str;
}
/**
 * trim Функция усечения указанных символов с правой и левой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & Headers::trim(string &str, const char * t){
	return ltrim(rtrim(str, t), t);
}
/**
 * split Метод разбива строки на составляющие
 * @param  str   исходная строка
 * @param  delim разделитель
 * @return       массив составляющих строки
 */
vector <string> Headers::split(const string str, const string delim){
	// Результат данных
	vector <string> result;
	// Если строка передана
	if(!str.empty()){
		string data;
		string::size_type i = 0;
		string::size_type j = str.find(delim);
		u_int len = delim.length();
		// Выполняем разбиение строк
		while(j != string::npos){
			data = str.substr(i, j - i);
			result.push_back(trim(data));
			i = ++j + (len - 1);
			j = str.find(delim, j);
			if(j == string::npos){
				data = str.substr(i, str.length());
				result.push_back(trim(data));
			}
		}
		// Если данные не существуют то устанавливаем строку по умолчанию
		if(result.empty()) result.push_back(str);
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
const string Headers::addToPath(const string path, const string file){
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
uid_t Headers::getUid(const char * name){
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
gid_t Headers::getGid(const char * name){
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
void Headers::setOwner(const char * path){
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
 * checkTypeId Метод определения типа идентификатора
 * @param  str строка идентификатора для определения типа
 * @return     тип идентификатора
 */
u_short Headers::checkTypeId(const string str){
	// Создаем объект сети
	Network nwk;
	// Тип идентификатора по умолчанию
	u_short type = 0;
	// Проверяем тип идентификатора, на то является ли он IPv4
	if(isIpV4(nwk.setLowIp(str))) type = 1;
	// Проверяем тип идентификатора, на то является ли он IPv6
	else if(isIpV6(nwk.setLowIp6(str))) type = 2;
	// Проверяем тип идентификатора, на то является ли он MAC адресом
	else if(isMac(str)) type = 3;
	// Проверяем тип идентификатора, на то является ли он доменом
	else if(isDomain(str)) type = 4;
	// Выводим результат
	return type;
}
/**
 * is_number Функция проверки является ли строка числом
 * @param  str строка для проверки
 * @return     результат проверки
 */
bool Headers::isNumber(const string &str){
	return !str.empty() && find_if(str.begin(), str.end(), [](char c){
		return !isdigit(c);
	}) == str.end();
}
/**
 * isDirExist Функция проверки существования каталога
 * @param  path адрес каталога
 * @return      результат проверки
 */
bool Headers::isDirExist(const char * path){
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
bool Headers::isFileExist(const char * path){
	// Структура проверка статистики
	struct stat info;
	// Проверяем переданный нам адрес
	if(stat(path, &info) != 0) return false;
	// Если это файл
	return (info.st_mode & S_IFMT) != 0;
}
/**
 * isAddress Метод проверки на то является ли строка адресом
 * @param  address строка адреса для проверки
 * @return         результат проверки
 */
bool Headers::isAddress(const string address){
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e(
		// Определение домена
		"(?:[\\w\\-\\.]+\\.[\\w\\-]+|"
		// Определение мак адреса
		"[A-Fa-f\\d]{2}\\:[A-Fa-f\\d]{2}\\:[A-Fa-f\\d]{2}\\:[A-Fa-f\\d]{2}\\:[A-Fa-f\\d]{2}\\:[A-Fa-f\\d]{2}|"
		// Определение ip адреса
		"\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}|"
		// Определение ip6 адреса (в полном формате)
		"[A-Fa-f\\d]{4}\\:[A-Fa-f\\d]{4}\\:[A-Fa-f\\d]{4}\\:[A-Fa-f\\d]{4}\\:[A-Fa-f\\d]{4}\\:[A-Fa-f\\d]{4}\\:[A-Fa-f\\d]{4}\\:[A-Fa-f\\d]{4})",
		regex::ECMAScript | regex::icase
	);
	// Выполняем проверку
	regex_search(address, match, e);
	// Выводим результат
	return !match.empty();
}
/**
 * isIpV4 Метод проверки на ip адрес, интернет протокола версии 4
 * @param  ip строка ip адреса для проверки
 * @return    результат проверки
 */
bool Headers::isIpV4(const string ip){
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
bool Headers::isIpV6(const string ip){
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
 * isIp Метод проверки на ip адрес
 * @param  ip строка ip адреса для проверки
 * @return    результат проверки
 */
bool Headers::isIp(const string ip){
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e(
		"(?:\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}|"
		"[A-Fa-f\\d]{4}\\:[A-Fa-f\\d]{4}\\:[A-Fa-f\\d]{4}\\:[A-Fa-f\\d]{4}\\:[A-Fa-f\\d]{4}\\:[A-Fa-f\\d]{4}\\:[A-Fa-f\\d]{4}\\:[A-Fa-f\\d]{4})",
		regex::ECMAScript | regex::icase
	);
	// Выполняем проверку
	regex_search(ip, match, e);
	// Выводим результат
	return !match.empty();
}
/**
 * isMac Метод проверки на mac адрес
 * @param  mac строка mac адреса для проверки
 * @return     результат проверки
 */
bool Headers::isMac(const string mac){
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e("[A-Fa-f\\d]{2}\\:[A-Fa-f\\d]{2}\\:[A-Fa-f\\d]{2}\\:[A-Fa-f\\d]{2}\\:[A-Fa-f\\d]{2}\\:[A-Fa-f\\d]{2}", regex::ECMAScript | regex::icase);
	// Выполняем проверку
	regex_search(mac, match, e);
	// Выводим результат
	return !match.empty();
}
/**
 * isDomain Метод проверки на доменное имя
 * @param  domain строка названия домена для проверки
 * @return        результат проверки
 */
bool Headers::isDomain(const string domain){
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
 * isLogin Метод проверки на логин пользователя
 * @param  login строка логина для проверки
 * @return       результат проверки
 */
bool Headers::isLogin(const string login){
	// Выводим результат проверки
	return !isAddress(login);
}
/**
 * clear Метод очистки данных
 */
void Headers::clear(){
	// Очищаем данные правил
	this->rules.clear();
}
/**
 * Headers Конструктор
 * @param log    объект лога для вывода информации
 * @param config конфигурационные данные
 */
Headers::Headers(LogApp * log, Config ** config){
	// Очищаем все параметры
	clear();
	// Запоминаем объект логов
	this->log = log;
	// Запоминаем параметры конфига
	this->config = config;
	// Выполняем чтение файла конфигурации
	read();
}