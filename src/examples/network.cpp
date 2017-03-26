// $ g++ -std=c++11 ./examples/network.cpp -o ./bin/network
// $ ./bin/network
#define _XOPEN_SOURCE

#include <regex>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <math.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

using namespace std;

typedef int int128_t __attribute__((mode(TI)));
typedef unsigned int uint128_t __attribute__((mode(TI)));

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
		void set(u_int ptr1 = 0, u_int ptr2 = 0, u_int ptr3 = 0, u_int ptr4 = 0){
			// Если проверки пройдены тогда добавляем данные массива
			if(ptr1 <= 255) this->ptr[0] = ptr1;
			if(ptr2 <= 255) this->ptr[1] = ptr2;
			if(ptr3 <= 255) this->ptr[2] = ptr3;
			if(ptr4 <= 255) this->ptr[3] = ptr4;
		}
		/**
		 * get Метод получения данных масива
		 * @return указатель на масив ip адреса
		 */
		const u_int * get(){
			// Выводим результат
			return ptr;
		}
		/**
		 * getStr Метод вывода результата в виде строки
		 * @return строка ip адреса
		 */
		const string getStr(){
			// Строка результата
			string result;
			// Переходим по всему массиву
			for(u_int i = 0; i < 4; i++){
				// Добавляем в строку значение ip адреса
				result.append(to_string(this->ptr[i]));
				// Проверяем нужно ли добавить точку
				if(i < 3) result.append(".");
			}
			// Выводим результат
			return result;
		}
		/**
		 * length Метод определения размера
		 * @return размер массива
		 */
		u_int size(){
			// Выводим размер массива
			return sizeof(this->ptr) / sizeof(u_int);
		}
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
 * toCase Функция перевода в указанный регистр
 * @param  str  строка для перевода в указанных регистр
 * @param  flag флаг указания типа регистра
 * @return      результирующая строка
 */
string toCase(string str, bool flag = false){
	// Переводим в указанный регистр
	transform(str.begin(), str.end(), str.begin(), (flag ? ::toupper : ::tolower));
	// Выводим результат
	return str;
}
/**
 * getMaskByNumber Функция получения маски из цифровых обозначений
 * @param  value цифровое обозначение маски
 * @return       объект с данными маски
 */
NTdata getMaskByNumber(u_int value){
	// Маска подсети
	NTdata mask;
	// Если маска найдена то запоминаем ее
	if(value < masks.size()){
		// Запоминаем данные маски
		mask = masks[value];
		// Запоминаем что структура заполнена
		mask.notEmpty = true;
	}
	// Выводим результат
	return mask;
}
/**
 * setZerroToStrIp Функция дописывает указанное количестов нулей к строке
 * @param str строка к которой нужно дописать нули
 */
void setZerroToStrIp(string &str){
	// Корректируем количество цифр
	if(str.length() < 3){
		// Дописываем количество нулей
		for(u_int i = 0; i < (4 - str.length()); i++) str = string("0") + str;
	}
}
/**
 * getMaskByString Функция получения маски из строки обозначения маски
 * @param  value строка с обозначением маски
 * @return       объект с данными маски
 */
NTdata getMaskByString(string value){
	// Маска подсети
	NTdata mask;
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e(
		"^(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})$",
		regex::ECMAScript | regex::icase
	);
	// Выполняем поиск ip адреса
	regex_search(value, match, e);
	// Если данные найдены
	if(!match.empty()){
		// Получаем символы
		string pir_1 = match[1].str();
		string pir_2 = match[2].str();
		string pir_3 = match[3].str();
		string pir_4 = match[4].str();
		// Корректируем количество цифр
		setZerroToStrIp(pir_1);
		setZerroToStrIp(pir_2);
		setZerroToStrIp(pir_3);
		setZerroToStrIp(pir_4);
		// Формируем ip адрес
		string ip = (pir_1 + string(".") + pir_2 + string(".") + pir_3 + string(".") + pir_4);
		// Переходим по всему массиву масок и ищем нашу
		for(u_int i = 0; i < masks.size(); i++){
			// Если нашли нашу маску то выводим результат
			if(ip.compare(masks[i].mask) == 0){
				// Запоминаем данные маски
				mask = masks[i];
				// Запоминаем что структура заполнена
				mask.notEmpty = true;
				// Выходим из цикла
				break;
			}
		}
	}
	// Выводим результат
	return mask;
}
/**
 * getDataIp Функция получает цифровые данные ip адреса
 * @param  ip данные ip адреса в виде строки
 * @return    оцифрованные данные ip адреса
 */
