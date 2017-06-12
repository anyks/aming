/* ГРУППЫ ПОЛЬЗОВАТЕЛЕЙ ПРОКСИ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _GROUPS_PROXY_ANYKS_
#define _GROUPS_PROXY_ANYKS_

#include <string>
#include <random>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <zlib.h>
#include <stdlib.h>
#include <sys/types.h>
#include "ini/ini.h"
#include "system/system.h"
#include "general/general.h"
#include "headers/headers.h"

// Устанавливаем область видимости
using namespace std;

/**
 * Groups Класс групп пользователей
 */
class Groups {
	private:
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
			vector <string> ip;		// Список ip адресов
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
		 * Data Структура данных группы
		 */
		struct Data {
			u_int id;				// Идентификатор группы
			u_short options;		// Основные параметры прокси
			string name;			// Название группы
			ID idnt;				// Блок списков идентификации групп
			IP ipv4;				// Блок ip адресов для протокола версии 4
			IP ipv6;				// Блок ip адресов для протокола версии 6
			Gzip gzip;				// Параметры gzip
			Proxy proxy;			// Параметры самого прокси-сервера
			Headers headers;		// Объект управления заголовками
			Connects connects;		// Контроль подключений клиента к серверу
			Timeouts timeouts;		// Таймауты подключений
			BufferSize buffers;		// Размеры буферов передачи данных
			Keepalive keepalive;	// Постоянное подключение
			vector <u_int> users;	// Список идентификаторов пользователей
		};
		// Тип поиска групп (0 - Из файла, 1 - из PAM, 2 - из LDAP)
		u_short typeSearch = 0;
		// Объект лога
		LogApp * log = NULL;
		// Конфигурационные данные
		Config * config = NULL;
		// Список групп
		unordered_map <u_int, Data> data;
		/**
		 * update Метод обновления групп
		 */
		void update();
		/**
		 * createDefaultData Метод создания группы с параметрами по умолчанию
		 * @param  id   идентификатор групыы
		 * @param  name название группы
		 * @return      созданная группа
		 */
		const Data createDefaultData(const u_int id, const string name);
		/**
		 * readGroupsFromFile Метод чтения данных групп из операционной системы
		 * @return результат операции
		 */
		const bool readGroupsFromPam();
		/**
		 * readGroupsFromFile Метод чтения данных групп из LDAP сервера
		 * @return результат операции
		 */
		const bool readGroupsFromLdap();
		/**
		 * readGroupsFromFile Метод чтения данных групп из файла
		 * @return результат операции
		 */
		const bool readGroupsFromFile();
	public:
		/**
		 * getDataById Метод получения данные группы по идентификатору группы
		 * @param  gid идентификатор группы
		 * @return     данные группы
		 */
		const Data getDataById(const u_int gid);
		/**
		 * getDataByName Метод получения данные группы по имени группы
		 * @param  groupName название группы
		 * @return           данные группы
		 */
		const Data getDataByName(const string groupName);
		/**
		 * getGroupIdByUser Метод получения идентификатор группы по идентификатору пользователя
		 * @param  uid идентификатор пользователя
		 * @return     идентификатор группы
		 */
		const vector <u_int> getGroupIdByUser(const u_int uid);
		/**
		 * getGroupIdByUser Метод получения идентификатор группы по имени пользователя
		 * @param  userName название пользователя
		 * @return          идентификатор группы
		 */
		const vector <u_int> getGroupIdByUser(const string userName);
		/**
		 * getGroupNameByUser Метод получения название группы по идентификатору пользователя
		 * @param  uid идентификатор пользователя
		 * @return     название группы
		 */
		const vector <string> getGroupNameByUser(const u_int uid);
		/**
		 * getGroupNameByUser Метод получения название группы по имени пользователя
		 * @param  userName название пользователя
		 * @return          название группы
		 */
		const vector <string> getGroupNameByUser(const string userName);
		/**
		 * checkUser Метод проверки принадлежности пользователя к группе
		 * @param  gid идентификатор группы
		 * @param  uid идентификатор пользователя
		 * @return     результат проверки
		 */
		const bool checkUser(const u_int gid, const u_int uid);
		/**
		 * checkUser Метод проверки принадлежности пользователя к группе
		 * @param  gid      идентификатор группы
		 * @param  userName название пользователя
		 * @return          результат проверки
		 */
		const bool checkUser(const u_int gid, const string userName);
		/**
		 * checkUser Метод проверки принадлежности пользователя к группе
		 * @param  groupName название группы
		 * @param  uid       идентификатор пользователя
		 * @return           результат проверки
		 */
		const bool checkUser(const string groupName, const u_int uid);
		/**
		 * checkUser Метод проверки принадлежности пользователя к группе
		 * @param  groupName название группы
		 * @param  userName  название пользователя
		 * @return           результат проверки
		 */
		const bool checkUser(const string groupName, const string userName);
		/**
		 * getUidByName Метод извлечения идентификатора пользователя по его имени
		 * @param  userName название пользователя
		 * @return          идентификатор пользователя
		 */
		const u_int getUidByName(const string userName);
		/**
		 * getIdByName Метод извлечения идентификатора группы по ее имени
		 * @param  groupName название группы
		 * @return           идентификатор группы
		 */
		const u_int getIdByName(const string groupName);
		/**
		 * getUserNameByUid Метод извлечения имени пользователя по его идентификатору
		 * @param  uid идентификатор пользователя
		 * @return     название пользователя
		 */
		const string getUserNameByUid(const u_int uid);
		/**
		 * getNameById Метод извлечения имени группы по ее идентификатору
		 * @param  gid идентификатор группы
		 * @return     название группы
		 */
		const string getNameById(const u_int gid);
		/**
		 * getNameUsers Метод получения списка пользователей в группе
		 * @param  gid идентификатор группы
		 * @return     список имен пользователей
		 */
		const vector <string> getNameUsers(const u_int gid);
		/**
		 * getNameUsers Метод получения списка пользователей в группе
		 * @param  groupName название группы
		 * @return           список имен пользователей
		 */
		const vector <string> getNameUsers(const string groupName);
		/**
		 * getIdUsers Метод получения списка пользователей в группе
		 * @param  gid идентификатор группы
		 * @return     список идентификаторов пользователей
		 */
		const vector <u_int> getIdUsers(const u_int gid);
		/**
		 * getIdUsers Метод получения списка пользователей в группе
		 * @param  groupName название группы
		 * @return           список идентификаторов пользователей
		 */
		const vector <u_int> getIdUsers(const string groupName);
		/**
		 * addUser Метод добавления пользователя
		 * @param  gid идентификатор группы
		 * @param  uid идентификатор пользователя
		 * @return     результат добавления
		 */
		const bool addUser(const u_int gid, const u_int uid);
		/**
		 * addUser Метод добавления пользователя
		 * @param  gid       идентификатор группы
		 * @param  userName  название пользователя
		 * @return           результат добавления
		 */
		const bool addUser(const u_int gid, const string userName);
		/**
		 * addUser Метод добавления пользователя
		 * @param  groupName название группы
		 * @param  uid       идентификатор пользователя
		 * @return           результат добавления
		 */
		const bool addUser(const string groupName, const u_int uid);
		/**
		 * addUser Метод добавления пользователя
		 * @param  groupName название группы
		 * @param  userName  название пользователя
		 * @return           результат добавления
		 */
		const bool addUser(const string groupName, const string userName);
		/**
		 * addGroup Метод добавления группы
		 * @param  id   идентификатор группы
		 * @param  name название группы
		 * @return      результат добавления
		 */
		const bool addGroup(const u_int id, const string name);
		/**
		 * Groups Конструктор
		 * @param config конфигурационные данные
		 * @param log    объект лога для вывода информации
		 */
		Groups(Config * config = NULL, LogApp * log = NULL);
};

#endif // _GROUPS_PROXY_ANYKS_
