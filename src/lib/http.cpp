/* HTTP ПАРСЕР ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2016
*/
#include "http.h"

// Устанавливаем пространство имен
using namespace std;
/**
 * split Функция разделения строк на составляющие
 * @param str   строка для поиска
 * @param delim разделитель
 * @param v     результирующий вектор
 */
void Http::split(const string &str, const string delim, vector <string> &v){
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
 * toCase Функция перевода в указанный регистр
 * @param  str  строка для перевода в указанных регистр
 * @param  flag флаг указания типа регистра
 * @return      результирующая строка
 */
string Http::toCase(string str, bool flag){
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
string & Http::rtrim(string &str, const char * t){
	str.erase(str.find_last_not_of(t) + 1);
	return str;
}
/**
 * ltrim Функция усечения указанных символов с левой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & Http::ltrim(string &str, const char * t){
	str.erase(0, str.find_first_not_of(t));
	return str;
}
/**
 * trim Функция усечения указанных символов с правой и левой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & Http::trim(string &str, const char * t){
	return ltrim(rtrim(str, t), t);
}
/**
 * getHeaders Функция извлечения данных http запроса
 * @param  str строка http запроса
 * @return     данные http запроса
 */
Http::HttpData Http::getHeaders(string str){
	// Создаем структуру данных
	HttpData data;
	// Проверяем существуют ли данные
	if(!str.empty()){
		// Определяем конец запроса
		size_t end_query = str.find("\r\n\r\n");
		// Если конец запроса найден
		if(end_query != string::npos){
			// Массив строк
			vector <string> strings, params;
			// Выполняем разбиение строк
			split(str, "\r\n", strings);
			// Если строки найдены
			if(!strings.empty()){
				// Позиция найденного разделителя
				size_t pos;
				// Запоминаем http запрос
				data.http = trim(strings[0]);
				// Переходим по всему массиву строк
				for(u_int i = 1; i < strings.size(); i++){
					// Выполняем поиск разделитель
					pos = strings[i].find(":");
					// Если разделитель найден
					if(pos != string::npos){
						// Получаем ключ
						string key = strings[i].substr(0, pos);
						// Получаем значение
						string val = strings[i].substr(pos + 1, strings[i].length() - (pos + 1));
						// Запоминаем найденны параметры
						data.headers.insert(pair <string, string>(toCase(trim(key)), trim(val)));
						// Запоминаем оригинальные параметры заголовков
						data.origin.insert(pair <string, string>(toCase(trim(key)), trim(key)));
					}
				}
			}
			// Получаем длину массива заголовков
			data.length = end_query + 4;
		}
	}
	// Выводим результат
	return data;
}
/**
 * Http::checkPort Функция проверки на качество порта
 * @param  port входная строка якобы содержащая порт
 * @return      результат проверки
 */
bool Http::checkPort(string port){
	// Если строка существует
	if(!port.empty()){
		// Проверяем цифры это или нет
		bool number = (port.find_first_not_of("0123456789") == string::npos);
		// Преобразуем строку в цифры
		if(number && ::atoi(port.c_str()) > 65535)
			// Если длина порта меньше максимального
			return false;
		// Если порт прошел все проверки
		else return true;
	}
	// Сообщаем что ничего не нашли
	return false;
}
/**
 * getConnection Функция извлечения данных подключения
 * @param  str строка запроса
 * @return     объект с данными запроса
 */
Http::connect Http::getConnection(string str){
	// Полученные данные подключения
	connect data;
	// Выполняем поиск протокола
	size_t pos = str.find("://");
	// Если протокол найден
	if(pos != (size_t) string::npos){
		// Выполняем разделение на протокол
		vector <string> prt;
		// Функция разбиения на составляющие
		split(str, "://", prt);
		// Если протокол найден
		if(!prt.empty()){
			// Запоминаем версию протокола
			data.protocol = prt[0];
			// Запоминаем оставшуюся часть строки
			str = prt[1];
		}
		// Очищаем память выделенную для вектора
		vector <string> ().swap(prt);
	// Устанавливаем протокол по умолчанию
	} else data.protocol = query.protocol;
	// Выполняем поиск порта и хоста
	pos = str.find(":");
	// Если хост и порт найдены
	if(pos != (size_t) string::npos){
		// Выполняем разделение на протокол
		vector <string> prt;
		// Функция разбиения на составляющие
		split(str, ":", prt);
		// Если порт и хост найдены
		if(!prt.empty()){
			// Запоминаем хост
			data.host = prt[0];
			// Запоминаем порт
			data.port = (checkPort(prt[1]) ? prt[1] : "80");
		}
		// Очищаем память выделенную для вектора
		vector <string> ().swap(prt);
	// Если хост и порт не найдены
	} else {
		// Запоминаем хост
		data.host = str;
		// Запоминаем порт
		data.port = "80";
	}
	// Устанавливаем номер порта в зависимости от типа протокола
	if(!strcmp(toCase(data.protocol).c_str(), "https")
	|| !strcmp(toCase(query.method).c_str(), "connect")) data.port = "443";
	// Устанавливаем версию протокола в зависимости от порта
	if(!strcmp(toCase(data.port).c_str(), "443")) data.protocol = "https";
	// Запоминаем найденный хост
	str = data.host;
	// Выполняем поиск дирректории в хосте
	pos = str.find("/");
	// Если дирректория найдена значит это не хост
	if(pos != (size_t) string::npos){
		// Извлекаем название домена
		str = str.substr(0, pos);
		// Выполняем поиск точки в домене
		pos = str.find(".");
		// Если это домен
		if(pos != (size_t) string::npos) data.host = str;
		// Если это не домен то устанавливаем что это корень
		else data.host = "/";
	}
	// Выводим результат
	return data;
}
/**
 * Http::getHeader Функция извлекает данные заголовка по его ключу
 * @param  key     ключ заголовка
 * @param  headers массив заголовков
 * @return         строка с данными заголовка
 */
string Http::getHeader(string key, map <string, string> headers){
	// Проверяем существует ли такой заголовок
	if(headers.count(key) > 0) return headers.find(key)->second;
	// Сообщаем что ничего не найдено
	return "";
}
/**
 * createHead Функция получения сформированного заголовка запроса
 */
void Http::createHead(){
	// Определяем тип прокси
	bool smart = (options & OPT_SMART);
	// Определяем разрешено ли сжатие
	bool gzip = (options & OPT_GZIP);
	// Определяем разрешено ли постоянное подключение
	bool keepalive = (options & OPT_KEEPALIVE);
	// Очищаем заголовок
	query.request.clear();
	// Создаем строку запроса
	query.request.append(
		toCase(query.method, true)
		+ string(" ") + query.path
		+ string(" ") + string("HTTP/")
		+ query.version + string("\r\n")
	);
	/*
	// Устанавливаем заголовок Host:
	query.request.append(
		string("Host: ") + query.host
		+ string(":") + query.port + string("\r\n")
	);
	*/
	// Устанавливаем заголовок Host:
	query.request.append(string("Host: ") + query.host + string("\r\n"));
	// Добавляем useragent
	if(!query.useragent.empty()) query.request.append(string("User-Agent: ") + query.useragent + string("\r\n"));
	// Добавляем остальные заголовки
	for(map <string, string>::iterator it = query.headers.begin(); it != query.headers.end(); ++it){
		// Фильтруем заголовки
		if((it->first != "host")
		&& (it->first != "user-agent")
		&& (it->first != "connection")
		&& (it->first != "proxy-authorization")
		&& (!gzip && (it->first != "accept-encoding"))
		&& (smart && (it->first != "proxy-connection"))){
			// Добавляем оставшиеся заголовки
			query.request.append(
				query.origin[it->first] + string(": ")
				+ it->second + string("\r\n")
			);
		}
	}
	// Если постоянное подключение запрещено
	if(!keepalive) query.connection = "close";
	// Добавляем заголовок connection
	if(!query.connection.empty()){
		// Устанавливаем заголовок подключения
		query.request.append(string("Connection: ") + query.connection + string("\r\n"));
		// Если это не smarty
		if(!smart){
			// Проверяем есть ли заголовок соединения прокси
			string pc = getHeader("proxy-connection", query.headers);
			// Добавляем заголовок закрытия подключения
			if(pc.empty()) query.request.append(string("Proxy-Connection: ") + query.connection + string("\r\n"));
		}
	}
	// Запоминаем конец запроса
	query.request.append(string("\r\n"));
}
/**
 * isConnect Метод проверяет является ли метод, методом connect
 * @return результат проверки на метод connect
 */
bool Http::isConnect(){
	// Сообщаем является ли метод, методом connect
	return !strcmp(query.method.c_str(), "connect");
}
/**
 * isClose Метод проверяет должно ли быть закрыто подключение
 * @return результат проверки на закрытие подключения
 */
bool Http::isClose(){
	// Сообщаем должно ли быть закрыто подключение
	return !strcmp(query.connection.c_str(), "close");
}
/**
 * isHttps Метод проверяет является ли подключение защищенным
 * @return результат проверки на защищенное подключение
 */
bool Http::isHttps(){
	// Сообщаем является ли продключение защищенным
	return !strcmp(query.protocol.c_str(), "https");
}
/**
 * isAlive Метод определения нужно ли держать соединение для прокси
 * @return результат проверки
 */
bool Http::isAlive(){
	// Если это версия протокола 1.1 и подключение установлено постоянное для прокси
	if(getVersion() > 1){
		// Проверяем указан ли заголовок отключения
		if(!strcmp(query.connection.c_str(), "close")) return false;
		// Иначе сообщаем что подключение должно жить долго
		return true;
	// Проверяем указан ли заголовок удержания соединения
	} else if(!strcmp(query.connection.c_str(), "keep-alive")) return true;
	// Сообщаем что подключение жить не должно
	return false;
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
		for(int i = 0; i < 8; i++) if(!strcmp(toCase(buf).c_str(), cmds[i])) return true;
	}
	// Сообщаем что это не http
	return false;
}
/**
 * generateHttp Метод генерации данных http запроса
 */
void Http::generateHttp(){
	// Разделяем на составляющие команду
	vector <string> cmd;
	// Функция разбиения на составляющие
	split(query.http, " ", cmd);
	// Получаем размер массива
	size_t size = cmd.size();
	// Если комманда существует
	if(size && (size == 3)){
		// Запоминаем метод запроса
		query.method = toCase(cmd[0]);
		// Запоминаем путь запроса
		query.path = cmd[1];
		// Разбиваем протокол и тип протокола на части
		vector <string> prt;
		// Функция разбиения на составляющие
		split(cmd[2], "/", prt);
		// Получаем размер массива
		size = prt.size();
		// Если данные найдены
		if(size && (size == 2)){
			// Запоминаем протокол запроса
			query.protocol = toCase(prt[0]);
			// Запоминаем версию протокола
			query.version = prt[1];
			// Извлекаем данные хоста
			string host = getHeader("host", query.headers);
			// Извлекаем данные авторизации
			string auth = getHeader("proxy-authorization", query.headers);
			// Получаем заголовок Proxy-Connection
			string proxy_connection = getHeader("proxy-connection", query.headers);
			// Получаем данные юзерагента
			query.useragent = getHeader("user-agent", query.headers);
			// Получаем данные заголовока коннекта
			query.connection = toCase(getHeader("connection", query.headers));
			// Если постоянное соединение не установлено
			if((options & OPT_SMART) && !proxy_connection.empty()) query.connection = proxy_connection;
			// Если хост найден
			if(!host.empty()){
				// Выполняем получение параметров подключения
				connect gcon = getConnection(query.path);
				// Выполняем получение параметров подключения
				connect scon = getConnection(host);
				// Создаем полный адрес запроса
				string fulladdr1 = scon.protocol + string("://") + scon.host;
				string fulladdr2 = fulladdr1 + "/";
				string fulladdr3 = fulladdr1 + string(":") + scon.port;
				string fulladdr4 = fulladdr3 + "/";
				// Определяем путь
				if(strcmp(toCase(query.method).c_str(), "connect")
				&& (!strcmp(toCase(query.path).c_str(), toCase(host).c_str())
				|| !strcmp(toCase(query.path).c_str(), toCase(fulladdr1).c_str())
				|| !strcmp(toCase(query.path).c_str(), toCase(fulladdr2).c_str())
				|| !strcmp(toCase(query.path).c_str(), toCase(fulladdr3).c_str())
				|| !strcmp(toCase(query.path).c_str(), toCase(fulladdr4).c_str()))) query.path = "/";
				// Выполняем удаление из адреса доменного имени
				else if(strstr(query.path.c_str(), fulladdr1.c_str()) != NULL){
					// Запоминаем текущий путь
					string tmp_path = query.path;
					// Вырезаем домер из пути
					tmp_path = tmp_path.replace(0, fulladdr1.length(), "");
					// Если путь существует
					if(!tmp_path.empty()) query.path = tmp_path;
				}
				// Запоминаем хост
				query.host = toCase(scon.host);
				// Запоминаем порт
				if(strcmp(scon.port.c_str(), gcon.port.c_str())
				&& !strcmp(gcon.port.c_str(), "80")){
					// Запоминаем протокол
					query.protocol = toCase(scon.protocol);
					// Уделяем предпочтение 443 порту
					query.port = scon.port;
				// Запоминаем порт такой какой он есть
				} else if(gcon.port.find_first_not_of("0123456789") == string::npos) {
					// Запоминаем протокол
					query.protocol = toCase(gcon.protocol);
					// Запоминаем порт
					query.port = gcon.port;
				// Устанавливаем значение порта и протокола по умолчанию
				} else {
					// Запоминаем протокол
					query.protocol = "http";
					// Устанавливаем http порт
					query.port = "80";
				}
			}
			// Если авторизация найдена
			if(!auth.empty()){
				// Выполняем разделение на тип и данные авторизации
				vector <string> lgn;
				// Функция разбиения на составляющие
				split(auth, " ", lgn);
				// Запоминаем размер массива
				size = lgn.size();
				// Если данные получены
				if(size && (size == 2)){
					// Запоминаем тип авторизации
					query.auth = toCase(lgn[0]);
					// Если это тип авторизация basic, тогда выполняем декодирования данных авторизации
					if(!strcmp(query.auth.c_str(), "basic")){
						// Выполняем декодирование логина и пароля
						string dauth = base64_decode(lgn[1].c_str());
						// Выполняем поиск авторизационных данных
						size_t pos = dauth.find(":");
						// Если протокол найден
						if(pos != string::npos){
							// Выполняем разделение на логин и пароль
							vector <string> lp;
							// Функция разбиения на составляющие
							split(dauth, ":", lp);
							// Запоминаем размер массива
							size = lp.size();
							// Если данные получены
							if(size && (size == 2)){
								// Запоминаем логин
								query.login = lp[0];
								// Запоминаем пароль
								query.password = lp[1];
							}
							// Очищаем память выделенную для вектора
							vector <string> ().swap(lp);
						}
					}
				}
				// Очищаем память выделенную для вектора
				vector <string> ().swap(lgn);
			}
		}
		// Очищаем память выделенную для вектора
		vector <string> ().swap(prt);
	}
	// Очищаем память выделенную для вектора
	vector <string> ().swap(cmd);
	// Генерируем параметры для запроса
	createHead();
}
/**
 * modify Функция модифицирования ответных данных
 * @param data ссылка на данные полученные от сервера
 */
void Http::modify(vector <char> &data){
	// Если заголовки не заняты тогда выполняем модификацию
	if(!_query.length){
		// Определяем тип прокси
		bool smart = (options & OPT_SMART);
		// Определяем разрешено ли выводить название агента
		bool agent = (options & OPT_AGENT);
		// Выполняем парсинг http запроса
		_query = getHeaders(data.data());
		// Проверяем есть ли заголовок соединения прокси
		string pc = (!smart ? getHeader("proxy-connection", _query.headers) : "");
		// Проверяем есть ли заголовок соединения
		string co = getHeader("connection", _query.headers);
		// Если заголовок не найден
		if(pc.empty() && !co.empty()){
			// Поздиция конца заголовков
			int pos = -1;
			// Получаем данные ответа
			const char * headers = data.data();
			// Ищем первое вхождение подстроки в строке
			const char * end_headers = strstr(headers, "\r\n\r\n");
			// Если завершение заголовка найдено
			if(end_headers != NULL) pos = end_headers - headers;
			// Если позиция найдена
			if(pos > -1){
				// Копируем заголовки
				string str(headers, pos);
				// Устанавливаем название прокси
				if(agent) str.append(string("\r\nProxy-Agent: ") + appname + string("/") + appver);
				// Добавляем заголовок закрытия подключения, если не smarty
				if(!smart) str.append(string("\r\nProxy-Connection: ") + co);
				// Начальные и конечные блоки данных
				vector <char> first, last;
				// Заполняем первичные данные структуры
				first.assign(str.begin(), str.end());
				// Заполняем последние данные структуры
				last.assign(data.begin() + pos, data.end());
				// Объединяем блоки
				copy(last.begin(), last.end(), back_inserter(first));
				// Заменяем первоначальный блок с данными
				data = first;
			}
		}
		// Очищаем объект
		_query.clear();
	}
}
/**
 * Http::checkCharEnd Функция проверяет по массиву символов, достигнут ли конец запроса
 * @param  buffer буфер с данными
 * @param  size   размер буфера
 * @param  chs    массив с символами завершающими запрос
 * @return        результат проверки
 */
bool Http::checkCharEnd(const char * buffer, size_t size, vector <short> chs){
	// Результат проверки
	bool check = false;
	// Выполняем реверс массива
	reverse(chs.begin(), chs.end());
	// Переходим по всему массиву
	for(u_int i = 0; i < chs.size(); i++){
		// Выполняем проверку завершающих символов
		if((short) buffer[size - (i + 1)] == chs[i])
			check = true;
		else {
			check = false;
			break;
		}
	}
	// Выводим результат
	return check;
}
/**
 * checkEnd Функция проверки завершения запроса
 * @param  buffer буфер с входящими данными
 * @param  size   размер входящих данных
 * @return        результат проверки
 */
Http::HttpEnd Http::checkEnd(const char * buffer, size_t size){
	// Создаем блок данных
	HttpEnd data;
	// Выполняем парсинг http запроса
	if(!_query.length) _query = getHeaders(buffer);
	// Если данные существуют
	if(!_query.http.empty()){
		// Проверяем есть ли размер вложений
		string cl = getHeader("content-length", _query.headers);
		// Проверяем есть ли чанкование
		string ch = getHeader("transfer-encoding", _query.headers);
		// Проверяем есть ли закрытие соединения
		string cc = getHeader("connection", _query.headers);
		// Если найден размер вложений
		if(!cl.empty() && (cl.find_first_not_of("0123456789") == string::npos)){
			// Определяем размер вложений
			int body_size = ::atoi(cl.c_str());
			// Получаем размер вложения
			if(size >= (_query.length + body_size)){
				// Заполняем структуру данными
				data.type = 4;
				// Заполняем размеры
				data.begin	= _query.length;
				data.end	= _query.length + body_size;
			}
		// Если это чанкование
		} else if(!ch.empty() && (ch.find("chunked") != string::npos)){
			// Если конец строки найден
			if(((size > 5) && (_query.length < size)) // 0\r\n\r\n
			&& (checkCharEnd(buffer, size, {48, 13, 10, 13, 10})
			|| checkCharEnd(buffer, size, {48, 10, 10}))){
				// Заполняем структуру данными
				data.type = 5;
				// Заполняем размеры
				data.begin	= _query.length;
				data.end	= size;
			}
		// Если это автозакрытие подключения
		} else if(!cc.empty() && (cc == "close")){
			// Если конец строки найден
			if(((size > 4) && (_query.length < size)) // \r\n\r\n
			&& (checkCharEnd(buffer, size, {13, 10, 13, 10})
			|| checkCharEnd(buffer, size, {10, 10}))){
				// Заполняем структуру данными
				data.type = 3;
				// Заполняем размеры
				data.begin	= _query.length;
				data.end	= size;
			}
		// Если указан тип данных но длина вложенных данных не указана
		} else if(!ch.empty()) data.type = 0;
		// Если найден конечный символ
		else if((short) buffer[size - 1] == 0) data.type = 2;
		// Если вложения не найдены
		else data.type = 1;
		// Если флаг установлен тогда очищаем структуру
		if(data.type) _query.clear();
	// Ечищаем структуру если пришел мусор
	} else _query.clear();
	// Выводим результат
	return data;
}
/**
 * parse Метод выполнения парсинга
 * @param  buffer буфер входящих данных из сокета
 * @param  size   размер переданных данных
 * @return        результат определения завершения запроса
 */
bool Http::parse(const char * buffer, size_t size){
	// Выполняем проверку завершения передачи данных
	HttpEnd check = checkEnd(buffer, size);
	// Если флаг установлен
	if(check.type){
		// Выполняем парсинг http запроса
		query = getHeaders(buffer);
		// Определяем тип запроса
		switch(check.type){
			// Если присутствуют вложения
			case 3:
			case 4: {
				// Извлекаем указанные данные
				query.entitybody.assign(buffer + check.begin, buffer + check.end);
				// Добавляем завершающий байт
				query.entitybody.push_back('\0');
			} break;
		}
		// Генерируем данные
		generateHttp();
		// Сообщаем что все удачно получено
		return true;
	}
	// Сообщаем что данные не получены
	return false;
}
/**
 * brokenRequest Метод получения ответа (неудачного отправленного запроса)
 * @return ответ в формате html
 */
Http::HttpQuery Http::brokenRequest(){
	// Устанавливаем дефолтное название прокси
	string defname = "ProxyAnyks/1.0";
	// Определяем позицию дефолтного названия
	size_t pos = html[9].find(defname);
	// Результирующая строка
	string result;
	// Если это домен
	if(pos != string::npos){
		// Заменяем дефолтное название на указанное
		result = html[9].replace(pos, defname.length(), appname + string("/") + appver);
	}
	// Выводим шаблон сообщения о неудачном отправленном запросе
	result = html[9];
	// Данные для вывода
	HttpQuery data(501, result);
	// Выводим результат
	return data;
}
/**
 * faultConnect Метод получения ответа (неудачного подключения к удаленному серверу)
 * @return ответ в формате html
 */
Http::HttpQuery Http::faultConnect(){
	// Устанавливаем дефолтное название прокси
	string defname = "ProxyAnyks/1.0";
	// Определяем позицию дефолтного названия
	size_t pos = html[6].find(defname);
	// Результирующая строка
	string result;
	// Если это домен
	if(pos != string::npos){
		// Заменяем дефолтное название на указанное
		result = html[6].replace(pos, defname.length(), appname + string("/") + appver);
	}
	// Выводим шаблон сообщения о неудачном подключении
	result = html[6];
	// Данные для вывода
	HttpQuery data(502, result);
	// Выводим результат
	return data;
}
/**
 * faultAuth Метод получения ответа (неудачной авторизации)
 * @return ответ в формате html
 */
Http::HttpQuery Http::faultAuth(){
	// Устанавливаем дефолтное название прокси
	string defname = "ProxyAnyks/1.0";
	// Определяем позицию дефолтного названия
	size_t pos = html[5].find(defname);
	// Результирующая строка
	string result;
	// Если это домен
	if(pos != string::npos){
		// Выводим шаблон сообщения о неудачной авторизации
		result = html[5].replace(pos, defname.length(), appname + string("/") + appver);
	}
	// Выводим шаблон сообщения о неудачной авторизации
	result = html[5];
	// Данные для вывода
	HttpQuery data(403, result);
	// Выводим результат
	return data;
}
/**
 * requiredAuth Метод получения ответа (запроса ввода логина и пароля)
 * @return ответ в формате html
 */
Http::HttpQuery Http::requiredAuth(){
	// Устанавливаем дефолтное название прокси
	string defname = "ProxyAnyks/1.0";
	// Определяем позицию дефолтного названия
	size_t pos = html[2].find(defname);
	// Результирующая строка
	string result;
	// Если это домен
	if(pos != string::npos){
		// Выводим шаблон сообщения о неудачной авторизации
		result = html[2].replace(pos, defname.length(), appname + string("/") + appver);
	}
	// Выводим шаблон сообщения о требовании авторизации
	result = html[2];
	// Данные для вывода
	HttpQuery data(407, result);
	// Выводим результат
	return data;
}
/**
 * authSuccess Метод получения ответа (подтверждения авторизации)
 * @return ответ в формате html
 */
Http::HttpQuery Http::authSuccess(){
	// Устанавливаем дефолтное название прокси
	string defname = "ProxyAnyks/1.0";
	// Определяем позицию дефолтного названия
	size_t pos = html[0].find(defname);
	// Результирующая строка
	string result;
	// Если это домен
	if(pos != string::npos){
		// Выводим шаблон сообщения о неудачной авторизации
		result = html[0].replace(pos, defname.length(), appname + string("/") + appver);
	}
	// Выводим шаблон сообщения о том что авторизация пройдена
	result = html[0];
	// Данные для вывода
	HttpQuery data(200, result);
	// Выводим результат
	return data;
}
/**
 * getQuery Метод получения сформированного http запроса
 * @return сформированный http запрос
 */
Http::HttpQuery Http::getQuery(){
	// Данные для вывода
	HttpQuery data(200, query.request, query.entitybody);
	// Выводим результат
	return data;
}
/**
 * getMethod Метод получения метода запроса
 * @return метод запроса
 */
string Http::getMethod(){
	// Выводим значение переменной
	return query.method;
}
/**
 * getHost Метод получения хоста запроса
 * @return хост запроса
 */
string Http::getHost(){
	// Выводим значение переменной
	return query.host;
}
/**
 * getPath Метод получения пути запроса
 * @return путь запроса
 */
string Http::getPath(){
	// Выводим значение переменной
	return query.path;
}
/**
 * getProtocol Метод получения протокола запроса
 * @return протокол запроса
 */
string Http::getProtocol(){
	// Выводим значение переменной
	return query.protocol;
}
/**
 * getAuth Метод получения метода авторизации запроса
 * @return метод авторизации
 */
string Http::getAuth(){
	// Выводим значение переменной
	return query.auth;
}
/**
 * getLogin Метод получения логина авторизации запроса
 * @return логин авторизации
 */
string Http::getLogin(){
	// Выводим значение переменной
	return query.login;
}
/**
 * getPassword Метод получения пароля авторизации запроса
 * @return пароль авторизации
 */
string Http::getPassword(){
	// Выводим значение переменной
	return query.password;
}
/**
 * getUseragent Метод получения юзерагента запроса
 * @return юзерагент
 */
string Http::getUseragent(){
	// Выводим значение переменной
	return query.useragent;
}
/**
 * getPort Метод получения порта запроса
 * @return порт удаленного ресурса
 */
u_int Http::getPort(){
	// Выводим значение переменной
	return (!query.port.empty() ? ::atoi(query.port.c_str()) : 80);
}
/**
 * getVersion Метод получения версии протокола запроса
 * @return версия протокола запроса
 */
float Http::getVersion(){
	// Выводим значение переменной
	return (!query.version.empty() ? ::atof(query.version.c_str()) : 1.0);
}
/**
 * setMethod Метод установки метода запроса
 * @param str строка с данными для установки
 */
void Http::setMethod(const string str){
	// Запоминаем данные
	query.method = str;
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setHost Метод установки хоста запроса
 * @param str строка с данными для установки
 */
void Http::setHost(const string str){
	// Запоминаем данные
	query.host = str;
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setPort Метод установки порта запроса
 * @param number номер порта для установки
 */
void Http::setPort(u_int number){
	// Запоминаем данные
	query.port = to_string(number);
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setPath Метод установки пути запроса
 * @param str строка с данными для установки
 */
void Http::setPath(const string str){
	// Запоминаем данные
	query.path = str;
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setProtocol Метод установки протокола запроса
 * @param str строка с данными для установки
 */
void Http::setProtocol(const string str){
	// Запоминаем данные
	query.protocol = str;
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setVersion Метод установки версии протокола запроса
 * @param number номер версии протокола
 */
void Http::setVersion(float number){
	// Запоминаем данные
	query.version = to_string(number);
	// Если это всего один символ тогда дописываем ноль
	if(query.version.length() == 1) query.version += ".0";
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setClose Метод установки принудительного отключения после запроса
 */
void Http::setClose(){
	// Запоминаем данные
	query.connection = "close";
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setAuth Метод установки метода авторизации запроса
 * @param str строка с данными для установки
 */
void Http::setAuth(const string str){
	// Запоминаем данные
	query.auth = str;
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setUseragent Метод установки юзерагента запроса
 * @param str строка с данными для установки
 */
void Http::setUseragent(const string str){
	// Запоминаем данные
	query.useragent = str;
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * Http::clear Метод очистки всех полученных данных
 */
void Http::clear(){
	// Очищаем заголовки
	_query.clear();
	query.clear();
}
/**
 * Http Конструктор
 * @param str строка содержащая название прокси-сервера
 * @param opt параметры прокси-сервера
 * @param ver версия прокси-сервера
 */
Http::Http(const string str, u_short opt, string ver){
	// Если имя передано то запоминаем его
	this->appname = str;
	// Устанавливаем версию системы
	this->appver = ver;
	// Запоминаем тип прокси-сервера
	this->options = opt;
}
/**
 * Http Деструктор
 */
Http::~Http(){
	// Очищаем заголовки
	_query.clear();
	query.clear();
	// Очищаем память выделенную для вектора
	vector <char> ().swap(query.entitybody);
}