IPdata getDataIp(string ip){
	// Данные ip адреса
	IPdata ipdata;
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e(
		"^(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})$",
		regex::ECMAScript | regex::icase
	);
	// Выполняем поиск ip адреса
	regex_search(ip, match, e);
	// Если данные найдены
	if(!match.empty()){
		// Получаем данные ip адреса
		ipdata.set(
			u_int(::atoi(match[1].str().c_str())),
			u_int(::atoi(match[2].str().c_str())),
			u_int(::atoi(match[3].str().c_str())),
			u_int(::atoi(match[4].str().c_str()))
		);
	}
	// Выводим данные ip адреса
	return ipdata;
}
/**
 * checkMask Метод проверки на соответствии маски
 * @param  ip   блок с данными ip адреса
 * @param  mask блок с данными маски сети
 * @return      результат проверки
 */
bool checkMask(IPdata ip, IPdata mask){
	// Переходим по всему блоку данных ip
	for(u_int i = 0; i < ip.size(); i++){
		// Определяем значение маски сети
		u_int msk = mask.get()[i];
		// Проверяем соответствует ли ip адрес маске
		if(msk && (ip.get()[i] > msk)) return false;
	}
	// Сообщаем что все удачно
	return true;
}
/**
 * checkMaskByString Метод проверки на соответствие маски по строке маски
 * @param  ip   данные ip адреса
 * @param  mask номер маски
 * @return      результат проверки
 */
bool checkMaskByString(string ip, string mask){
	// Оцифровываем данные ip
	IPdata ipdata = getDataIp(ip);
	// Оцифровываем данные маски
	IPdata maskdata = getDataIp(mask);
	// Выводим результат проверки
	return checkMask(ipdata, maskdata);
}
/**
 * checkMaskByNumber Метод проверки на соответствие маски по номеру маски
 * @param  ip   данные ip адреса
 * @param  mask номер маски
 * @return      результат проверки
 */
bool checkMaskByNumber(string ip, u_int mask){
	// Оцифровываем данные ip
	IPdata ipdata = getDataIp(ip);
	// Оцифровываем данные маски
	IPdata maskdata = getDataIp(getMaskByNumber(mask).mask);
	// Выводим результат проверки
	return checkMask(ipdata, maskdata);
}
/**
 * imposeMask Метод наложения маски
 * @param  ip   блок с данными ip адреса
 * @param  mask блок с данными маски сети
 * @return      блок с данными сети
 */
IPdata imposeMask(IPdata ip, IPdata mask){
	// Объект сети
	IPdata network;
	// Параметры ip адреса
	u_int ptr[4];
	// Переходим по всему блоку данных ip
	for(u_int i = 0; i < ip.size(); i++){
		// Получаемд анные ip адреса
		ptr[i] = ip.get()[i];
		// Накладываем маску
		if(ptr[i] > mask.get()[i]) ptr[i] = 0;
	}
	// Добавляем данные в объект
	network.set(ptr[0], ptr[1], ptr[2], ptr[3]);
	// Выводим результат
	return network;
}
/**
 * getNetwork Функция извлечения данных сети
 * @param  str адрес подключения (127.0.0.1/255.000.000.000 или 127.0.0.1/8 или 127.0.0.1)
 * @return    параметры подключения
 */
NKdata getNetwork(string str){
	// Результат получения данных сети
	NKdata result;
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e(
		"^(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})(?:\\/(\\d+|\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}))?$",
		regex::ECMAScript | regex::icase
	);
	// Выполняем поиск ip адреса и маски сети
	regex_search(str, match, e);
	// Если данные найдены
	if(!match.empty()){
		// ip адрес
		string ip, mask;
		// Если ip адрес найден
		ip = match[1].str();
		// Если маска найдена
		mask = match[2].str();
		// Устанавливаем маску по умолчанию
		if(mask.empty()) mask = "0";
		// Ищем формат маски
		if(mask.find(".") == string::npos){
			// Запоминаем данные маски
			mask = getMaskByNumber(::atoi(mask.c_str())).mask;
		}
		// ip адрес сети
		string network;
		// Оцифровываем данные ip
		IPdata ipdata = getDataIp(ip);
		// Оцифровываем данные маски
		IPdata maskdata = getDataIp(mask);
		// Оцифровываем данные сети
		IPdata networkdata = imposeMask(ipdata, maskdata);
		// Формируем результат
		result = {ipdata, maskdata, networkdata};
	}
	// Выводим результат
	return result;
}
/**
 * isLocal Метод проверки на то является ли ip адрес локальным
 * @param  ip адрес подключения ip
 * @return    результат проверки (-1 - запрещенный, 0 - локальный, 1 - глобальный)
 */
