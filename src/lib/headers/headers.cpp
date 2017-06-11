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
 * get Метод получения правил клиента
 * @param client     идентификатор клиента
 * @param addGeneral добавлять в список общие правила
 * @return           сформированный список правил
 */
vector <Headers::Params> Headers::get(const string client, bool addGeneral){
	// Создаем вектор
	vector <Params> rules;
	// Если клиент передан
	if(!client.empty()){
		// Создаем объект сети
		Network nwk;
		// Выполняем проверку на инверсию
		auto check = isNot(client);
		// Получаем идентификатор пользователя
		string userId = toCase(check.str);
		// Получаем типы идентификаторов
		u_short utype = checkTypeId(userId);
		// Если это ip адреса то преобразуем их
		// Для клиента
		if(utype == 1)		userId	= nwk.setLowIp(userId);		// Если это IPv4
		else if(utype == 2)	userId	= nwk.setLowIp6(userId);	// Если это IPv6
		// Если правило для клиента найдено, удаляем
		if(!check.inv && (this->rules.count(userId) > 0)){
			// Получаем данные объекта
			auto user = this->rules[userId];
			// Добавляем в массив данные
			rules.assign(user.begin(), user.end());
		// Если это инверсия то перебираем все что есть кроме звездочек
		} else if(check.inv) {
			// Выполняем перебор всех правил
			for(auto it = this->rules.begin(); it != this->rules.end(); ++it){
				// Если это не звездочки
				if(it->first.compare("*") != 0) copy(it->second.begin(), it->second.end(), back_inserter(rules));
			}
		}
		// Если общие правила найдены
		if(addGeneral && (this->rules.count("*") > 0)){
			// Получаем обище правила
			auto general = this->rules["*"];
			// Выполняем добавление общих правил
			copy(general.begin(), general.end(), back_inserter(rules));
		}
	}
	// Выводим результат
	return rules;
}
/**
 * add Метод добавления новых параметров фильтрации заголовков
 * @param client идентификатор клиента
 * @param params параметры фильтрации
 */
void Headers::add(const string client, Headers::Params params){
	// Если клиент передан
	if(!client.empty()){
		// Создаем объект сети
		Network nwk;
		// Получаем идентификатор пользователя
		string userId = toCase(client);
		// Получаем типы идентификаторов
		u_short utype = checkTypeId(userId);
		// Если это ip адреса то преобразуем их
		// Для клиента
		if(utype == 1)		userId	= nwk.setLowIp(userId);		// Если это IPv4
		else if(utype == 2)	userId	= nwk.setLowIp6(userId);	// Если это IPv6
		// Если правило для клиента найдено
		if(this->rules.count(userId) > 0){
			// Добавляем параметры в список
			this->rules[userId].push_back(params);
		// Если правила для клиента не найдено
		} else {
			// Создаем вектор
			vector <Params> rules = {params};
			// Добавляем его в список
			this->rules.insert(pair <const string, vector <Params>> (userId, rules));
		}
	}
}
/**
 * rm Метод удаления параметров фильтрации заголовков
 * @param client идентификатор клиента
 */
void Headers::rm(const string client){
	// Если клиент передан
	if(!client.empty()){
		// Создаем объект сети
		Network nwk;
		// Получаем идентификатор пользователя
		string userId = toCase(client);
		// Получаем типы идентификаторов
		u_short utype = checkTypeId(userId);
		// Если это ip адреса то преобразуем их
		// Для клиента
		if(utype == 1)		userId	= nwk.setLowIp(userId);		// Если это IPv4
		else if(utype == 2)	userId	= nwk.setLowIp6(userId);	// Если это IPv6
		// Если правило для клиента найдено, удаляем
		if(this->rules.count(userId) > 0) this->rules.erase(userId);
	}
}
/**
 * read Метод чтения из файла параметров
 */
