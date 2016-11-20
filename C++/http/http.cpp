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
Http::http_data Http::getHeaders(string str){
	// Создаем структуру данных
	http_data data;
	// Проверяем существуют ли данные
	if(str.length()){
		// Определяем конец запроса
		size_t end_query = str.find("\r\n\r\n");
		// Если конец запроса найден
		if(end_query != string::npos){
			// Массив строк
			vector <string> strings, params;
			// Выполняем разбиение строк
			split(str, "\r\n", strings);
			// Если строки найдены
			if(strings.size()){
				// Позиция найденного разделителя
				size_t pos;
				// Запоминаем http запрос
				data.http = toCase(trim(strings[0]));
				// Переходим по всему массиву строк
				for(int i = 1; i < strings.size(); i++){
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
	// Очищаем строку
	str.clear();
	// Если заголовок найден
	if((pos1 != string::npos) && (pos2 != string::npos)){
		// Получаем размер всей строки
		int length = buf.length();
		// Определяем начальную позицию
		int start = pos1 + istr.length();
		// Определяем конечную позицию
		int end = pos2 - start;
		// Запоминаем новое значение буфера
		buf = buf.substr(start, end);
		// Извлекаем данные заголовка
		str = trim(buf);
	}
	// Выводим результат
	return str;
}
/**
 * Http::checkPort Функция проверки на качество порта
 * @param  port входная строка якобы содержащая порт
 * @return      результат проверки
 */
bool Http::checkPort(string port){
	// Получаем длину строку
	size_t len = port.length();
	// Если строка существует
	if(len){
		// Результирующая строка
		string result = "";
		// Переходим по всей строке и ищем неположенные символы
		for(size_t i = 0; i < len; i++){
			// Получаем текущий символ
			char symbol = port.c_str()[i];
			// Если найдены символы любые кроме цифр
			if((symbol > '9') || (symbol < '0')) return false;
			// Добавляем в результирующую строку символы цифр
			else result += symbol;
		}
		// Преобразуем строку в цифры
		if(::atoi(result.c_str()) > 65535)
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
	// Размеры данных
	size_t size;
	// Если протокол найден
	if(pos != (size_t) string::npos){
		// Выполняем разделение на протокол
		vector <string> prt;
		// Функция разбиения на составляющие
		split(str, "://", prt);
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
	if(pos != (size_t) string::npos){
		// Выполняем разделение на протокол
		vector <string> prt;
		// Функция разбиения на составляющие
		split(str, ":", prt);
		// Запоминаем размер массива
		size = prt.size();
		// Если порт и хост найдены
		if(size){
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
	if(!strcmp(toCase(data.protocol).c_str(), "https")) data.port = "443";
	// Устанавливаем версию протокола в зависимости от порта
	else if(!strcmp(toCase(data.port).c_str(), "443")) data.protocol = "https";
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
 * parser Функция парсера http запроса
 * @param buffer буфер входящих данных полученных из сокета
 */
void Http::parser(string buffer){
	// Если буфер верный
	if(buffer.length()){
		// Запоминаем первоначальный запрос
		query = buffer;
		// Выполняем парсинг заголовков
		split(query, "\r\n", headers);
		// Размеры данных
		size_t size = headers.size();
		// Если заголовки найдены
		if(size){
			// Очищаем массив заголовков
			other.clear();
			// Запоминаем команду запроса
			command = headers[0];
			// Разделяем на составляющие команду
			vector <string> cmd;
			// Функция разбиения на составляющие
			split(command, " ", cmd);
			// Получаем размер массива
			size = cmd.size();
			// Если комманда существует
			if(size && (size == 3)){
				// Запоминаем метод запроса
				method = toCase(cmd[0]);
				// Запоминаем путь запроса
				path = cmd[1];
				// Разбиваем протокол и тип протокола на части
				vector <string> prt;
				// Функция разбиения на составляющие
				split(cmd[2], "/", prt);
				// Получаем размер массива
				size = prt.size();
				// Если данные найдены
				if(size && (size == 2)){
					// Запоминаем протокол запроса
					protocol = toCase(prt[0]);
					// Запоминаем версию протокола
					version = prt[1];
					// Перебираем остальные заголовки
					for(u_int i = 1; i < headers.size(); i++){
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
								vector <string> lgn;
								// Функция разбиения на составляющие
								split(ath, " ", lgn);
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
											vector <string> lp;
											// Функция разбиения на составляющие
											split(dauth, ":", lp);
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
	if(connection.length() && !strcmp(version.c_str(), "1.0")) connection = "close";
	// Очищаем заголовок
	head.clear();
	// Создаем строку запроса
	head.append(toCase(method, true) + string(" ") + path + string(" ") + string("HTTP/") + version + string("\r\n"));
	// Устанавливаем заголовок Host:
	head.append(string("Host: ") + host + string(":") + port + string("\r\n"));
	// Добавляем useragent
	if(useragent.length()) head.append(string("User-Agent: ") + useragent + string("\r\n"));
	// Добавляем остальные заголовки
	for(u_int i = 0; i < other.size(); i++){
		/*
		// Если это не заголовок контента, то добавляем в список остальные заголовки (Not Gzip)
		if((toCase(other[i]).find("content-length:") == string::npos)
		&& (toCase(other[i]).find("gzip") == string::npos)) head.append(other[i] + string("\r\n"));
		*/
		// Если это не заголовок контента, то добавляем в список остальные заголовки (Gzip)
		if(toCase(other[i]).find("content-length:") == string::npos) head.append(other[i] + string("\r\n"));
	}
	// Добавляем заголовок connection
	if(connection.length()) head.append(string("Connection: ") + connection + string("\r\n"));
	// Если тело запроса существует то добавляем размер тела
	if(entitybody != NULL) head.append(string("Content-Length: ") + to_string(strlen(entitybody)) + string("\r\n"));
	// Запоминаем конец запроса
	head.append(string("\r\n"));
	// Если существует тело запроса то добавляем его
	if(entitybody != NULL) head.append(entitybody);
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

void Http::generateHttp(){
	cout << "http = " << query2.http.c_str() << endl;

	if(entitybody != NULL) cout << "entitybody = " << entitybody << endl;

	for(map <string, string>::iterator it = query2.headers.begin(); it != query2.headers.end(); ++it)
		cout << query2.origin[it->first] << " => " << it->second << '\n';
}

/**
 * parse Метод выполнения парсинга
 * @param  buffer буфер входящих данных из сокета
 * @param  size   размер переданных данных
 * @return        результат определения завершения запроса
 */
bool Http::parse2(const char * buffer, size_t size){
	// Выполняем парсинг http запроса
	if(!query2.length) query2 = getHeaders(buffer);
	// Если данные существуют
	if(query2.length){
		// Проверяем есть ли размер вложений
		string cl = query2.headers.find("content-length")->second;
		// Проверяем есть ли чанкование
		string ch = query2.headers.find("transfer-encoding")->second;
		// Если найден размер вложений
		if(cl.length()){
			// Размер вложений
			int body_size = ::atoi(cl.c_str());
			// Получаем размер вложения
			if(size >= (query2.length + body_size)){
				// Создаем тело запроса
				entitybody = new char[(const int) body_size + 1];
				// Заполняем нулями буфер
				memset(entitybody, 0, body_size + 1);
				// Извлекаем указанные данные
				//strncpy(entitybody, buffer + query2.length, body_size);
				copy(buffer + query2.length, buffer + (query2.length + body_size), entitybody);
				// Генерацию данных
				generateHttp();
				// Сообщаем что все удачно получено
				return true;
			}
		// Если это чанкование
		} else if(ch.length() && (ch.find("chunked") != string::npos)){
			// Формируем блок с данными
			string str = buffer + query2.length;
			// Размер вложений
			int body_size = str.find("0\r\n\r\n");
			// Если конец строки найден
			if(body_size != string::npos){
				// Создаем тело запроса
				entitybody = new char[(const int) (body_size + 5) + 1];
				// Заполняем нулями буфер
				memset(entitybody, 0, (body_size + 5) + 1);
				// Извлекаем указанные данные
				//strncpy(entitybody, str.c_str(), body_size + 5);
				copy(buffer + query2.length, buffer + (query2.length + body_size + 5), entitybody);
				// Генерацию данных
				generateHttp();
				// Сообщаем что все удачно получено
				return true;
			}
		// Если указан тип данных но длина вложенных данных не указана
		} else if(ch.length()) return false;
		// Если вложения не найдены
		else {
			// Генерацию данных
			generateHttp();
			// Сообщаем что все удачно получено
			return true;
		}
	}
	// Выводим результат
	return false;
}
/**
 * parse Метод выполнения парсинга
 * @param  buffer буфер входящих данных из сокета
 * @param  size   размер переданных данных
 * @return        результат определения завершения запроса
 */
bool Http::parse(const char * buffer, size_t size){
	// Символы завершения ввода заголовков
	const char * endPos = strstr(buffer, "\r\n\r\n");
	// Если данные переданы правильно
	if(endPos != NULL){
		// Получаем длину передаваемых данных
		const int len = ::atoi(findHeaderParam("Content-length", buffer).c_str());
		// Если вложения не найдены тогда передаем на обработку
		if(!len){
			// Выполняем парсинг заголовков http запроса
			parser(buffer);
			// Выполняем генерацию результирующего запроса
			createHead();
			// Выходим
			return true;
		// Иначе ищем вложения
		} else {
			// Определяем длину заголовка
			const int qlen = ((endPos + 4) - buffer) + 1;
			// Получаем заголовок запроса
			char * query = new char[qlen];
			// Копируем заголовок
			strncpy(query, buffer, qlen - 1);
			// Устанавливаем конец строки
			query[qlen - 1] = '\0';
			// Определяем длину тела вложения запроса
			const int dlen = ((int) size - (qlen + 1));
			// Если все байты тела вложения запроса получены
			if(dlen >= len){
				// Выделяем память под тело запроса
				entitybody = new char[dlen + 1];
				// Копируем тело вложения запроса
				strncpy(entitybody, buffer + strlen(query), len);
				// Устанавливаем конец строки
				entitybody[dlen] = '\0';
				// Выполняем парсинг заголовков http запроса
				parser(query);
				// Выполняем генерацию результирующего запроса
				createHead();
				// Удаляем выделенную память под заголовки запроса
				delete [] query;
				// Выходим из функции
				return true;
			}
			// Удаляем выделенную память под заголовки запроса
			delete [] query;
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
		return html[9].replace(pos, defname.length(), appname + string("/") + appver);
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
		return html[6].replace(pos, defname.length(), appname + string("/") + appver);
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
		return html[5].replace(pos, defname.length(), appname + string("/") + appver);
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
		return html[2].replace(pos, defname.length(), appname + string("/") + appver);
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
		return html[0].replace(pos, defname.length(), appname + string("/") + appver);
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
 * setClose Метод установки принудительного отключения после запроса
 */
void Http::setClose(){
	// Запоминаем данные
	connection = "close";
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
Http::Http(const string str, string ver){
	// Если имя передано то запоминаем его
	appname = str;
	// Устанавливаем версию системы
	appver = ver;
}
/**
 * Http Деструктор
 */
Http::~Http(){
	

	if(entitybody != NULL) delete [] entitybody;
	// Очищаем заголовки
	query2.clear();

	// Очищаем память выделенную для вектора
	vector <string> ().swap(other);
	vector <string> ().swap(headers);
}