int isLocal(const string ip){
	// Получаем данные ip адреса
	IPdata ipdata = getDataIp(ip);
	// Переходим по всему массиву адресов
	for(u_int i = 0; i < locals.size(); i++){
		// Если ip адрес соответствует маске
		if(checkMaskByNumber(ip, locals[i].mask)){
			// Получаем маску каждой сети
			string mask = getMaskByNumber(locals[i].mask).mask;
			// Оцифровываем данные маски
			IPdata maskdata = getDataIp(mask);
			// Накладываем маску на ip адрес
			IPdata networkdata = imposeMask(ipdata, maskdata);
			// Накладываем маску на ip адрес из списка
			if(networkdata.getStr()
			.compare(locals[i].network) == 0){
				// Проверяем является ли адрес запрещенным
				if(locals[i].allow) return 0;
				// Сообщаем что адрес запрещен
				else return -1;
			}
		}
	}
	// Сообщаем что адрес глобальный
	return 1;
}

/**
 * getLow1Ip6 Функция упрощения IPv6 адреса первого порядка
 * @param  ip адрес интернет протокола версии 6
 * @return    упрощенный вид ip адреса первого порядка
 */
string getLow1Ip6(const string ip){
	// Результирующий ip адрес
	string ipv6;
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e(
		"^\\[?([ABCDEFabcdef\\d]{4})\\:([ABCDEFabcdef\\d]{4})"
		"\\:([ABCDEFabcdef\\d]{4})\\:([ABCDEFabcdef\\d]{4})"
		"\\:([ABCDEFabcdef\\d]{4})\\:([ABCDEFabcdef\\d]{4})"
		"\\:([ABCDEFabcdef\\d]{4})\\:([ABCDEFabcdef\\d]{4})\\]?$",
		regex::ECMAScript | regex::icase
	);
	// Выполняем поиск ip адреса
	regex_search(ip, match, e);
	// Если данные найдены
	if(!match.empty()){
		// Строка для поиска
		string str;
		// Определяем количество элементов в массиве
		u_int len = match.size();
		// Регулярное выражение для поиска старших нулей
		regex e("^(0+)([ABCDEFabcdef\\d]+)$");
		// Переходим по всему массиву и заменяем 0000 на 0 и убираем старшие нули
		for(u_int i = 1; i < len; i++){
			// Получаем группу байт адреса для обработки
			str = match[i].str();
			// Если это вся группа нулей
			if(str.compare("0000") == 0) str = "0";
			// Заменяем старшие нули
			else str = regex_replace(str, e, "$2");
			// Формируем исходный адрес
			ipv6.append(str);
			// Проверяем нужно ли добавить точку
			if(i < (len - 1)) ipv6.append(":");
		}
	}
	// Выводим результат
	return ipv6;
}
/**
 * getLow2Ip6 Функция упрощения IPv6 адреса второго порядка
 * @param  ip адрес интернет протокола версии 6
 * @return    упрощенный вид ip адреса второго порядка
 */
