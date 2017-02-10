/* МОДУЛЬ РАБОТЫ С СЕТЯМИ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _NETWORK_ANYKS_
#define _NETWORK_ANYKS_

#include <regex>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <math.h>
#include <sys/types.h>

// Устанавливаем область видимости
using namespace std;

/**
 * IPdata Класс содержащий данные ip адреса
 */
class IPdata {
	private:
		// Параметры ip адреса
		u_int ptr[4];
	public:
		/**
		 * set Метод установки данных ip адреса
		 * @param ptr массив значений ip адреса
		 */
		void set(u_int ptr1 = 0, u_int ptr2 = 0, u_int ptr3 = 0, u_int ptr4 = 0);
		/**
		 * get Метод получения данных масива
		 * @return указатель на масив ip адреса
		 */
		const u_int * get();
		/**
		 * getStr Метод вывода результата в виде строки
		 * @return строка ip адреса
		 */
		const string getStr();
		/**
		 * length Метод определения размера
		 * @return размер массива
		 */
		u_int size();
		/**
		 * IPdata Конструктор
		 */
		IPdata();
};
/**
 * NTdata Структура данных типов сетей
 */
struct NTdata {
	char mask[17];		// Маска сети
	char nmask[17];		// Маска подсети
	char smask[17];		// Начальный адрес
	char emask[17];		// Конечный адрес
	char invers[17];	// Инверсия
	char format[13];	// Распределение байт (С — сеть, Х — хост)
	char type[3];		// Тип сети
	float cls;			// Класс типа сети
	u_int number;		// Цифровое значение маски
	u_int sbytes;		// Первые биты
	u_long counts;		// Количество доступных адресов
	u_long maxnwk;		// Число возможных адресов сетей
	u_long maxhst;		// Число возможных адресов хостов
	bool notEmpty;		// Структура заполнена или нет
};
/**
 * NLdata Структура содержащая параметры локальных и запрещенных сетей
 */
struct NLdata {
	string ip;		// ip адрес сети
	string network;	// Адрес сети
	u_int mask;		// Маска сети
	bool allow;		// Разрешен (true - разрешен, false - запрещен)
};
/**
 * NLdata6 Структура содержащая параметры локальных и запрещенных сетей IPv6
 */
struct NLdata6 {
	string ip;		// ip адрес сети
	string eip;		// Конечный ip адрес
	u_int prefix;	// Префикс сети
	bool allow;		// Разрешен (true - разрешен, false - запрещен)
};
/**
 * NKdata Структура содержащая данные подключения
 */
struct NKdata {
	IPdata ip;		// Данные ip адреса
	IPdata mask;	// Данные маски сети
	IPdata network;	// Данные сети
};
/**
 * Network Класс для работы с сетями
 */
