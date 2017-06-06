/* МОДУЛЬ АВТОРИЗАЦИИ LDAP ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _AUTH_LDAP_ANYKS_
#define _AUTH_LDAP_ANYKS_

#include <ldap.h>
#include <stdio.h>
#include <stdlib.h>
#include "system/system.h"
#include "general/general.h"

// Устанавливаем область видимости
using namespace std;

/**
 * AuthLDAP Класс авторизации LDAP
 */
class AuthLDAP {
	private:
		// Объект лога
		LogApp * log = NULL;
		// Конфигурационные данные
		Config * config = NULL;
		// Активация модуля
		bool enabled = false;
		// Версия LDAP протокола
		u_int version = LDAP_VERSION2;
		// Тип скопа
		u_int scope = LDAP_SCOPE_SUBTREE;
		// Адрес сервера
		string server;
		// Фильтр поиска
		string filter;
		// DN поиска пользователей
		string userdn;
		/**
		 * authLDAP Метод авторизации на LDAP сервере
		 * @param ld       объект LDAP подключения
		 * @param dn       dn пользователя
		 * @param password пароль пользователя
		 * @return         результат авторизации
		 */
		const bool authLDAP(LDAP * ld, const string dn, const string password);
	public:
		/**
		 * checkUser Метод проверки авторизации пользователя
		 * @param  user     логин пользователя
		 * @param  password пароль пользователя
		 * @return          результат проверки
		 */
		const bool checkUser(const string user, const string password);
		/**
		 * AuthLDAP Конструктор
		 * @param log    объект лога для вывода информации
		 * @param config конфигурационные данные
		 */
		AuthLDAP(LogApp * log = NULL, Config * config = NULL);
};

#endif // _AUTH_LDAP_ANYKS_