string getLow2Ip6(const string ip){
	// Результирующий ip адрес
	string ipv6;
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e(
		"^\\[?([ABCDEFabcdef\\d]{1,4})\\:([ABCDEFabcdef\\d]{1,4})"
		"\\:([ABCDEFabcdef\\d]{1,4})\\:([ABCDEFabcdef\\d]{1,4})"
		"\\:([ABCDEFabcdef\\d]{1,4})\\:([ABCDEFabcdef\\d]{1,4})"
		"\\:([ABCDEFabcdef\\d]{1,4})\\:([ABCDEFabcdef\\d]{1,4})\\]?$",
		regex::ECMAScript | regex::icase
	);
	// Выполняем поиск ip адреса
	regex_search(ip, match, e);
	// Если данные найдены
	if(!match.empty()){
		// Копируем ip адрес
		string str = ip;
		// Массив найденных элементов
		vector <string> fstr;
		// Регулярное выражение для поиска старших нулей
		regex e("(?:^|\\:)([0\\:]+)(?:\\:|$)");
		// Выполняем удаление из ip адреса нулей до тех пор пока все не удалим
		while(true){
			// Выполняем поиск ip адреса
			regex_search(str, match, e);
			// Если данные найдены
			if(!match.empty()){
				// Копируем найденные данные
				string delim = match[0].str();
				// Ищем строку еще раз
				size_t pos = str.find(delim);
				// Если позиция найдена
				if(pos != string::npos){
					// Удаляем из строки найденный символ
					str.replace(pos, delim.length(), "");
				}
				// Добавляем в массив найденные данные
				fstr.push_back(delim);
			// Иначе выходим
			} else break;
		}
		// Если массив существует
		if(!fstr.empty()){
			// Индекс вектора с максимальным значением
			u_int max = 0, index = -1;
			// Ищем максимальное значение в массиве
			for(u_int i = 0; i < fstr.size(); i++){
				// Получаем длину строки
				size_t len = fstr[i].length();
				// Если размер строки еще больше то запоминаем его
				if(len > max){
					// Запоминаем текущий размер строки
					max = len;
					// Запоминаем индекс в массиве
					index = i;
				}
			}
			// Создаем регулярное выражение для поиска
			regex e(fstr[index]);
			// Заменяем найденный элемент на ::
			ipv6 = regex_replace(ip, e, "::");
		// Запоминаем адрес так как он есть
		} else ipv6 = str;
	}
	// Выводим результат
	return ipv6;
}
/**
 * getLowIp6 Функция упрощения IPv6 адреса
 * @param  ip адрес интернет протокола версии 6
 * @return    упрощенный вид ip адреса
 */
string getLowIp6(const string ip){
	// Выполняем преобразование первого порядка
	string str = getLow1Ip6(ip);
	// Если строка не существует то присваиваем исходную
	if(str.empty()) str = ip;
	// Выполняем преобразование второго порядка
	return getLow2Ip6(str);
}
/**
 * setLow2Ip6 Функция восстановления IPv6 адреса второго порядка
 * @param  ip адрес интернет протокола версии 6
 * @return    восстановленный вид ip адреса второго порядка
 */
string setLow2Ip6(const string ip){
	// Результирующий ip адрес
	string ipv6;
	// Копируем ip адрес
	string str = ip;
	// Ищем строку еще раз
	size_t pos = str.find("::");
	// Если позиция найдена
	if(pos != string::npos){
		// Массив найденных элементов
		vector <string> fstr;
		// Результат работы регулярного выражения
		smatch match;
		// Устанавливаем правило регулярного выражения
		regex e("([ABCDEFabcdef\\d]{1,4})", regex::ECMAScript | regex::icase);
		// Выполняем удаление из ip адреса хексетов
		while(true){
			// Выполняем поиск ip адреса
			regex_search(str, match, e);
			// Если данные найдены
			if(!match.empty()){
				// Копируем полученные данные
				string delim = match[1].str();
				// Ищем строку еще раз
				size_t pos = str.find(delim);
				// Если позиция найдена
				if(pos != string::npos){
					// Удаляем из строки найденный символ
					str.replace(pos, delim.length(), "");
				}
				// Добавляем в массив найденные данные
				fstr.push_back(delim);
			// Если хексет не найден то выходим
			} else break;
		}
		// Определяем количество хексетов
		u_int lhex = fstr.size();
		// Если количество хексетов меньше 8 то определяем сколько не хватает
		if(lhex < 8){
			// Маска
			string mask = ":";
			// Составляем маску
			for(u_int i = 0; i < (8 - lhex); i++) mask.append("0:");
			// Копируем полученные данные
			ipv6 = ip;
			// Удаляем из строки найденный символ
			ipv6.replace(pos, 2, mask);
			// Устанавливаем правило регулярного выражения
			regex e("(?:^\\[?\\:|\\:\\]?$)");
			// Заменяем найденные не верные элементы
			ipv6 = regex_replace(ipv6, e, "");
		}
	}
	// Выводим результат
	return ipv6;
}
/**
 * setLow1Ip6 Функция восстановления IPv6 адреса первого порядка
 * @param  ip адрес интернет протокола версии 6
 * @return    восстановленный вид ip адреса первого порядка
 */