void Headers::read(){
	// Если конфигурационный файл существует
	if(this->config != NULL){
		// Получаем данные каталога где хранится файл с правилами
		const string dir = this->config->proxy.dir;
		// Получаем имя файла
		const string filename = addToPath(dir, this->name + ".headers");
		// Проверяем на существование адреса
		if(!filename.empty()
		// Проверяем существует ли такой каталог
		&& isDirExist(dir.c_str())
		// Проверяем существует ли такой файл
		&& isFileExist(filename.c_str())){
			// Устанавливаем права на файл
			setOwner(filename.c_str(), this->config->proxy.user, this->config->proxy.group);
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
							// Action
							"(ADD|RM|\\*)(?:\\s+|\\t+)"
							// Route
							"(IN|OUT|\\*)(?:\\s+|\\t+)"
							// User
							"([\\w\\.\\-\\@\\:\\!]+|\\*)(?:\\s+|\\t+)"
							// Method
							"([A-Za-z\\s\\|\\!]+|\\*)(?:\\s+|\\t+)"
							// Path
							"([^\\s\\r\\n\\t]+)(?:\\s+|\\t+)"
							// Server
							"([\\w\\.\\-\\@\\:\\!]+|\\*)(?:\\s+|\\t+)"
							// Regex
							"([^\\s\\r\\n\\t]+|\\*)(?:\\s+|\\t+)"
							// Headers
							"([^\\r\\n\\t]+)",
							regex::ECMAScript | regex::icase
						);
						// Выполняем извлечение данных
						regex_search(filedata, match, e);
						// Если данные найдены
						if(!match.empty()){
							// Создаем объект сети
							Network nwk;
							// Получаем идентификатор пользователя
							string userId = toCase(match[3].str());
							// Получаем идентификатор сервера
							string serverId = toCase(match[6].str());
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
								// Тип идентификатора клиента
								utype,
								// Тип идентификатора сервера
								stype,
								// Метод запроса
								toCase(match[1].str()),
								// Направление трафика
								toCase(match[2].str()),
								// Данные сервера
								toCase(serverId),
								// Путь запроса
								match[5].str(),
								// Регулярные выражения
								match[7].str(),
								// Методы
								split(toCase(match[4].str()), "|"),
								// Заголовки
								split(match[8].str(), "|")
							};
							// Добавляем полученные параметры в список
							add(userId, params);
						}
					}
				}
				// Закрываем файл
				file.close();
			}
		// Выводим сообщение в лог
		} else if(!filename.empty() && (this->log != NULL)){
			// Выводим сообщение в лог, что файл не найден
			this->log->write(LOG_WARNING, 0, "headers file (%s) is not found", filename.c_str());
		}
	}
}
/**
 * modifyHeaders Метод модификации заголовков
 * @param server идентификатор сервера
 * @param rules  правила фильтрации
 * @param http   блок с http данными
 */
