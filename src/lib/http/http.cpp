/* HTTP ПАРСЕР ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#include "http.h"

// Устанавливаем пространство имен
using namespace std;

/**
 * toCase Функция перевода в указанный регистр
 * @param  str  строка для перевода в указанных регистр
 * @param  flag флаг указания типа регистра
 * @return      результирующая строка
 */
string toCase(string str, bool flag = false){
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
bool isNumber(const string &str){
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
string & rtrim(string &str, const char * t = " \t\n\r\f\v"){
	str.erase(str.find_last_not_of(t) + 1);
	return str;
}
/**
 * ltrim Функция усечения указанных символов с левой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & ltrim(string &str, const char * t = " \t\n\r\f\v"){
	str.erase(0, str.find_first_not_of(t));
	return str;
}
/**
 * trim Функция усечения указанных символов с правой и левой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & trim(string &str, const char * t = " \t\n\r\f\v"){
	return ltrim(rtrim(str, t), t);
}
/**
 * checkPort Функция проверки на качество порта
 * @param  port входная строка якобы содержащая порт
 * @return      результат проверки
 */
bool checkPort(string str){
	// Если строка существует
	if(!str.empty()){
		// Преобразуем строку в цифры
		if(::isNumber(str)){
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
 * split Функция разделения строк на составляющие
 * @param str   строка для поиска
 * @param delim разделитель
 * @param v     результирующий вектор
 */
void HttpHeaders::split(const string &str, const string delim, vector <string> &v){
	string::size_type i = 0;
	string::size_type j = str.find(delim);
	u_int len = delim.length();
	// Выполняем разбиение строк
	while(j != string::npos){
		v.push_back(str.substr(i, j - i));
		i = ++j + (len - 1);
		j = str.find(delim, j);
		if(j == string::npos) v.push_back(str.substr(i, str.length()));
	}
}
/**
 * getHeader Функция извлекает данные заголовка по его ключу
 * @param  key ключ заголовка
 * @return     строка с данными заголовка
 */
HttpHeaders::Header HttpHeaders::getHeader(string key){
	// Проверяем существует ли такой заголовок
	if(this->headers.count(key) > 0) return this->headers.find(key)->second;
	// Сообщаем что ничего не найдено
	return {"", ""};
}
/**
 * clear Метод очистки данных
 */
void HttpHeaders::clear(){
	// Очищаем данные http заголовков
	this->headers.clear();
}
/**
 * create Метод создания объекта http заголовков
 * @param buffer буфер с текстовыми данными
 */
bool HttpHeaders::create(const char * buffer){
	// Очищаем заголовки
	clear();
	// Запоминаем буфер входящих данных
	string str = buffer;
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e(
		"^((?:(?:(?:OPTIONS|GET|HEAD|POST|PUT|PATCH|DELETE|TRACE|CONNECT)"
		"\\s+[^\\r\\n\\s]+\\s+[A-Za-z]+\\/[\\d\\.]+\\r\\n)|(?:[A-Za-z]+"
		"\\/[\\d\\.]+\\s+\\d+\\s+[^\\r\\n]+\\r\\n))(?:[\\w\\-]+\\s*\\:"
		"\\s*[^\\r\\n]+\\r\\n)+\\r\\n)",
		regex::ECMAScript | regex::icase
	);
	// Выполняем поиск протокола
	regex_search(str, match, e);
	// Если данные найдены
	if(!match.empty() && (match.size() == 2)){
		// Запоминаем первые символы
		str = match[1];
		// Массив строк
		vector <string> strings;
		// Выполняем разбиение строк
		split(str, "\r\n", strings);
		// Если строки найдены
		if(!strings.empty()){
			// Переходим по всему массиву строк
			for(u_int i = 1; i < strings.size(); i++){
				// Результат работы регулярного выражения
				smatch match;
				// Устанавливаем правило регулярного выражения
				regex e("\\b([\\w\\-]+)\\s*\\:\\s*([\\s\\S]+)", regex::ECMAScript | regex::icase);
				// Выполняем поиск протокола
				regex_search(strings[i], match, e);
				// Если заголовок найден
				if(!match.empty() && (match.size() == 3)){
					// Получаем ключ
					string key = match[1];
					// Получаем значение
					string val = match[2];
					// Создаем заголовок
					Header header = {::trim(key), ::trim(val)};
					// Запоминаем найденны параметры
					this->headers.insert(pair <string, Header>(::toCase(header.head), header));
				}
			}
			// Если данные получены
			return this->headers.size();
		}
	}
	// Сообщаем что ничего не вышло
	return false;
}
/**
 * empty Метод определяет наличие данных
 * @return проверка о наличи данных
 */
bool HttpHeaders::empty(){
	// Выводим существования данных http заголовков
	return this->headers.empty();
}
/**
 * size Метод получения размера
 * @return данные размера
 */
size_t HttpHeaders::size(){
	// Выводим размер http заголовков
	return this->headers.size();
}
/**
 * cbegin Метод получения начального итератора
 * @return начальный итератор
 */
map <string, HttpHeaders::Header>::const_iterator HttpHeaders::cbegin() const noexcept {
	// Выводим начальный итератор
	return this->headers.cbegin();
}
/**
 * cend Метод получения конечного итератора
 * @return конечный итератор
 */
map <string, HttpHeaders::Header>::const_iterator HttpHeaders::cend() const noexcept {
	// Выводим конечный итератор
	return this->headers.cend();
}
/**
 * ~HttpHeaders Деструктор
 */
HttpHeaders::~HttpHeaders(){
	// Удаляем данные http заголовков
	map <string, Header> ().swap(this->headers);
}
/**
 * clear Метод очистки данных
 */
void HttpQuery::clear(){
	// Очищаем все переменные
	this->code		= 0;
	this->offset	= 0;
	this->result.clear();
}
/**
 * data Метод получения данных запроса
 * @return данные запроса
 */
const char * HttpQuery::data(){
	// Выводим данные http запроса
	return (!this->result.empty() ? this->result.data() : "");
}
/**
 * size Метод получения размера
 * @return данные размера
 */
size_t HttpQuery::size(){
	// Выводим размер http запроса
	return this->result.size();
}
/**
 * empty Метод определяет наличие данных
 * @return проверка о наличи данных
 */
bool HttpQuery::empty(){
	// Выводим существования данных http запроса
	return this->result.empty();
}
/**
 * HttpQuery Конструктор
 * @param code       код сообщения
 * @param mess       данные сообщения
 * @param entitybody вложенные данные
 */
HttpQuery::HttpQuery(short code, string mess, vector <char> entitybody){
	// Очищаем вектор
	this->result.clear();
	// Если строка существует
	if(!mess.empty()){
		// Устанавливаем код сообщения
		this->code = code;
		// Копируем в вектор сам запрос
		this->result.assign(mess.begin(), mess.end());
		// Если данные существуют
		if(!entitybody.empty()){
			// Копируем в результирующий вектор данные вложения
			copy(entitybody.begin(), entitybody.end(), back_inserter(this->result));
		}
	}
}
/**
 * ~HttpQuery Деструктор
 */
HttpQuery::~HttpQuery(){
	// Очищаем все переменные
	this->code		= 0;
	this->offset	= 0;
	vector <char> ().swap(this->result);
}
/**
 * getHttpRequest Метод получения сформированного http запроса только с добавлением заголовков
 * @return сформированный http запрос
 */
vector <char> HttpData::getHttpRequest(){
	// Определяем тип прокси
	bool smart = (this->options & OPT_SMART);
	// Определяем разрешено ли выводить название агента
	bool agent = (this->options & OPT_AGENT);
	// Формируем запрос
	string request = (this->http + string("\r\n"));
	// Тип подключения
	string connection = this->headers.getHeader("connection").value;
	// Если заголовок не найден тогда устанавливаем по умолчанию
	if(connection.empty()) connection = "close";
	// Добавляем остальные заголовки
	for(auto it = this->headers.cbegin(); it != this->headers.cend(); ++it){
		// Фильтруем заголовки
		if((it->first != "connection")
		&& (it->first != "proxy-authorization")
		&& (!smart || (smart && (it->first != "proxy-connection")))){
			// Добавляем оставшиеся заголовки
			request.append(it->second.head + string(": ") + it->second.value + string("\r\n"));
		}
	}
	// Устанавливаем название прокси
	if(agent) request.append(string("Proxy-Agent: ") + this->appName + string("/") + this->appVersion + string("\r\n"));
	// Если это dumb прокси
	if(!smart){
		// Проверяем есть ли заголовок соединения прокси
		string proxy_connection = this->headers.getHeader("proxy-connection").value;
		// Добавляем заголовок закрытия подключения
		if(proxy_connection.empty()) request.append(string("Proxy-Connection: ") + connection + string("\r\n"));
	// Если это smart прокси
	} else {
		// Получаем заголовок Proxy-Connection
		string proxy_connection = this->headers.getHeader("proxy-connection").value;
		// Если постоянное соединение не установлено
		if(!proxy_connection.empty()) connection = proxy_connection;
	}
	// Добавляем тип подключения
	request.append(string("Connection: ") + connection + string("\r\n"));
	// Запоминаем конец запроса
	request.append(string("\r\n"));
	// Создаем результат
	vector <char> result(request.begin(), request.end());
	// Выводим результат
	return result;
}
/**
 * createHead Функция получения сформированного заголовка запроса
 */
void HttpData::createHead(){
	// Определяем тип прокси
	bool smart = (this->options & OPT_SMART);
	// Определяем разрешено ли сжатие
	bool gzip = (this->options & OPT_GZIP);
	// Определяем разрешено ли выводить название агента
	bool agent = (this->options & OPT_AGENT);
	// Определяем разрешено ли постоянное подключение
	bool keepalive = (this->options & OPT_KEEPALIVE);
	// Очищаем заголовок
	this->request.clear();
	// Создаем строку запроса
	this->request.append(
		::toCase(this->method, true)
		+ string(" ") + this->path
		+ string(" ") + string("HTTP/")
		+ this->version + string("\r\n")
	);
	/*
	// Устанавливаем заголовок Host:
	this->request.append(
		string("Host: ") + this->host
		+ string(":") + this->port + string("\r\n")
	);
	*/
	// Устанавливаем заголовок Host:
	this->request.append(string("Host: ") + this->host + string("\r\n"));
	// Добавляем useragent
	if(!this->useragent.empty()) this->request.append(string("User-Agent: ") + this->useragent + string("\r\n"));
	// Добавляем остальные заголовки
	for(auto it = this->headers.cbegin(); it != this->headers.cend(); ++it){
		// Фильтруем заголовки
		if((it->first != "host")
		&& (it->first != "user-agent")
		&& (it->first != "connection")
		&& (it->first != "proxy-authorization")
		&& (gzip || (!gzip && (it->first != "accept-encoding")))
		&& (!smart || (smart && (it->first != "proxy-connection")))){
			// Добавляем оставшиеся заголовки
			this->request.append(it->second.head + string(": ") + it->second.value + string("\r\n"));
		}
	}
	// Устанавливаем название прокси
	if(agent) this->request.append(string("Proxy-Agent: ") + this->appName + string("/") + this->appVersion + string("\r\n"));
	// Если постоянное подключение запрещено
	if(!keepalive) this->connection = "close";
	// Добавляем заголовок connection
	if(!this->connection.empty()){
		// Устанавливаем заголовок подключения
		this->request.append(string("Connection: ") + this->connection + string("\r\n"));
		// Если это dumb прокси
		if(!smart){
			// Проверяем есть ли заголовок соединения прокси
			string pc = this->headers.getHeader("proxy-connection").value;
			// Добавляем заголовок закрытия подключения
			if(pc.empty()) this->request.append(string("Proxy-Connection: ") + this->connection + string("\r\n"));
		}
	}
	// Запоминаем конец запроса
	this->request.append(string("\r\n"));
}
/**
 * getConnection Функция извлечения данных подключения
 * @param  str строка запроса
 * @return     объект с данными запроса
 */
HttpData::Connect HttpData::getConnection(string str){
	// Полученные данные подключения
	Connect data;
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex ep("\\b([A-Za-z]+):\\/\\/", regex::ECMAScript | regex::icase);
	// Выполняем поиск протокола
	regex_search(str, match, ep);
	// Если протокол найден
	if(!match.empty() && (match.size() == 2)){
		// Запоминаем версию протокола
		data.protocol = ::toCase(match[1]);
	// Устанавливаем протокол по умолчанию
	} else data.protocol = ::toCase(this->protocol);
	// Устанавливаем правило для поиска
	regex eh("\\b([\\w\\.\\-]+\\.[A-Za-z]+)(?:\\:(\\d+))?", regex::ECMAScript | regex::icase);
	// Выполняем поиск домена и порта
	regex_search(str, match, eh);
	// Если протокол найден
	if(!match.empty() && (match.size() > 1)){
		// Запоминаем хост
		data.host = ::toCase(match[1]);
		// Если порт найден, тогда запоминаем его
		if(match.size() == 3) data.port = match[2];
		// Устанавливаем порт по умолчанию
		else data.port = "80";
	}
	// Устанавливаем номер порта в зависимости от типа протокола
	if(!strcmp(data.protocol.c_str(), "https")
	|| !strcmp(this->method.c_str(), "connect")) data.port = "443";
	// Устанавливаем версию протокола в зависимости от порта
	if(!strcmp(data.port.c_str(), "443")) data.protocol = "https";
	// Выводим результат
	return data;
}
/**
 * clear Метод очистки структуры
 */
void HttpData::clear(){
	// Обнуляем размер
	this->length = 0;
	// Очищаем строки
	this->query.clear();
	this->http.clear();
	this->auth.clear();
	this->method.clear();
	this->path.clear();
	this->protocol.clear();
	this->version.clear();
	this->host.clear();
	this->port.clear();
	this->login.clear();
	this->password.clear();
	this->useragent.clear();
	this->connection.clear();
	this->request.clear();
	// Очищаем карту заголовков
	this->headers.clear();
}
/**
 * genDataConnect Метод генерации данных для подключения
 */
void HttpData::genDataConnect(){
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e("\\b([A-Za-z]+)\\s+([\\s\\S]+)\\s+([A-Za-z]+)\\/([\\d\\.]+)", regex::ECMAScript | regex::icase);
	// Выполняем поиск протокола
	regex_search(this->http, match, e);
	// Если данные найдены
	if(!match.empty() && (match.size() == 5)){
		// Запоминаем метод запроса
		this->method = ::toCase(match[1]);
		// Запоминаем путь запроса
		this->path = match[2];
		// Запоминаем протокол запроса
		this->protocol = ::toCase(match[3]);
		// Запоминаем версию протокола
		this->version = match[4];
		// Извлекаем данные хоста
		string host = this->headers.getHeader("host").value;
		// Извлекаем данные авторизации
		string auth = this->headers.getHeader("proxy-authorization").value;
		// Получаем заголовок Proxy-Connection
		string proxy_connection = this->headers.getHeader("proxy-connection").value;
		// Получаем данные юзерагента
		this->useragent = this->headers.getHeader("user-agent").value;
		// Получаем данные заголовока коннекта
		this->connection = ::toCase(this->headers.getHeader("connection").value);
		// Если постоянное соединение не установлено
		if((this->options & OPT_SMART) && !proxy_connection.empty()) this->connection = proxy_connection;
		// Если хост найден
		if(!host.empty()){
			// Выполняем получение параметров подключения
			Connect gcon = getConnection(this->path);
			// Выполняем получение параметров подключения
			Connect scon = getConnection(host);
			// Создаем полный адрес запроса
			string fulladdr1 = scon.protocol + string("://") + scon.host;
			string fulladdr2 = fulladdr1 + "/";
			string fulladdr3 = fulladdr1 + string(":") + scon.port;
			string fulladdr4 = fulladdr3 + "/";
			// Определяем путь
			if(strcmp(::toCase(this->method).c_str(), "connect")
			&& (!strcmp(::toCase(this->path).c_str(), ::toCase(host).c_str())
			|| !strcmp(::toCase(this->path).c_str(), ::toCase(fulladdr1).c_str())
			|| !strcmp(::toCase(this->path).c_str(), ::toCase(fulladdr2).c_str())
			|| !strcmp(::toCase(this->path).c_str(), ::toCase(fulladdr3).c_str())
			|| !strcmp(::toCase(this->path).c_str(), ::toCase(fulladdr4).c_str()))) this->path = "/";
			// Выполняем удаление из адреса доменного имени
			else if(strstr(this->path.c_str(), fulladdr1.c_str()) != NULL){
				// Запоминаем текущий путь
				string tmp_path = this->path;
				// Вырезаем домер из пути
				tmp_path = tmp_path.replace(0, fulladdr1.length(), "");
				// Если путь существует
				if(!tmp_path.empty()) this->path = tmp_path;
			}
			// Запоминаем хост
			this->host = scon.host;
			// Запоминаем порт
			if(strcmp(scon.port.c_str(), gcon.port.c_str())
			&& !strcmp(gcon.port.c_str(), "80")){
				// Запоминаем протокол
				this->protocol = scon.protocol;
				// Уделяем предпочтение 443 порту
				this->port = scon.port;
			// Запоминаем порт такой какой он есть
			} else if(::checkPort(gcon.port)) {
				// Запоминаем протокол
				this->protocol = gcon.protocol;
				// Запоминаем порт
				this->port = gcon.port;
			// Устанавливаем значение порта и протокола по умолчанию
			} else {
				// Запоминаем протокол
				this->protocol = "http";
				// Устанавливаем http порт
				this->port = "80";
			}
		}
		// Если авторизация найдена
		if(!auth.empty()){
			// Устанавливаем правило регулярного выражения
			regex e("\\b([A-Za-z]+)\\s+([\\s\\S]+)", regex::ECMAScript | regex::icase);
			// Выполняем поиск протокола
			regex_search(auth, match, e);
			// Если данные найдены
			if(!match.empty() && (match.size() == 3)){
				// Запоминаем тип авторизации
				this->auth = ::toCase(match[1]);
				// Если это тип авторизация basic, тогда выполняем декодирования данных авторизации
				if(!strcmp(this->auth.c_str(), "basic")){
					// Выполняем декодирование логина и пароля
					string dauth = base64_decode(match[2]);
					// Устанавливаем правило регулярного выражения
					regex e("\\b([\\s\\S]+)\\:([\\s\\S]+)", regex::ECMAScript | regex::icase);
					// Выполняем поиск протокола
					regex_search(dauth, match, e);
					// Если данные найдены
					if(!match.empty() && (match.size() == 3)){
						// Запоминаем логин
						this->login = match[1];
						// Запоминаем пароль
						this->password = match[2];
					}
				}
			}
		}
	}
	// Генерируем параметры для запроса
	createHead();
}
/**
 * isConnect Метод проверяет является ли метод, методом connect
 * @return результат проверки на метод connect
 */
bool HttpData::isConnect(){
	// Сообщаем является ли метод, методом connect
	return !strcmp(this->method.c_str(), "connect");
}
/**
 * isClose Метод проверяет должно ли быть закрыто подключение
 * @return результат проверки на закрытие подключения
 */
bool HttpData::isClose(){
	// Сообщаем должно ли быть закрыто подключение
	return !strcmp(this->connection.c_str(), "close");
}
/**
 * isHttps Метод проверяет является ли подключение защищенным
 * @return результат проверки на защищенное подключение
 */
bool HttpData::isHttps(){
	// Сообщаем является ли продключение защищенным
	return !strcmp(this->protocol.c_str(), "https");
}
/**
 * isAlive Метод определения нужно ли держать соединение для прокси
 * @return результат проверки
 */
bool HttpData::isAlive(){
	// Если это версия протокола 1.1 и подключение установлено постоянное для прокси
	if(getVersion() > 1){
		// Проверяем указан ли заголовок отключения
		if(!strcmp(this->connection.c_str(), "close")) return false;
		// Иначе сообщаем что подключение должно жить долго
		return true;
	// Проверяем указан ли заголовок удержания соединения
	} else if(!strcmp(this->connection.c_str(), "keep-alive")) return true;
	// Сообщаем что подключение жить не должно
	return false;
}
/**
 * size Метод получения размера запроса
 * @return размер запроса
 */
size_t HttpData::size(){
	// Выводим размер запроса
	return this->length;
}
/**
 * getPort Метод получения порта запроса
 * @return порт удаленного ресурса
 */
u_int HttpData::getPort(){
	// Выводим значение переменной
	return (!this->port.empty() ? ::atoi(this->port.c_str()) : 80);
}
/**
 * getVersion Метод получения версии протокола запроса
 * @return версия протокола запроса
 */
float HttpData::getVersion(){
	// Выводим значение переменной
	return (!this->version.empty() ? ::atof(this->version.c_str()) : 1.0);
}
/**
 * getHttp Метод получения http запроса
 * @return http запрос
 */
string HttpData::getHttp(){
	// Выводим значение переменной
	return this->http;
}
/**
 * getMethod Метод получения метода запроса
 * @return метод запроса
 */
string HttpData::getMethod(){
	// Выводим значение переменной
	return this->method;
}
/**
 * getHost Метод получения хоста запроса
 * @return хост запроса
 */
string HttpData::getHost(){
	// Выводим значение переменной
	return this->host;
}
/**
 * getPath Метод получения пути запроса
 * @return путь запроса
 */
string HttpData::getPath(){
	// Выводим значение переменной
	return this->path;
}
/**
 * getProtocol Метод получения протокола запроса
 * @return протокол запроса
 */
string HttpData::getProtocol(){
	// Выводим значение переменной
	return this->protocol;
}
/**
 * getAuth Метод получения метода авторизации запроса
 * @return метод авторизации
 */
string HttpData::getAuth(){
	// Выводим значение переменной
	return this->auth;
}
/**
 * getLogin Метод получения логина авторизации запроса
 * @return логин авторизации
 */
string HttpData::getLogin(){
	// Выводим значение переменной
	return this->login;
}
/**
 * getPassword Метод получения пароля авторизации запроса
 * @return пароль авторизации
 */
string HttpData::getPassword(){
	// Выводим значение переменной
	return this->password;
}
/**
 * getUseragent Метод получения юзерагента запроса
 * @return юзерагент
 */
string HttpData::getUseragent(){
	// Выводим значение переменной
	return this->useragent;
}
/**
 * getQuery Метод получения буфера запроса
 * @return буфер запроса
 */
string HttpData::getQuery(){
	// Выводим значение переменной
	return this->query;
}
/**
 * setEntitybody Метод добавления данных вложения
 * @param buffer буфер с данными вложения
 * @param size   размер буфера
 */
bool HttpData::setEntitybody(const char * buffer, size_t size){
	// Если данные в объекте существуют
	if(this->length){
		// Размер вложений
		u_int body_size = 0;
		// Проверяем есть ли размер вложений
		string cl = this->headers.getHeader("content-length").value;
		// Если найден размер вложений
		// Определяем размер вложений
		if(!cl.empty() && ::isNumber(cl)) body_size = (::atoi(cl.c_str()) + this->length);
		// Если вложения не найдены
		else {
			// Проверяем есть ли чанкование
			string ch = this->headers.getHeader("transfer-encoding").value;
			// Если это чанкование
			if(!ch.empty() && (ch.find("chunked") != string::npos)){
				// Выполняем поиск подстроки
				const char * pch = strstr(buffer + this->length, "0\r\n\r\n");
				// Если конец передачи данных мы нашли
				// Определяем размер вложений
				if(pch != NULL) body_size = ((pch - buffer) - this->length + 5);
				// Если конец не найден
				else {
					// Ищем дальше
					pch = strstr(buffer + this->length, "0\r\n");
					// Если конец передачи данных мы нашли
					// Определяем размер вложений
					if(pch != NULL) body_size = ((pch - buffer) - this->length + 3);
				}
			// Сообщаем что мы закончили
			} else return true;
		}
		// Если данные вложений есть тогда устанавливаем их и выходим
		if(body_size <= size){
			// Извлекаем указанные данные
			this->entitybody.assign(buffer + this->length, buffer + body_size);
			// Увеличиваем максимальный размер данных
			this->length += body_size;
			// Сообщаем что мы закончили
			return true;
		}
	}
	// Сообщаем что ничего не найдено
	return false;
}
/**
 * setMethod Метод установки метода запроса
 * @param str строка с данными для установки
 */
void HttpData::setMethod(const string str){
	// Запоминаем данные
	this->method = str;
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setHost Метод установки хоста запроса
 * @param str строка с данными для установки
 */
void HttpData::setHost(const string str){
	// Запоминаем данные
	this->host = str;
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setPort Метод установки порта запроса
 * @param number номер порта для установки
 */
void HttpData::setPort(u_int number){
	// Запоминаем данные
	this->port = to_string(number);
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setPath Метод установки пути запроса
 * @param str строка с данными для установки
 */
void HttpData::setPath(const string str){
	// Запоминаем данные
	this->path = str;
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setProtocol Метод установки протокола запроса
 * @param str строка с данными для установки
 */
void HttpData::setProtocol(const string str){
	// Запоминаем данные
	this->protocol = str;
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setVersion Метод установки версии протокола запроса
 * @param number номер версии протокола
 */
void HttpData::setVersion(float number){
	// Запоминаем данные
	this->version = to_string(number);
	// Если это всего один символ тогда дописываем ноль
	if(this->version.length() == 1) this->version += ".0";
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setAuth Метод установки метода авторизации запроса
 * @param str строка с данными для установки
 */
void HttpData::setAuth(const string str){
	// Запоминаем данные
	this->auth = str;
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setUseragent Метод установки юзерагента запроса
 * @param str строка с данными для установки
 */
void HttpData::setUseragent(const string str){
	// Запоминаем данные
	this->useragent = str;
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setClose Метод установки принудительного отключения после запроса
 */
void HttpData::setClose(){
	// Запоминаем данные
	this->connection = "close";
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * createRequest Функция создания ответа сервера
 * @param  index   индекс в массиве ответа
 * @param  request номер ответа
 * @return         объект с данными ответа
 */
HttpQuery HttpData::createRequest(u_short index, u_short request){
	// Устанавливаем дефолтное название прокси
	string defname = "ProxyAnyks/1.0";
	// Определяем позицию дефолтного названия
	size_t pos = this->html[index].find(defname);
	// Результирующая строка
	string result;
	// Если это домен
	if(pos != string::npos){
		// Заменяем дефолтное название на указанное
		result = this->html[index]
		.replace(pos, defname.length(), this->appName + string("/") + this->appVersion);
	}
	// Выводим шаблон сообщения о неудачном отправленном запросе
	result = this->html[index];
	// Данные для вывода
	HttpQuery data(request, result);
	// Выводим результат
	return data;
}
/**
 * brokenRequest Метод получения ответа (неудачного отправленного запроса)
 * @return ответ в формате html
 */
HttpQuery HttpData::brokenRequest(){
	// Выводим результат
	return createRequest(9, 501);
}
/**
 * faultConnect Метод получения ответа (неудачного подключения к удаленному серверу)
 * @return ответ в формате html
 */
HttpQuery HttpData::faultConnect(){
	// Выводим результат
	return createRequest(6, 502);
}
/**
 * pageNotFound Метод получения ответа (страница не найдена)
 * @return ответ в формате html
 */
HttpQuery HttpData::pageNotFound(){
	// Выводим результат
	return createRequest(4, 404);
}
/**
 * faultAuth Метод получения ответа (неудачной авторизации)
 * @return ответ в формате html
 */
HttpQuery HttpData::faultAuth(){
	// Выводим результат
	return createRequest(5, 403);
}
/**
 * requiredAuth Метод получения ответа (запроса ввода логина и пароля)
 * @return ответ в формате html
 */
HttpQuery HttpData::requiredAuth(){
	// Выводим результат
	return createRequest(2, 407);
}
/**
 * authSuccess Метод получения ответа (подтверждения авторизации)
 * @return ответ в формате html
 */
HttpQuery HttpData::authSuccess(){
	// Выводим результат
	return createRequest(0, 200);
}
/**
 * getRequest Метод получения сформированного http запроса
 * @return сформированный http запрос
 */
HttpQuery HttpData::getRequest(){
	// Данные для вывода
	HttpQuery data(200, this->request, this->entitybody);
	// Выводим результат
	return data;
}
/**
 * init Метод инициализации класса
 * @param  str     строка http запроса
 * @param  name    название приложения
 * @param  version версия приложения
 * @param  options опции http парсера
 * @return         данные http запроса
 */
void HttpData::init(const string str, const string name, const string version, const u_short options){
	// Проверяем существуют ли данные
	if(!str.empty()){
		// Очищаем полученные данные
		clear();
		// Запоминаем название приложения
		this->appName = name;
		// Запоминаем версию приложения
		this->appVersion = version;
		// Запоминаем параметры http парсера
		this->options = options;
		// Результат работы регулярного выражения
		smatch match;
		// Устанавливаем правило регулярного выражения
		regex e(
			"^((?:(?:OPTIONS|GET|HEAD|POST|PUT|PATCH|DELETE|TRACE|CONNECT)"
			"\\s+[^\\r\\n\\s]+\\s+[A-Za-z]+\\/[\\d\\.]+\\r\\n)|(?:[A-Za-z]+"
			"\\/[\\d\\.]+\\s+\\d+\\s+[^\\r\\n]+\\r\\n))((?:[\\w\\-]+\\s*\\:"
			"\\s*[^\\r\\n]+\\r\\n)+\\r\\n)",
			regex::ECMAScript | regex::icase
		);
		// Выполняем поиск протокола
		regex_search(str, match, e);
		// Если данные найдены
		if(!match.empty() && (match.size() == 3)){
			// Получаем строку запроса
			this->http = match[1];
			// Запоминаем http запрос
			this->http = ::trim(this->http);
			// Запоминаем первые символы
			this->query = match[0];
			// Создаем объект с заголовками
			this->headers.create(this->query.c_str());
			// Получаем длину массива заголовков
			this->length = this->query.length();
		}
		// Генерируем данные подключения
		genDataConnect();
	}
}
/**
 * ~HttpData Деструктор
 */
HttpData::~HttpData(){
	// Очищаем полученные данные
	clear();
	// Очищаем память выделенную для вектора
	vector <char> ().swap(this->entitybody);
}
/**
 * isHttp Метод проверки на то http это или нет
 * @param  buffer буфер входящих данных
 * @return        результат проверки
 */
bool Http::isHttp(const string buffer){
	// Если буфер существует
	if(!buffer.empty()){
		// Создаем новый буфер
		char buf[4];
		// Шаблон основных комманд
		char cmds[8][4] = {"get", "hea", "pos", "put", "pat", "del", "tra", "con"};
		// Копируем нужное количество символов
		strncpy(buf, buffer.c_str(), 3);
		// Устанавливаем завершающий символ
		buf[3] = '\0';
		// Переходим по всему массиву команд
		for(int i = 0; i < 8; i++) if(!strcmp(::toCase(buf).c_str(), cmds[i])) return true;
	}
	// Сообщаем что это не http
	return false;
}
/**
 * parse Функция извлечения данных из буфера
 * @param buffer буфер с входящими запросами
 * @param size   размер входящих данных
 * @param flag   обрабатывать весь блок данных
 */
size_t Http::parse(const char * buffer, size_t size, bool flag){
	// Определяем максимальный размер данных
	size_t maxsize = 0;
	// Результат работы регулярного выражения
	smatch match;
	// Создаем строку для поиска
	string str(buffer, size);
	// Устанавливаем правило регулярного выражения
	regex e(
		"^([^\\r\\n\\s]*)((?:(?:(?:OPTIONS|GET|HEAD|POST|PUT|PATCH|DELETE|TRACE|CONNECT)"
		"\\s+[^\\r\\n\\s]+\\s+[A-Za-z]+\\/[\\d\\.]+\\r\\n)|(?:[A-Za-z]+"
		"\\/[\\d\\.]+\\s+\\d+\\s+[^\\r\\n]+\\r\\n))(?:[\\w\\-]+\\s*\\:"
		"\\s*[^\\r\\n]+\\r\\n)+\\r\\n)",
		regex::ECMAScript | regex::icase
	);
	// Выполняем поиск протокола
	regex_search(str, match, e);
	// Если данные найдены
	if(!match.empty() && (match.size() == 3)){
		// Запоминаем первые символы
		string badchars = match[1];
		// Увеличиваем значение общих найденных символов
		maxsize += badchars.length();
		// Выполняем парсинг http запроса
		HttpData httpData;
		// Выполняем инициализацию объекта
		httpData.init(match[2], name, version, options);
		// Добавляем вложенные данные
		if(httpData.setEntitybody(buffer, size)){
			// Добавляем в массив объект подключения
			this->httpData.push_back(httpData);
			// Увеличиваем общий размер
			maxsize += httpData.size();
			// Следующий размер данных
			size_t nsize = strlen(buffer + maxsize);
			// Если есть еще данные
			if(flag && (nsize >= 34) && (nsize <= (size - maxsize))){
				// Выполняем интерпретацию следующей порции данных
				return parse(buffer + maxsize, size - maxsize, flag);
			// Выводим размер так как он есть
			} else return maxsize;
		}
	}
	// Сообщаем что ничего не найдено
	return 0;
}
/**
 * modify Функция модифицирования ответных данных
 * @param data ссылка на данные полученные от сервера
 */
void Http::modify(vector <char> &data){
	// Получаем данные ответа
	const char * headers = data.data();
	// Создаем объект запроса
	HttpData httpQuery;
	// Выполняем инициализацию объекта
	httpQuery.init(headers, this->name, this->version, this->options);
	// Если данные распарсены
	if(httpQuery.size()){
		// Если завершение заголовка найдено
		u_int pos = (strstr(headers, "\r\n\r\n") - headers) + 4;
		// Получаем данные запроса
		vector <char> last, query = httpQuery.getHttpRequest();
		// Заполняем последние данные структуры
		last.assign(data.begin() + pos, data.end());
		// Объединяем блоки
		copy(last.begin(), last.end(), back_inserter(query));
		// Заменяем первоначальный блок с данными
		data = query;
	}
}
/**
 * Http::clear Метод очистки всех полученных данных
 */
void Http::clear(){
	// Очищаем массив с объектами запросов
	this->httpData.clear();
}
/**
 * Http Конструктор
 * @param name    строка содержащая название прокси сервера
 * @param options параметры прокси сервера
 */
Http::Http(const string name, const u_short options){
	// Если имя передано то запоминаем его
	this->name = name;
	// Запоминаем тип прокси сервера
	this->options = options;
	// Устанавливаем версию системы
	this->version = APP_VERSION;
}
/**
 * ~Http Деструктор
 */
Http::~Http(){
	// Очищаем полученные данные
	clear();
	// Очищаем память выделенную для вектора
	vector <HttpData> ().swap(this->httpData);
}