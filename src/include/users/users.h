/* ПОЛЬЗОВАТЕЛИ ПРОКСИ AMING */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _USERS_PROXY_AMING_
#define _USERS_PROXY_AMING_

#include <regex>
#include <string>
#include <random>
#include <vector>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include <zlib.h>
#include <stdlib.h>
#include <sys/types.h>
#include "ini/ini.h"
#include "nwk/nwk.h"
#include "system/system.h"
#include "general/general.h"
#include "ldap2/ldap.h"
#include "groups/groups.h"
#include "headers/headers.h"

// Устанавливаем область видимости
using namespace std;

/**
 * Users Класс пользователей
 */
class Users {
	private:
		/**
		 * Ldap Структура ldap
		 */
		 struct Ldap {
			string dnUser;
			string dnConfig;
			string scopeUser;
			string scopeConfig;
			string filterUser;
			string filterConfig;
		};
		/**
		 * Timeout Структура таймаутов
		 */
		struct Timeouts {
			size_t read;	// Таймаут на чтение данных
			size_t write;	// Таймаут на запись данных
			size_t upgrade;	// Таймаут на работу в режиме переключения протоколов
		};
		/**
		 * IP Структура ip адреса
		 */
		struct IP {
			vector <string> ip;			// Список ip адресов
			vector <string> resolver;	// Список dns серверов
		};
		/**
		 * ID Структура идентификаторов подключений
		 */
		 struct ID {
			vector <string> ip4;	// Список ip4 адресов
			vector <string> ip6;	// Список ip6 адресов
			vector <string> mac;	// Список mac адресов
		};
		/**
		 * BufferSize Структура размеров буфера
		 */
		struct BufferSize {
			long read;		// Буфер на чтение
			long write;		// Буфер на запись
		};
		/**
		 * Keepalive Структура параметров постоянного подключения
		 */
		struct Keepalive {
			int keepcnt;	// Максимальное количество попыток
			int keepidle;	// Интервал времени в секундах через которое происходит проверка подключения
			int keepintvl;	// Интервал времени в секундах между попытками
		};
		/**
		 * Connects Структура контроля подключений клиента к серверу
		 */
		struct Connects {
			size_t size;	// Максимальный размер скачиваемых данных в байтах
			u_int connect;	// Максимальное количество подключений (одного клиента к прокси серверу)
		};
		/**
		 * Gzip Структура параметров сжатия данных на уровне прокси сервера
		 */
		struct Gzip {
			bool vary;					// Разрешает или запрещает выдавать в ответе поле заголовка “Vary: Accept-Encoding”
			int level;					// Тип сжатия (default - по умолчанию, best - лучшее сжатие, speed - лучшая скорость, no - без сжатия)
			long length;				// Минимальная длина данных после которых включается сжатие (работает только с Content-Length)
			size_t chunk;				// Максимальный размер чанка в байтах
			string regex;				// Не сжимать контент, UserAgent которого соответсвует регулярному выражению
			vector <string> vhttp;		// Версия http протокола
			vector <string> proxied;	// Разрешает или запрещает сжатие ответа методом gzip для проксированных запросов
			vector <string> types;		// Разрешает сжатие ответа методом gzip для указанных MIME-типов
		};
		/**
		 * Proxy Структура параметров самого прокси-сервера
		 */
		struct Proxy {
			bool reverse;		// Обратный прокси (доступ из сети в локальную сеть)
			bool transfer;		// Активация поддержки прокси листа
			bool forward;		// Прямой прокси (доступ во внешнюю сеть)
			bool subnet;		// Активация режима мульти-сетевого взаимодействия когда выход на сеть определяется по ip адресу и в зависимости от него выбирается ipv6 или ipv4 сеть
			bool pipelining;	// Активация конвеерной обработки
		};
		/**
		 * Data Структура данных пользователя
		 */
		struct Data {
			uid_t id;				// Идентификатор пользователя
			u_short options;		// Основные параметры прокси
			u_short type;			// Тип пользователя (0 - файлы, 1 - PAM, 2 - LDAP)
			string name;			// Название пользователя
			string pass;			// Пароль пользователя
			string auth;			// Тип авторизации клиентов
			string desc;			// Описание пользователя
			ID idnt;				// Блок списков идентификации пользователя
			IP ipv4;				// Блок ip адресов для протокола версии 4
			IP ipv6;				// Блок ip адресов для протокола версии 6
			Gzip gzip;				// Параметры gzip
			Proxy proxy;			// Параметры самого прокси-сервера
			Headers headers;		// Объект управления заголовками
			Connects connects;		// Контроль подключений клиента к серверу
			Timeouts timeouts;		// Таймауты подключений
			BufferSize buffers;		// Размеры буферов передачи данных
			Keepalive keepalive;	// Постоянное подключение
		};
		// Время в течение которого обновлять нельзя
		time_t maxUpdate = 0;
		// Время последнего обновления данных
		time_t lastUpdate = 0;
		// Тип поиска групп (0 - Из файла, 1 - из PAM, 2 - из LDAP)
		u_short typeSearch = 0;
		// Тип поиска конфигурационных данных
		u_short typeConfigs = 0; // (0 - Из файла, 1 - из LDAP)
		// Объект ldap подклчюения
		Ldap ldap;
		// Объект лога
		LogApp * log = nullptr;
		// Конфигурационные данные
		Config * config = nullptr;
		// Данные групп
		Groups * groups = nullptr;
		// Список пользователей
		unordered_map <uid_t, Data> data;
		/**
		 * setProxyOptions Функция добавления опций прокси
		 * @param option       опция для добавления
		 * @param proxyOptions список существующих опций
		 * @param flag         флаг добавления или удаления опции
		 */
		void setProxyOptions(const u_short option, u_short &proxyOptions, const bool flag = false);
		/**
		 * setDataUserFromLdap Метод заполнения данных пользователя из LDAP
		 * @param user объект пользователя
		 */
		void setDataUserFromLdap(Data &user);
		/**
		 * setDataUserFromFile Метод заполнения данных пользователя из конфигурационного файла
		 * @param user объект пользователя
		 * @param ini  указатель на объект конфигурации
		 */
		void setDataUserFromFile(Data &user, INI * ini = nullptr);
		/**
		 * setDataUser Метод заполнения данных пользователя
		 * @param user объект пользователя
		 * @param ini  указатель на объект конфигурации
		 */
		void setDataUser(Data &user, INI * ini = nullptr);
		/**
		 * createDefaultData Метод создания пользователя с параметрами по умолчанию
		 * @param  id   идентификатор пользователя
		 * @param  name название пользователя
		 * @return      созданный пользователь
		 */
		const Data createDefaultData(const uid_t id, const string name);
		/**
		 * readUsersFromLdap Метод чтения данных пользователей из LDAP сервера
		 * @return результат операции
		 */
		const bool readUsersFromLdap();
		/**
		 * readUsersFromPam Метод чтения данных пользователей из операционной системы
		 * @return результат операции
		 */
		const bool readUsersFromPam();
		/**
		 * readUsersFromFile Метод чтения данных пользователей из файла
		 * @return результат операции
		 */
		const bool readUsersFromFile();
		/**
		 * update Метод обновления пользователей
		 */
		const bool update();
	public:
		/**
		 * getAllUsers Метод получения данных всех пользователей
		 * @return     список данных всех пользователей
		 */
		const vector <const Data *> getAllUsers();
		/**
		 * getUserByConnect Метод поиска данных пользователя по данным коннекта
		 * @param ip  адрес интернет протокола клиента
		 * @param mac аппаратный адрес сетевого интерфейса клиента
		 * @return    данные пользователя
		 */
		const Data * getUserByConnect(const string ip = "", const string mac = "");
		/**
		 * getDataById Метод получения данные пользователя по идентификатору
		 * @param  uid идентификатор пользователя
		 * @return     данные пользователя
		 */
		const Data * getDataById(const uid_t uid);
		/**
		 * getDataByName Метод получения данные пользователя по имени
		 * @param  groupName название пользователя
		 * @return           данные пользователя
		 */
		const Data * getDataByName(const string userName);
		/**
		 * checkUserById Метод проверки на существование пользователя
		 * @param  uid идентификатор пользователя
		 * @return     результат проверки
		 */
		const bool checkUserById(const uid_t uid);
		/**
		 * checkGroupByName Метод проверки на существование пользователя
		 * @param  userName название пользователя
		 * @return          результат проверки
		 */
		const bool checkUserByName(const string userName);
		/**
		 * getIdByName Метод извлечения идентификатора пользователя по его имени
		 * @param  userName название пользователя
		 * @return          идентификатор пользователя
		 */
		const uid_t getIdByName(const string userName);
		/**
		 * getNameById Метод извлечения имени пользователя по его идентификатору
		 * @param  uid идентификатор пользователя
		 * @return     название пользователя
		 */
		const string getNameById(const uid_t uid);
		/**
		 * getIdAllUsers Метод получения списка всех пользователей
		 * @return список идентификаторов пользователей
		 */
		const vector <uid_t> getIdAllUsers();
		/**
		 * Users Конструктор
		 * @param config конфигурационные данные
		 * @param log    объект лога для вывода информации
		 * @param groups объект групп пользователей
		 */
		Users(Config * config = nullptr, LogApp * log = nullptr, Groups * groups = nullptr);
};

#endif // _USERS_PROXY_AMING_
