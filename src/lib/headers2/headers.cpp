/* МОДУЛЬ УПРАВЛЕНИЯ ЗАГОЛОВКАМИ AMING */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#include "headers2/headers.h"

// Устанавливаем область видимости
using namespace std;

/**
 * readFromLDAP Метод чтения данных из LDAP сервера
 */
void Headers2::readFromLDAP(){

}
/**
 * readFromFile Метод чтения данных из файла
 */
void Headers2::readFromFile(){
	// Получаем название файла
	const string name = getName();
	// Если название существует и файл тоже
	if(!name.empty() && checkAvailable(name)){
		// Получаем данные каталога где хранится файл с правилами
		const string dir = this->config->proxy.dir;
		// Получаем имя файла
		const string filename = Anyks::addToPath(dir, name + ".headers");
		// Проверяем на существование адреса
		if(!filename.empty()
		// Проверяем существует ли такой каталог
		&& Anyks::isDirExist(dir.c_str())
		// Проверяем существует ли такой файл
		&& Anyks::isFileExist(filename.c_str())){
			// Устанавливаем права на файл
			Anyks::setOwner(filename.c_str(), this->config->proxy.user, this->config->proxy.group);
			// Строка чтения из файла
			string filedata;
			// Открываем файл на чтение
			ifstream file(filename.c_str());
			// Если файл открыт
			if(file.is_open()){
				// Очищаем блок с правилами
				this->rules.clear();
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
							"((?:ADD|RM|\\*)(?:\\s*\\|\\s*(?:ADD|RM|\\*))*)(?:\\s+|\\t+)"
							// Traffic
							"((?:IN|OUT|\\*)(?:\\s*\\|\\s*(?:IN|OUT|\\*))*)(?:\\s+|\\t+)"
							// Client
							"((?:\\!?[\\w\\.\\-\\@\\:\\!\\/]+|\\*)(?:\\s*\\|\\s*(?:\\!?[\\w\\.\\-\\@\\:\\!\\/]+|\\*))*)(?:\\s+|\\t+)"
							// Server
							"((?:\\!?[\\w\\.\\-\\@\\:\\!\\/]+|\\*)(?:\\s*\\|\\s*(?:\\!?[\\w\\.\\-\\@\\:\\!\\/]+|\\*))*)(?:\\s+|\\t+)"
							// Method
							"(\\!?(?:" PROXY_HTTP_METHODS "|\\*)(?:\\s*\\|\\s*\\!?(?:" PROXY_HTTP_METHODS "|\\*))*)(?:\\s+|\\t+)"
							// Path
							"((?:\\!?\\/[\\w\\-\\_]*(?:\\/[\\w\\-\\_]*)*|\\*)(?:\\s*\\|\\s*(?:\\!?\\/[\\w\\-\\_]*(?:\\/[\\w\\-\\_]*)*|\\*))*)(?:\\s+|\\t+)"
							// Query
							"((?:\\!?\\?\\w+\\=[^\\s\\r\\n\\t]+|\\*)(?:\\s*\\|\\s*(?:\\!?\\?\\w+\\=[^\\s\\r\\n\\t]+|\\*))*)(?:\\s+|\\t+)"
							// Agent
							"([^\\s\\r\\n\\t]+|\\*)(?:\\s+|\\t+)"
							// User
							"(\\!?[\\w]{1,30}(?:\\s*\\|\\s*(?:\\!?[\\w]{1,30}|\\*))*)(?:\\s+|\\t+)"
							// Group
							"(\\!?[\\w]{1,30}(?:\\s*\\|\\s*(?:\\!?[\\w]{1,30}|\\*))*)(?:\\s+|\\t+)"
							// Headers
							"([^\\r\\n\\t]+)$",
							regex::ECMAScript | regex::icase
						);
						// Выполняем извлечение данных
						regex_search(filedata, match, e);
						// Если данные найдены
						if(!match.empty()){
							// Получаем блок данных экшенов
							auto raw_actions = Anyks::split(match[1].str(), "|");
							// Получаем блок данных направления трафика
							auto raw_traffic = Anyks::split(match[2].str(), "|");
							// Получаем блок данных клиентов
							auto raw_clients = Anyks::split(match[3].str(), "|");
							// Получаем блок данных серверов
							auto raw_servers = Anyks::split(match[4].str(), "|");
							// Получаем блок данных методов
							auto raw_methods = Anyks::split(match[5].str(), "|");
							// Получаем блок данных путей
							auto raw_paths = Anyks::split(match[6].str(), "|");
							// Получаем блок данных запросов
							auto raw_queries = Anyks::split(match[7].str(), "|");
							// Получаем блок данных агента
							auto raw_agent = match[8].str();
							// Получаем блок данных пользователей
							auto raw_users = Anyks::split(match[9].str(), "|");
							// Получаем блок данных групп
							auto raw_groups = Anyks::split(match[10].str(), "|");
							// Получаем блок данных заголовков
							auto raw_headers = Anyks::split(match[11].str(), "|");
							/* Начинаем извлечение данных */
							// Получаем список всех групп
							auto data_groups = this->groups->getAllGroups();
							// Если группы существуют
							if(!data_groups.empty()){
								/**
								 * createNode Функция создания списка нод
								 * @param [Array] список значений которые необходимо обработать
								 * @return        список созданных нод
								 */
								auto createNode = [](vector <string> list){
									// Список клиентов
									vector <Node> nodes;
									// Переходим по списку
									for(auto it = list.cbegin(); it != list.cend(); ++it){
										// Извлекаем строку с типом
										string str = * it;
										// Объект клиента
										Node node;
										// Если это не звездочка
										if(str.compare("*") != 0){
											// Определяем тип записи
											const u_int type = Anyks::getTypeAmingByString(* it);
											// Если тип данных определен
											if(type != AMING_NULL){
												// Определяем что нужно выполнить для данного типа
												switch(type){
													// Запоминаем mac адрес
													case AMING_MAC: node.mac = str; break;
													// Если это ip адрес версии протокола 4
													case AMING_IPV4: node.ip4 = str; break;
													// Если это ip адрес версии протокола 6
													case AMING_IPV6: node.ip6 = str; break;
													// Если это домен
													case AMING_DOMAIN: node.domain = str; break;
													// Если это сеть
													case AMING_NETWORK: {
														// Создаем объект сети
														Network nwk;
														// Получаем данные сети
														string ipNwk = nwk.getIPByNetwork(str);
														// Получаем версию протокола
														u_int version = nwk.checkNetworkByIp(ipNwk);
														// Запоминаем данные префикса
														node.prefix = nwk.getPrefixByNetwork(str);
														// Проверяем тип ip адреса
														switch(version){
															case 4: node.ip4 = ipNwk; break;
															case 6: node.ip6 = ipNwk; break;
														}
													} break;
												}
												// Добавляем в список нод
												nodes.push_back(node);
											}
										}
									}
									// Выводим результат
									return nodes;
								};
								// Создаем объект с правилами
								const Rules rules = {
									// Запоминаем данные агента
									raw_agent,
									// Список клиентов
									createNode(raw_clients),
									// Список серверов
									createNode(raw_servers),
									// Список путей
									raw_paths,
									// Список запросов
									raw_queries,
									// Список заголовков
									raw_headers
								};
								// Объект со списком методов
								unordered_map <string, Rules> methods = {
									{"get", {}},
									{"put", {}},
									{"head", {}},
									{"post", {}},
									{"patch", {}},
									{"trace", {}},
									{"delete", {}},
									{"connect", {}},
									{"options", {}}
								};
								// Объект со списком направлений траффика
								map <bool, unordered_map <string, Rules>> traffics = {{true, {}}, {false, {}}};
								// Объект со списком экшенов
								map <bool, map <bool, unordered_map <string, Rules>>> actions = {{true, {}}, {false, {}}};
								// Объект со списком пользователей
								map <uid_t, map <bool, map <bool, unordered_map <string, Rules>>>> users;
								// Переходим по всему списку методов
								for(auto it = raw_methods.cbegin(); it != raw_methods.cend(); ++it){
									// Определяем метод
									string method = * it;
									// Приводим к нижнему регистру
									method = Anyks::toCase(method);
									// Если метод не является звездочкой
									if(method.compare("*") != 0){
										// Получаем нужный нам метод и добавляем туда правила
										methods.at(method) = rules;
									// Если найдена звездочка то добавляем во все методы
									} else {
										// Переходим по всему списку методов
										for(auto mt = methods.cbegin(); mt != methods.cend(); ++mt){
											// Добавляем правила
											methods.at(mt->first) = rules;
										}
										// Выходим из цикла
										break;
									}
								}
								// Переходим по всему списку направлений трафика
								for(auto it = raw_traffic.cbegin(); it != raw_traffic.cend(); ++it){
									// Определяем трафик
									string traffic = * it;
									// Приводим к нижнему регистру
									traffic = Anyks::toCase(traffic);
									// Если трафик не является звездочкой
									if(traffic.compare("*") != 0){
										// Если это входящий трафик
										if(traffic.compare("in") == 0) traffics.at(true) = methods;
										// Если это исходящий трафик
										if(traffic.compare("out") == 0) traffics.at(false) = methods;
									// Если найдена звездочка то добавляем во все методы
									} else {
										// Запоминаем параметры для всех видов трафика
										traffics.at(true)	= methods;
										traffics.at(false)	= methods;
										// Выходим из цикла
										break;
									}
								}
								// Переходим по всему списку экшенов
								for(auto it = raw_actions.cbegin(); it != raw_actions.cend(); ++it){
									// Определяем экшен
									string action = * it;
									// Приводим к нижнему регистру
									action = Anyks::toCase(action);
									// Если экшен не является звездочкой
									if(action.compare("*") != 0){
										// Если это добавление заголовков
										if(action.compare("add") == 0) actions.at(true) = traffics;
										// Если это удаление заголовков
										if(action.compare("rm") == 0) actions.at(false) = traffics;
									// Если найдена звездочка то добавляем во все экшены
									} else {
										// Запоминаем параметры для всех видов экшенов
										actions.at(true)	= traffics;
										actions.at(false)	= traffics;
										// Выходим из цикла
										break;
									}
								}
								/**
								 * createRules Функция создания правил
								 * @param [gid_t] идентификатор группы
								 */
								auto createRules = [&users, &actions, &raw_users, this](gid_t gid){
									// Переходим по всему списку пользователей
									for(auto it = raw_users.cbegin(); it != raw_users.cend(); ++it){
										// Определяем пользователя
										string user = * it;
										// Если пользователь не является звездочкой
										if(user.compare("*") != 0){
											// Идентификатор пользователя
											uid_t uid = 0;
											// Определяем идентификатор пользователя
											if(Anyks::isNumber(user)) uid = ::atoi(user.c_str());
											// Если это название пользователя
											else uid = this->groups->getUidByName(user);
											// Если пользователь принадлежит группе
											if(this->groups->checkUser(gid, uid)){
												// Добавляем список экшенов к пользователю
												users.emplace(uid, actions);
											}
										// Если найдена звездочка то добавляем во все экшены
										} else {
											// Очищаем список пользователей
											users.clear();
											// Запрашиваем список всех пользователей группы
											auto uids = this->groups->getIdUsers(gid);
											// Переходим по всем идентификаторам пользователей и добавляем туда экшены
											for(auto it = uids.cbegin(); it != uids.cend(); ++it) users.emplace(* it, actions);
											// Выходим из цикла
											break;
										}
									}
									// Добавляем в список правил
									this->rules.emplace(gid, users);
								};
								// Переходим по всему массиву групп
								for(auto it = raw_groups.cbegin(); it != raw_groups.cend(); ++it){
									// Определяем группу
									string group = * it;
									// Очищаем список пользователей
									users.clear();
									// Если группа не является звездочкой
									if(group.compare("*") != 0){
										// Идентификатор группы
										gid_t gid = 0;
										// Если это идентификатор группы
										if(Anyks::isNumber(group)) gid = ::atoi(group.c_str());
										// Если это название группы
										else gid = this->groups->getIdByName(group);
										// Выполняем создание правила
										if(this->groups->checkGroupById(gid)) createRules(gid);
									// Если найдена звездочка то добавляем во все группы
									} else {
										// Переходим по всему списку групп
										for(auto it = data_groups.cbegin(); it != data_groups.cend(); ++it){
											// Выполняем создание правила
											createRules(it->id);
										}
										// Выходим из цикла
										break;
									}
								}
							// Если группы не найдены, выводим сообщение об ошибке
							} else if(this->log) this->log->write(LOG_ERROR, 0, "groups not found for headers file (%s)", filename.c_str());
						// Если синтаксис файла заголовков не верный
						} else if(this->log) this->log->write(LOG_ERROR, 0, "Invalid syntax for header file (%s)", filename.c_str());
					}
				}
				// Закрываем файл
				file.close();
			// Выводим сообщение об ошибке
			} else if(this->log) this->log->write(LOG_ERROR, 0, "headers file (%s) is cannot open", filename.c_str());
		// Выводим сообщение в лог
		} else if(!filename.empty() && this->log){
			// Выводим сообщение в лог, что файл не найден
			this->log->write(LOG_WARNING, 0, "headers file (%s) is not found", filename.c_str());
		}
	}
}
/**
 * checkAvailable Метод проверки на существование параметров заголовков
 * @param  name название файла с параметрами
 * @return      результат проверки
 */
