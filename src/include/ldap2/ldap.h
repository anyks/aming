/* МОДУЛЬ LDAP AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  copyright:  © 2017 anyks.com
*/

#ifndef _PROXY_LDAP_AMING_
#define _PROXY_LDAP_AMING_

#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <ldap.h>
#include <stdio.h>
#include <stdlib.h>
#include "system/system.h"
#include "general/general.h"

// Устанавливаем область видимости
using namespace std;

/**
 * ALDAP Класс работы с LDAP
 */
class ALDAP {
	private:
		/**
        * Data Объект с данными
        */
        struct Data {
			string dn;
			string key;
			unordered_map <string, vector <string>> vals;
		};
        // Объект лога
		LogApp * log = nullptr;
		// Конфигурационные данные
		Config * config = nullptr;
		// Активация модуля
		bool enabled = false;
		// Версия LDAP протокола
		u_int version = LDAP_VERSION2;
		// Адрес сервера
		string server;
		/**
		* getScope Метод определения скопа по названию
		* @param scope название скопа
		* @return      числовое значение скопа
		 */
		const u_int getScope(const string scope);
		/**
		 * auth Метод авторизации на LDAP сервере
		 * @param ld       объект LDAP подключения
		 * @param dn       dn пользователя
		 * @param password пароль пользователя
		 * @return         результат авторизации
		 */
		const bool auth(LDAP * ld, const string dn, const string password);
	public:
		/**
		* checkAuth Метод проверки авторизации
		* @param  dn       dn поиска
		* @param  password пароль для авторизации
		* @param  scope    скоп поиска
		* @param  filter   фильтр поиска
		* @return          результат проверки авторизации
		*/
		const bool checkAuth(const string dn, const string password, const string scope = "", const string filter = "(objectClass=top)");
		/**
		* data Метод поиска запрашиваемых данных
		* @param  dn     dn поиска
		* @param  key    ключ поиска
		* @param  scope  скоп поиска
		* @param  filter фильтр поиска
		* @return        искомые данные
		*/
		const vector <Data> data(const string dn, const string key, const string scope = "", const string filter = "(objectClass=top)");
		/**
		 * ALDAP Конструктор
		 * @param config конфигурационные данные
		 * @param log    объект лога для вывода информации
		 */
		ALDAP(Config * config = nullptr, LogApp * log = nullptr);
};

#endif // _PROXY_LDAP_AMING_