string setLow1Ip6(const string ip){
	// Результирующий ip адрес
	string ipv6;
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e(
		"^\\[?([ABCDEFabcdef\\d]{1,4})\\:([ABCDEFabcdef\\d]{1,4})"
		"\\:([ABCDEFabcdef\\d]{1,4})\\:([ABCDEFabcdef\\d]{1,4})"
		"\\:([ABCDEFabcdef\\d]{1,4})\\:([ABCDEFabcdef\\d]{1,4})"
		"\\:([ABCDEFabcdef\\d]{1,4})\\:([ABCDEFabcdef\\d]{1,4})\\]?$",
		regex::ECMAScript | regex::icase
	);
	// Выполняем поиск ip адреса
	regex_search(ip, match, e);
	// Если данные найдены
	if(!match.empty()){
		// Копируем ip адрес
		string str = ip;
		// Массив найденных элементов
		vector <string> fstr;
		// Результат работы регулярного выражения
		smatch match;
		// Устанавливаем правило регулярного выражения
		regex e("([ABCDEFabcdef\\d]{1,4})", regex::ECMAScript | regex::icase);
		// Выполняем удаление из ip адреса хексетов
		while(true){
			// Выполняем поиск ip адреса
			regex_search(str, match, e);
			// Если данные найдены
			if(!match.empty()){
				// Копируем полученные данные
				string delim = match[1].str();
				// Ищем строку еще раз
				size_t pos = str.find(delim);
				// Если позиция найдена
				if(pos != string::npos){
					// Удаляем из строки найденный символ
					str.replace(pos, delim.length(), "");
				}
				// Добавляем в массив найденные данные
				fstr.push_back(delim);
			// Если хексет не найден то выходим
			} else break;
		}
		// Размер массива хексетов
		u_int len = fstr.size();
		// Переходим по всему массиву
		for(u_int i = 0; i < len; i++){
			// Определяем длину хексета
			u_int size = fstr[i].length();
			// Если размер хексета меньше 4 то дописываем нули
			if(size < 4){
				// Дописываем столько нулей, сколько необходимо
				for(u_int j = 0; j < (4 - size); j++) fstr[i] = (string("0") + fstr[i]);
			}
			// Формируем результат
			ipv6.append(fstr[i]);
			// Проверяем нужно ли добавить точку
			if(i < (len - 1)) ipv6.append(":");
		}
	}
	// Выводим результат
	return ipv6;
}
/**
 * setLowIp6 Функция восстановления IPv6 адреса
 * @param  ip адрес интернет протокола версии 6
 * @return    восстановленный вид ip адреса
 */
string setLowIp6(const string ip){
	// Выполняем преобразование первого порядка
	string str = setLow2Ip6(ip);
	// Если строка не существует то присваиваем исходную
	if(str.empty()) str = ip;
	// Выполняем преобразование второго порядка
	return setLow1Ip6(str);
}
/**
 * imposePrefix6 Метод наложения префикса
 * @param  ip6    адрес интернет протокола версии 6
 * @param  prefix префикс сети
 * @return        результат наложения префикса
 */
