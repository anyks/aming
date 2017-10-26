/* МОДУЛЬ УПРАВЛЕНИЯ ЗАГОЛОВКАМИ AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  copyright:  © 2017 anyks.com
*/

#include "headers2/headers.h"

// Устанавливаем область видимости
using namespace std;

/**
 * copyListParams Реализация функции копирования параметров которых нет в списке
 * @param list1 объекты существуещего списка
 * @param list2 объекты которые нужно добавить
 */
struct {
	template <typename T>
	void operator()(T &list1, T list2) const {
		// Переходим по всему списку копируемых объектов
		for(auto it = list2.cbegin(); it != list2.cend(); ++it){
			// Если группа не существует
			if(find(list1.begin(), list1.end(), * it) == list1.end()){
				// Если запись не существует то добавляем его в список
				list1.push_back(* it);
			}
		}
	};
} copyListParams;
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
 * modifyHeaders Метод модификации заголовков
 * @param action  метод работы с заголовками
 * @param headers список заголовков
 * @param http    блок с http данными
 */
void Headers2::modifyHeaders(const bool action, const vector <string> headers, HttpData &http){
	// Если данные существуют
	if(!headers.empty()){
		// Переходим по всему списку заголовков
		for(auto it = headers.cbegin(); it != headers.cend(); ++it){
			// Результат работы регулярного выражения
			smatch match;
			// Если нужно добавить заголовоки
			if(action){
				// Устанавливаем правило регулярного выражения
				regex e("^([\\w\\-]+)\\s*\\:\\s*([^\\r\\n\\t\\s]+)$", regex::ECMAScript | regex::icase);
				// Выполняем проверку
				regex_search(* it, match, e);
				// Если данные найдены
				if(!match.empty()) http.setHeader(match[1].str(), match[2].str());
			// Если нужно удалить заголовки
			} else {
				// Устанавливаем правило регулярного выражения
				regex e("^([\\w\\-]+)\\s*\\:?", regex::ECMAScript | regex::icase);
				// Выполняем проверку
				regex_search(* it, match, e);
				// Если данные найдены
				if(!match.empty()) http.rmHeader(match[1].str());
			}
		}
	}
}
/**
 * modifyHeaders Метод модификации заголовков
 * @param action  метод работы с заголовками
 * @param headers список заголовков
 * @param data    строка с данными запроса или ответа
 * @param http    блок с http данными
 */
void Headers2::modifyHeaders(const bool action, const vector <string> headers, string &data, HttpData &http){
	// Если данные существуют
	if(!headers.empty() && !data.empty()){
		// Переходим по всему списку заголовков
		for(auto it = headers.cbegin(); it != headers.cend(); ++it){
			// Результат работы регулярного выражения
			smatch match;
			// Если нужно добавить заголовоки
			if(action){
				// Устанавливаем правило регулярного выражения
				regex e("^([\\w\\-]+)\\s*\\:\\s*([^\\r\\n\\t\\s]+)$", regex::ECMAScript | regex::icase);
				// Выполняем проверку
				regex_search(* it, match, e);
				// Если данные найдены
				if(!match.empty()) http.addHeaderToString(match[1].str(), match[2].str(), data);
			// Если нужно удалить заголовки
			} else {
				// Устанавливаем правило регулярного выражения
				regex e("^([\\w\\-]+)\\s*\\:?", regex::ECMAScript | regex::icase);
				// Выполняем проверку
				regex_search(* it, match, e);
				// Если данные найдены
				if(!match.empty()) http.rmHeaderInString(match[1].str(), data);
			}
		}
	}
}
/**
 * addParams Метод добавления новых параметров в список правил
 * @param params параметры для добавления
 */
