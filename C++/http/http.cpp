#include "http.h"

// Устанавливаем пространство имен
using namespace std;

/**
 * Http::split Функция разбиения строки на указанные составляющие
 * @param str   строка которую нужно разбить на составляющие
 * @param delim разделитель в строке
 * @return      массив строк полученных при разделении
 */
vector <string> Http::split(const string str, const string delim){
	// Начальный символ для обрезки строки
	int i = 0;
	// Результирующий вектор
	vector <string> result;
	// Позиция разделителя в строке
	int pos = str.find(delim);
	// Перебираем строку до тех пор пока не переберем её целиком
	while(pos != string::npos){
		// Добавляем найденную позицию в строке
		result.push_back(str.substr(i, pos - i));
		// Запоминаем новую позицию
		i = delim.length() + pos;
		// Выполняем поиск новой позиции
		pos = str.find(delim, i);
		// Если позиция достигнута тогда продолжаем дальше
		if(pos == string::npos) result.push_back(str.substr(i, str.length()));
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
 * getHeaderParam Функция получения содержимое заголовка
 * @param  head  заголовок в котором ищем параметры
 * @param  param параметр для поиска
 * @return       найденный параметр
 */
string Http::getHeaderParam(string head, string param){
	// Запоминаем переданные параметры
	string str;
	// Прибавляем к строке параметра доветочие
	param += ":";
	// Выполняем поиск заголовка
	size_t pos = toCase(head).find(toCase(param));
	// Если заголовок найден
	if(pos != string::npos){
		// Формируем строку
		head = head.substr(pos + param.length(), head.length() - (pos + param.length()));
		// Извлекаем данные
		str = trim(head);
	}
	// Выводим результат
	return str;
}
/**
 * findHeaderParam Функция поиска значение заголовка
 * @param  str строка поиска
 * @param  buf буфер в котором осуществляется поиск
 * @return     выводим значение заголовка
 */
string Http::findHeaderParam(string str, string buf){
	// Создаем новый буфер памяти
	string istr = toCase(str) + ":";
	// Ищем начало заголовка Content-length
	size_t pos1 = toCase(buf).find(istr);
	// Ищем конец заголовка Content-length
	size_t pos2 = toCase(buf).find("\r\n", pos1);
	// Если заголовок найден
	if((pos1 != string::npos) && (pos2 != string::npos)){
		// Получаем размер всей строки
		int length = buf.length();
		// Определяем начальную позицию
		int start = pos1 + istr.length();
		// Определяем конечную позицию
		int end = length - ((length - start) + (length - pos2));
		// Запоминаем новое значение буфера
		buf = buf.substr(start, end);
		// Извлекаем данные заголовка
		str = trim(buf);
	}
	// Выводим результат
	return str;
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
	// Размеры данных
	size_t size;
	// Если протокол найден
	if(pos != string::npos){
		// Выполняем разделение на протокол
		vector <string> prt = split(str, "://");
		// Запоминаем размер массива
		size = prt.size();
		// Если протокол найден
		if(size){
			// Запоминаем версию протокола
			data.protocol = prt[0];
			// Запоминаем оставшуюся часть строки
			str = prt[1];
		}
		// Очищаем память выделенную для вектора
		vector <string> ().swap(prt);
	// Устанавливаем протокол по умолчанию
	} else data.protocol = protocol;
	// Выполняем поиск порта и хоста
	pos = str.find(":");
	// Если хост и порт найдены
	if(pos != string::npos){
		// Выполняем разделение на протокол
		vector <string> prt = split(str, ":");
		// Запоминаем размер массива
		size = prt.size();
		// Если порт и хост найдены
		if(size){
			// Запоминаем хост
			data.host = prt[0];
			// Запоминаем порт
			data.port = prt[1];
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
	if(!strcmp(toCase(data.protocol).c_str(), "https")) data.port = "443";
	// Устанавливаем версию протокола в зависимости от порта
	else if(!strcmp(toCase(data.port).c_str(), "443")) data.protocol = "https";
	// Запоминаем найденный хост
	str = data.host;
	// Выполняем поиск дирректории в хосте
	pos = str.find("/");
	// Если дирректория найдена значит это не хост
	if(pos != string::npos){
		// Извлекаем название домена
		str = str.substr(0, pos);
		// Выполняем поиск точки в домене
		pos = str.find(".");
		// Если это домен
		if(pos != string::npos) data.host = str;
		// Если это не домен то устанавливаем что это корень
		else data.host = "/";
	}
	// Выводим результат
	return data;
}
/**
 * parser Функция парсера http запроса
 * @param buffer буфер входящих данных полученных из сокета
 */
void Http::parser(string buffer){
	// Если буфер верный
	if(buffer.length()){
		// Запоминаем первоначальный запрос
		query = buffer;
		// Выполняем парсинг заголовков
		headers = split(query, "\r\n");
		// Размеры данных
		size_t size = headers.size();
		// Если заголовки найдены
		if(size){
			// Запоминаем команду запроса
			command = headers[0];
			// Разделяем на составляющие команду
			vector <string> cmd = split(command, " ");
			// Получаем размер массива
			size = cmd.size();
			// Если комманда существует
			if(size && (size == 3)){
				// Запоминаем метод запроса
				method = toCase(cmd[0]);
				// Запоминаем путь запроса
				path = cmd[1];
				// Разбиваем протокол и тип протокола на части
				vector <string> prt = split(cmd[2], "/");
				// Получаем размер массива
				size = prt.size();
				// Если данные найдены
				if(size && (size == 2)){
					// Запоминаем протокол запроса
					protocol = toCase(prt[0]);
					// Запоминаем версию протокола
					version = prt[1];
					// Перебираем остальные заголовки
					for(int i = 1; i < headers.size(); i++){
						// Если все заголовки считаны тогда выходим
						if(headers[i].length()){
							// Получаем данные заголовок хоста
							string hst = getHeaderParam(headers[i], "Host");
							// Получаем данные юзерагента
							string usg = getHeaderParam(headers[i], "User-Agent");
							// Получаем данные заголовока коннекта
							string cnn = getHeaderParam(headers[i], "Connection");
							// Получаем данные заголовока коннекта для прокси
							string pcn = getHeaderParam(headers[i], "Proxy-Connection");
							// Получаем данные авторизации
							string ath = getHeaderParam(headers[i], "Proxy-Authorization");
							// Если заголовок хоста найден
							if(hst.length()){
								// Выполняем получение параметров подключения
								connect gcon = getConnection(path);
								// Выполняем получение параметров подключения
								connect scon = getConnection(hst);
								// Создаем полный адрес запроса
								string fulladdr1 = scon.protocol + string("://") + scon.host;
								string fulladdr2 = fulladdr1 + "/";
								string fulladdr3 = fulladdr1 + string(":") + scon.port;
								string fulladdr4 = fulladdr3 + "/";
								// Определяем путь
								if(strcmp(toCase(method).c_str(), "connect")
								&& (!strcmp(toCase(path).c_str(), toCase(hst).c_str())
								|| !strcmp(toCase(path).c_str(), toCase(fulladdr1).c_str())
								|| !strcmp(toCase(path).c_str(), toCase(fulladdr2).c_str())
								|| !strcmp(toCase(path).c_str(), toCase(fulladdr3).c_str())
								|| !strcmp(toCase(path).c_str(), toCase(fulladdr4).c_str()))) path = "/";
								// Выполняем удаление из адреса доменного имени
								else if(strstr(path.c_str(), fulladdr1.c_str()) != NULL){
									// Запоминаем текущий путь
									string tmp_path = path;
									// Вырезаем домер из пути
									tmp_path = tmp_path.replace(0, fulladdr1.length(), "");
									// Если путь существует
									if(tmp_path.length()) path = tmp_path;
								}
								// Запоминаем хост
								host = toCase(scon.host);
								// Запоминаем порт
								if(strcmp(scon.port.c_str(), gcon.port.c_str())
								&& !strcmp(gcon.port.c_str(), "80")){
									// Запоминаем протокол
									protocol = toCase(scon.protocol);
									// Уделяем предпочтение 443 порту
									port = scon.port;
								// Запоминаем порт такой какой он есть
								} else {
									// Запоминаем протокол
									protocol = toCase(gcon.protocol);
									// Запоминаем порт
									port = gcon.port;
								}
							// Если авторизация найдена
							} else if(ath.length()){
								// Выполняем разделение на тип и данные авторизации
								vector <string> lgn = split(ath, " ");
								// Запоминаем размер массива
								size = lgn.size();
								// Если данные получены
								if(size){
									// Запоминаем тип авторизации
									auth = toCase(lgn[0]);
									// Если это тип авторизация basic, тогда выполняем декодирования данных авторизации
									if(!strcmp(auth.c_str(), "basic")){
										// Выполняем декодирование логина и пароля
										string dauth = base64_decode(lgn[1].c_str());
										// Выполняем поиск авторизационных данных
										size_t pos = dauth.find(":");
										// Если протокол найден
										if(pos != string::npos){
											// Выполняем разделение на логин и пароль
											vector <string> lp = split(dauth, ":");
											// Запоминаем размер массива
											size = lp.size();
											// Если данные получены
											if(size){
												// Запоминаем логин
												login = lp[0];
												// Запоминаем пароль
												password = lp[1];
											}
											// Очищаем память выделенную для вектора
											vector <string> ().swap(lp);
										}
									}
								}
								// Очищаем память выделенную для вектора
								vector <string> ().swap(lgn);
							// Если юзерагент найден
							} else if(usg.length()) useragent = usg;
							// Если заголовок коннекта прокси найден
							else if(pcn.length()) pconnection = toCase(pcn);
							// Если заголовок коннекта найден
							else if(cnn.length()) connection = toCase(cnn);
							// Если это все остальные заголовки то просто добавляем их в список
							else if(headers[i].length()) other.push_back(headers[i]);
						// Иначе выходим из цикла
						} else break;
					}
				}
				// Очищаем память выделенную для вектора
				vector <string> ().swap(prt);
			}
			// Очищаем память выделенную для вектора
			vector <string> ().swap(cmd);
		}
	}
}
/**
 * Если версия протокола 1.0 то там нет keep-alive и должен выставлять значение clone (идет запрос, ответ и отсоединение)
 * Если версия протокола 1.1 и там установлен keep-alive то отсоединение должно быть по таймеру которое выставлено на ожидание соединения (время жизни например 30 секунд)
 */
/**
 * createHead Функция получения сформированного заголовка запроса
 */
void Http::createHead(){
	// Если это не метод CONNECT то меняем заголовок Connection на close
	if(!connection.length() && !strcmp(version.c_str(), "1.0")) connection = "close";
	// Создаем строку запроса
	head.append(toCase(method, true) + string(" ") + path + string(" ") + string("HTTP/") + version + string("\r\n"));
	// Устанавливаем заголовок Host:
	head.append(string("Host: ") + host + string(":") + port + string("\r\n"));
	// Добавляем useragent
	if(useragent.length()) head.append(string("User-Agent: ") + useragent + string("\r\n"));
	// Добавляем остальные заголовки
	for(int i = 0; i < other.size(); i++){
		// Если это не заголовок контента, то добавляем в список остальные заголовки
		if(toCase(other[i]).find("content-length:") == string::npos) head.append(other[i] + string("\r\n"));
	}
	// Добавляем заголовок connection
	if(connection.length()) head.append(string("Connection: ") + connection + string("\r\n"));
	// Если тело запроса существует то добавляем размер тела
	if(entitybody.length()) head.append(string("Content-Length: ") + to_string(entitybody.length()) + string("\r\n"));
	// Запоминаем конец запроса
	head.append(string("\r\n"));
	// Если существует тело запроса то добавляем его
	if(entitybody.length()) head.append(entitybody);
}
/**
 * isAlive Метод определения нужно ли держать соединение для прокси
 * @return результат проверки
 */
bool Http::isAlive(){
	// Если это версия протокола 1.1 и подключение установлено постоянное для прокси
	if(!strcmp(version.c_str(), "1.1") && pconnection.length()
	&& !strcmp(toCase(pconnection).c_str(), "keep-alive")) return true;
	else return false;
}
/**
 * isHttp Метод проверки на то http это или нет
 * @param  buffer буфер входящих данных
 * @return        результат проверки
 */
bool Http::isHttp(const string buffer){
	// Если буфер существует
	if(buffer.length()){
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
 * parse Метод выполнения парсинга
 * @param  buffer буфер входящих данных из сокета
 * @return        результат определения завершения запроса
 */
bool Http::parse(const string buffer){
	// Выполняем поиск размера передаваемых данных
	string len_data = findHeaderParam("Content-length", buffer);
	// Символы завершения ввода заголовков
	const char * endPos = strstr(buffer.c_str(), "\r\n\r\n");
	// Если все данные переданы то выходим
	if(!len_data.length() && (endPos != NULL)){
		// Выполняем парсинг заголовков http запроса
		parser(buffer);
		// Выполняем генерацию результирующего запроса
		createHead();
		// Выходим
		return true;
	// Сообщаем что размер найден
	} else if(endPos != NULL) {
		// Получаем длину передаваемых данных
		const int len = ::atoi(len_data.c_str());
		// Создаем новый буфер с данными
		string buf(endPos + 4);
		// Урезаем строку до нужных размеров
		buf = buf.substr(0, len);
		// Если все байты считаны
		if(buf.length() >= len){
			// Запоминаем данные тела запроса
			entitybody = buf;
			// Выполняем парсинг заголовков http запроса
			parser(buffer);
			// Выполняем генерацию результирующего запроса
			createHead();
			// Выходим из функции
			return true;
		}
	}
	// Возвращаем что ничего еще не найдено
	return false;
}
/**
 * brokenRequest Метод получения ответа (неудачного отправленного запроса)
 * @return ответ в формате html
 */
string Http::brokenRequest(){
	// Устанавливаем дефолтное название прокси
	string defname = "ProxyAnyks/1.0";
	// Определяем позицию дефолтного названия
	size_t pos = html[9].find(defname);
	// Если это домен
	if(pos != string::npos){
		// Заменяем дефолтное название на указанное
		return html[9].replace(pos, defname.length(), appname + string("/") + to_string(appver));
	}
	// Выводим шаблон сообщения о неудачном отправленном запросе
	return html[9];
}
/**
 * faultConnect Метод получения ответа (неудачного подключения к удаленному серверу)
 * @return ответ в формате html
 */
string Http::faultConnect(){
	// Устанавливаем дефолтное название прокси
	string defname = "ProxyAnyks/1.0";
	// Определяем позицию дефолтного названия
	size_t pos = html[6].find(defname);
	// Если это домен
	if(pos != string::npos){
		// Заменяем дефолтное название на указанное
		return html[6].replace(pos, defname.length(), appname + string("/") + to_string(appver));
	}
	// Выводим шаблон сообщения о неудачном подключении
	return html[6];
}
/**
 * faultAuth Метод получения ответа (неудачной авторизации)
 * @return ответ в формате html
 */
string Http::faultAuth(){
	// Устанавливаем дефолтное название прокси
	string defname = "ProxyAnyks/1.0";
	// Определяем позицию дефолтного названия
	size_t pos = html[5].find(defname);
	// Если это домен
	if(pos != string::npos){
		// Выводим шаблон сообщения о неудачной авторизации
		return html[5].replace(pos, defname.length(), appname + string("/") + to_string(appver));
	}
	// Выводим шаблон сообщения о неудачной авторизации
	return html[5];
}
/**
 * requiredAuth Метод получения ответа (запроса ввода логина и пароля)
 * @return ответ в формате html
 */
string Http::requiredAuth(){
	// Устанавливаем дефолтное название прокси
	string defname = "ProxyAnyks/1.0";
	// Определяем позицию дефолтного названия
	size_t pos = html[2].find(defname);
	// Если это домен
	if(pos != string::npos){
		// Выводим шаблон сообщения о неудачной авторизации
		return html[2].replace(pos, defname.length(), appname + string("/") + to_string(appver));
	}
	// Выводим шаблон сообщения о требовании авторизации
	return html[2];
}
/**
 * authSuccess Метод получения ответа (подтверждения авторизации)
 * @return ответ в формате html
 */
string Http::authSuccess(){
	// Устанавливаем дефолтное название прокси
	string defname = "ProxyAnyks/1.0";
	// Определяем позицию дефолтного названия
	size_t pos = html[0].find(defname);
	// Если это домен
	if(pos != string::npos){
		// Выводим шаблон сообщения о неудачной авторизации
		return html[0].replace(pos, defname.length(), appname + string("/") + to_string(appver));
	}
	// Выводим шаблон сообщения о том что авторизация пройдена
	return html[0];
}
/**
 * getMethod Метод получения метода запроса
 * @return метод запроса
 */
string Http::getMethod(){
	// Выводим значение переменной
	return (method.length() ? method : "");
}
/**
 * getHost Метод получения хоста запроса
 * @return хост запроса
 */
string Http::getHost(){
	// Выводим значение переменной
	return (host.length() ? host : "");
}
/**
 * getPath Метод получения пути запроса
 * @return путь запроса
 */
string Http::getPath(){
	// Выводим значение переменной
	return (path.length() ? path : "");
}
/**
 * getProtocol Метод получения протокола запроса
 * @return протокол запроса
 */
string Http::getProtocol(){
	// Выводим значение переменной
	return (protocol.length() ? protocol : "");
}
/**
 * getAuth Метод получения метода авторизации запроса
 * @return метод авторизации
 */
string Http::getAuth(){
	// Выводим значение переменной
	return (auth.length() ? auth : "");
}
/**
 * getLogin Метод получения логина авторизации запроса
 * @return логин авторизации
 */
string Http::getLogin(){
	// Выводим значение переменной
	return (login.length() ? login : "");
}
/**
 * getPassword Метод получения пароля авторизации запроса
 * @return пароль авторизации
 */
string Http::getPassword(){
	// Выводим значение переменной
	return (password.length() ? password : "");
}
/**
 * getUseragent Метод получения юзерагента запроса
 * @return юзерагент
 */
string Http::getUseragent(){
	// Выводим значение переменной
	return (useragent.length() ? useragent : "");
}
/**
 * getQuery Метод получения сформированного http запроса
 * @return сформированный http запрос
 */
string Http::getQuery(){
	// Выводим значение переменной
	return (head.length() ? head : "");
}
/**
 * getPort Метод получения порта запроса
 * @return порт удаленного ресурса
 */
int Http::getPort(){
	// Выводим значение переменной
	return (port.length() ? ::atoi(port.c_str()) : 0);
}
/**
 * getVersion Метод получения версии протокола запроса
 * @return версия протокола запроса
 */
float Http::getVersion(){
	// Выводим значение переменной
	return (version.length() ? ::atof(version.c_str()) : 0);
}
/**
 * setMethod Метод установки метода запроса
 * @param str строка с данными для установки
 */
void Http::setMethod(const string str){
	// Запоминаем данные
	method = str;
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setHost Метод установки хоста запроса
 * @param str строка с данными для установки
 */
void Http::setHost(const string str){
	// Запоминаем данные
	host = str;
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setPort Метод установки порта запроса
 * @param str строка с данными для установки
 */
void Http::setPort(const string str){
	// Запоминаем данные
	port = str;
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setPath Метод установки пути запроса
 * @param str строка с данными для установки
 */
void Http::setPath(const string str){
	// Запоминаем данные
	path = str;
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setProtocol Метод установки протокола запроса
 * @param str строка с данными для установки
 */
void Http::setProtocol(const string str){
	// Запоминаем данные
	protocol = str;
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setVersion Метод установки версии протокола запроса
 * @param str строка с данными для установки
 */
void Http::setVersion(const string str){
	// Запоминаем данные
	version = str;
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setAuth Метод установки метода авторизации запроса
 * @param str строка с данными для установки
 */
void Http::setAuth(const string str){
	// Запоминаем данные
	auth = str;
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setUseragent Метод установки юзерагента запроса
 * @param str строка с данными для установки
 */
void Http::setUseragent(const string str){
	// Запоминаем данные
	useragent = str;
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * Http Конструктор
 * @param str строка содержащая название сервиса
 */
Http::Http(const string str, float ver){
	// Если имя передано то запоминаем его
	appname = str;
	// Устанавливаем версию системы
	appver = ver;
}
/**
 * Http Деструктор
 */
Http::~Http(){
	// Очищаем память выделенную для вектора
	vector <string> ().swap(other);
	vector <string> ().swap(headers);
}