const string imposePrefix6(const string ip6, u_int prefix){
	// Получаем строку с ip адресом
	string str;
	// Если префикс передан
	if(prefix){
		// Преобразуем ip адрес
		str = setLowIp6(ip6);
		// Если строка существует то продолжаем
		if(!str.empty()){
			// Если префикс меньше 128
			if(prefix < 128){
				// Копируем ip адрес
				string ip = str;
				// Массив найденных элементов
				vector <string> fstr;
				// Результат работы регулярного выражения
				smatch match;
				// Устанавливаем правило регулярного выражения
				regex e("([ABCDEFabcdef\\d]{4})", regex::ECMAScript | regex::icase);
				// Выполняем удаление из ip адреса хексетов
				while(true){
					// Выполняем поиск ip адреса
					regex_search(ip, match, e);
					// Если данные найдены
					if(!match.empty()){
						// Копируем полученные данные
						string delim = match[1].str();
						// Ищем строку еще раз
						size_t pos = ip.find(delim);
						// Если позиция найдена
						if(pos != string::npos){
							// Удаляем из строки найденный символ
							ip.replace(pos, delim.length(), "");
						}
						// Добавляем в массив найденные данные
						fstr.push_back(delim);
					// Если хексет не найден то выходим
					} else break;
				}
				// Искомое число префикса
				u_int fprefix = prefix;
				// Ищем ближайшее число префикса
				while(fmod(fprefix, 4)) fprefix++;
				// Получаем длину адреса
				int len = (fprefix / 16);
				// Компенсируем диапазон
				if(!len) len = 1;
				// Очищаем строку
				str.clear();
				// Переходим по всему полученному массиву
				for(u_int i = 0; i < len; i++){
					// Формируем ip адрес
					str.append(fstr[i]);
					// Добавляем разделитель
					str.append(":");
				}
				// Добавляем оставшиеся нули
				for(u_int i = len; i < 8; i++){
					// Формируем ip адрес
					str.append("0000");
					// Добавляем разделитель
					if(i < 7) str.append(":");
				}
			}
		}
		// Выполняем упрощение ip адреса
		str = getLowIp6(str);
	}
	// Выводим полученную строку
	return str;
}
/**
 * strIp6ToHex64 Функция преобразования строки ip адреса в 16-й вид
 * @param  ip данные ip адреса интернет протокола версии 6
 * @return    результат в 16-м виде
 */
__uint64_t strIp6ToHex64(const string ip){
	// Результат работы функции
	__uint64_t result = 0;
	// Создаем поток
	stringstream strm;
	// Устанавливаем правило регулярного выражения
	regex e("[^ABCDEFabcdef\\d]", regex::ECMAScript | regex::icase);
	// Убираем лишние символы из 16-го выражения
	string str = regex_replace(ip, e, "");
	// Если число слишком длинное
	if(str.length() > 16) str = str.erase(15, str.length());
	// Записываем полученную строку в поток
	strm << str;
	// Выполняем преобразование в 16-й вид
	strm >> std::hex >> result;
	// Выводим результат
	return result;
}
/**
 * checkRange Метод проверки входит ли ip адрес в указанный диапазон
 * @param  ip  ip данные ip адреса интернет протокола версии 6
 * @param  bip начальный диапазон ip адресов
 * @param  eip конечный диапазон ip адресов
 * @return     результат проверки
 */
bool checkRange6(const string ip, const string bip, const string eip){
	// Результат проверки
	bool result = false;
	// Если все данные переданы
	if(!ip.empty() && !bip.empty() && !eip.empty()){
		// Переводим в целочисленный вид
		__uint64_t nip	= strIp6ToHex64(setLowIp6(ip));
		__uint64_t nbip	= strIp6ToHex64(setLowIp6(bip));
		__uint64_t neip	= strIp6ToHex64(setLowIp6(eip));
		// Выполняем сравнение
		result = ((nip >= nbip) && (nip <= neip));
	}
	// Сообщаем что проверка не удалась
	return result;
}
/**
 * isLocal6 Метод проверки на то является ли ip адрес локальным
 * @param  ip адрес подключения IPv6
 * @return    результат проверки (-1 - запрещенный, 0 - локальный, 1 - глобальный)
 */
int isLocal6(const string ip){
	// Искомый результат
	int result = 1;
	// Результат сравнения
	bool compare = false;
	// Переходим по всему массиву адресов
	for(u_int i = 0; i < locals6.size(); i++){
		// Если сравнение пройдено и есть еще конечная сеть
		if(!locals6[i].eip.empty())
			// Выполняем дополнительную проверку на диапазон сетей
			compare = checkRange6(ip, locals6[i].ip, locals6[i].eip);
		else {
			// Преобразуем сеть в полный вид
			string network = toCase(setLowIp6(locals6[i].ip));
			// Накладываем на ip адрес префикс сети
			string ipv6 = imposePrefix6(ip, locals6[i].prefix);
			// Преобразуем ip адрес в полный вид
			ipv6 = toCase(setLowIp6(ipv6));
			// Формируем векторы данных
			vector <char> mip(ipv6.begin(), ipv6.end());
			vector <char> nwk(network.begin(), network.end());
			// Начинаем проверять совпадения
			for(u_int j = 0; j < mip.size(); j++){
				// Если значение в маске совпадает тогда продолжаем проверку
				if((mip[j] == nwk[j]) || (nwk[j] == '0')) compare = true;
				else {
					// Запоминаем что сравнение не удалось
					compare = false;
					// Выходим
					break;
				}
			}
		}
		// Формируем результат
		if(compare) result = (!locals6[i].allow && compare ? -1 : (compare ? 0 : 1));
	}
	// Если локальный адрес найден
	return result;
}
/**
 * checkIPByNetwork6 Метод проверки, принадлежит ли ip адресу указанной сети
 * @param  ip  данные ip адреса интернет протокола версии 6
 * @param  nwk адрес сети (2001:db8::/32)
 * @return     результат проверки
 */