void Headers2::addParams(const map <bool, map <bool, unordered_map <string, Headers2::Rules>>> * params){
	// Если входящие параметры верные
	if((params != nullptr) && !params->empty()){
		/**
		 * copyListNodes Функция копирования нод которых нет в списке
		 * @param node1 ноды существуещего списка
		 * @param node2 ноды которые нужно добавить
		 */
		auto copyListNodes = [](vector <Node> &node1, vector <Node> node2){
			// Переходим по всему объекту нод 1
			for(auto it1 = node2.cbegin(); it1 != node2.cend(); ++it1){
				// Результат поиска
				bool result = false;
				// Переходим по всему объекту нод 2
				for(auto it2 = node1.cbegin(); it2 != node1.cend(); ++it2){
					// Выполняем проверку на совпадение данных
					if((it1->type != it2->type)
					|| (it1->data.compare(it2->data) != 0)) result = true;
				}
				// Если параметры не найдены в списке
				if(!result) node1.push_back(* it1);
			}
		};
		// Переходим по всему списку пользователей
		for(auto it = params->cbegin(); it != params->cend(); ++it){
			// Получаем тип экшена
			const bool action = it->first;
			// Получаем список трафика
			const auto * traffics = &it->second;
			// Если такой экшен существует
			if(this->rules.count(action) > 0){
				// Запоминаем данные трафика
				auto * realTraffics = &this->rules.find(action)->second;
				// Если трафик существуют
				if(!traffics->empty()){
					// Переходим по всему списку трафика
					for(auto it = traffics->cbegin(); it != traffics->cend(); ++it){
						// Получаем тип трафика
						const bool traffic = it->first;
						// Получаем список методов
						const auto * methods = &it->second;
						// Если такой трафик существует
						if(realTraffics->count(traffic) > 0){
							// Запоминаем данные методов
							auto * realMethods = &realTraffics->find(traffic)->second;
							// Если методы существуют
							if(!methods->empty()){
								// Переходим по всему списку методов
								for(auto it = methods->cbegin(); it != methods->cend(); ++it){
									// Получаем название метода
									const string method = it->first;
									// Получаем список правил
									const auto * rules = &it->second;
									// Если такой метод существует
									if(realMethods->count(method) > 0){
										// Запоминаем данные правил
										auto * realRules = &realMethods->find(method)->second;
										// Если правила существуют
										if(rules != nullptr){
											// Заменяем данные запросов
											realRules->query = rules->query;
											// Заменяем данные userAgent
											realRules->userAgent = rules->userAgent;
											// Выполняем копирование нод клиента
											copyListNodes(realRules->clients, rules->clients);
											// Выполняем копирование нод сервера
											copyListNodes(realRules->servers, rules->servers);
											// Выполняем копирование групп пользователей
											copyListParams(realRules->groups, rules->groups);
											// Выполняем копирование пользователей
											copyListParams(realRules->users, rules->users);
											// Выполняем копирование путей
											copyListParams(realRules->paths, rules->paths);
											// Выполняем копирование заголовков
											copyListParams(realRules->headers, rules->headers);
										}
									// Если такой метод не существует
									} else realMethods->emplace(method, * rules);
								}
							}
						// Если такой трафик не существует
						} else realTraffics->emplace(traffic, * methods);
					}
				}
			// Если такой экшен не существует
			} else this->rules.emplace(action, * traffics);
		}
	}
}
/**
 * createRulesList Метод созданий списка правил
 * @param params список параметров
 */
