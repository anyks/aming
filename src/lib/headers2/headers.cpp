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
 * @param gid    идентификатор группы
 * @param params параметры для добавления
 */
void Headers2::addParams(const gid_t gid, const map <uid_t, map <bool, map <bool, unordered_map <string, Headers2::Rules>>>> * params){
	// Если входящие параметры верные
	if((gid > 0) && (params != nullptr) && !params->empty()){
		// Проверяем существует ли такая группа
		if(this->rules.count(gid) > 0){
			// Получаем данные пользователей
			auto * users = &this->rules.find(gid)->second;
			// Если данные пользователя получены
			if(!users->empty()){
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
				/**
				 * copyListParams Функция копирования параметров которых нет в списке
				 * @param list1 объекты существуещего списка
				 * @param list2 объекты которые нужно добавить
				 */
				auto copyListParams = [](vector <string> &list1, vector <string> list2){
					// Переходим по всему списку копируемых объектов
					for(auto it = list2.cbegin(); it != list2.cend(); ++it){
						// Выполняем проверку на существование записи в списке
						const string str = * find(list1.begin(), list1.end(), * it);
						// Если запись не существует то добавляем его в список
						if(str.empty()) list1.push_back(* it);
					}
				};
				// Переходим по всему списку пользователей
				for(auto it = params->cbegin(); it != params->cend(); ++it){
					// Получаем идентификатор пользователя
					const uid_t uid = it->first;
					// Получаем список экшенов
					const auto * actions = &it->second;
					// Если экшен существует в списке
					if(users->count(uid) > 0){
						// Запоминаем экшены существующие
						auto * realActions = &users->find(uid)->second;
						// Если экшены существуют
						if(!actions->empty()){
							// Переходим по всему списку экшенов
							for(auto it = actions->cbegin(); it != actions->cend(); ++it){
								// Получаем тип экшена
								const bool action = it->first;
								// Получаем список трафика
								const auto * traffics = &it->second;
								// Если такой экшен существует
								if(realActions->count(action) > 0){
									// Запоминаем данные трафика
									auto * realTraffics = &realActions->find(action)->second;
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
								} else realActions->emplace(action, * traffics);
							}
						}
					// Добавляем список экшенов к пользователю
					} else users->emplace(uid, * actions);
				}
				// Обновляем данные пользователей
				this->rules.at(gid) = * users;
			// Если пользователи не найдены, то просто добавляем данные
			} else this->rules.at(gid) = * params;
		// Иначе просто добавляем данные
		} else this->rules.emplace(gid, * params);
	}
}
/**
 * createRulesList Метод созданий списка правил
 * @param params список параметров
 */
