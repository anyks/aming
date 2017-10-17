#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

// Устанавливаем область видимости
using namespace std;

// g++ ./examples/md5.cpp -I/usr/local/Cellar/openssl/1.0.2l/include /usr/local/Cellar/openssl/1.0.2l/lib/libcrypto.a -o ./bin/md5

/**
 * md5 Метод получения md5 хэша из строки
 * @param text текст для перевода в строку
 */
const string md5(const string text){
	// Результат работы функции
	string result;
	// Если текст передан
	if(!text.empty()){
		// Массив полученных значений
		u_char digest[16];
		// Создаем контекст
		MD5_CTX ctx;
		// Выполняем инициализацию контекста
		MD5_Init(&ctx);
		// Выполняем расчет суммы
		MD5_Update(&ctx, text.c_str(), text.length());
		// Копируем полученные данные
		MD5_Final(digest, &ctx);
		// Строка md5
		char mdString[33];
		// Заполняем массив нулями
		memset(mdString, 0, 33);
		// Заполняем строку данными md5
		for(u_short i = 0; i < 16; i++) sprintf(&mdString[i * 2], "%02x", u_int(digest[i]));
		// Выводим результат
		result = mdString;
	}
	// Выводим результат
	return result;
}

/**
 * sha1 Метод получения sha1 хэша из строки
 * @param text текст для перевода в строку
 */
const string sha1(const string text){
	// Результат работы функции
	string result;
	// Если текст передан
	if(!text.empty()){
		// Массив полученных значений
		u_char digest[20];
		// Создаем контекст
		SHA_CTX ctx;
		// Выполняем инициализацию контекста
		SHA1_Init(&ctx);
		// Выполняем расчет суммы
		SHA1_Update(&ctx, text.c_str(), text.length());
		// Копируем полученные данные
		SHA1_Final(digest, &ctx);
		// Строка sha1
		char mdString[41];
		// Заполняем массив нулями
		memset(mdString, 0, 41);
		// Заполняем строку данными sha1
		for(u_short i = 0; i < 20; i++) sprintf(&mdString[i * 2], "%02x", u_int(digest[i]));
		// Выводим результат
		result = mdString;
	}
	// Выводим результат
	return result;
}

/**
 * sha256 Метод получения sha256 хэша из строки
 * @param text текст для перевода в строку
 */
const string sha256(const string text){
	// Результат работы функции
	string result;
	// Если текст передан
	if(!text.empty()){
		// Массив полученных значений
		u_char digest[32];
		// Создаем контекст
		SHA256_CTX ctx;
		// Выполняем инициализацию контекста
		SHA256_Init(&ctx);
		// Выполняем расчет суммы
		SHA256_Update(&ctx, text.c_str(), text.length());
		// Копируем полученные данные
		SHA256_Final(digest, &ctx);
		// Строка sha256
		char mdString[65];
		// Заполняем массив нулями
		memset(mdString, 0, 65);
		// Заполняем строку данными sha256
		for(u_short i = 0; i < 32; i++) sprintf(&mdString[i * 2], "%02x", u_int(digest[i]));
		// Выводим результат
		result = mdString;
	}
	// Выводим результат
	return result;
}

/**
 * sha512 Метод получения sha512 хэша из строки
 * @param text текст для перевода в строку
 */
const string sha512(const string text){
	// Результат работы функции
	string result;
	// Если текст передан
	if(!text.empty()){
		// Массив полученных значений
		u_char digest[64];
		// Создаем контекст
		SHA512_CTX ctx;
		// Выполняем инициализацию контекста
		SHA512_Init(&ctx);
		// Выполняем расчет суммы
		SHA512_Update(&ctx, text.c_str(), text.length());
		// Копируем полученные данные
		SHA512_Final(digest, &ctx);
		// Строка sha512
		char mdString[129];
		// Заполняем массив нулями
		memset(mdString, 0, 129);
		// Заполняем строку данными sha512
		for(u_short i = 0; i < 64; i++) sprintf(&mdString[i * 2], "%02x", u_int(digest[i]));
		// Выводим результат
		result = mdString;
	}
	// Выводим результат
	return result;
}


int main() {
   
    printf("md5 digest: %s\n", sha512("user").c_str());
 
    return 0;
}