bool checkIPByNetwork6(const string ip, const string nwk){
	// Результат сравнения
	bool compare = false;
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e("^([\\s\\S]+)\\/(\\d+)$", regex::ECMAScript | regex::icase);
	// Выполняем поиск ip адреса и префикса сети
	regex_search(nwk, match, e);
	// Если данные найдены
	if(!match.empty()){
		// Преобразуем сеть в полный вид
		string network = toCase(setLowIp6(match[1].str()));
		// Накладываем на ip адрес префикс сети
		string ipv6 = imposePrefix6(ip, ::atoi(match[2].str().c_str()));
		// Преобразуем ip адрес в полный вид
		ipv6 = toCase(setLowIp6(ipv6));
		// Формируем векторы данных
		vector <char> mip(ipv6.begin(), ipv6.end());
		vector <char> nwk(network.begin(), network.end());
		// Начинаем проверять совпадения
		for(u_int j = 0; j < mip.size(); j++){
			// Если значение в маске совпадает тогда продолжаем проверку
			if((mip[j] == nwk[j]) || (nwk[j] == '0')) compare = true;
			else {
				// Запоминаем что сравнение не удалось
				compare = false;
				// Выходим
				break;
			}
		}
	}
	// Выводим результат сравнения
	return compare;
}

/**
 * checkIPByNetwork Метод проверки, принадлежит ли ip адресу указанной сети
 * @param  ip  данные ip адреса интернет протокола версии 4
 * @param  nwk адрес сети (192.168.0.0/16)
 * @return     результат проверки
 */
bool checkIPByNetwork(const string ip, const string nwk){
	// Получаем данные ip адреса
	IPdata ipdata = getDataIp(ip);
	// Получаем данные сети
	NKdata nwkdata = getNetwork(nwk);
	// Накладываем маску на ip адрес
	IPdata networkdata = imposeMask(ipdata, nwkdata.mask);
	// Накладываем маску на ip адрес из списка
	if(networkdata.getStr()
	.compare(nwkdata.network.getStr()) == 0) return true;
	// Сообщачем что ничего не найдено
	return false;
}

class A {
	private:
		int d = 153;
	public:
		//int (const * ff) (const int) const;
		//void * ff;
		// void (* function)(int x);

		function <void (int)> remove;

		void m(){
			//sleep(5);
			this->remove(10);
		}
};

class B {
	public:
		int m = 13;
	private:
		int d = 153;
	public:
		
		void f2(int x){
			cout << x << endl;
		}

		void f1(){
			A t;

			t.remove = [this](int x){
				// cout << (d + m + x) << endl;
				f2(x);
			};

			t.m();
		};
		
};


int f1 (int a) {
    cout << "f1: " << a << endl;
    return a*2;
}

void f2 (int (* ff) (int), int a) {
    int i = ff (a);
    cout << "f2: " << i << endl;

}

void f3(void * ctx, size_t param){
	memcpy(ctx, &param, sizeof(param));
}


