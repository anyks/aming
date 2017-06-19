/* МОДУЛЬ РАБОТЫ С СЕТЯМИ AMING */
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
 * IPdata Конструктор
 */
IPdata::IPdata(){
	// Запоминаем не существующий ip адрес
	for(u_int i = 0; i < 4; i++) this->ptr[i] = 256;
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
 * rmZerroToStrIp Функция удаляем указанное количестов нулей из строки
 * @param str строка из которой нужно удалить нули
 */
void Network::rmZerroToStrIp(string &str){
	// Корректируем количество цифр
	if(!str.empty()){
		// Копируем строку
		const string octet = str;
		// Формируем регулярное выражение
		regex e("^0{1,2}([0-9]{1,2})$");
		// Выполняем удаление лишних нулей
		str = regex_replace(octet, e, "$1");
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
 * checkRange Метод проверки входит ли ip адрес в указанный диапазон
 * @param  ip  ip данные ip адреса интернет протокола версии 6
 * @param  bip начальный диапазон ip адресов
 * @param  eip конечный диапазон ip адресов
 * @return     результат проверки
 */
bool Network::checkRange6(const string ip, const string bip, const string eip){
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
 * checkIPByNetwork Метод проверки, принадлежит ли ip адресу указанной сети
 * @param  ip  данные ip адреса интернет протокола версии 4
 * @param  nwk адрес сети (192.168.0.0/16)
 * @return     результат проверки
 */
bool Network::checkIPByNetwork(const string ip, const string nwk){
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
/**
 * checkIPByNetwork6 Метод проверки, принадлежит ли ip адресу указанной сети
 * @param  ip  данные ip адреса интернет протокола версии 6
 * @param  nwk адрес сети (2001:db8::/32)
 * @return     результат проверки
 */
bool Network::checkIPByNetwork6(const string ip, const string nwk){
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
		// Формируем вектор данных ip адреса
		vector <string> mip = split(ipv6, ":");
		// Если данные получены
		if(!mip.empty()){
			// Если первый хекстет нулевой значит это локальный адрес
			if(mip[0].compare("0000") == 0) compare = true;
			// Выполняем сравнение по хекстетам
			else {
				// Формируем вектор данных сети
				vector <string> nwk = split(network, ":");
				// Начинаем проверять совпадения
				for(u_int j = 0; j < mip.size(); j++){
					// Если значение в маске совпадает тогда продолжаем проверку
					if((mip[j].compare(nwk[j]) == 0)
					|| (nwk[j].compare("0000") == 0)) compare = true;
					else {
						// Запоминаем что сравнение не удалось
						compare = false;
						// Выходим
						break;
					}
				}
			}
		}
	}
	// Выводим результат сравнения
	return compare;
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
		"^(\\d{1,3}(?:\\.\\d{1,3}){3})(?:\\/(\\d+|\\d{1,3}(?:\\.\\d{1,3}){3}))?$",
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
 * setLowIp4 Функция восстановления IPv4 адреса
 * @param  ip адрес интернет протокола версии 4
 * @return    восстановленный вид ip адреса
 */
const string Network::setLowIp(const string ip){
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
		return (pir_1 + string(".") + pir_2 + string(".") + pir_3 + string(".") + pir_4);
	}
	// Выводим результат
	return ip;
}
/**
 * getLowIp Функция упрощения IPv4 адреса
 * @param  ip адрес интернет протокола версии 4
 * @return    упрощенный вид ip адреса
 */
const string Network::getLowIp(const string ip){
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
		// Получаем символы
		string pir_1 = match[1].str();
		string pir_2 = match[2].str();
		string pir_3 = match[3].str();
		string pir_4 = match[4].str();
		// Корректируем количество цифр
		rmZerroToStrIp(pir_1);
		rmZerroToStrIp(pir_2);
		rmZerroToStrIp(pir_3);
		rmZerroToStrIp(pir_4);
		// Формируем ip адрес
		return (pir_1 + string(".") + pir_2 + string(".") + pir_3 + string(".") + pir_4);
	}
	// Выводим результат
	return ip;
}
/**
 * getLow1Ip6 Функция упрощения IPv6 адреса первого порядка
 * @param  ip адрес интернет протокола версии 6
 * @return    упрощенный вид ip адреса первого порядка
 */
const string Network::getLow1Ip6(const string ip){
	// Результирующий ip адрес
	string ipv6;
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e(
		"^\\[?([A-Fa-f\\d]{4})\\:([A-Fa-f\\d]{4})"
		"\\:([A-Fa-f\\d]{4})\\:([A-Fa-f\\d]{4})"
		"\\:([A-Fa-f\\d]{4})\\:([A-Fa-f\\d]{4})"
		"\\:([A-Fa-f\\d]{4})\\:([A-Fa-f\\d]{4})\\]?$",
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
		regex e("^(0+)([A-Fa-f\\d]+)$");
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
const string Network::getLow2Ip6(const string ip){
	// Результирующий ip адрес
	string ipv6;
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e(
		"^\\[?([A-Fa-f\\d]{1,4})\\:([A-Fa-f\\d]{1,4})"
		"\\:([A-Fa-f\\d]{1,4})\\:([A-Fa-f\\d]{1,4})"
		"\\:([A-Fa-f\\d]{1,4})\\:([A-Fa-f\\d]{1,4})"
		"\\:([A-Fa-f\\d]{1,4})\\:([A-Fa-f\\d]{1,4})\\]?$",
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
 * setLow1Ip6 Функция восстановления IPv6 адреса первого порядка
 * @param  ip адрес интернет протокола версии 6
 * @return    восстановленный вид ip адреса первого порядка
 */
const string Network::setLow1Ip6(const string ip){
	// Результирующий ip адрес
	string ipv6;
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e(
		"^\\[?([A-Fa-f\\d]{1,4})\\:([A-Fa-f\\d]{1,4})"
		"\\:([A-Fa-f\\d]{1,4})\\:([A-Fa-f\\d]{1,4})"
		"\\:([A-Fa-f\\d]{1,4})\\:([A-Fa-f\\d]{1,4})"
		"\\:([A-Fa-f\\d]{1,4})\\:([A-Fa-f\\d]{1,4})\\]?$",
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
		regex e("([A-Fa-f\\d]{1,4})", regex::ECMAScript | regex::icase);
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
 * setLow2Ip6 Функция восстановления IPv6 адреса второго порядка
 * @param  ip адрес интернет протокола версии 6
 * @return    восстановленный вид ip адреса второго порядка
 */
const string Network::setLow2Ip6(const string ip){
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
		regex e("([A-Fa-f\\d]{1,4})", regex::ECMAScript | regex::icase);
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
 * imposePrefix6 Метод наложения префикса
 * @param  ip6    адрес интернет протокола версии 6
 * @param  prefix префикс сети
 * @return        результат наложения префикса
 */
const string Network::imposePrefix6(const string ip6, u_int prefix){
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
				regex e("([A-Fa-f\\d]{4})", regex::ECMAScript | regex::icase);
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
 * getLowIp6 Функция упрощения IPv6 адреса
 * @param  ip адрес интернет протокола версии 6
 * @return    упрощенный вид ip адреса
 */
const string Network::getLowIp6(const string ip){
	// Выполняем преобразование первого порядка
	string str = getLow1Ip6(ip);
	// Если строка не существует то присваиваем исходную
	if(str.empty()) str = ip;
	// Выполняем преобразование второго порядка
	return getLow2Ip6(str);
}
/**
 * setLowIp6 Функция восстановления IPv6 адреса
 * @param  ip адрес интернет протокола версии 6
 * @return    восстановленный вид ip адреса
 */
const string Network::setLowIp6(const string ip){
	// Выполняем преобразование первого порядка
	string str = setLow2Ip6(ip);
	// Если строка не существует то присваиваем исходную
	if(str.empty()) str = ip;
	// Выполняем преобразование второго порядка
	return setLow1Ip6(str);
}
/**
 * strIp6ToHex64 Функция преобразования строки ip адреса в 16-й вид
 * @param  ip данные ip адреса интернет протокола версии 6
 * @return    результат в 16-м виде
 */
__uint64_t Network::strIp6ToHex64(const string ip){
	// Результат работы функции
	__uint64_t result = 0;
	// Создаем поток
	stringstream strm;
	// Устанавливаем правило регулярного выражения
	regex e("[^A-Fa-f\\d]", regex::ECMAScript | regex::icase);
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
 * checkNetworkByIp Функция определения типа сети по ip адресу
 * @param  ip данные ip адреса
 * @return    тип сети в 10-м виде
 */
const u_int Network::checkNetworkByIp(const string ip){
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e(
		"^\\d{1,3}(?:\\.\\d{1,3}){3}$",
		regex::ECMAScript | regex::icase
	);
	// Выполняем поиск ip адреса
	regex_search(ip, match, e);
	// Если данные найдены
	if(!match.empty()) return 4;
	// Если это не IPv4
	else {
		// Устанавливаем правило регулярного выражения
		regex e(
			"^\\[?[A-Fa-f\\d]{1,4}(?:\\:[A-Fa-f\\d]{1,4}){7}\\]?$",
			regex::ECMAScript | regex::icase
		);
		// Приводим ip адрес к новой форме
		const string ip2 = setLow2Ip6(ip);
		// Выполняем поиск ip адреса
		regex_search(ip2, match, e);
		// Если данные найдены
		if(!match.empty()) return 6;
		// Сообщаем что протокол не определен
		else return 0;
	}
}
/**
 * isLocal Метод проверки на то является ли ip адрес локальным
 * @param  ip адрес подключения ip
 * @return    результат проверки (-1 - запрещенный, 0 - локальный, 1 - глобальный)
 */
const int Network::isLocal(const string ip){
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e(
		"^\\d{1,3}(?:\\.\\d{1,3}){3}$",
		regex::ECMAScript | regex::icase
	);
	// Выполняем поиск ip адреса
	regex_search(ip, match, e);
	// Если данные найдены
	if(!match.empty()){
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
	// Если это не ip адрес то запрещаем работу
	return -1;
}
/**
 * isLocal6 Метод проверки на то является ли ip адрес локальным
 * @param  ip адрес подключения IPv6
 * @return    результат проверки (-1 - запрещенный, 0 - локальный, 1 - глобальный)
 */
const int Network::isLocal6(const string ip){
	// Искомый результат
	int result = 1;
	// Результат сравнения
	bool compare = false;
	// Переходим по всему массиву адресов
	for(u_int i = 0; i < this->locals6.size(); i++){
		// Если сравнение пройдено и есть еще конечная сеть
		if(!this->locals6[i].eip.empty())
			// Выполняем дополнительную проверку на диапазон сетей
			compare = checkRange6(ip, this->locals6[i].ip, this->locals6[i].eip);
		else {
			// Преобразуем сеть в полный вид
			string network = toCase(setLowIp6(this->locals6[i].ip));
			// Накладываем на ip адрес префикс сети
			string ipv6 = imposePrefix6(ip, this->locals6[i].prefix);
			// Преобразуем ip адрес в полный вид
			ipv6 = toCase(setLowIp6(ipv6));
			// Формируем вектор данных ip адреса
			vector <string> mip = split(ipv6, ":");
			// Если данные получены
			if(!mip.empty()){
				// Если первый хекстет нулевой значит это локальный адрес
				if(mip[0].compare("0000") == 0) compare = true;
				// Выполняем сравнение по хекстетам
				else {
					// Формируем вектор данных сети
					vector <string> nwk = split(network, ":");
					// Начинаем проверять совпадения
					for(u_int j = 0; j < mip.size(); j++){
						// Если значение в маске совпадает тогда продолжаем проверку
						if((mip[j].compare(nwk[j]) == 0) || (nwk[j].compare("0000") == 0)) compare = true;
						else {
							// Запоминаем что сравнение не удалось
							compare = false;
							// Выходим
							break;
						}
					}
				}
			}
		}
		// Формируем результат
		if(compare) result = (!locals6[i].allow ? -1 : 0);
	}
	// Если локальный адрес найден
	return result;
}