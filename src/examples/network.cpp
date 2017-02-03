#include <regex>
#include <string>
#include <vector>
#include <iostream>
#include <sys/types.h>

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

int main(int len, char * buff[]){
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
	
	cout << " +++++++++++1 " << isLocal("194.186.207.116") << endl;
	

	/*
	// Маска 1
	NTdata mask1 = getMaskByString("255.255.0.0");
	NTdata mask2 = getMaskByNumber(16);

	if(mask1.notEmpty) cout << " ++++++++++= mask1 = " << mask1.mask << " ===== " << mask1.number << endl;
	if(mask2.notEmpty) cout << " ++++++++++= mask2 = " << mask2.mask << " ===== " << mask2.number << endl;
	*/

	return 0;
}