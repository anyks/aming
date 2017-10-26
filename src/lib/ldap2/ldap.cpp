/* МОДУЛЬ LDAP AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  copyright:  © 2017 anyks.com
*/

#include "ldap2/ldap.h"

// Устанавливаем область видимости
using namespace std;

/**
 * getScope Метод определения скопа по названию
 * @param scope название скопа
 * @return      числовое значение скопа
 */
const u_int ALDAP::getScope(const string scope){
	// Результат работы функции
	u_int result = LDAP_SCOPE_SUBTREE;
	// Если скоп передан
	if(!scope.empty()){
		// Определяем тип скопа
		if(scope.compare("sub") == 0)		result = LDAP_SCOPE_SUBTREE;
		else if(scope.compare("one") == 0)	result = LDAP_SCOPE_ONELEVEL;
		else if(scope.compare("base") == 0)	result = LDAP_SCOPE_BASE;
    }
	// Выводим результат
	return result;
}
/**
 * auth Метод авторизации на LDAP сервере
 * @param ld       объект LDAP подключения
 * @param dn       dn пользователя
 * @param password пароль пользователя
 * @return         результат авторизации
 */
const bool ALDAP::auth(LDAP * ld, const string dn, const string password){
	// Выполняем авторизацию
	int rc = ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &this->version);
	// Устанавливаем версию протокола
	if(rc != LDAP_SUCCESS){
		// Выводим в консоль информацию
		if(this->log) this->log->write(LOG_ERROR, 0, "set version ldap filed: %s", ldap_err2string(rc));
		// Выводим сообщение что авторизация не удалась
		return false;
	}
	// Если это версия LDAP SASL
	#if defined(LDAP_API_FEATURE_X_OPENLDAP) && LDAP_API_FEATURE_X_OPENLDAP >= 20300
		// Создаем структуру с данными проля
		struct berval cred = {0, nullptr};
		// Устанавливаем длину пароля
		cred.bv_len = password.length();
		// Создаем буфер под данные пароля
		cred.bv_val = new char[cred.bv_len + 1];
		// Заполняем буфер нулями
		memset(cred.bv_val, 0, cred.bv_len);
		// Копируем данные пароля
		strcpy(cred.bv_val, password.c_str());
		// Выполняем авторизацию
		rc = ldap_sasl_bind_s(ld, dn.c_str(), LDAP_SASL_SIMPLE, &cred, nullptr, nullptr, nullptr);
		// Удаляем выделенную ранее память
		delete [] cred.bv_val;
		// Зануляем структуру
		memset(&cred, 0, sizeof(cred));
	// Если это не LDAP SASL
	#else
		// Выполняем простую авторизацию
		rc = ldap_simple_bind_s(ld, dn.c_str(), password.c_str());
	#endif
	// Если авторизация не удалась
	if(rc != LDAP_SUCCESS){
		// Выводим в консоль информацию
		if(this->log) this->log->write(LOG_ERROR, 0, "ldap auth filed: %s", ldap_err2string(rc));
		// Выводим сообщение что авторизация не удалась
		return false;
	}
	// Сообщаем что авторизация удачная
	return true;
}
/**
* checkAuth Метод проверки авторизации
* @param  dn       dn поиска
* @param  password пароль для авторизации
* @param  scope    скоп поиска
* @param  filter   фильтр поиска
* @return          результат проверки авторизации
 */
const bool ALDAP::checkAuth(const string dn, const string password, const string scope, const string filter){
	// Результат проверки на авторизацию
	bool auth = false;
	// Если сервер активирован
	if(this->enabled && !dn.empty() && !password.empty() && !scope.empty() && !filter.empty()){
		// Объект LDAP подключения
		LDAP * ld = nullptr, * uld = nullptr;
		// Выполняем инициализацию подключения к серверу
		if(ldap_initialize(&ld, this->server.c_str())){
			// Выводим в консоль информацию
			if(this->log) this->log->write(LOG_ERROR, 0, "ldap initialize filed");
			// Сообщаем что авторизация не прошла
			return auth;
		}
		// Выполняем авторизацию на сервере
		if(this->auth(ld, this->config->ldap.binddn, this->config->ldap.bindpw)){
			// Найденный DN пользователя
			char * udn = nullptr;
			// Указатели на сообщения LDAP
			LDAPMessage * result, * e;
			// Получаем значение скопа
			const u_int cscope = getScope(scope);
			// Выполняем поиск в ldap пользователя
			int rc = ldap_search_ext_s(ld, dn.c_str(), cscope, filter.c_str(), nullptr, 0, nullptr, nullptr, nullptr, 0, &result);
			// Если поиск не удался
			if(rc != LDAP_SUCCESS){
				// Выводим в консоль информацию
				if(this->log) this->log->write(LOG_ERROR, 0, "ldap search filed: %s", ldap_err2string(rc));
				// Очищаем подключение
				ldap_unbind_ext(ld, nullptr, nullptr);
				// Сообщаем что поиск не удачный
				return auth;
			}
			// Выполняем перебор найденных данных
			for(e = ldap_first_entry(ld, result); e != nullptr; e = ldap_next_entry(ld, e)){
				// Если DN пользователя найден
				if((udn = ldap_get_dn(ld, e)) != nullptr){
					// Выполняем инициализацию подключения к серверу
					if(ldap_initialize(&uld, this->server.c_str())){
						// Выводим в консоль информацию
						if(this->log) this->log->write(LOG_ERROR, 0, "ldap user initialize filed");
						// Очищаем подключение
						ldap_unbind_ext(ld, nullptr, nullptr);
						// Сообщаем что авторизация не прошла
						return auth;
					}
					// Проверяем авторизацию
					auth = this->auth(uld, udn, password);
					// Выполняем очистику объекта LDAP
					ldap_unbind_ext(uld, nullptr, nullptr);
					// Очищаем полученный DN
					ldap_memfree(udn);
					// Если пользователь найден то выходим
					if(auth) break;
				}
			}
			// Очищаем данные сообщения
			if(result) ldap_msgfree(result);
		}
		// Очищаем подключение
		ldap_unbind_ext(ld, nullptr, nullptr);
	}
	// Сообщаем что авторизация не прошла
	return auth;
}
/**
* data Метод поиска запрашиваемых данных
* @param  dn     dn поиска
* @param  key    ключ поиска
* @param  scope  скоп поиска
* @param  filter фильтр поиска
* @return        искомые данные
 */