void Headers::modifyHeaders(const string server, vector <Headers::Params> rules, HttpData &http){
	// Если правила клиента существуют
	if(!server.empty() && !rules.empty()){
		// Направление трафика
		bool routeIn = false, routeAll = false, action = false;
		// Определяем метод запроса
		const string method = http.getMethod();
		// Определяем статус запроса
		const u_int status = http.getStatus();
		// Выполняем поиск указанного сервера
		for(u_int i = 0; i < rules.size(); i++){
			// Результат проверки фильтров
			bool result = false;
			// Определяем нужно ли учитывать направление трафика
			if(rules[i].route.compare("*") == 0) routeAll = true;
			else routeAll = false;
			// Определяем направление трафика
			if(rules[i].route.compare("in") == 0)		routeIn = true;
			else if(rules[i].route.compare("out") == 0)	routeIn = false;
			// Накладываем фильтр на направление трафика
			if(routeAll || ((!status && !routeIn) || (status && routeIn))) result = true;
			// Если фильтр сработал
			if(result){
				// Переходим по всем методам запросов
				for(u_int j = 0; j < rules[i].methods.size(); j++){
					// Выполняем проверку на инверсию
					auto check = isNot(rules[i].methods[j]);
					// Если это инверсия
					if(check.inv){
						// Если это инверсия и метод совпал тогда запрещаем дальнейшие действия и выходим из цикла
						if(check.str.compare(method) == 0){
							// Запоминаем что все не удачно
							result = false;
							// Выходим из цикла
							break;
						// Иначе разрешаем работу
						} else result = true;
					// Если это не инверсия
					} else {
						// Если это звездочка или не инверсия и метод найден, тогда разрешаем дальнейшие действия и выходим
						if((check.str.compare("*") == 0)
						|| (check.str.compare(method) == 0)){
							// Запоминаем что все удачно
							result = true;
							// Выходим из цикла
							break;
						// Запрещаем дальнейшие действия
						} else result = false;
					}
				}
				// Если фильтр сработал
				if(result){
					// Если сервер указан конкретный
					if(rules[i].server.compare("*") != 0){
						// Выполняем проверку на инверсию
						auto check = isNot(rules[i].server);
						// Если это инверсия
						if(check.inv){
							// Определяем как надо искать сервер
							switch(rules[i].stype){
								// Если поиск идет по ip адресу
								case 1:
								case 2: if(check.str.compare(server) == 0) result = false; break;
								// Если поиск идет по домену
								case 4: if(toCase(check.str).compare(toCase(http.getHost())) == 0) result = false; break;
								// Метод по умолчанию
								default: result = false;
							}
						// Если это не инверсия
						} else {
							// Определяем как надо искать сервер
							switch(rules[i].stype){
								// Если поиск идет по ip адресу
								case 1:
								case 2: if(check.str.compare(server) != 0) result = false; break;
								// Если поиск идет по домену
								case 4: if(toCase(check.str).compare(toCase(http.getHost())) != 0) result = false; break;
								// Метод по умолчанию
								default: result = false;
							}
						}
					}
					// Если фильтр сработал и это исходящий трафик, проверяем на путь запроса
					if(result){
						// Если путь указан конкретный
						if(rules[i].path.compare("*") != 0){
							// Выполняем проверку на инверсию
							auto check = isNot(rules[i].path);
							// Если это инверсия, и адрес запроса совпадает, запрещаем дальнейшие действия
							if(check.inv){
								// Проверяем совпадает ли адрес
								if(toCase(check.str)
								.compare(toCase(http.getPath())) == 0) result = false;
							// Если это не инверсия, и адрес запроса совпадает, тогда разрешаем дальнейшие действия
							} else {
								// Проверяем совпадает ли адрес
								if(toCase(check.str)
								.compare(toCase(http.getPath())) != 0) result = false;
							}
						}
						// Если фильтр сработал и это исходящий трафик
						if(result && (rules[i].regex.compare("*") != 0)){
							// Проверяем на соответствие юзер-агента
							// Результат работы регулярного выражения
							smatch match;
							// Устанавливаем правило регулярного выражения
							regex e(rules[i].regex, regex::ECMAScript | regex::icase);
							// Получаем UserAgent
							const string userAgent = http.getUseragent();
							// Выполняем проверку
							regex_search(userAgent, match, e);
							// Выводим результат
							result = !match.empty();
						}
					}
				}
			}
			// Если фильтры сработали то выполняем модификацию
			if(result){
				// Определяем тип действия, удаление или добавление заголовков
				if(rules[i].action.compare("add") == 0)		action = true;
				else if(rules[i].action.compare("rm") == 0)	action = false;
				// Переходим по всему массиву заголовков
				for(u_int j = 0; j < rules[i].headers.size(); j++){
					// Результат работы регулярного выражения
					smatch match;
					// Исправляем название заголовка
					const string header = rules[i].headers[j];
					// Если нужно добавить заголовки
					if(action){
						// Устанавливаем правило регулярного выражения
						regex e("^([\\w\\-]+)\\s*\\:\\s*([^\\r\\n\\t\\s]+)$", regex::ECMAScript | regex::icase);
						// Выполняем проверку
						regex_search(header, match, e);
						// Если данные найдены
						if(!match.empty()) http.setHeader(match[1].str(), match[2].str());
					// Если нужно удалить заголовки
					} else {
						// Устанавливаем правило регулярного выражения
						regex e("^([\\w\\-]+)\\s*\\:?", regex::ECMAScript | regex::icase);
						// Выполняем проверку
						regex_search(header, match, e);
						// Если данные найдены
						if(!match.empty()) http.rmHeader(match[1].str());
					}
				}
			}
		}
	}
}
/**
 * modifyHeaders Метод модификации заголовков
 * @param server идентификатор сервера
 * @param rules  правила фильтрации
 * @param data   строка с данными запроса или ответа
 * @param http   блок с http данными
 */