int main(int len, char * buff[]){
	
	B d;

	d.f1();

	/*
	A m;
	B d;

	d.m = 15;

	d.f1();
	*/

	//m.function=d.f1;

	//m.function(NULL, NULL);

	//m.pf = d.f;

	//m.ff(13);
	

	f2(f1, 5);


	/*
	NKdata data = getNetwork("46.39.231.203/255.255.225.0");

	for(u_int i = 0; i < data.network.size(); i++){
		cout << " +++++++++++0 " << data.network.get()[i] << endl;
	}

	cout << " =========1 " << (int) checkMaskByString("46.39.231.203", "255.255.252.0") << endl;
	cout << " =========2 " << (int) checkMaskByNumber("192.168.1.200", 25) << endl;
	*/
	/*
	// Получаем данные ip адреса
	IPdata ipdata = getDataIp("192.88.99.1");
	// Получаем маску каждой сети
	string mask = getMaskByNumber(32).mask;
	// Оцифровываем данные маски
	IPdata maskdata = getDataIp(mask);
	// Накладываем маску на ip адрес
	IPdata networkdata = imposeMask(ipdata, maskdata);

	cout << " +++++++++++++++++++++ " << networkdata.getStr() << endl;
	*/
	
	// cout << " +++++++++++1 " << getLow1Ip6("[FF80:0000:0000:0000:0123:1234:ABCD:EF12]") << endl;
	
	// cout << " ============1 " << imposePrefix6("[FE80:0001:000F:000A:0123:1234:ABCD:EF12]", 48) << endl;

	// cout << " ============1 " << checkIPByNetwork6("[2001:db8:11a3:09d7:1f34:8a2e:07a0:765d]", "2001:db8::/32") << endl;

	// cout << " ============1 " << checkIPByNetwork("43.15.55.21", "43.15.0.0/16") << endl;

	// cout << " ------------ " << strToHex6("[2001:db8:11a3:09d7:1f34:8a2e:07a0:765d]") << endl;

	

	/*
	__uint64_t u64 = strIp6ToHex64("[2001:db8:11a3:09d7:1f34:8a2e:07a0:765d]");
  	printf("%llx\n", (unsigned long long)(u64 & 0xFFFFFFFFFFFFFFFF));
  	*/
  	
  	cout << " ************** " << isLocal6("[feff::]") << endl; //fe80


	std::stringstream stream;
	stream << std::hex << 154;
	std::string result( stream.str() );

	std::stringstream ss;
	ss << "your id is " << std::hex << 0x0daffa0;
	const std::string s = ss.str();

	std::stringstream str;
	string s1 = "9a";
	str << s1;
	int value;
	str >> std::hex >> value;

	cout << " ========== " << result << " == " << s << " == " << value << endl;





	// Работа с датами (перевод даты в "Wed, 15 Mar 2017 14:10:08 GMT")
	struct tm tm;
	char buf[255];

	memset(&tm, 0, sizeof(struct tm));
	strptime("Wed, 15 Mar 2017 14:10:08 GMT", "%a, %d %b %Y %X %Z", &tm);
	strftime(buf, sizeof(buf), "%A, %d %b %Y %H:%M:%S %Z", &tm);

	time_t seconds = mktime(&tm);

	cout << " ++++++++++++++1 " << buf << " == " << seconds << endl;

	//__time64_t ss;
	// time64_t dd;

	char buf2[255];
	memset(buf2, 0, sizeof(buf2));
	struct tm * now_tm = gmtime(&seconds);
	strftime(buf2, sizeof(buf), "%a, %d %b %Y %X %OZ", now_tm);

	cout << " ++++++++++++++2 " << buf2 << endl;


	struct DM {
		size_t k;
		size_t m;
		size_t d;
		size_t p = 18434353;
	};

	DM dm;
	dm.k = 77;
	dm.m = 324;
	dm.d = 1123;

	unsigned char * raw_data = reinterpret_cast <unsigned char *> (&dm);

	
	for(size_t i = 0, k = 0; i < sizeof(DM); i += sizeof(size_t), k++){
		size_t d;
		memcpy(&d, raw_data + i, sizeof(size_t));
		cout << "----------- " << k << " == " << d << endl;
	}
	
	string data1 = "Hello World";

	unsigned char * raw_data1 = reinterpret_cast <unsigned char *> (strdup(data1.data()));

	string data2 = reinterpret_cast <char *> (raw_data1);

	cout << " --------- " << data2 << endl;

	size_t par = 13;

	f3(&par, 22);

	cout << " =========== " << par << endl;


	/*
	// Маска 1
	NTdata mask1 = getMaskByString("255.255.0.0");
	NTdata mask2 = getMaskByNumber(16);

	if(mask1.notEmpty) cout << " ++++++++++= mask1 = " << mask1.mask << " ===== " << mask1.number << endl;
	if(mask2.notEmpty) cout << " ++++++++++= mask2 = " << mask2.mask << " ===== " << mask2.number << endl;
	*/

	return 0;
}