class Network {
	private:
		// Набор локальных сетей IPv6
		vector <NLdata6> locals6 = {
			{"::1", "", 128, true},
			{"2001::", "", 32, false},
			{"2001:db8::", "", 32, true},
			{"64:ff9b::", "", 96, false},
			{"2002::", "", 16, false},
			{"fe80::", "febf::", 10, true},
			{"fec0::", "feff::", 10, true},
			{"fc00::", "", 7, true},
			{"ff00::", "", 8, false}
		};
		// Набор локальных сетей
		vector <NLdata> locals = {
			{"0.0.0.0", "0.0.0.0", 8, false},
			{"0.0.0.0", "0.0.0.0", 32, false},
			{"100.64.0.0", "100.64.0.0", 10, false},
			{"169.254.0.0", "169.254.0.0", 16, false},
			{"224.0.0.0", "224.0.0.0", 4, false},
			{"224.0.0.0", "224.0.0.0", 24, false},
			{"224.0.1.0", "224.0.0.0", 8, false},
			{"239.0.0.0", "239.0.0.0", 8, false},
			{"240.0.0.0", "240.0.0.0", 4, false},
			{"255.255.255.255", "255.255.255.255", 32, false},
			{"10.0.0.0", "10.0.0.0", 8, true},
			{"127.0.0.0", "127.0.0.0", 8, true},
			{"172.16.0.0", "172.16.0.0", 12, true},
			{"192.0.0.0", "192.0.0.0", 24, true},
			{"192.0.0.0", "192.0.0.0", 29, true},
			{"192.0.0.170", "192.0.0.170", 32, true},
			{"192.0.0.171", "192.0.0.171", 32, true},
			{"192.0.2.0", "192.0.2.0", 24, true},
			{"192.88.99.0", "192.88.99.0", 24, true},
			{"192.88.99.1", "192.88.99.1", 32, true},
			{"192.168.0.0", "192.168.0.0", 16, true},
			{"198.51.100.0", "198.51.100.0", 24, true},
			{"198.18.0.0", "198.18.0.0", 15, true},
			{"203.0.113.0", "203.0.113.0", 24, true}
		};
		// Набор масок подсетей
		vector <NTdata> masks = {
			{"000.000.000.000\0", "255.0.0.0\0", "0.0.0.0\0", "127.255.255.255\0", "255.255.255.255\0", "С.Х.Х.Х\0", "A\0", 256, 0, 0, 4294967296, 128, 16777214},	// 0
			{"128.000.000.000\0", "255.0.0.0\0", "0.0.0.0\0", "127.255.255.255\0", "127.255.255.255\0", "С.Х.Х.Х\0", "A\0", 128, 1, 0, 2147483648, 128, 16777214},	// 1
			{"192.000.000.000\0", "255.0.0.0\0", "0.0.0.0\0", "127.255.255.255\0", "63.255.255.255\0", "С.Х.Х.Х\0", "A\0", 64, 2, 0, 1073741824, 128, 16777214},	// 2
			{"224.000.000.000\0", "255.0.0.0\0", "0.0.0.0\0", "127.255.255.255\0", "31.255.255.255\0", "С.Х.Х.Х\0", "A\0", 32, 3, 0, 536870912, 128, 16777214},		// 3
			{"240.000.000.000\0", "255.0.0.0\0", "0.0.0.0\0", "127.255.255.255\0", "15.255.255.255\0", "С.Х.Х.Х\0", "A\0", 16, 4, 0, 268435456, 128, 16777214},		// 4
			{"248.000.000.000\0", "255.0.0.0\0", "0.0.0.0\0", "127.255.255.255\0", "7.255.255.255\0", "С.Х.Х.Х\0", "A\0", 8, 5, 0, 134217728, 128, 16777214},		// 5
			{"252.000.000.000\0", "255.0.0.0\0", "0.0.0.0\0", "127.255.255.255\0", "3.255.255.255\0", "С.Х.Х.Х\0", "A\0", 4, 6, 0, 67108864, 128, 16777214},		// 6
			{"254.000.000.000\0", "255.0.0.0\0", "0.0.0.0\0", "127.255.255.255\0", "1.255.255.255\0", "С.Х.Х.Х\0", "A\0", 2, 7, 0, 33554432, 128, 16777214},		// 7
			{"255.000.000.000\0", "255.0.0.0\0", "0.0.0.0\0", "127.255.255.255\0", "0.255.255.255\0", "С.Х.Х.Х\0", "A\0", 1, 8, 0, 16777216, 128, 16777214},		// 8
			{"255.128.000.000\0", "255.255.0.0\0", "128.0.0.0\0", "191.255.255.255\0", "0.127.255.255\0", "С.С.Х.Х\0", "B\0", 128, 9, 10, 8388608, 16384, 65534},	// 9
			{"255.192.000.000\0", "255.255.0.0\0", "128.0.0.0\0", "191.255.255.255\0", "0.63.255.255\0", "С.С.Х.Х\0", "B\0", 64, 10, 10, 4194304, 16384, 65534},	// 10
			{"255.224.000.000\0", "255.255.0.0\0", "128.0.0.0\0", "191.255.255.255\0", "0.31.255.255\0", "С.С.Х.Х\0", "B\0", 32, 11, 10, 2097152, 16384, 65534},	// 11
			{"255.240.000.000\0", "255.255.0.0\0", "128.0.0.0\0", "191.255.255.255\0", "0.15.255.255\0", "С.С.Х.Х\0", "B\0", 16, 12, 10, 1048576, 16384, 65534},	// 12
			{"255.248.000.000\0", "255.255.0.0\0", "128.0.0.0\0", "191.255.255.255\0", "0.7.255.255\0", "С.С.Х.Х\0", "B\0", 8, 13, 10, 524288, 16384, 65534},		// 13
			{"255.252.000.000\0", "255.255.0.0\0", "128.0.0.0\0", "191.255.255.255\0", "0.3.255.255\0", "С.С.Х.Х\0", "B\0", 4, 14, 10, 262144, 16384, 65534},		// 14
			{"255.254.000.000\0", "255.255.0.0\0", "128.0.0.0\0", "191.255.255.255\0", "0.1.255.255\0", "С.С.Х.Х\0", "B\0", 2, 15, 10, 131072, 16384, 65534},		// 15
			{"255.255.000.000\0", "255.255.0.0\0", "128.0.0.0\0", "191.255.255.255\0", "0.0.255.255\0", "С.С.Х.Х\0", "B\0", 1, 16, 10, 65536, 16384, 65534},		// 16
			{"255.255.128.000\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.127.255\0", "С.С.С.Х\0", "C\0", 128, 17, 110, 32768, 2097152, 254},	// 17
			{"255.255.192.000\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.63.255\0", "С.С.С.Х\0", "C\0", 64, 18, 110, 16384, 2097152, 254},		// 18
			{"255.255.224.000\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.31.255\0", "С.С.С.Х\0", "C\0", 32, 19, 110, 8192, 2097152, 254},		// 19
			{"255.255.240.000\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.15.255\0", "С.С.С.Х\0", "C\0", 16, 20, 110, 4096, 2097152, 254},		// 20
			{"255.255.248.000\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.7.255\0", "С.С.С.Х\0", "C\0", 8, 21, 110, 2048, 2097152, 254},		// 21
			{"255.255.252.000\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.3.255\0", "С.С.С.Х\0", "C\0", 4, 22, 110, 1024, 2097152, 254},		// 22
			{"255.255.254.000\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.1.255\0", "С.С.С.Х\0", "C\0", 2, 23, 110, 512, 2097152, 254},			// 23
			{"255.255.255.000\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.0.255\0", "С.С.С.Х\0", "C\0", 1, 24, 110, 256, 2097152, 254},			// 24
			{"255.255.255.128\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.0.127\0", "С.С.С.Х\0", "C\0", 0.5, 25, 110, 128, 2097152, 127},		// 25
			{"255.255.255.192\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.0.63\0", "С.С.С.Х\0", "C\0", 0.25, 26, 110, 64, 2097152, 	63},		// 26
			{"255.255.255.224\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.0.31\0", "С.С.С.Х\0", "C\0", 0.125, 27, 110, 32, 2097152, 31},		// 27
			{"255.255.255.240\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.0.15\0", "С.С.С.Х\0", "C\0", 0.0625, 28, 110, 16, 2097152, 15},		// 28
			{"255.255.255.248\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.0.7\0", "С.С.С.Х\0", "C\0", 0.03125, 29, 110, 8, 2097152, 7},			// 29
			{"255.255.255.252\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.0.3\0", "С.С.С.Х\0", "C\0", 0.015625, 30, 110, 4, 2097152, 3},		// 30
			{"255.255.255.254\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.0.1\0", "С.С.С.Х\0", "C\0", 0.0078125, 31, 110, 2, 2097152, 1},		// 31
			{"255.255.255.255\0", "255.255.255.0\0", "192.0.0.0\0", "223.255.255.255\0", "0.0.0.0\0", "С.С.С.Х\0", "C\0", 0.00390625, 32, 110, 1, 2097152, 	0}		// 32
		};
		/**
		 * setZerroToStrIp Функция дописывает указанное количестов нулей к строке
		 * @param str строка к которой нужно дописать нули
		 */
		void setZerroToStrIp(string &str);
		/**
		 * checkMask Метод проверки на соответствии маски
		 * @param  ip   блок с данными ip адреса
		 * @param  mask блок с данными маски сети
		 * @return      результат проверки
		 */
		bool checkMask(IPdata ip, IPdata mask);
		/**
		 * toCase Функция перевода в указанный регистр
		 * @param  str  строка для перевода в указанных регистр
		 * @param  flag флаг указания типа регистра
		 * @return      результирующая строка
		 */
		const string toCase(string str, bool flag = false);
		/**
		 * getLow1Ip6 Функция упрощения IPv6 адреса первого порядка
		 * @param  ip адрес интернет протокола версии 6
		 * @return    упрощенный вид ip адреса первого порядка
		 */
		const string getLow1Ip6(const string ip);
		/**
		 * getLow2Ip6 Функция упрощения IPv6 адреса второго порядка
		 * @param  ip адрес интернет протокола версии 6
		 * @return    упрощенный вид ip адреса второго порядка
		 */
		const string getLow2Ip6(const string ip);
		/**
		 * setLow1Ip6 Функция восстановления IPv6 адреса первого порядка
		 * @param  ip адрес интернет протокола версии 6
		 * @return    восстановленный вид ip адреса первого порядка
		 */
		const string setLow1Ip6(const string ip);
		/**
		 * setLow2Ip6 Функция восстановления IPv6 адреса второго порядка
		 * @param  ip адрес интернет протокола версии 6
		 * @return    восстановленный вид ip адреса второго порядка
		 */
		const string setLow2Ip6(const string ip);
	public:
		/**
		 * getMaskByNumber Функция получения маски из цифровых обозначений
		 * @param  value цифровое обозначение маски
		 * @return       объект с данными маски
		 */
		NTdata getMaskByNumber(u_int value);
		/**
		 * getMaskByString Функция получения маски из строки обозначения маски
		 * @param  value строка с обозначением маски
		 * @return       объект с данными маски
		 */
		NTdata getMaskByString(string value);
		/**
		 * getDataIp Функция получает цифровые данные ip адреса
		 * @param  ip данные ip адреса в виде строки
		 * @return    оцифрованные данные ip адреса
		 */
		IPdata getDataIp(string ip);
		/**
		 * checkRange Метод проверки входит ли ip адрес в указанный диапазон
		 * @param  ip  ip данные ip адреса интернет протокола версии 6
		 * @param  bip начальный диапазон ip адресов
		 * @param  eip конечный диапазон ip адресов
		 * @return     результат проверки
		 */
		bool checkRange6(const string ip, const string bip, const string eip);
		/**
		 * checkMaskByString Метод проверки на соответствие маски по строке маски
		 * @param  ip   данные ip адреса
		 * @param  mask номер маски
		 * @return      результат проверки
		 */
		bool checkMaskByString(string ip, string mask);
		/**
		 * checkMaskByNumber Метод проверки на соответствие маски по номеру маски
		 * @param  ip   данные ip адреса
		 * @param  mask номер маски
		 * @return      результат проверки
		 */
		bool checkMaskByNumber(string ip, u_int mask);
		/**
		 * checkIPByNetwork Метод проверки, принадлежит ли ip адресу указанной сети
		 * @param  ip  данные ip адреса интернет протокола версии 4
		 * @param  nwk адрес сети (192.168.0.0/16)
		 * @return     результат проверки
		 */
		bool checkIPByNetwork(const string ip, const string nwk);
		/**
		 * checkIPByNetwork6 Метод проверки, принадлежит ли ip адресу указанной сети
		 * @param  ip  данные ip адреса интернет протокола версии 6
		 * @param  nwk адрес сети (2001:db8::/32)
		 * @return     результат проверки
		 */
		bool checkIPByNetwork6(const string ip, const string nwk);
		/**
		 * imposeMask Метод наложения маски
		 * @param  ip   блок с данными ip адреса
		 * @param  mask блок с данными маски сети
		 * @return      блок с данными сети
		 */
		IPdata imposeMask(IPdata ip, IPdata mask);
		/**
		 * getNetwork Функция извлечения данных сети
		 * @param  str адрес подключения (127.0.0.1/255.000.000.000 или 127.0.0.1/8 или 127.0.0.1)
		 * @return    параметры подключения
		 */
		NKdata getNetwork(string str);
		/**
		 * imposePrefix6 Метод наложения префикса
		 * @param  ip6    адрес интернет протокола версии 6
		 * @param  prefix префикс сети
		 * @return        результат наложения префикса
		 */
		const string imposePrefix6(const string ip6, u_int prefix);
		/**
		 * getLowIp6 Функция упрощения IPv6 адреса
		 * @param  ip адрес интернет протокола версии 6
		 * @return    упрощенный вид ip адреса
		 */
		const string getLowIp6(const string ip);
		/**
		 * setLowIp6 Функция восстановления IPv6 адреса
		 * @param  ip адрес интернет протокола версии 6
		 * @return    восстановленный вид ip адреса
		 */
		const string setLowIp6(const string ip);
		/**
		 * strIp6ToHex64 Функция преобразования строки ip адреса в 16-й вид
		 * @param  ip данные ip адреса интернет протокола версии 6
		 * @return    результат в 16-м виде
		 */
		__uint64_t strIp6ToHex64(const string ip);
		/**
		 * isLocal Метод проверки на то является ли ip адрес локальным
		 * @param  ip адрес подключения ip
		 * @return    результат проверки (-1 - запрещенный, 0 - локальный, 1 - глобальный)
		 */
		int isLocal(const string ip);
		/**
		 * isLocal6 Метод проверки на то является ли ip адрес локальным
		 * @param  ip адрес подключения IPv6
		 * @return    результат проверки (-1 - запрещенный, 0 - локальный, 1 - глобальный)
		 */
		int isLocal6(const string ip);
};

#endif // _NETWORK_ANYKS_