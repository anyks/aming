/* МОДУЛЬ АВТОРИЗАЦИИ LDAP AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  copyright:  © 2017 anyks.com
*/

#ifndef _AUTH_LDAP_AMING_
#define _AUTH_LDAP_AMING_

#include <string>
#include <vector>
#include <iostream>
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
		LogApp * log = nullptr;
		// Конфигурационные данные
		Config * config = nullptr;
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
		 * @param config конфигурационные данные
		 * @param log    объект лога для вывода информации
		 */
		AuthLDAP(Config * config = nullptr, LogApp * log = nullptr);
};

#endif // _AUTH_LDAP_AMING_
