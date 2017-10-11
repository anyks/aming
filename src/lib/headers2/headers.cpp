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
 * isNot Метод проверки на инверсию
 * @param  str строка для проверки
 * @return     результат проверки
 */
const Headers2::IsNot Headers2::isNot(const string str){
	// Результат проверки
	bool result = str[0] == '!';
	// Выполняем проверку на первый символ
	return {result, (result ? str.substr(1, str.length() - 1) : str)};
}
/**
 * createRulesList Метод созданий списка правил
 * @param params список параметров
 */
void Headers2::createRulesList(const Headers2::Params params){
	// Получаем список всех групп
	auto data_groups = this->ausers->getAllGroups();
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
		// Получаем данные регулярное выражение юзер-агента
		const string userAgent = (!params.userAgent.empty() && (params.userAgent.compare("*") != 0) ? params.userAgent : string(""));
		// Создаем объект с правилами
		const Rules rules = {
			// Запоминаем данные агента
			userAgent,
			// Список клиентов
			createNode(params.clients),
			// Список серверов
			createNode(params.servers),
			// Список путей
			params.paths,
			// Список запросов
			params.queries,
			// Список заголовков
			params.headers
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
		for(auto it = params.methods.cbegin(); it != params.methods.cend(); ++it){
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
		for(auto it = params.traffic.cbegin(); it != params.traffic.cend(); ++it){
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
		for(auto it = params.actions.cbegin(); it != params.actions.cend(); ++it){
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
		auto createRules = [&users, &actions, &params, this](gid_t gid){
			// Переходим по всему списку пользователей
			for(auto it = params.users.cbegin(); it != params.users.cend(); ++it){
				// Определяем пользователя
				string user = * it;
				// Если пользователь не является звездочкой
				if(user.compare("*") != 0){
					// Идентификатор пользователя
					uid_t uid = 0;
					// Определяем идентификатор пользователя
					if(Anyks::isNumber(user)) uid = ::atoi(user.c_str());
					// Если это название пользователя
					else uid = this->ausers->getUidByName(user);
					// Если пользователь принадлежит группе
					if(this->ausers->checkUserInGroup(gid, uid)){
						// Добавляем список экшенов к пользователю
						users.emplace(uid, actions);
					}
				// Если найдена звездочка то добавляем во все экшены
				} else {
					// Очищаем список пользователей
					users.clear();
					// Запрашиваем список всех пользователей группы
					auto uids = this->ausers->getIdUsersInGroup(gid);
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
		for(auto it = params.groups.cbegin(); it != params.groups.cend(); ++it){
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
				else gid = this->ausers->getGidByName(group);
				// Выполняем создание правила
				if(this->ausers->checkGroupById(gid)) createRules(gid);
			// Если найдена звездочка то добавляем во все группы
			} else {
				// Переходим по всему списку групп
				for(auto it = data_groups.cbegin(); it != data_groups.cend(); ++it){
					// Выполняем создание правила
					createRules((* it)->id);
				}
				// Выходим из цикла
				break;
			}
		}
	// Если группы не найдены, выводим сообщение об ошибке
	} else if(this->log) this->log->write(LOG_ERROR, 0, "groups not found for headers rules");
}
/**
 * readFromLDAP Метод чтения данных из LDAP сервера
 */
void Headers2::readFromLDAP(){
	// Получаем название файла
	const string name = getName();
	// Если название существует и файл тоже
	if(!name.empty() && checkAvailable(name)){
		// Создаем объект подключения LDAP
		ALDAP ldap(this->config, this->log);
		// Создаем DN запроса
		const string dn = (string("ah=") + name + string(",") + this->ldap.dn);
		// Список параметров
		const string lParams =	"amingHeadersUser,amingHeadersGroup,amingHeadersAgent,"
								"amingHeadersAction,amingHeadersTraffic,amingHeadersMethod,"
								"amingHeadersPath,amingHeadersQuery,amingHeadersHeaders,"
								"amingHeadersClientDomain,amingHeadersClientMac,"
								"amingHeadersClientIp4,amingHeadersClientIp6,"
								"amingHeadersClientNetwork4,amingHeadersClientNetwork6,"
								"amingHeadersServerDomain,amingHeadersServerMac,"
								"amingHeadersServerIp4,amingHeadersServerIp6,"
								"amingHeadersServerNetwork4,amingHeadersServerNetwork6";
		// Запрашиваем данные пользователей
		auto data = ldap.data(dn, lParams, this->ldap.scope, this->ldap.filter);
		// Если данные получены
		if(!data.empty()){
			// Переходим по всему объекту данных
			for(auto it = data.cbegin(); it != data.cend(); ++it){
				// Создаем строку обозначающую все данные
				const string str_all = "*";
				// Создаем значение по умолчанию
				const vector <string> list_all = {str_all};
				// Создаем параметры для создания правил
				Params params = {
					str_all,
					list_all,
					list_all,
					list_all,
					list_all,
					list_all,
					list_all,
					list_all,
					list_all,
					list_all,
					list_all
				};
				// Переходим по всему массиву полученных объектов
				for(auto dt = it->vals.cbegin(); dt != it->vals.cend(); ++dt){
					// Если список значений существует
					if(!dt->second.empty()){
						// Если это список пользователей
						if(dt->first.compare("amingHeadersUser") == 0) params.users = dt->second;
						// Если это список групп
						else if(dt->first.compare("amingHeadersGroup") == 0) params.groups = dt->second;
						// Если это список методов
						else if(dt->first.compare("amingHeadersMethod") == 0) params.methods = dt->second;
						// Если это список путей
						else if(dt->first.compare("amingHeadersPath") == 0) params.paths = dt->second;
						// Если это список параметров запросов
						else if(dt->first.compare("amingHeadersQuery") == 0) params.queries = dt->second;
						// Если это список заголовков
						else if(dt->first.compare("amingHeadersHeaders") == 0) params.headers = dt->second;
						// Если это регулярное выражение юзер-агента
						else if(dt->first.compare("amingHeadersAgent") == 0) params.userAgent = dt->second[0];
						// Если это список экшенов
						else if(dt->first.compare("amingHeadersAction") == 0){
							// Создаем список экшенов
							vector <string> actions;
							// Переходим по списку экшенов
							for(auto at = dt->second.cbegin(); at != dt->second.cend(); ++at){
								// Получаем значение списка
								string action = * at;
								// Приводим к нижнему регистру
								action = Anyks::toCase(action);
								// Определяем тип экшена
								if(action.compare("true") == 0)			actions.push_back("add");
								else if(action.compare("false") == 0)	actions.push_back("rm");
							}
							// Добавляем список экшенов
							params.actions = actions;
						}
						// Если это направление трафика
						else if(dt->first.compare("amingHeadersTraffic") == 0){
							// Создаем список направление трафика
							vector <string> traffic;
							// Переходим по списку направления трафика
							for(auto tt = dt->second.cbegin(); tt != dt->second.cend(); ++tt){
								// Получаем значение списка
								string param = * tt;
								// Приводим к нижнему регистру
								param = Anyks::toCase(param);
								// Определяем тип направления трафика
								if(param.compare("true") == 0)			traffic.push_back("in");
								else if(param.compare("false") == 0)	traffic.push_back("out");
							}
							// Добавляем список направления трафика
							params.traffic = traffic;
						}
						// Если это данные клиента
						else if((dt->first.compare("amingHeadersClientDomain") == 0)
						|| (dt->first.compare("amingHeadersClientMac") == 0)
						|| (dt->first.compare("amingHeadersClientIp4") == 0)
						|| (dt->first.compare("amingHeadersClientIp6") == 0)
						|| (dt->first.compare("amingHeadersClientNetwork4") == 0)
						|| (dt->first.compare("amingHeadersClientNetwork6") == 0)){
							// Если звездочка существует тогда очищаем объект
							if(find(params.clients.begin(), params.clients.end(), str_all) != params.clients.end()){
								// Очищаем объект клиентов
								params.clients.clear();
							}
							// Переходим по всему списку
							for(auto dom = dt->second.cbegin(); dom != dt->second.cend(); ++dom){
								// Добавляем в список клиентов
								params.clients.push_back(* dom);
							}
						}
						// Если это данные сервера
						else if((dt->first.compare("amingHeadersServerDomain") == 0)
						|| (dt->first.compare("amingHeadersServerMac") == 0)
						|| (dt->first.compare("amingHeadersServerIp4") == 0)
						|| (dt->first.compare("amingHeadersServerIp6") == 0)
						|| (dt->first.compare("amingHeadersServerNetwork4") == 0)
						|| (dt->first.compare("amingHeadersServerNetwork6") == 0)){
							// Если звездочка существует тогда очищаем объект
							if(find(params.servers.begin(), params.servers.end(), str_all) != params.servers.end()){
								// Очищаем объект серверов
								params.servers.clear();
							}
							// Переходим по всему списку
							for(auto dom = dt->second.cbegin(); dom != dt->second.cend(); ++dom){
								// Добавляем в список серверов
								params.servers.push_back(* dom);
							}
						}
					}
				}
				// Начинаем извлечение данных
				createRulesList(params);
			}
		}
	// Конфигурационные данные не найдены
	} else if(this->log) this->log->write(LOG_ERROR, 0, "headers params is not found");
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
							// Создаем параметры для создания правил
							const Params params = {
								// Получаем блок данных агента
								match[8].str(),
								// Получаем блок данных экшенов
								Anyks::split(match[1].str(), "|"),
								// Получаем блок данных направления трафика
								Anyks::split(match[2].str(), "|"),
								// Получаем блок данных клиентов
								Anyks::split(match[3].str(), "|"),
								// Получаем блок данных серверов
								Anyks::split(match[4].str(), "|"),
								// Получаем блок данных методов
								Anyks::split(match[5].str(), "|"),
								// Получаем блок данных путей
								Anyks::split(match[6].str(), "|"),
								// Получаем блок данных запросов
								Anyks::split(match[7].str(), "|"),
								// Получаем блок данных пользователей
								Anyks::split(match[9].str(), "|"),
								// Получаем блок данных групп
								Anyks::split(match[10].str(), "|"),
								// Получаем блок данных заголовков
								Anyks::split(match[11].str(), "|")
							};
							// Начинаем извлечение данных
							createRulesList(params);
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
	// Конфигурационные данные не найдены
	} else if(this->log) this->log->write(LOG_ERROR, 0, "headers params is not found");
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
 * get Метод получения правил клиента
 * @param gid     идентификатор группы
 * @param uid     идентификатор пользователя
 * @param action  экшен
 * @param traffic направление трафика
 * @param method  метод запроса
 * @return        сформированный список правил
 */
const unordered_map <string, Headers2::Rules> Headers2::get(const gid_t gid, const uid_t uid, const bool action, const bool traffic, const string method){
	// Правила вывода данных
	unordered_map <string, Rules> result;
	// Если данные клиента переданы
	if(gid && uid){
		// Проверяем существует ли такая группа
		if(this->rules.count(gid) > 0){
			// Получаем данные группы
			auto group = this->rules.find(gid)->second;
			// Проверяем существует ли данный пользователь
			if(group.count(uid) > 0){
				// Получаем список методов
				auto methods = group.find(uid)->second.find(action)->second.find(traffic)->second;
				// Приводим к нижнему регистру
				const string tmpMethod = Anyks::toCase(method);
				// Если это звездочка
				if(tmpMethod.compare("*") == 0){
					// Выводим весь список
					result = methods;
				// Добавляем правила
				} else if(methods.count(tmpMethod) > 0){
					// Добавляем в список только конкретный метод
					result.emplace(tmpMethod, methods.find(tmpMethod)->second);
				}
			}
		}
	}
	// Выводим результат
	return result;
}
/**
 * add Метод добавления новых параметров фильтрации заголовков
 * @param gid     идентификатор группы
 * @param uid     идентификатор пользователя
 * @param action  экшен
 * @param traffic направление трафика
 * @param method  метод запроса
 * @param ctx     правила работы с заголовками
 */
void Headers2::add(const gid_t gid, const uid_t uid, const bool action, const bool traffic, const string method, void * ctx){
	// Если данные клиента переданы
	if(gid && uid && ctx){
		// Получаем данные подключения
		Rules * rules = reinterpret_cast <Rules *> (ctx);
		// Проверка на существование данных
		bool exist = false;
		// Проверяем существует ли такая группа
		if(this->rules.count(gid) > 0){
			// Получаем данные группы
			auto group = this->rules.find(gid)->second;
			// Проверяем существует ли данный пользователь
			if(group.count(uid) > 0) exist = true;
		}
		// Если данные не существуют
		if(!exist){
			// Проверка на существование данных
			bool exist = true;
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
			// Приводим к нижнему регистру
			const string tmpMethod = Anyks::toCase(method);
			// Если это звездочка
			if(tmpMethod.compare("*") == 0){
				// Переходим по всему списку и добавляем правила
				for(auto it = methods.cbegin(); it != methods.cend(); ++it){
					// Добавляем в список
					methods.at(it->first) = * rules;
				}
			// Добавляем правила
			} else if(methods.count(tmpMethod) > 0) methods.at(tmpMethod) = * rules;
			// Сообщаем что методы не найдены
			else exist = false;
			// Если методы найдены
			if(exist){
				// Добавляем методы
				traffics.at(traffic) = methods;
				// Добавляем трафик
				actions.at(action) = traffics;
				// Если пользователь принадлежит группе
				if(this->ausers->checkUserInGroup(gid, uid)){
					// Добавляем экшены
					users.emplace(uid, actions);
					// Добавляем в список правил
					this->rules.emplace(gid, users);
				}
			}
		// Если данные уже существуют
		} else {
			// Приводим к нижнему регистру
			const string tmpMethod = Anyks::toCase(method);
			// Получаем список пользователей
			auto users = this->rules.find(gid)->second;
			// Получаем список методов
			auto * methods = &users.find(uid)->second.find(action)->second.find(traffic)->second;
			// Если это звездочка
			if(tmpMethod.compare("*") == 0){
				// Переходим по всему списку и добавляем правила
				for(auto it = methods->cbegin(); it != methods->cend(); ++it){
					// Добавляем в список
					methods->at(it->first) = * rules;
				}
			// Добавляем правила
			} else if(methods->count(tmpMethod) > 0) methods->at(tmpMethod) = * rules;
			// Добавляем в список правил
			this->rules.at(gid) = users;
		}
	}
}
/**
 * rm Метод удаления параметров фильтрации заголовков
 * @param gid идентификатор группы
 * @param uid идентификатор пользователя
 */
void Headers2::rm(const gid_t gid, const uid_t uid){
	// Если данные клиента переданы
	if(gid && uid && (this->rules.count(gid) > 0)){
		// Получаем данные группы
		auto group = this->rules.find(gid)->second;
		// Ищем данные пользователя
		if(group.count(uid) > 0) group.find(uid)->second.clear();
	}
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
 * @param ausers объект пользователей
 */
Headers2::Headers2(Config * config, LogApp * log, AUsers * ausers){
	// Если конфигурационные данные переданы
	if(config && ausers){
		// Очищаем все параметры
		clear();
		// Запоминаем объект логов
		this->log = log;
		// Запоминаем параметры конфига
		this->config = config;
		// Запоминаем параметры групп
		this->ausers = ausers;
		// Запоминаем название конфигурационного файла
		this->names.push_front(this->config->proxy.name);
		// Запоминаем тип поиска параметров заголовков
		this->typeSearch = 0;
		// Запоминаем время в течение которого запрещено обновлять данные
		this->maxUpdate = 600;
		// Устанавливаем параметры подключения LDAP
		this->ldap = {
			"ou=headers,ou=aming,dc=agro24,dc=dev",
			"one",
			"(objectClass=amingHeaders)"
		};
		// Выполняем чтение файла конфигурации
		read();
	}
}