const vector <ALDAP::Data> ALDAP::data(const string dn, const string key, const string scope, const string filter){
	// Массив данных для поиска
	vector <Data> data;
	// Если сервер активирован
	if(this->enabled && !dn.empty() && !filter.empty() && !scope.empty() && !key.empty()){
		// Объект LDAP подключения
		LDAP * ld = nullptr;
		// Выполняем инициализацию подключения к серверу
		if(ldap_initialize(&ld, this->server.c_str())){
			// Выводим в консоль информацию
			if(this->log) this->log->write(LOG_ERROR, 0, "ldap initialize filed");
			// Сообщаем что авторизация не прошла
			return data;
		}
		// Выполняем авторизацию на сервере
		if(auth(ld, this->config->ldap.binddn, this->config->ldap.bindpw)){
			// Указатели на сообщения LDAP
			LDAPMessage * result, * e;
			// Получаем значение скопа
			const u_int cscope = getScope(scope);
			// Выполняем поиск в ldap данных
			int rc = ldap_search_ext_s(ld, dn.c_str(), cscope, filter.c_str(), nullptr, 0, nullptr, nullptr, nullptr, 0, &result);
			// Если поиск не удался
			if(rc != LDAP_SUCCESS){
				// Выводим в консоль информацию
				if(this->log) this->log->write(LOG_ERROR, 0, "ldap search filed: %s", ldap_err2string(rc));
				// Очищаем подключение
				ldap_unbind_ext(ld, nullptr, nullptr);
				// Сообщаем что поиск не удачный
				return data;
			}
			// Найденный DN пользователя
			char * cdn = nullptr;
			// Выполняем перебор найденных данных
			for(e = ldap_first_entry(ld, result); e != nullptr; e = ldap_next_entry(ld, e)){
				// Если DN пользователя найден
				if((cdn = ldap_get_dn(ld, e)) != nullptr){
					// Массив значений
					unordered_map <string, vector <string>> vals;
					// Выполняем разбор ключей
					vector <string> keys = Anyks::split(key, ",");
					// Переходим по всем ключам
					for(auto it = keys.cbegin(); it != keys.cend(); ++it){
						// Получаем название ключа
						const string key = * it;
						// Если ключ существует
						if(!key.empty()){
							// Запрашиваем данные для указанного ключа
							char ** params = ldap_get_values(ld, e, key.c_str());
							// Переходим по всему объекту ключей и добавляем найденные данные в список
							for(u_int i = 0; i < ldap_count_values(params); i++){
								// Если ключ в списке найден
								if(vals.count(key)){
									// Добавляем в уже существующих список остальные ключи
									vals.find(key)->second.push_back(params[i]);
								// Добавляем в список найденные значения
								} else vals.insert(pair <string, vector <string>>(key, {params[i]}));
							}
						}
					}
					// Добавляем полученные данные в список
					data.push_back({cdn, key, vals});
					// Очищаем полученный DN
					ldap_memfree(cdn);
				}
			}
			// Очищаем данные сообщения
			if(result) ldap_msgfree(result);
		}
		// Очищаем подключение
		ldap_unbind_ext(ld, nullptr, nullptr);
	}
	// Выводим собранные данные
	return data;
}
/**
 * ALDAP Конструктор
 * @param config конфигурационные данные
 * @param log    объект лога для вывода информации
 */
ALDAP::ALDAP(Config * config, LogApp * log){
	// Если сервер активирован
	if(config && config->ldap.enabled
	&& !config->ldap.server.empty()){
		// Запоминаем объект логов
		this->log = log;
		// Запоминаем адрес объекта конфигурации
		this->config = config;
		// Запоминаем адрес сервера
		this->server = this->config->ldap.server;
		// Определяем активирован модуль или нет
		this->enabled = this->config->ldap.enabled;
		// Устанавливаем версию ldap
		switch(this->config->ldap.version){
			// Если это версия 3
			case 3: this->version = LDAP_VERSION3; break;
			// По умолчанию версия 2
			default: this->version = LDAP_VERSION2;
		}
		// Объект LDAP подключения
		LDAP * ld = nullptr;
		// Выполняем инициализацию подключения к серверу
		if(ldap_initialize(&ld, this->server.c_str())){
			// Устанавливаем что сервер деактивирован
			this->enabled = false;
			// Выводим в консоль информацию
			if(this->log) this->log->write(LOG_ERROR, 0, "ldap initialize filed");
		// Если коннект к серверу удачный
		} else {
			// Выполняем авторизацию на сервере
			this->enabled = auth(ld, this->config->ldap.binddn, this->config->ldap.bindpw);
			// Очищаем подключение
			ldap_unbind_ext(ld, nullptr, nullptr);
		}
	}
}
