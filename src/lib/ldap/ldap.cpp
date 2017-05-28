/* МОДУЛЬ АВТОРИЗАЦИИ LDAP ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#include "ldap/ldap.h"

// Устанавливаем область видимости
using namespace std;

/**
 * authLDAP Метод авторизации на LDAP сервере
 * @param ld       объект LDAP подключения
 * @param dn       dn пользователя
 * @param password пароль пользователя
 * @return         результат авторизации
 */
const bool AuthLDAP::authLDAP(LDAP * ld, const string dn, const string password){
	// Выполняем авторизацию
	int rc = ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &this->version);
	// Устанавливаем версию протокола
	if(rc != LDAP_SUCCESS){
		// Выводим в консоль информацию
		this->log->write(LOG_ERROR, 0, "set version ldap filed: %s", ldap_err2string(rc));
		// Выводим сообщение что авторизация не удалась
		return false;
	}
	// Если это версия LDAP SASL
	#if defined(LDAP_API_FEATURE_X_OPENLDAP) && LDAP_API_FEATURE_X_OPENLDAP >= 20300
		// Создаем структуру с данными проля
		struct berval cred = {0, NULL};
		// Устанавливаем длину пароля
		cred.bv_len = password.length();
		// Создаем буфер под данные пароля
		cred.bv_val = new char[cred.bv_len + 1];
		// Заполняем буфер нулями
		memset(cred.bv_val, 0, cred.bv_len);
		// Копируем данные пароля
		strcpy(cred.bv_val, password.c_str());
		// Выполняем авторизацию
		rc = ldap_sasl_bind_s(ld, dn.c_str(), LDAP_SASL_SIMPLE, &cred, NULL, NULL, NULL);
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
		this->log->write(LOG_ERROR, 0, "ldap auth filed: %s", ldap_err2string(rc));
		// Выводим сообщение что авторизация не удалась
		return false;
	}
	// Сообщаем что авторизация удачная
	return true;
}
/**
 * checkUser Метод проверки авторизации пользователя
 * @param  user     логин пользователя
 * @param  password пароль пользователя
 * @return          результат проверки
 */
const bool AuthLDAP::checkUser(const string user, const string password){
	// Результат проверки на авторизацию
	bool auth = false;
	// Если сервер активирован
	if(this->enabled){
		// Параметр для поиска
		const char * key = "%v";
		// Выполняем поиск ключа
		size_t pos = this->filter.find(key);
		// Если ключ найден
		if(pos != string::npos){
			// Найденный DN пользователя
			char * dn = NULL;
			// Объект LDAP подключения
			LDAP * ld = NULL, * uld = NULL;
			// Выполняем инициализацию подключения к серверу
			if(ldap_initialize(&ld, this->server.c_str())){
				// Выводим в консоль информацию
				this->log->write(LOG_ERROR, 0, "ldap initialize filed");
				// Сообщаем что авторизация не прошла
				return auth;
			}
			// Выполняем авторизацию на сервере
			if(authLDAP(ld, this->config->ldap.binddn, this->config->ldap.bindpw)){
				// Указатели на сообщения LDAP
				LDAPMessage * result, * e;
				// Формируем наш фильтр
				string filter = this->filter;
				// Заменяем ключ на логин пользователя
				filter = filter.replace(pos, strlen(key), user);
				// Выполняем поиск в ldap пользователя
				int rc = ldap_search_ext_s(ld, this->userdn.c_str(), this->scope, filter.c_str(), NULL, 0, NULL, NULL, NULL, 0, &result);
				// Если поиск не удался
				if(rc != LDAP_SUCCESS){
					// Выводим в консоль информацию
					this->log->write(LOG_ERROR, 0, "ldap search filed: %s", ldap_err2string(rc));
					// Очищаем подключение
					ldap_unbind_ext(ld, NULL, NULL);
					// Сообщаем что поиск не удачный
					return auth;
				}
				// Выполняем перебор найденных данных
				for(e = ldap_first_entry(ld, result ); e != NULL; e = ldap_next_entry(ld, e)){
					// Если DN пользователя найден
					if((dn = ldap_get_dn(ld, e)) != NULL){
						// Выполняем инициализацию подключения к серверу
						if(ldap_initialize(&uld, this->server.c_str())){
							// Выводим в консоль информацию
							this->log->write(LOG_ERROR, 0, "ldap user initialize filed");
							// Очищаем подключение
							ldap_unbind_ext(ld, NULL, NULL);
							// Сообщаем что авторизация не прошла
							return auth;
						}
						// Проверяем авторизацию
						auth = authLDAP(uld, dn, password);
						// Выполняем очистику объекта LDAP
						ldap_unbind_ext(uld, NULL, NULL);
						// Очищаем полученный DN
						ldap_memfree(dn);
						// Если пользователь найден то выходим
						if(auth) break;
					}
				}
				// Очищаем данные сообщения
				if(result) ldap_msgfree(result);
			}
			// Очищаем подключение
			ldap_unbind_ext(ld, NULL, NULL);
		}
	}
	// Сообщаем что авторизация не прошла
	return auth;
}
/**
 * AuthLDAP Конструктор
 * @param log    объект лога для вывода информации
 * @param config конфигурационные данные
 */
AuthLDAP::AuthLDAP(LogApp * log, Config * config){
	// Если сервер активирован
	if((config != NULL)
	&& config->ldap.enabled
	&& !config->ldap.server.empty()){
		// Запоминаем объект логов
		this->log = log;
		// Запоминаем адрес объекта конфигурации
		this->config = config;
		// Запоминаем адрес сервера
		this->server = this->config->ldap.server;
		// DN поиска пользователей
		this->userdn = this->config->ldap.userdn;
		// Фильтр поиска
		this->filter = this->config->ldap.filter;
		// Определяем активирован модуль или нет
		this->enabled = this->config->ldap.enabled;
		// Устанавливаем версию ldap
		switch(this->config->ldap.version){
			// Если это версия 3
			case 3: this->version = LDAP_VERSION3; break;
			// По умолчанию версия 2
			default: this->version = LDAP_VERSION2;
		}
		// Определяем тип скопа
		if(this->config->ldap.scope.compare("sub") == 0)		this->scope = LDAP_SCOPE_SUBTREE;
		else if(this->config->ldap.scope.compare("one") == 0)	this->scope = LDAP_SCOPE_ONELEVEL;
		else if(this->config->ldap.scope.compare("base") == 0)	this->scope = LDAP_SCOPE_BASE;
		// Объект LDAP подключения
		LDAP * ld = NULL;
		// Выполняем инициализацию подключения к серверу
		if(ldap_initialize(&ld, this->server.c_str())){
			// Устанавливаем что сервер деактивирован
			this->enabled = false;
			// Выводим в консоль информацию
			this->log->write(LOG_ERROR, 0, "ldap initialize filed");
		// Если коннект к серверу удачный
		} else {
			// Выполняем авторизацию на сервере
			this->enabled = authLDAP(ld, this->config->ldap.binddn, this->config->ldap.bindpw);
			// Очищаем подключение
			ldap_unbind_ext(ld, NULL, NULL);
		}
	}
}