const bool Headers2::checkAvailable(const string name){
	// Результат проверки
	bool result = false;
	// Если название существует
	if(!name.empty()){
		// Получаем данные каталога где хранится файл с правилами
		const string dir = this->config->proxy.dir;
		// Получаем имя файла
		const string filename = Anyks::addToPath(dir, name + ".headers");
		// Проверяем существуют ли параметры заголовков
		if(Anyks::isDirExist(dir.c_str()) && Anyks::isFileExist(filename.c_str())) result = true;
	}
	// Выводим результат
	return result;
}
/**
 * getName Метод получения имени конфига
 */
const string Headers2::getName(){
	// Результат
	string result;
	// Переходим по всему объекту имен
	for(auto it = this->names.cbegin(); it != this->names.cend(); ++it){
		// Проверяем существует ли файл конфигурации
		if(checkAvailable(* it)){
			// Запоминаем результат
			result = * it;
			// Выходим из цикла
			break;
		}
	}
	// Выводим результат
	return result;
}
/**
 * read Метод чтения из параметров
 */
void Headers2::read(){
	// Создаем текущее время генерации
	time_t curUpdate = time(nullptr);
	// Если время ожидания уже вышло, выполняем обновление данных
	if((this->lastUpdate + this->maxUpdate) < curUpdate){
		// Запоминаем текущее время
		this->lastUpdate = curUpdate;
		// Очищаем блок данных
		this->rules.clear();
		// Определяем тип работы
		switch(this->typeSearch){
			// Считываем данные из файлов
			case 0: readFromFile(); break;
			// Считываем данные из LDAP
			case 1: readFromLDAP(); break;
			// Считываем данные из файлов и LDAP
			case 2: {
				readFromFile();
				readFromLDAP();
			} break;
		}
	}
}
/**
 * clear Метод очистки данных
 */
void Headers2::clear(){
	// Очищаем данные правил
	this->rules.clear();
	this->names.clear();
}
/**
 * addName Метод добавления нового имени конфига
 * @param name название файла с параметрами
 */
void Headers2::addName(const string name){
	// Если название существует
	if(!name.empty()) this->names.push_front(name);
}
/**
 * Headers Конструктор
 * @param config конфигурационные данные
 * @param log    объект лога для вывода информации
 */
Headers2::Headers2(Config * config, LogApp * log, Groups * groups){
	// Если конфигурационные данные переданы
	if(config && groups){
		// Очищаем все параметры
		clear();
		// Запоминаем объект логов
		this->log = log;
		// Запоминаем параметры конфига
		this->config = config;
		// Запоминаем параметры групп
		this->groups = groups;
		// Запоминаем название конфигурационного файла
		this->names.push_front(this->config->proxy.name);
		// Запоминаем тип поиска параметров заголовков
		this->typeSearch = 0;
		// Запоминаем время в течение которого запрещено обновлять данные
		this->maxUpdate = 600;
		// Выполняем чтение файла конфигурации
		read();
	}
}