void Headers2::createRulesList(const Headers2::Params params){
	// Получаем список всех групп
	auto dataGroups = this->ausers->getAllGroups();
	// Если группы существуют
	if(!dataGroups.empty()){
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
		// Создаем объект с правилами
		const Rules rules = {
			// Список запросов
			params.query,
			// Запоминаем данные агента
			params.userAgent,
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
						// Если пользователь принадлежит группе
						if(this->ausers->checkUserInGroup(gid, uid)){
							// Добавляем список экшенов к пользователю
							users.emplace(uid, actions);
						}
					// Если это инверсия
					} else {
						// Очищаем список пользователей
						users.clear();
						// Запрашиваем список всех пользователей группы
						auto uids = this->ausers->getIdUsersInGroup(gid);
						// Проверяем является ли это идентификатором
						const bool numberFlag = Anyks::isNumber(check.str);
						// Переходим по всем идентификаторам пользователей и добавляем туда экшены
						for(auto it = uids.cbegin(); it != uids.cend(); ++it){
							// Выполняем создание правила
							if((numberFlag && ((* it) != uid_t(::atoi(check.str.c_str()))))
							|| (!numberFlag && (this->ausers->getUserNameByUid(* it).compare(check.str) != 0))) users.emplace(* it, actions);
						}
						// Выходим из цикла
						break;
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
			addParams(gid, &users);
		};
		// Переходим по всему массиву групп
		for(auto it = params.groups.cbegin(); it != params.groups.cend(); ++it){
			// Определяем группу
			string group = * it;
			// Очищаем список пользователей
			users.clear();
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
					// Выполняем создание правила
					if(this->ausers->checkGroupById(gid)) createRules(gid);
				// Если это инверсия
				} else {
					// Проверяем является ли это идентификатором
					const bool numberFlag = Anyks::isNumber(check.str);
					// Переходим по всему списку групп
					for(auto it = dataGroups.cbegin(); it != dataGroups.cend(); ++it){
						// Выполняем создание правила
						if((numberFlag && ((* it)->id != gid_t(::atoi(check.str.c_str()))))
						|| (!numberFlag && (this->ausers->getGroupNameByGid((* it)->id).compare(check.str) != 0))) createRules((* it)->id);
					}
					// Выходим из цикла
					break;
				}
			// Если найдена звездочка то добавляем во все группы
			} else {
				// Переходим по всему списку групп
				for(auto it = dataGroups.cbegin(); it != dataGroups.cend(); ++it){
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
 * @param ip     IP адрес пользователя
 * @param mac    MAC адрес пользователя
 * @param sip    IP адрес сервера
 * @param sdm    доменное имя сервера
 * @param uat    юзер-агент браузера
 * @param path   путь запроса 
 * @param query  параметры запроса
 * @param method метод запроса
 * @param rules  список правил
 * @return       сформированный список заголовков
 */
const vector <string> Headers2::findHeaders(const string ip, const string mac, const string sip, const string sdm, const string uat, const string path, const string query, const string method, const Rules * rules){
	// Список правил
	vector <string> result;
	// Если входящие параметры верные
	if(!ip.empty() && !mac.empty() && !sip.empty() && !sdm.empty() && (rules != nullptr)){
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
		// Проверяем ip адрес клиента
		const bool checkCIP = checkData(ip, &rules->clients);
		// Проверяем mac адрес клиента
		const bool checkCMac = (!checkCIP ? checkData(mac, &rules->clients) : checkCIP);
		// Проверяем  ip адрес сервера
		const bool checkSIP = checkData(sip, &rules->servers);
		// Проверяем доменное имя сервера
		const bool checkSDM = (!checkSIP ? checkData(sdm, &rules->servers) : checkSIP);
		// Выполняем проверку путей
		const bool pathFound = checkPath(path, &rules->paths);
		// Проверяем на корректность запроса к серверу
		const bool queryFound = checkRegexp(query, rules->query);
		// Проверяем на корректность UserAgent
		const bool userAgentFound = checkRegexp(uat, rules->userAgent);
		// Определяем найден ли клиент
		const bool clientFound = (checkCIP || checkCMac);
		// Определяем найден ли сервер
		const bool serverFound = (checkSIP || checkSDM);
		// Если и сервер и клиент найдены тогда добавляем в список правила
		if(clientFound && serverFound && queryFound && userAgentFound && pathFound){
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
 * @param gid     идентификатор группы
 * @param uid     идентификатор пользователя
 * @param ip      IP адрес пользователя
 * @param mac     MAC адрес пользователя
 * @param sip     IP адрес сервера
 * @param sdm     доменное имя сервера
 * @param uat     юзер-агент браузера
 * @param path    путь запроса 
 * @param query   параметры запроса
 * @param method  метод запроса
 * @param traffic направление трафика
 * @param action  экшен
 * @return        сформированный список правил
 */
const vector <string> Headers2::get(const gid_t gid, const uid_t uid, const string ip, const string mac, const string sip, const string sdm, const string uat, const string path, const string query, const string method, const bool traffic, const bool action){
	// Правила вывода данных
	vector <string> result;
	// Если данные клиента переданы
	if((gid > -1) && (uid > -1) && !ip.empty() && !mac.empty() && !sip.empty() && !this->rules.empty()){
		// Проверяем существует ли такая группа
		if(this->rules.count(gid) > 0){
			// Получаем данные пользователей
			auto users = this->rules.find(gid)->second;
			// Проверяем существует ли данный пользователь
			if(users.count(uid) > 0){
				// Получаем список методов
				auto methods = users.find(uid)->second.find(action)->second.find(traffic)->second;
				// Приводим к нижнему регистру
				const string tmpMethod = Anyks::toCase(method);
				// Если это звездочка
				if(tmpMethod.compare("*") == 0){
					// Переходим по всем методам запросов
					for(auto it = methods.cbegin(); it != methods.cend(); ++it){
						// Выполняем запрос для каждого метода
						result = findHeaders(ip, mac, sip, sdm, uat, path, query, it->first, &it->second);
					}
				// Добавляем правила
				} else if(methods.count(tmpMethod) > 0){
					// Извлекаем правило
					auto rules = methods.find(tmpMethod)->second;
					// Добавляем правила для конкретного метода
					result = findHeaders(ip, mac, sip, sdm, uat, path, query, tmpMethod, &rules);
				}
			}
		}
	}
	// Выводим результат
	return result;
}
/**
 * get Метод получения правил клиента
 * @param ip      IP адрес пользователя
 * @param mac     MAC адрес пользователя
 * @param sip     IP адрес сервера
 * @param sdm     доменное имя сервера
 * @param uat     юзер-агент браузера
 * @param path    путь запроса 
 * @param query   параметры запроса
 * @param method  метод запроса
 * @param traffic направление трафика
 * @param action  экшен
 * @return        сформированный список правил
 */
const vector <string> Headers2::get(const string ip, const string mac, const string sip, const string sdm, const string uat, const string path, const string query, const string method, const bool traffic, const bool action){
	// Правила вывода данных
	vector <string> result;
	// Если данные клиента переданы
	if(!ip.empty() && !mac.empty() && !sip.empty() && !this->rules.empty()){
		// Переходим по всему объекту групп
		for(auto ig = this->rules.cbegin(); ig != this->rules.cend(); ++ig){
			// Переходим по всему объекту пользователей
			for(auto iu = ig->second.cbegin(); iu != ig->second.cend(); ++iu){
				// Получаем список методов
				auto methods = iu->second.find(action)->second.find(traffic)->second;
				// Приводим к нижнему регистру
				const string tmpMethod = Anyks::toCase(method);
				// Если это звездочка
				if(tmpMethod.compare("*") == 0){
					// Переходим по всем методам запросов
					for(auto it = methods.cbegin(); it != methods.cend(); ++it){
						// Выполняем запрос для каждого метода
						result = findHeaders(ip, mac, sip, sdm, uat, path, query, it->first, &it->second);
					}
				// Добавляем правила
				} else if(methods.count(tmpMethod) > 0){
					// Извлекаем правило
					auto rules = methods.find(tmpMethod)->second;
					// Добавляем правила для конкретного метода
					result = findHeaders(ip, mac, sip, sdm, uat, path, query, tmpMethod, &rules);
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
		/**
		 * modifyHeadersForUser Функция модификации заголовков по пользовательским данным
		 * @param client указатель на блок с данными пользователя
		 * @param http   объект http данных запроса
		 */
		auto modifyHeadersForUser = [&agent, &path, &query, &host, &method, &traffic, this](const string ip, const string mac, const string sip, const AParams::AUser * client, HttpData &http){
			// Получаем идентификатор пользователя
			const uid_t uid = client->user.uid;
			// Если группы существуют
			if(!client->groups.empty()){
				// Переходим по всему списку групп
				for(auto it = client->groups.cbegin(); it != client->groups.cend(); it++){
					// Получаем идентификатор группы
					const gid_t gid = it->gid;
					// Запрашиваем список правил
					auto headersAdd = get(gid, uid, ip, mac, sip, host, agent, path, query, method, traffic, true);
					auto headersRm = get(gid, uid, ip, mac, sip, host, agent, path, query, method, traffic, false);
					// Добавляем заголовки
					modifyHeaders(true, headersAdd, http);
					// Удаляем заголовки
					modifyHeaders(false, headersRm, http);
				}
			}
		};
		// Если данные пользователя существуют
		if(client.user != nullptr){
			// Выполняем модификацию заголовков для пользователя
			modifyHeadersForUser(client.ip, client.mac, client.sip, client.user, http);
		// Если пользователь не установлен
		} else {
			// Пытаемся найти по ip и mac адресу
			auto user = this->ausers->searchUser(client.ip, client.mac);
			// Выполняем модификацию заголовков для пользователя
			if(user.auth) modifyHeadersForUser(client.ip, client.mac, client.sip, &user, http);
			// Если пользователь не найден тогда запрашиваем общие данные для всех пользователей
			else {
				// Запрашиваем списоки правил
				auto headersAdd = get(client.ip, client.mac, client.sip, host, agent, path, query, method, traffic, true);
				auto headersRm = get(client.ip, client.mac, client.sip, host, agent, path, query, method, traffic, false);
				// Добавляем заголовки
				modifyHeaders(true, headersAdd, http);
				// Удаляем заголовки
				modifyHeaders(false, headersRm, http);
			}
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
			/**
			 * modifyHeadersForUser Функция модификации заголовков по пользовательским данным
			 * @param client указатель на блок с данными пользователя
			 * @param http   объект http данных запроса
			 */
			auto modifyHeadersForUser = [&agent, &path, &query, &host, &method, &traffic, &data, this](const string ip, const string mac, const string sip, const AParams::AUser * client, HttpData &http){
				// Получаем идентификатор пользователя
				const uid_t uid = client->user.uid;
				// Если группы существуют
				if(!client->groups.empty()){
					// Переходим по всему списку групп
					for(auto it = client->groups.cbegin(); it != client->groups.cend(); it++){
						// Получаем идентификатор группы
						const gid_t gid = it->gid;
						// Запрашиваем список правил
						auto headersAdd = get(gid, uid, ip, mac, sip, host, agent, path, query, method, traffic, true);
						auto headersRm = get(gid, uid, ip, mac, sip, host, agent, path, query, method, traffic, false);
						// Добавляем заголовки
						modifyHeaders(true, headersAdd, data, http);
						// Удаляем заголовки
						modifyHeaders(false, headersRm, data, http);
						// Выполняем модификацию основных заголовков
						data = http.modifyHeaderString(data);
					}
				}
			};
			// Если данные пользователя существуют
			if(client.user != nullptr){
				// Выполняем модификацию заголовков для пользователя
				modifyHeadersForUser(client.ip, client.mac, client.sip, client.user, http);
			// Если пользователь не установлен
			} else {
				// Пытаемся найти по ip и mac адресу
				auto user = this->ausers->searchUser(client.ip, client.mac);
				// Выполняем модификацию заголовков для пользователя
				if(user.auth) modifyHeadersForUser(client.ip, client.mac, client.sip, &user, http);
				// Если пользователь не найден тогда запрашиваем общие данные для всех пользователей
				else {
					// Запрашиваем списоки правил
					auto headersAdd = get(client.ip, client.mac, client.sip, host, agent, path, query, method, traffic, true);
					auto headersRm = get(client.ip, client.mac, client.sip, host, agent, path, query, method, traffic, false);
					// Добавляем заголовки
					modifyHeaders(true, headersAdd, data, http);
					// Удаляем заголовки
					modifyHeaders(false, headersRm, data, http);
					// Выполняем модификацию основных заголовков
					data = http.modifyHeaderString(data);
				}
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
