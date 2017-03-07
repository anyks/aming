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


		// Выводим сообщение в лог
		} else if(!filename.empty() && this->log){
			// Выводим сообщение в лог, что файл не найден
			this->log->write(LOG_WARNING, 0, "headers file (%s) is not found", filename.c_str());
		}
	}
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
}