/* МОДУЛЬ РАБОТЫ С СЕТЯМИ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/

#include "nwk/nwk.h"

// Устанавливаем область видимости
using namespace std;

/**
 * set Метод установки данных ip адреса
 * @param ptr массив значений ip адреса
 */
void IPdata::set(u_int ptr1, u_int ptr2, u_int ptr3, u_int ptr4){
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
const u_int * IPdata::get(){
	// Выводим результат
	return ptr;
}
/**
 * getStr Метод вывода результата в виде строки
 * @return строка ip адреса
 */
const string IPdata::getStr(){
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
u_int IPdata::size(){
	// Выводим размер массива
	return sizeof(this->ptr) / sizeof(u_int);
}
/**
 * getMaskByNumber Функция получения маски из цифровых обозначений
 * @param  value цифровое обозначение маски
 * @return       объект с данными маски
 */
NTdata Network::getMaskByNumber(u_int value){
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
void Network::setZerroToStrIp(string &str){
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
NTdata Network::getMaskByString(string value){
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
IPdata Network::getDataIp(string ip){
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
bool Network::checkMask(IPdata ip, IPdata mask){
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
bool Network::checkMaskByString(string ip, string mask){
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
bool Network::checkMaskByNumber(string ip, u_int mask){
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
IPdata Network::imposeMask(IPdata ip, IPdata mask){
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
NKdata Network::getNetwork(string str){
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
int Network::isLocal(const string ip){
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