void Headers::modifyHeaders(const string server, vector <Headers::Params> rules, string &data, HttpData &http){
	// Если правила клиента существуют
	if(!server.empty() && !rules.empty() && !data.empty()){
		// Направление трафика
		bool routeIn = false, routeAll = false, action = false;
		// Определяем метод запроса
		const string method = http.getMethod();
		// Определяем статус запроса
		const u_int status = http.getStatus();
		// Выполняем поиск указанного сервера
		for(u_int i = 0; i < rules.size(); i++){
			// Результат проверки фильтров
			bool result = false;
			// Определяем нужно ли учитывать направление трафика
			if(rules[i].route.compare("*") == 0) routeAll = true;
			else routeAll = false;
			// Определяем направление трафика
			if(rules[i].route.compare("in") == 0)		routeIn = true;
			else if(rules[i].route.compare("out") == 0)	routeIn = false;
			// Накладываем фильтр на направление трафика
			if(routeAll || ((!status && !routeIn) || (status && routeIn))) result = true;
			// Если фильтр сработал
			if(result){
				// Переходим по всем методам запросов
				for(u_int j = 0; j < rules[i].methods.size(); j++){
					// Выполняем проверку на инверсию
					auto check = isNot(rules[i].methods[j]);
					// Если это инверсия
					if(check.inv){
						// Если это инверсия и метод совпал тогда запрещаем дальнейшие действия и выходим из цикла
						if(check.str.compare(method) == 0){
							// Запоминаем что все не удачно
							result = false;
							// Выходим из цикла
							break;
						// Иначе разрешаем работу
						} else result = true;
					// Если это не инверсия
					} else {
						// Если это звездочка или не инверсия и метод найден, тогда разрешаем дальнейшие действия и выходим
						if((check.str.compare("*") == 0)
						|| (check.str.compare(method) == 0)){
							// Запоминаем что все удачно
							result = true;
							// Выходим из цикла
							break;
						// Запрещаем дальнейшие действия
						} else result = false;
					}
				}
				// Если фильтр сработал
				if(result){
					// Если сервер указан конкретный
					if(rules[i].server.compare("*") != 0){
						// Выполняем проверку на инверсию
						auto check = isNot(rules[i].server);
						// Если это инверсия
						if(check.inv){
							// Определяем как надо искать сервер
							switch(rules[i].stype){
								// Если поиск идет по ip адресу
								case 1:
								case 2: if(check.str.compare(server) == 0) result = false; break;
								// Если поиск идет по домену
								case 4: if(toCase(check.str).compare(toCase(http.getHost())) == 0) result = false; break;
								// Метод по умолчанию
								default: result = false;
							}
						// Если это не инверсия
						} else {
							// Определяем как надо искать сервер
							switch(rules[i].stype){
								// Если поиск идет по ip адресу
								case 1:
								case 2: if(check.str.compare(server) != 0) result = false; break;
								// Если поиск идет по домену
								case 4: if(toCase(check.str).compare(toCase(http.getHost())) != 0) result = false; break;
								// Метод по умолчанию
								default: result = false;
							}
						}
					}
					// Если фильтр сработал и это исходящий трафик, проверяем на путь запроса
					if(result){
						// Если путь указан конкретный
						if(rules[i].path.compare("*") != 0){
							// Выполняем проверку на инверсию
							auto check = isNot(rules[i].path);
							// Если это инверсия, и адрес запроса совпадает, запрещаем дальнейшие действия
							if(check.inv){
								// Проверяем совпадает ли адрес
								if(toCase(check.str)
								.compare(toCase(http.getPath())) == 0) result = false;
							// Если это не инверсия, и адрес запроса совпадает, тогда разрешаем дальнейшие действия
							} else {
								// Проверяем совпадает ли адрес
								if(toCase(check.str)
								.compare(toCase(http.getPath())) != 0) result = false;
							}
						}
						// Если фильтр сработал и это исходящий трафик
						if(result && (rules[i].regex.compare("*") != 0)){
							// Проверяем на соответствие юзер-агента
							// Результат работы регулярного выражения
							smatch match;
							// Устанавливаем правило регулярного выражения
							regex e(rules[i].regex, regex::ECMAScript | regex::icase);
							// Получаем UserAgent
							const string userAgent = http.getUseragent();
							// Выполняем проверку
							regex_search(userAgent, match, e);
							// Выводим результат
							result = !match.empty();
						}
					}
				}
			}
			// Если фильтры сработали то выполняем модификацию
			if(result){
				// Определяем тип действия, удаление или добавление заголовков
				if(rules[i].action.compare("add") == 0)		action = true;
				else if(rules[i].action.compare("rm") == 0)	action = false;
				// Переходим по всему массиву заголовков
				for(u_int j = 0; j < rules[i].headers.size(); j++){
					// Результат работы регулярного выражения
					smatch match;
					// Исправляем название заголовка
					const string header = rules[i].headers[j];
					// Если нужно добавить заголовки
					if(action){
						// Устанавливаем правило регулярного выражения
						regex e("^([\\w\\-]+)\\s*\\:\\s*([^\\r\\n\\t\\s]+)$", regex::ECMAScript | regex::icase);
						// Выполняем проверку
						regex_search(header, match, e);
						// Если данные найдены
						if(!match.empty()) http.addHeaderToString(match[1].str(), match[2].str(), data);
					// Если нужно удалить заголовки
					} else {
						// Устанавливаем правило регулярного выражения
						regex e("^([\\w\\-]+)\\s*\\:?", regex::ECMAScript | regex::icase);
						// Выполняем проверку
						regex_search(header, match, e);
						// Если данные найдены
						if(!match.empty()) http.rmHeaderInString(match[1].str(), data);
					}
				}
			}
		}
	}
}
/**
 * isNot Метод проверки на инверсию
 * @param  str строка для проверки
 * @return     результат проверки
 */
