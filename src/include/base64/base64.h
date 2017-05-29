/* МОДУЛЬ BASE64 ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _BASE64_ANYKS_
#define _BASE64_ANYKS_

#include <string>
#include <sys/types.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

// Устанавливаем область видимости
using namespace std;

/**
 * Base64 Класс
 */
class Base64 {
	private:
		/**
		 * base64 Функция кодирования и декодирования base64
		 * @param  in   буфер входящих данных
		 * @param  out  буфер исходящих данных
		 * @param  lin  размер буфера входящих данных
		 * @param  lout размер буфера исходящих данных
		 * @param  mode режим работы (false - кодирование, true - декодирование)
		 * @return      размер полученных данных
		 */
		const int base64(const unsigned char * in, char * out, u_int lin, u_int lout, const bool mode = false);
	public:
		/**
		 * encode Метод кодирования в base64
		 * @param  str входящая строка для кодирования
		 * @return     результирующая строка
		 */
		const string encode(const string str);
		/**
		 * decode Метод декодирования из base64
		 * @param  str входящая строка для декодирования
		 * @return     результирующая строка
		 */
		const string decode(const string str);
};

#endif // _BASE64_ANYKS_