void Headers2::createRulesList(const Headers2::Params params){
	// Получаем список всех групп
	auto dataGroups = this->ausers->getAllGroups();
	// Получаем список всех пользователей
	auto dataUsers = this->ausers->getAllUsers();
	// Если группы и пользователи существуют
	if(!dataGroups.empty() && !dataUsers.empty()){
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
					node.type = Anyks::getTypeAmingByString(str);
					// Если тип данных определен
					if(node.type != AMING_NULL){
						// Запоминаем сами данные
						node.data = str;
						// Добавляем в список нод
						nodes.push_back(node);
					}
				// Если разрешены любые протоколы
				} else {
					// Устанавливаем тип ноды
					node.type = AMING_NULL;
					// Разрешены любые протоколы
					node.data = str;
					// Добавляем в список нод
					nodes.push_back(node);
				}
			}
			// Выводим результат
			return nodes;
		};
		// Список групп пользователей
		vector <gid_t> groups;
		// Список пользователей
		vector <uid_t> users;
		// Переходим по всему массиву групп
		for(auto it = params.groups.cbegin(); it != params.groups.cend(); ++it){
			// Определяем группу
			string group = * it;
			// Если группа не является звездочкой
			if(group.compare("*") != 0){
				// Выполняем проверку на инверсию
				auto check = isNot(group);
				// Если это не инверсия
				if(!check.inv){
					// Идентификатор группы
					gid_t gid = -1;
					// Если это идентификатор группы
					if(Anyks::isNumber(check.str)) gid = ::atoi(check.str.c_str());
					// Если это название группы
					else gid = this->ausers->getGidByName(check.str);
					// Выполняем добавление группы
					if(this->ausers->checkGroupById(gid)) groups.push_back(gid);
				// Если это инверсия
				} else {
					// Проверяем является ли это идентификатором
					const bool numberFlag = Anyks::isNumber(check.str);
					// Переходим по всему списку групп
					for(auto it = dataGroups.cbegin(); it != dataGroups.cend(); ++it){
						// Выполняем добавление группы
						if((numberFlag && ((* it)->id != gid_t(::atoi(check.str.c_str()))))
						|| (!numberFlag && (this->ausers->getGroupNameByGid((* it)->id).compare(check.str) != 0))) groups.push_back((* it)->id);
					}
					// Выходим из цикла
					break;
				}
			// Если найдена звездочка то добавляем во все группы
			} else {
				// Переходим по всему списку групп
				for(auto it = dataGroups.cbegin(); it != dataGroups.cend(); ++it){
					// Выполняем добавление группы
					groups.push_back((* it)->id);
				}
				// Выходим из цикла
				break;
			}
		}
		// Переходим по всему списку пользователей
		for(auto it = params.users.cbegin(); it != params.users.cend(); ++it){
			// Определяем пользователя
			string user = * it;
			// Если пользователь не является звездочкой
			if(user.compare("*") != 0){
				// Выполняем проверку на инверсию
				auto check = isNot(user);
				// Если это не инверсия
				if(!check.inv){
					// Идентификатор пользователя
					uid_t uid = -1;
					// Определяем идентификатор пользователя
					if(Anyks::isNumber(check.str)) uid = ::atoi(check.str.c_str());
					// Если это название пользователя
					else uid = this->ausers->getUidByName(check.str);
					// Выполняем добавление пользователя
					if(this->ausers->checkUserById(uid)) users.push_back(uid);
				// Если это инверсия
				} else {
					// Проверяем является ли это идентификатором
					const bool numberFlag = Anyks::isNumber(check.str);
					// Переходим по всему списку пользователей
					for(auto it = dataUsers.cbegin(); it != dataUsers.cend(); ++it){
						// Выполняем добавление пользователя
						if((numberFlag && ((* it)->id != uid_t(::atoi(check.str.c_str()))))
						|| (!numberFlag && (this->ausers->getUserNameByUid((* it)->id).compare(check.str) != 0))) users.push_back((* it)->id);
					}
					// Выходим из цикла
					break;
				}
			// Если найдена звездочка то добавляем во все экшены
			} else {
				// Переходим по всему списку групп
				for(auto it = dataUsers.cbegin(); it != dataUsers.cend(); ++it){
					// Выполняем добавление группы
					users.push_back((* it)->id);
				}
				// Выходим из цикла
				break;
			}
		}
		// Создаем объект с правилами
		const Rules rules = {
			// Список запросов
			params.query,
			// Запоминаем данные агента
			params.userAgent,
			// Список групп и пользователей
			users, groups,
			// Список клиентов
			createNode(params.clients),
			// Список серверов
			createNode(params.servers),
			// Список путей
			params.paths,
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
		// Переходим по всему списку методов
		for(auto it = params.methods.cbegin(); it != params.methods.cend(); ++it){
			// Определяем метод
			string method = * it;
			// Приводим к нижнему регистру
			method = Anyks::toCase(method);
			// Если метод не является звездочкой
			if(method.compare("*") != 0){
				// Выполняем проверку на инверсию
				auto check = isNot(method);
				// Получаем нужный нам метод и добавляем туда правила
				if(!check.inv) methods.at(check.str) = rules;
				// Если это инверсия
				else {
					// Переходим по всему списку методов
					for(auto mt = methods.cbegin(); mt != methods.cend(); ++mt){
						// Добавляем правила
						if(mt->first.compare(check.str) != 0) methods.at(mt->first) = rules;
					}
					// Выходим из цикла
					break;
				}
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
				// Выполняем проверку на инверсию
				auto check = isNot(traffic);
				// Если это не инверсия
				if(!check.inv){
					// Если это входящий трафик
					if(check.str.compare("in") == 0) traffics.at(true) = methods;
					// Если это исходящий трафик
					if(check.str.compare("out") == 0) traffics.at(false) = methods;
				// Если это инверсия
				} else {
					// Если это входящий трафик
					if(check.str.compare("in") == 0) traffics.at(false) = methods;
					// Если это исходящий трафик
					if(check.str.compare("out") == 0) traffics.at(true) = methods;
				}
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
				// Выполняем проверку на инверсию
				auto check = isNot(action);
				// Если это не инверсия
				if(!check.inv){
					// Если это добавление заголовков
					if(check.str.compare("add") == 0) actions.at(true) = traffics;
					// Если это удаление заголовков
					if(check.str.compare("rm") == 0) actions.at(false) = traffics;
				// Если это инверсия
				} else {
					// Если это добавление заголовков
					if(check.str.compare("add") == 0) actions.at(false) = traffics;
					// Если это удаление заголовков
					if(check.str.compare("rm") == 0) actions.at(true) = traffics;
				}
			// Если найдена звездочка то добавляем во все экшены
			} else {
				// Запоминаем параметры для всех видов экшенов
				actions.at(true)	= traffics;
				actions.at(false)	= traffics;
				// Выходим из цикла
				break;
			}
		}
		// Добавляем в список правил
		addParams(&actions);
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
					str_all,
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
						// Если это список заголовков
						else if(dt->first.compare("amingHeadersHeaders") == 0) params.headers = dt->second;
						// Если это список параметров запросов
						else if(dt->first.compare("amingHeadersQuery") == 0) params.query = dt->second[0];
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
							"([^\\s\\r\\n\\t]+|\\*)(?:\\s+|\\t+)"
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
								// Получаем блок данных запросов
								match[7].str(),
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
 * findRules Метод поиска заголовков
 * @param request запрос на получение данных
 * @param method  метод запроса
 * @param rules   список правил
 * @return        сформированный список заголовков
 */
const vector <string> Headers2::findHeaders(Headers2::Client * request, const string method, const Rules * rules){
	// Список правил
	vector <string> result;
	// Если входящие параметры верные
	if((request != nullptr) && (rules != nullptr)){
		/**
		 * checkData Функция сравнения данных
		 * @param data  данные для сравнения
		 * @param nodes список нод для сравнения
		 */
		auto checkData = [this](const string data, const vector <Node> * nodes){
			// Результат проверки
			bool result = false;
			// Если список не пустой
			if(!data.empty() && !nodes->empty()){
				// Определяем входящие данные
				const string tmpData = Anyks::toCase(data);
				// Определяем тип ip адреса
				const u_short type = Anyks::getTypeAmingByString(tmpData);
				// Переходим по всем спискам клиентов
				for(auto it = nodes->cbegin(); it != nodes->cend(); ++it){
					// Если не разрешены любые протоколы
					if(it->data.compare("*") != 0){
						// Если типы входящих данных клиента эквивалентны
						if(type == it->type){
							// Выполняем проверку на инверсию
							auto check = isNot(Anyks::toCase(it->data));
							// Определяем тип данных
							switch(type){
								// Если это IPv4
								case AMING_IPV4: {
									// Если это не инверсия
									if(!check.inv){
										// Выполняем проверку на совпадение ip адресов
										result = (check.str.compare(tmpData) == 0);
										// Выходим из цикла
										if(result) goto stop;
									// Если это инверсия
									} else {
										// Выполняем проверку на не совпадение ip адресов
										result = (check.str.compare(tmpData) != 0);
										// Выходим из цикла
										if(!result) goto stop;
									}
								} break;
								// Если это IPv6
								case AMING_IPV6: {
									// Создаем объект сети
									Network nwk;
									// Если это не инверсия
									if(!check.inv){
										// Выполняем проверку на совпадение ip адресов
										result = nwk.compareIP6(check.str, tmpData);
										// Выходим из цикла
										if(result) goto stop;
									// Если это инверсия
									} else {
										// Выполняем проверку на не совпадение ip адресов
										result = !nwk.compareIP6(check.str, tmpData);
										// Выходим из цикла
										if(!result) goto stop;
									}
								} break;
								// Если это MAC адрес
								case AMING_MAC: {
									// Если это не инверсия
									if(!check.inv){
										// Проверяем на совпадение mac адресов
										result = (check.str.compare(tmpData) == 0);
										// Выходим из цикла
										if(result) goto stop;
									// Если это инверсия
									} else {
										// Проверяем на не совпадение mac адресов
										result = (check.str.compare(tmpData) != 0);
										// Выходим из цикла
										if(!result) goto stop;
									}
								} break;
								// Если это домен
								case AMING_DOMAIN: {
									// Если это не инверсия
									if(!check.inv){
										// Проверяем на совпадение доменов
										result = Anyks::checkDomainByMask(tmpData, check.str);
										// Выходим из цикла
										if(result) goto stop;
									// Если это инверсия
									} else {
										// Проверяем на не совпадение mac адресов
										result = !Anyks::checkDomainByMask(tmpData, check.str);
										// Выходим из цикла
										if(!result) goto stop;
									}
								} break;
							}
						// Если нужно проверить принадлежит ли ip адрес указанной сети
						} else if((it->type == AMING_NETWORK) && ((type == AMING_IPV4) || (type == AMING_IPV6))){
							// Создаем объект сети
							Network nwk;
							// Выполняем проверку на инверсию
							auto check = isNot(Anyks::toCase(it->data));
							// Получаем ip адрес сети
							const string ip = nwk.getIPByNetwork(check.str);
							// Если типы протоколов совпадают
							if(type == Anyks::getTypeAmingByString(ip)){
								// Если это не инверсия
								if(!check.inv){
									// Проверяем на совпадение mac адресов
									result = (type == AMING_IPV4 ? nwk.checkIPByNetwork(tmpData, check.str) : nwk.checkIPByNetwork6(tmpData, check.str));
									// Выходим из цикла
									if(result) goto stop;
								// Если это инверсия
								} else {
									// Проверяем на совпадение mac адресов
									result = (type == AMING_IPV4 ? !nwk.checkIPByNetwork(tmpData, check.str) : !nwk.checkIPByNetwork6(tmpData, check.str));
									// Выходим из цикла
									if(!result) goto stop;
								}
							}
						}
					// Если разрешены любые протоколы
					} else {
						// Запоминаем что все проверки пройдены
						result = true;
						// Выходим из цикла
						break;
					}
				}
			}
			// Устанавливаем метку
			stop:
			// Выводим результат
			return result;
		};
		/**
		 * checkPath Функция сравнения путей
		 * @param data путь для сравнения
		 * @param list список путей для сравнения
		 */
		auto checkPath = [this](const string data, const vector <string> * list){
			// Результат проверки
			bool result = false;
			// Если список не пустой
			if(!data.empty() && !list->empty()){
				// Переводим путь в нижний регистр
				const string tmp = Anyks::toCase(data);
				// Переходим по всему списку путей
				for(auto it = list->cbegin(); it != list->cend(); ++it){
					// Если это не для любых данных
					if(it->compare("*") != 0){
						// Выполняем проверку на инверсию
						auto check = isNot(Anyks::toCase(* it));
						// Получаем первую строку адреса
						const string path = Anyks::getPathByString(check.str);
						// Если это не инверсия
						if(!check.inv){
							// Запоминаем что все проверки пройдены
							result = (path.compare(tmp) == 0);
							// Выходим из цикла
							if(result) break;
						// Если это инверсия
						} else {
							// Если пути не совпадают
							result = (path.compare(tmp) != 0);
							// Выходим из цикла
							if(!result) break;
						}
					// Если это для любых данных
					} else {
						// Запоминаем что все проверки пройдены
						result = true;
						// Выходим из цикла
						break;
					}
				}
			}
			// Выводим результат
			return result;
		};
		/**
		 * checkRegexp Функция проверки данных по регулярному выражению
		 * @param data данные для проверки
		 * @param reg  регулярное выражение
		 */
		auto checkRegexp = [](const string data, const string reg){
			// Результат проверки
			bool result = false;
			// Если это не любые параметры
			if(!reg.empty() && (reg.compare("*") != 0)){
				// Устанавливаем правило регулярного выражения
				regex e(reg, regex::ECMAScript | regex::icase);
				// Запоминаем результат
				result = regex_match(data, e);
			// Сообщаем что проверка на параметры запроса пройдена
			} else result = true;
			// Выводим результат
			return result;
		};
		/**
		 * checkGroups Функция проверки данных групп пользователя
		 * @param list1 список групп для проверки
		 * @param list2 список групп у правил заголовков
		 */
		auto checkGroups = [](const vector <gid_t> * list1, const vector <gid_t> * list2){
			// Результат проверки
			bool result = false;
			// Если список групп для заголовков не существует
			if(list2->empty()) result = true;
			// Если же список групп существует
			else {
				// Переходим по всему списку переданных групп
				for(auto it = list1->cbegin(); it != list1->cend(); ++it){
					// Если группа найдена в списке тогда останавливаем поиск
					if(find(list2->begin(), list2->end(), * it) != list2->end()){
						// Запоминаем что группа найдена
						result = true;
						// Выходим из цикла
						break;
					}
				}
			}
			// Выводим результат
			return result;
		};
		/**
		 * checkUsers Функция проверки данных пользователя
		 * @param uid  идентификатор пользователя
		 * @param list список пользователей у правил заголовков
		 */
		auto checkUsers = [](const uid_t uid, const vector <uid_t> * list){
			// Результат проверки
			bool result = false;
			// Если список пользователей для заголовков не существует
			if(list->empty()) result = true;
			// Если же список пользователей существует
			else result = (find(list->begin(), list->end(), uid) != list->end());
			// Выводим результат
			return result;
		};
		// Проверяем ip адрес клиента
		const bool checkCIP = checkData(request->ip, &rules->clients);
		// Проверяем mac адрес клиента
		const bool checkCMac = (!checkCIP ? checkData(request->mac, &rules->clients) : checkCIP);
		// Проверяем  ip адрес сервера
		const bool checkSIP = checkData(request->sip, &rules->servers);
		// Проверяем доменное имя сервера
		const bool checkSDM = (!checkSIP ? checkData(request->domain, &rules->servers) : checkSIP);
		// Выполняем проверку путей
		const bool pathFound = checkPath(request->path, &rules->paths);
		// Проверяем на корректность запроса к серверу
		const bool queryFound = checkRegexp(request->query, rules->query);
		// Проверяем на корректность UserAgent
		const bool userAgentFound = checkRegexp(request->agent, rules->userAgent);
		// Проверяем корректность групп
		const bool groupFound = checkGroups(&request->groups, &rules->groups);
		// Проверяем корректность пользователя
		const bool userFound = checkUsers(request->user, &rules->users);
		// Определяем найден ли клиент
		const bool clientFound = (checkCIP || checkCMac);
		// Определяем найден ли сервер
		const bool serverFound = (checkSIP || checkSDM);
		// Если и сервер и клиент найдены тогда добавляем в список правила
		if(clientFound && serverFound && queryFound && userAgentFound && pathFound && groupFound && userFound){
			// Переходим по всему списку заголовков
			for(auto it = rules->headers.cbegin(); it != rules->headers.cend(); ++it){
				// Выполняем проверку на существование заголовка в списке
				const string str = * find(result.begin(), result.end(), * it);
				// Если заголовок не существует то добавляем его в список
				if(str.empty()) result.push_back(* it);
			}
		}
	}
	// Выводим результат
	return result;
}
/**
 * get Метод получения правил клиента
 * @param request запрос на получение данных
 * @return        сформированный список правил
 */
const vector <string> Headers2::get(Headers2::Client * request){
	// Правила вывода данных
	vector <string> result;
	// Если данные клиента переданы
	if((request != nullptr) && !request->ip.empty() && !request->mac.empty() && !request->sip.empty() && !this->rules.empty()){
		// Проверяем существует ли такой экшен
		if(this->rules.count(request->action) > 0){
			// Получаем список методов
			auto methods = this->rules.find(request->action)->second.find(request->traffic)->second;
			// Приводим к нижнему регистру
			const string tmpMethod = Anyks::toCase(request->method);
			// Если это звездочка
			if(tmpMethod.compare("*") == 0){
				// Переходим по всем методам запросов
				for(auto it = methods.cbegin(); it != methods.cend(); ++it){
					// Выполняем запрос для каждого метода
					result = findHeaders(request, it->first, &it->second);
				}
			// Добавляем правила
			} else if(methods.count(tmpMethod) > 0){
				// Извлекаем правило
				auto rules = methods.find(tmpMethod)->second;
				// Добавляем правила для конкретного метода
				result = findHeaders(request, tmpMethod, &rules);
			}
		}
	}
	// Выводим результат
	return result;
}
/**
 * add Метод добавления новых параметров фильтрации заголовков
 * @param action  экшен
 * @param traffic направление трафика
 * @param method  метод запроса
 * @param ctx     правила работы с заголовками
 */
void Headers2::add(const bool action, const bool traffic, const string method, void * ctx){
	// Если данные клиента переданы
	if(ctx != nullptr){
		// Получаем данные подключения
		Rules * rules = reinterpret_cast <Rules *> (ctx);
		// Проверка на существование данных
		bool exist = false;
		// Проверяем существует ли такой экшен
		if(this->rules.count(action) > 0) exist = true;
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
				this->rules.emplace(action, traffics);
			}
		// Если данные уже существуют
		} else {
			// Приводим к нижнему регистру
			const string tmpMethod = Anyks::toCase(method);
			// Получаем список методов
			auto * methods = &this->rules.find(action)->second.find(traffic)->second;
			// Если это звездочка
			if(tmpMethod.compare("*") == 0){
				// Переходим по всему списку и добавляем правила
				for(auto it = methods->cbegin(); it != methods->cend(); ++it){
					// Добавляем в список
					methods->at(it->first) = * rules;
				}
			// Добавляем правила
			} else if(methods->count(tmpMethod) > 0) methods->at(tmpMethod) = * rules;
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
 * setOptions Метод установки новых параметров для парсинга http данных
 * @param options параметры для парсинга http данных
 */
void Headers2::setOptions(const u_short options){
	// Запоминаем основные параметры прокси
	if(options != 0x00) this->options = options;
}
/**
 * modify Метод модификации заголовков
 * @param client данные клиента
 * @param http   блок с данными запроса или ответа
 */
void Headers2::modify(AParams::Client client, HttpData &http){
	// Если правило для клиента найдено
	if(!client.ip.empty() && !client.mac.empty() && !client.sip.empty()){
		// Определяем метод запроса
		const string method = http.getMethod();
		// Получаем UserAgent
		const string agent = http.getUseragent();
		// Получаем хост сервера
		const string host = http.getHost();
		// Получаем путь запроса
		string path = Anyks::toCase(http.getPath());
		// Запоминаем параметры запроса
		string query = path;
		// Приводим путь к нормальному виду
		path = Anyks::getPathByString(path);
		// Приводим параметры запроса к нормальному виду
		query = Anyks::getQueryByString(query);
		// Определяем направление трафика
		const bool traffic = (http.getStatus() != 0);
		// Экшен добавления заголовков
		const bool actionAdd = true;
		// Экшен удаления заголовков
		const bool actionRm = false;
		// Объект клиента
		Client requestAdd	= {actionAdd, traffic, static_cast <uid_t> (-1), {}, client.ip, client.mac, client.sip, host, agent, path, query, method};
		Client requestRm	= {actionRm, traffic, static_cast <uid_t> (-1), {}, client.ip, client.mac, client.sip, host, agent, path, query, method};
		/**
		 * modifyHeadersForUser Функция модификации заголовков по пользовательским данным
		 * @param client указатель на блок с данными пользователя
		 * @param http   объект http данных запроса
		 */
		auto modifyHeadersForUser = [&requestAdd, &requestRm, &actionAdd, &actionRm, this](const AParams::AUser * client, HttpData &http){
			// Если клиент существует
			if(client != nullptr){
				// Запоминаем данные пользователя
				requestAdd.user	= client->user.uid;
				requestRm.user	= client->user.uid;
				// Переходим по всему списку групп
				for(auto it = client->groups.cbegin(); it != client->groups.cend(); ++it){
					// Запоминаем данные групп
					requestAdd.groups.push_back(it->gid);
					requestRm.groups.push_back(it->gid);
				}
			}
			// Запрашиваем список правил
			auto headersAdd = get(&requestAdd);
			auto headersRm = get(&requestRm);
			// Добавляем заголовки
			modifyHeaders(actionAdd, headersAdd, http);
			// Удаляем заголовки
			modifyHeaders(actionRm, headersRm, http);
		};
		// Если данные пользователя существуют
		if(client.user != nullptr){
			// Выполняем модификацию заголовков для пользователя
			modifyHeadersForUser(client.user, http);
		// Если пользователь не установлен
		} else {
			// Пытаемся найти по ip и mac адресу
			auto user = this->ausers->searchUser(client.ip, client.mac);
			// Выполняем модификацию заголовков для пользователя
			if(user.auth) modifyHeadersForUser(&user, http);
			// Если пользователь не найден тогда запрашиваем общие данные для всех пользователей
			else modifyHeadersForUser(nullptr, http);
		}
	}
}
/**
 * modify Метод модификации заголовков
 * @param client данные клиента
 * @param data   строка с данными запроса или ответа
 */
void Headers2::modify(AParams::Client client, string &data){
	// Если правило для клиента найдено
	if(!client.ip.empty() && !client.mac.empty() && !client.sip.empty()){
		// Создаем http объект
		HttpData http;
		// Получаем опции прокси сервера
		const u_short options = ((client.user != nullptr) && (this->ausers != nullptr) ? this->ausers->getOptionsByUid(client.user->user.uid) : this->options);
		// Выполняем обработку данных
		if(http.parse(data.c_str(), data.size(), this->config->proxy.name, options)){
			// Определяем метод запроса
			const string method = http.getMethod();
			// Получаем UserAgent
			const string agent = http.getUseragent();
			// Получаем хост сервера
			const string host = http.getHost();
			// Получаем путь запроса
			string path = Anyks::toCase(http.getPath());
			// Запоминаем параметры запроса
			string query = path;
			// Приводим путь к нормальному виду
			path = Anyks::getPathByString(path);
			// Приводим параметры запроса к нормальному виду
			query = Anyks::getQueryByString(query);
			// Определяем направление трафика
			const bool traffic = (http.getStatus() != 0);
			// Экшен добавления заголовков
			const bool actionAdd = true;
			// Экшен удаления заголовков
			const bool actionRm = false;
			// Объект клиента
			Client requestAdd	= {actionAdd, traffic, static_cast <uid_t> (-1), {}, client.ip, client.mac, client.sip, host, agent, path, query, method};
			Client requestRm	= {actionRm, traffic, static_cast <uid_t> (-1), {}, client.ip, client.mac, client.sip, host, agent, path, query, method};
			/**
			 * modifyHeadersForUser Функция модификации заголовков по пользовательским данным
			 * @param client указатель на блок с данными пользователя
			 * @param http   объект http данных запроса
			 */
			auto modifyHeadersForUser = [&requestAdd, &requestRm, &actionAdd, &actionRm, &data, this](const AParams::AUser * client, HttpData &http){
				// Если клиент существует
				if(client != nullptr){
					// Запоминаем данные пользователя
					requestAdd.user	= client->user.uid;
					requestRm.user	= client->user.uid;
					// Переходим по всему списку групп
					for(auto it = client->groups.cbegin(); it != client->groups.cend(); ++it){
						// Запоминаем данные групп
						requestAdd.groups.push_back(it->gid);
						requestRm.groups.push_back(it->gid);
					}
				}
				// Запрашиваем список правил
				auto headersAdd = get(&requestAdd);
				auto headersRm = get(&requestRm);
				// Добавляем заголовки
				modifyHeaders(actionAdd, headersAdd, data, http);
				// Удаляем заголовки
				modifyHeaders(actionRm, headersRm, data, http);
				// Выполняем модификацию основных заголовков
				data = http.modifyHeaderString(data);
			};
			// Если данные пользователя существуют
			if(client.user != nullptr){
				// Выполняем модификацию заголовков для пользователя
				modifyHeadersForUser(client.user, http);
			// Если пользователь не установлен
			} else {
				// Пытаемся найти по ip и mac адресу
				auto user = this->ausers->searchUser(client.ip, client.mac);
				// Выполняем модификацию заголовков для пользователя
				if(user.auth) modifyHeadersForUser(&user, http);
				// Если пользователь не найден тогда запрашиваем общие данные для всех пользователей
				else modifyHeadersForUser(nullptr, http);
			}
		}
	}
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
		// Запоминаем основные параметры прокси
		this->options = this->config->options;
		// Запоминаем параметры групп
		this->ausers = ausers;
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
		// Запоминаем название конфигурационного файла
		if(this->typeSearch == 0) this->names.push_front(this->config->proxy.name);
		// Выполняем чтение файла конфигурации
		read();
	}
}