const Headers::IsNot Headers::isNot(const string str){
	// Результат проверки
	bool result = str[0] == '!';
	// Выполняем проверку на первый символ
	return {result, (result ? str.substr(1, str.length() - 1) : str)};
}
/**
 * checkTypeId Метод определения типа идентификатора
 * @param  str строка идентификатора для определения типа
 * @return     тип идентификатора
 */
const u_short Headers::checkTypeId(const string str){
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
 * isAddress Метод проверки на то является ли строка адресом
 * @param  address строка адреса для проверки
 * @return         результат проверки
 */
const bool Headers::isAddress(const string address){
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e(
		// Определение домена
		"(?:[\\w\\-\\.]+\\.[\\w\\-]+|"
		// Определение мак адреса
		"[A-Fa-f\\d]{2}(?:\\:[A-Fa-f\\d]{2}){5}|"
		// Определение ip адреса
		"\\d{1,3}(?:\\.\\d{1,3}){3}|"
		// Определение ip6 адреса (в полном формате)
		"[A-Fa-f\\d]{4}(?:\\:[A-Fa-f\\d]{4}){7})",
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
const bool Headers::isIpV4(const string ip){
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e("\\d{1,3}(?:\\.\\d{1,3}){3}", regex::ECMAScript | regex::icase);
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
const bool Headers::isIpV6(const string ip){
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e("[A-Fa-f\\d]{4}(?:\\:[A-Fa-f\\d]{4}){7}", regex::ECMAScript | regex::icase);
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
const bool Headers::isIp(const string ip){
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e(
		"(?:\\d{1,3}(?:\\.\\d{1,3}){3}|"
		"[A-Fa-f\\d]{4}(?:\\:[A-Fa-f\\d]{4}){7})",
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
const bool Headers::isMac(const string mac){
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e("[A-Fa-f\\d]{2}(?:\\:[A-Fa-f\\d]{2}){5}", regex::ECMAScript | regex::icase);
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
const bool Headers::isDomain(const string domain){
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
const bool Headers::isLogin(const string login){
	// Выводим результат проверки
	return !isAddress(login);
}
/**
 * checkAvailable Метод проверки на существование параметров заголовков
 * @param  name название файла с параметрами
 * @return      результат проверки
 */
const bool Headers::checkAvailable(const string name){
	// Результат проверки
	bool result = false;
	// Если название существует
	if(!name.empty()){
		// Получаем данные каталога где хранится файл с правилами
		const string dir = this->config->proxy.dir;
		// Получаем имя файла
		const string filename = addToPath(dir, name + ".headers");
		// Проверяем существуют ли параметры заголовков
		if(isDirExist(dir.c_str()) && isFileExist(filename.c_str())) result = true;
	}
	// Выводим результат
	return result;
}
/**
 * clear Метод очистки данных
 */
void Headers::clear(){
	// Очищаем данные правил
	this->rules.clear();
}
/**
 * modify Метод модификации заголовков
 * @param ip     ip адрес клиента
 * @param mac    мак адрес клиента
 * @param server адрес сервера
 * @param http   блок с данными запроса или ответа
 */
void Headers::modify(const string ip, const string mac, const string server, HttpData &http){
	// Если правило для клиента найдено
	if(!ip.empty() && !mac.empty() && !server.empty()){
		// Создаем объект сети
		Network nwk;
		// Получаем идентификатор сервера
		string serverId = toCase(server);
		// Получаем типы идентификаторов
		u_short stype = checkTypeId(serverId);
		// Если это ip адреса то преобразуем их
		// Для сервера
		if(stype == 1)		serverId	= nwk.setLowIp(serverId);	// Если это IPv4
		else if(stype == 2)	serverId	= nwk.setLowIp6(serverId);	// Если это IPv6
		// Получаем правила клиента по ip адресу
		auto rules1 = get(ip, false);
		// Получаем правила клиента по mac адресу
		auto rules2 = get(mac, false);
		// Получаем общие правила для клиента
		auto rules3 = get("*", false);
		// Выполняем модификацию заголовков
		modifyHeaders(serverId, rules1, http);
		modifyHeaders(serverId, rules2, http);
		modifyHeaders(serverId, rules3, http);
	}
}
/**
 * modify Метод модификации заголовков
 * @param ip     ip адрес клиента
 * @param mac    мак адрес клиента
 * @param server адрес сервера
 * @param data   строка с данными запроса или ответа
 */
void Headers::modify(const string ip, const string mac, const string server, string &data){
	// Если правило для клиента найдено
	if(!ip.empty() && !mac.empty() && !server.empty() && !data.empty()){
		// Создаем http объект
		HttpData http;
		// Выполняем обработку данных
		if(http.parse(data.c_str(), data.size(), this->config->proxy.name, this->options)){
			// Создаем объект сети
			Network nwk;
			// Получаем идентификатор сервера
			string serverId = toCase(server);
			// Получаем типы идентификаторов
			u_short stype = checkTypeId(serverId);
			// Если это ip адреса то преобразуем их
			// Для сервера
			if(stype == 1)		serverId	= nwk.setLowIp(serverId);	// Если это IPv4
			else if(stype == 2)	serverId	= nwk.setLowIp6(serverId);	// Если это IPv6
			// Получаем правила клиента по ip адресу
			auto rules1 = get(ip, false);
			// Получаем правила клиента по mac адресу
			auto rules2 = get(mac, false);
			// Получаем общие правила для клиента
			auto rules3 = get("*", false);
			// Выполняем модификацию заголовков
			modifyHeaders(serverId, rules1, data, http);
			modifyHeaders(serverId, rules2, data, http);
			modifyHeaders(serverId, rules3, data, http);
			// Выполняем модификацию основных заголовков
			data = http.modifyHeaderString(data);
		}
	}
}
/**
 * Headers Конструктор
 * @param config  конфигурационные данные
 * @param log     объект лога для вывода информации
 * @param options основные параметры прокси
 * @param name    название конфигурационного файла
 */
Headers::Headers(Config * config, LogApp * log, const u_short options, const string name){
	// Если конфигурационные данные переданы
	if(config != NULL){
		// Очищаем все параметры
		clear();
		// Запоминаем объект логов
		this->log = log;
		// Запоминаем параметры конфига
		this->config = config;
		// Запоминаем название конфигурационного файла
		this->name = (!name.empty() ? name : this->config->proxy.name);
		// Запоминаем основные параметры прокси
		this->options = (options != 0x0 ? options : this->config->options);
		// Выполняем чтение файла конфигурации
		read();
	}
}