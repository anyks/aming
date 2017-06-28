/* HTTP ПАРСЕР AMING */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#include "http/http.h"

// Устанавливаем пространство имен
using namespace std;

/**
 * getHeaders Метод получения заголовков
 * @return сформированные заголовки
 */
const string HttpData::HttpHeaders::getHeaders(){
	// Сформированные заголовки
	string headers;
	// Переходим по всем заголовкам и формируем общий результат
	for(auto it = this->headers.begin(); it != this->headers.end(); it++){
		// Формируем результат
		headers.append(it->head + string(": ") + it->value + string("\r\n"));
	}
	// Добавляем конец заголовков
	headers.append("\r\n");
	// Выводим результат
	return headers;
}
/**
 * getHeader Метод извлекает данные заголовка по его ключу
 * @param  key ключ заголовка
 * @return     строка с данными заголовка
 */
HttpData::HttpHeaders::Header HttpData::HttpHeaders::getHeader(const string key){
	// Присваиваем значения строк
	string ckey = key;
	// Убираем пробелы
	ckey = Anyks::toCase(Anyks::trim(ckey));
	// Переходим по всему массиву и ищем там нужный нам заголовок
	for(auto it = this->headers.begin(); it != this->headers.end(); it++){
		// Если заголовок найден
		if(Anyks::toCase(it->head).compare(ckey) == 0) return (* it);
	}
	// Сообщаем что ничего не найдено
	return {"", ""};
}
/**
 * data Метод получения сырых данных
 * @return сырые данные
 */
const u_char * HttpData::HttpHeaders::data(){
	// Если заголовки собраны
	if(isEnd()){
		// Сформированные заголовки
		string headers;
		// Получаем количество заголовков
		const size_t size = getHeadersSize();
		// Переходим по всему фектору заголовков
		for(size_t i = 0; i < size; i++){
			// Если это не заголовок Age
			if((Anyks::toCase(this->headers[i].head).compare("age") != 0)
			&& (Anyks::toCase(this->headers[i].head).compare("content-length") != 0)
			&& (Anyks::toCase(this->headers[i].head).compare("transfer-encoding") != 0)
			&& !((Anyks::toCase(this->headers[i].head).compare("content-encoding") == 0)
			&& (Anyks::toCase(this->headers[i].value).find("gzip") != string::npos))){
				// Получаем название заголовка
				string header = this->headers[i].head;
				// Получаем значение заголовка
				string value = this->headers[i].value;
				// Если это ETag
				if(Anyks::toCase(header).compare("etag") == 0){
					// Выполняем поиск экранируемого символа
					size_t pos = value.find("W/");
					// Если найден экранируемный символ то удаляем его
					if(pos != string::npos) value = value.substr(pos + 2, value.length() - 2);
				}
				// Добавляем заголовок
				headers.append(header + string("<-|params|->") + value);
				// Если это не последний элемент то добавляем разделитель
				if(i < (size - 1)) headers.append("<-|heads|->");
			}
		}
		// Заполняем контейнер данными
		this->raw.assign(headers.begin(), headers.end());
	}
	// Выводим результат
	return this->raw.data();
}
/**
 * set Метод установки сырых данных
 * @param data сырые данные
 * @param size размер сырых данных
 */
void HttpData::HttpHeaders::set(const u_char * data, size_t size){
	// Данные заголовков
	string headers;
	// Если размер данных существует
	if(size) headers.assign(reinterpret_cast <const char *> (data), size);
	// Если заголовки существуют
	if(!headers.empty()){
		// Выполняем очистку
		clear();
		// Выполняем разбивки на чанки
		vector <string> data;
		// Выполняем получение заголовков
		Anyks::split(headers, "<-|heads|->", data);
		// Если данные существуют
		if(!data.empty()){
			// Переходим по всему массиву заголовков
			for(auto it = data.begin(); it != data.end(); it++){
				// Выполняем разбор заголовка
				vector <string> header;
				// Выполняем получение заголовка
				Anyks::split(* it, "<-|params|->", header);
				// Добавляем заголовок в список
				if(!header.empty()) append(* header.begin(), * (header.end() - 1));
			}
			// Запоминаем что все заголовки добавлены
			setEnd();
		}
	}
}
/**
 * clear Метод очистки данных
 */
void HttpData::HttpHeaders::clear(){
	// Запоминаем что заголовки не заполнены
	this->end = false;
	// Очищаем сырые данные
	this->raw.clear();
	// Очищаем данные http заголовков
	this->headers.clear();
}
/**
 * remove Метод удаления заголовка по ключу
 * @param key ключ заголовка
 */
void HttpData::HttpHeaders::remove(const string key){
	// Присваиваем значения строк
	string ckey = key;
	// Убираем пробелы
	ckey = Anyks::toCase(Anyks::trim(ckey));
	// Ищем такой ключ
	for(auto it = this->headers.begin(); it != this->headers.end(); it++){
		// Если найшли наш ключ
		if(Anyks::toCase(it->head).compare(ckey) == 0){
			// Удаляем указанный заголовок
			this->headers.erase(it);
			// Выходим из цикла
			break;
		}
	}
}
/**
 * append Метод добавления заголовка
 * @param key ключ
 * @param val значение
 */
void HttpData::HttpHeaders::append(const string key, const string val){
	// Присваиваем значения строк
	string ckey = key;
	string cval = val;
	// Убираем пробелы
	ckey = Anyks::trim(ckey);
	cval = Anyks::trim(cval);
	// Приводим к нижнему регистру ключ для удаления
	string rkey = Anyks::toCase(ckey);
	// Удаляем сначала исходный заголовок
	if((rkey.compare("cookie") != 0)
	&& (rkey.compare("set-cookie") != 0)) remove(rkey);
	// Запоминаем найденны параметры
	this->headers.push_back({ckey, cval});
}
/**
 * setEnd Метод установки завершения передачи данных
 */
void HttpData::HttpHeaders::setEnd(){
	// Запоминаем что заголовки заполнены
	this->end = true;
}
/**
 * create Метод создания объекта http заголовков
 * @param buffer буфер с текстовыми данными
 */
const bool HttpData::HttpHeaders::create(const char * buffer){
	// Очищаем заголовки
	clear();
	// Запоминаем буфер входящих данных
	string str = buffer;
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e(
		"^(?:(?:(?:(?:OPTIONS|GET|HEAD|POST|PUT|PATCH|DELETE|TRACE|CONNECT)"
		"\\s+[^\\r\\n\\s]+\\s+[A-Za-z]+\\/[\\d\\.]+\\r\\n)|(?:[A-Za-z]+"
		"\\/[\\d\\.]+\\s+\\d+(?:\\s+[^\\r\\n]+)?\\r\\n))((?:[\\w\\-]+\\s*\\:"
		"\\s*[^\\r\\n]+\\r\\n)+)\\r\\n)",
		regex::ECMAScript | regex::icase
	);
	// Выполняем поиск протокола
	regex_search(str, match, e);
	// Если данные найдены
	if(!match.empty()){
		// Запоминаем первые символы
		str = match[1].str();
		// Массив строк
		vector <string> strings;
		// Выполняем разбиение строк
		Anyks::split(str, "\r\n", strings);
		// Если строки найдены
		if(!strings.empty()){
			// Переходим по всему массиву строк
			for(auto it = strings.begin(); it != strings.end(); it++){
				// Результат работы регулярного выражения
				smatch match;
				// Устанавливаем правило регулярного выражения
				regex e("\\b([\\w\\-]+)\\s*\\:\\s*([\\s\\S]+)", regex::ECMAScript | regex::icase);
				// Выполняем поиск протокола
				regex_search(* it, match, e);
				// Если заголовок найден
				if(!match.empty()) this->append(match[1].str(), match[2].str());
			}
			// Запоминаем что заголовки сформированны
			setEnd();
			// Если данные получены
			return getHeadersSize();
		}
	}
	// Сообщаем что ничего не вышло
	return false;
}
/**
 * isEnd Метод проверки завершения формирования заголовков
 * @return результат проверки
 */
const bool HttpData::HttpHeaders::isEnd(){
	// Выводим результат проверки
	return this->end;
}
/**
 * empty Метод определяет наличие данных
 * @return проверка о наличи данных
 */
const bool HttpData::HttpHeaders::empty(){
	// Выводим существования данных http заголовков
	return this->headers.empty();
}
/**
 * size Метод получения размеров сырых данных
 * @return размер сырых данных
 */
const size_t HttpData::HttpHeaders::size(){
	// Если размер данные не существует, выполняем генерацию данных
	if(this->raw.empty()) data();
	// Выводим результат
	return this->raw.size();
}
/**
 * getHeadersSize Метод получения размера
 * @return данные размера
 */
const size_t HttpData::HttpHeaders::getHeadersSize(){
	// Выводим размер http заголовков
	return this->headers.size();
}
/**
 * cbegin Метод получения начального итератора
 * @return начальный итератор
 */
vector <HttpData::HttpHeaders::Header>::const_iterator HttpData::HttpHeaders::cbegin() const noexcept {
	// Выводим начальный итератор
	return this->headers.cbegin();
}
/**
 * cend Метод получения конечного итератора
 * @return конечный итератор
 */
vector <HttpData::HttpHeaders::Header>::const_iterator HttpData::HttpHeaders::cend() const noexcept {
	// Выводим конечный итератор
	return this->headers.cend();
}
/**
 * ~HttpHeaders Деструктор
 */
HttpData::HttpHeaders::~HttpHeaders(){
	// Очищаем структуры
	clear();
	// Удаляем сырые данные
	vector <u_char> ().swap(this->raw);
	// Удаляем данные http заголовков
	vector <Header> ().swap(this->headers);
}
/**
 * operator = Оператор присваивания
 * @param chunk сторонний объект чанка
 * @return      указатель на текущий объект
 */
/*
HttpData::HttpBody::Chunk & HttpData::HttpBody::Chunk::operator = (HttpData::HttpBody::Chunk chunk){
	// Запоминаем размеры чанка
	this->size	= chunk.size;
	this->hsize	= chunk.hsize;
	// Если объект существовал ранее то удаляем его
	if(!this->content.empty()) this->content.clear();
	// Выполняем копирование данных
	copy(chunk.get(), chunk.get() + chunk.size, back_inserter(this->content));
	// Выводим указатель на текущий объект
	return * this;
}
*/
/**
 * init Метод инициализации чанка
 * @param data данные для присваивания
 * @param size размер данных
 */
void HttpData::HttpBody::Chunk::init(const char * data, const size_t size){
	// Если данные существуют
	if(data && size){
		// Выполняем копирование данных
		this->content.assign(data, data + size);
		// Создаем поток
		stringstream stream;
		// Заполняем поток данными
		stream << std::hex << this->content.size();
		// Получаем размер сжатых данных в 16-м виде
		string hsize(stream.str());
		// Запоминаем размер в 16-м виде
		this->hsize = hsize;
	}
}
/**
 * Chunk Конструктор
 * @param data данные для присваивания
 * @param size размер данных
 */
HttpData::HttpBody::Chunk::Chunk(const char * data, const size_t size){
	// Выполняем инициализацию чанка
	init(data, size);
}
/**
 * getBodySize Метод определения размера данных
 * @param  chunked чанкованием
 * @return         размер тела
 */
const size_t HttpData::HttpBody::getBodySize(const bool chunked){
	// Выводим размер тела
	size_t size = 0;
	// Если это чанкование
	if(chunked){
		// Переходим по всем чанкам и считаем размеры
		for(auto it = this->chunks.begin(); it != this->chunks.end(); it++) size += it->content.size();
	// Иначе просто считаем размер блока
	} else size = this->body.size();
	// Выводим результат
	return size;
}
/**
 * size Метод получения размеров сырых данных
 * @return размер сырых данных
 */
const size_t HttpData::HttpBody::size(){
	// Если размер данные не существует, выполняем генерацию данных
	if(this->raw.empty()) data();
	// Выводим результат
	return this->raw.size();
}
/**
 * compress_gzip Метод сжатия данных методом GZIP
 * @param  str   строка для сжатия данных
 * @return       результат сжатия
 */
const string HttpData::HttpBody::compress_gzip(const string &str){
	// Создаем поток zip
	z_stream zs;
	// Результирующий размер данных
	int ret = 0;
	// Результирующая строка с данными
	string outstring;
	// Заполняем его нулями
	memset(&zs, 0, sizeof(zs));
	// Если поток инициализировать не удалось, выходим
	if(deflateInit2(&zs, this->levelGzip, Z_DEFLATED, MOD_GZIP_ZLIB_WINDOWSIZE + 16, MOD_GZIP_ZLIB_CFACTOR, Z_DEFAULT_STRATEGY) == Z_OK){
		// Заполняем входные данные буфера
		zs.next_in = (Bytef *) str.data();
		// Указываем размер входного буфера
		zs.avail_in = str.size();
		// Создаем буфер с сжатыми данными
		char * zbuff = new char[(const size_t) zs.avail_in];
		// Выполняем сжатие данных
		do {
			// Устанавливаем буфер для получения результата
			zs.next_out = reinterpret_cast<Bytef *> (zbuff);
			// Устанавливаем максимальный размер буфера
			zs.avail_out = sizeof(zbuff);
			// Выполняем сжатие
			ret = deflate(&zs, Z_FINISH);
			// Если данные добавлены не полностью
			if(outstring.size() < zs.total_out)
				// Добавляем оставшиеся данные
				outstring.append(zbuff, zs.total_out - outstring.size());
		} while(ret == Z_OK);
		// Удаляем буфер данных
		delete [] zbuff;
	}
	// Завершаем сжатие
	deflateEnd(&zs);
	// Если сжатие не удалось то очищаем выходные данные
	if(ret != Z_STREAM_END) outstring.clear();
	// Выводим результат
	return outstring;
}
/**
 * decompress_gzip Метод рассжатия данных методом GZIP
 * @param  str   строка для расжатия данных
 * @return       результат расжатия
 */
const string HttpData::HttpBody::decompress_gzip(const string &str){
	// Создаем поток zip
	z_stream zs;
	// Результирующий размер данных
	int ret = 0;
	// Результирующая строка с данными
	string outstring;
	// Заполняем его нулями
	memset(&zs, 0, sizeof(zs));
	// Если поток инициализировать не удалось, выходим
	if(inflateInit2(&zs, MOD_GZIP_ZLIB_WINDOWSIZE + 16) == Z_OK){
		// Заполняем входные данные буфера
		zs.next_in = (Bytef *) str.data();
		// Указываем размер входного буфера
		zs.avail_in = str.size();
		// Получаем размер выходных данных
		const size_t osize = (zs.avail_in * 10);
		// Выполняем расжатие данных
		do {
			// Создаем буфер с результирующими данными
			char * zbuff = new char[osize];
			// Устанавливаем буфер для получения результата
			zs.next_out = reinterpret_cast<Bytef *> (zbuff);
			// Устанавливаем максимальный размер буфера
			zs.avail_out = osize;
			// Выполняем расжатие
			ret = inflate(&zs, 0);
			// Если данные добавлены не полностью
			if(outstring.size() < zs.total_out)
				// Добавляем оставшиеся данные
				outstring.append(zbuff, zs.total_out - outstring.size());
			// Удаляем буфер данных
			delete [] zbuff;
		} while(ret == Z_OK);
	}
	// Завершаем расжатие
	inflateEnd(&zs);
	// Если сжатие не удалось то очищаем выходные данные
	if(ret != Z_STREAM_END) outstring.clear();
	// Выводим результат
	return outstring;
}
/**
 * compressData Метод сжатия данных
 * @param  buffer буфер с данными
 * @param  size   размер передаваемых данных
 * @return        данные сжатого чанка
 */
const string HttpData::HttpBody::compressData(const char * buffer, const size_t size){
	// Вектор для исходящих данных
	vector <char> data;
	// Создаем поток zip
	z_stream zs;
	// Заполняем его нулями
	memset(&zs, 0, sizeof(z_stream));
	/**
	 * const static int NO_COMPRESSION (4)
	 * const static int BEST_SPEED (4)
	 * const static int BEST_COMPRESSION (2)
	 * const static int DEFAULT_COMPRESSION (2)
	 */
	// Если поток инициализировать не удалось, выходим
	if(deflateInit(&zs, this->levelGzip) == Z_OK){
		// Максимальный размер выходного массива
		size_t size_out = size;
		// Создаем буфер с сжатыми данными
		char * zbuff = new char [(const size_t) size_out];
		// Заполняем входные данные буфера
		zs.next_in = reinterpret_cast <Bytef *> (const_cast <char *> (buffer));
		// Указываем размер входного буфера
		zs.avail_in = static_cast <uInt> (size);
		// Устанавливаем выходной буфер
		zs.next_out = reinterpret_cast <Bytef *> (zbuff);
		// Указываем максимальный размер выходног буфера
		zs.avail_out = static_cast <uInt> (size_out);
		// Получаем контрольную сумму
		int checksum = crc32(0, zs.next_in, zs.avail_in);
		// Выполняем сжатие данных буфера
		int const result = deflate(&zs, Z_FINISH);
		// Запоминаем размер полученных сжатых данных
		size_out = zs.total_out;
		// Завершаем сжатие
		deflateEnd(&zs);
		// Заполняем вектор полученными данными
		if(result == Z_STREAM_END){
			// Префикс архива
			const short prefix = 2;
			// Суфикс архива
			const short suffix = 4;
			// Описание заголовков gzip: http://www.gzip.org/zlib/rfc-gzip.html
			/*
			* '\037', '\213'			- Магические константы gzip
			* 0x08						- Метод сжатия "defalte"
			* 0x01						- Текстовые данные
			* 0x00, 0x00, 0x00, 0x00	- TimeStamp не устанавливаем
			* 0x02						- Флаг максимального сжатия
			* 0x03						- Операционная система Unix
			*/
			const char gzipheader[] = {'\037', '\213', 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03};
			// Высчитываем конечный размер сжимаемых данных
			size_out -= prefix + suffix;
			// Преобразуем в указатель на char
			const char * cchecksum = reinterpret_cast <char *> (&checksum);
			// Добавляем заголовки архива в буфер данных чанка
			copy(gzipheader, gzipheader + sizeof(gzipheader), back_inserter(data));
			// Копируем в вектор полученные сжатые данные
			copy(zbuff + prefix, zbuff + prefix + size_out, back_inserter(data));
			// Добавляем данные контрольной суммы также в чанк
			copy(cchecksum, cchecksum + sizeof(checksum), back_inserter(data));
		}
		// Удаляем выделенную ранее память
		delete [] zbuff;
	}
	// Выводим результат
	return string(data.data(), data.size());
}
/**
 * createChunk Метод создания чанка
 * @param buffer буфер с данными
 * @param size   размер передаваемых данных
 */
void HttpData::HttpBody::createChunk(const char * buffer, const size_t size){
	// Копируемый размер данных
	size_t copySize = size;
	// Количество обработанных байт
	size_t used = 0;
	// Выполняем создание чанков до тех пор пока данные существуют
	while(copySize){
		// Определяем размер копируемых данных
		size_t size = (copySize < this->chunkSize ? copySize : this->chunkSize);
		// Создаем массив с данными
		vector <char> data(buffer + used, buffer + used + size);
		// Создаем чанк
		Chunk chunk(data.data(), data.size());
		// Добавляем чанк в массив
		this->chunks.push_back(chunk);
		// Уменьшаем количество копируемых данных
		copySize -= size;
		// Увеличиваем количество обработанных данных
		used += size;
	}
}
/**
 * clear Метод сброса параметров
 */
void HttpData::HttpBody::clear(){
	// Сбрасываем заполненность данных
	this->end = false;
	// Сбрасываем режимы сжатия
	this->intGzip = false;
	this->extGzip = false;
	// Сбрасываем полученные чанки
	this->chunks.clear();
	// Очищаем тело
	this->body.clear();
	this->rody.clear();
	// Очищаем сырые данные
	this->raw.clear();
}
/**
 * set Метод установки сырых данных
 * @param data сырые данные
 * @param size размер сырых данных
 */
void HttpData::HttpBody::set(const u_char * data, size_t size){
	// Если данные существуют
	if(size){
		// Размер полученных данных
		size_t size_data;
		// Извлекаем размер данных
		memcpy(&size_data, data, sizeof(size_t));
		// Запоминаем старое значение внутреннего сжатия
		bool intGzip = this->intGzip;
		// Если размер тела существует
		if(size_data){
			// Выполняем очистку
			clear();
			// Возвращаем значение внутреннего сжатия
			this->intGzip = intGzip;
			// Выделяем динамически память
			char * buffer = new char [size_data];
			// Извлекаем данные адреса
			memcpy(buffer, data + sizeof(size_data), size_data);
			// Добавляем данные тела
			addData(buffer, size_data, size_data, true);
			// Удаляем полученные данные
			delete [] buffer;
		}
	}
}
/**
 * setChunkSize Метод установки размера чанков
 * @param size размер чанков в байтах
 */
void HttpData::HttpBody::setChunkSize(const size_t size){
	// Устанавливаем максимальный размер чанков в байтах
	this->chunkSize = size;
}
/**
 * setLevelGzip Метод установки уровня сжатия
 * @param levelGzip уровень сжатия
 */
void HttpData::HttpBody::setLevelGzip(const u_int levelGzip){
	// Устанавливаем уровнь сжатия
	this->levelGzip = levelGzip;
}
/**
 * setEnd Метод установки завершения передачи данных
 * (активируется при отключении сервера от прокси, все это нужно для протокола HTTP1.0 при Connection = close)
 */
void HttpData::HttpBody::setEnd(){
	// Запоминаем что все данные получены
	this->end = true;
	// Запоминаем сырые данные
	if(!this->extGzip) this->rody = this->body;
	// Иначе выполняем дешифровку данных
	else this->rody = decompress_gzip(this->body);
	// Создаем чанки для несжатых данных
	if(!this->intGzip) createChunk(this->body.data(), this->body.size());
	// Если это не gzip
	else if(!this->extGzip && this->intGzip){
		// Выполняем сжатие тела
		string data = compress_gzip(this->body);
		// Создаем тело в сжатом виде
		if(!data.empty()) this->body = data;
		// Если сжатие не удалось тогда не сжимаем данные
		else this->intGzip = false;
		// Создаем чанки в сжатом виде
		createChunk(this->body.data(), this->body.size());
	}
}
/**
 * isEnd Метод проверки завершения формирования тела
 * @return результат проверки
 */
const bool HttpData::HttpBody::isEnd(){
	// Выводим результат проверки заполненности данных
	return this->end;
}
/**
 * isIntCompress Метод проверки на активацию внутреннего сжатия
 * @return результат проверки
 */
const bool HttpData::HttpBody::isIntCompress(){
	// Выводим результат проверки активации внутреннего сжатия
	return this->intGzip;
}
/**
 * isExtCompress Метод проверки на активацию внешнего сжатия
 * @return результат проверки
 */
const bool HttpData::HttpBody::isExtCompress(){
	// Выводим результат проверки активации внешнего сжатия
	return this->extGzip;
}
/**
 * getLevelGzip Метод получения уровня сжатия
 */
const u_int HttpData::HttpBody::getLevelGzip(){
	// Выводим метод получения уровня сжатия
	return this->levelGzip;
}
/**
 * getChunkSize Метод получения размера чанков
 */
const size_t HttpData::HttpBody::getChunkSize(){
	// Выводим размер чанков
	return this->chunkSize;
}
/**
 * addData Метод добавления данных тела
 * @param  buffer буфер с данными
 * @param  size   размер передаваемых данных
 * @param  length тип данных (0 - по умолчанию, 1 - чанки, все остальные - по размеру)
 * @param  strict жесткие правила проверки (при установки данного флага, данные принимаются только в точном соответствии)
 * @return        количество обработанных байт
 */
const size_t HttpData::HttpBody::addData(const char * buffer, const size_t size, const size_t length, const bool strict){
	// Количество прочитанных байт
	size_t readbytes = 0;
	// Если данные тела еще не заполнены
	if(!this->end && size && length){
		// Определяем тип данных
		switch(length){
			// Обработка по умолчанию
			case 1: {
				// Увеличиваем количество использованных байт
				readbytes = size;
				// Добавляем данные тела
				this->body.append(buffer, readbytes);
			} break;
			// Обрабатываем чанкованием
			case 2: {
				// Выполняем поиск завершения передачи чанков
				size_t pos = string(buffer, size).find("0\r\n\r\n");
				// Определяем позицию
				if(pos != string::npos){
					// Запоминаем сколько байт мы прочитали
					readbytes = (pos + 7);
					// Запоминаем что все данные получены
					this->end = true;
				// Если это не конец, запоминаем сколько байт мы прочитали
				} else readbytes = size;
				// Иначе добавляем все что есть
				this->body.append(buffer, readbytes);
				// Если все данные получены, выполняем преобразование тела
				if(this->end){
					// Новый буфер данных
					string body;
					// Смещение
					size_t offset = 0;
					// Получаем размер буфера
					size_t len = this->body.size();
					// Получаем буфер для поиска
					const char * bodyBuffer = this->body.data();
					// Выполняем обработку до тех пор пока все не обработаем
					while(true){
						// Получаем новую строку
						string data = string(bodyBuffer + offset, len - offset);
						// Выполняем поиск завершения передачи чанков
						size_t pos = data.find("\r\n");
						// Если нашли переход
						if(pos != string::npos){
							// Размер чанка
							size_t ichunk = 0;
							// Получаем размер данных в 16-й системе
							string hsize(data, 0, pos);
							// Если данные найдены
							if(!hsize.empty()){
								// Если это конец тогда выходим
								if(hsize.compare("0") == 0) break;
								// Создаем поток
								stringstream stream;
								// Заполняем поток данными
								stream << hsize;
								// Извлекаем размер в 10-м виде
								stream >> std::hex >> ichunk;
								// Увеличиваем смещение
								offset += hsize.length() + 2;
								// Извлекаем данные чанка
								body.append(data, pos + 2, ichunk);
								// Если сжатие не установлено
								if(!this->intGzip){
									// Создаем чанк
									Chunk chunk(bodyBuffer + offset, ichunk);
									// Добавляем чанк
									this->chunks.push_back(chunk);
								}
								// Увеличиваем смещение
								offset += ichunk;
								// Если смещение ушло далеко то выходим
								if(offset > len) break;
							// Смещаемся на два байта
							} else offset += 2;
						// Если чанки не найдены тогда выходим
						} else break;
					}
					// Запоминаем сырые данные
					if(!this->extGzip) this->rody = body;
					// Иначе выполняем дешифровку данных
					else this->rody = decompress_gzip(body);
					// Если это не gzip
					if(!this->extGzip && this->intGzip){
						// Выполняем сжатие тела
						string data = compress_gzip(body);
						// Создаем тело в сжатом виде
						if(!data.empty()) this->body = data;
						// Если сжатие не удалось тогда не сжимаем данные
						else {
							// Запоминаем не сжатые данные
							this->body = body;
							// Отключаем сжатие
							this->intGzip = false;
						}
						// Создаем чанки в сжатом виде
						createChunk(this->body.data(), this->body.size());
					// Просто сохраняем данные тела
					} else this->body = body;
				}
			} break;
			// Если это размер данных
			default: {
				// Если это строгий режим
				// и размер переданных данных соответствует
				if(strict && (size < length)) return 0;
				// Определяем текущий размер сохраненных данных
				size_t csize = this->body.size();
				// Копируемый размер данных
				size_t copySize = (size > length ? length : size);
				// Если итоговый размер больше установленного
				if((copySize + csize) > length) copySize = length - csize;
				// Запоминаем сколько байт мы прочитали
				readbytes = copySize;
				// Добавляем данные тела
				this->body.append(buffer, readbytes);
				// Определяем все ли данные заполнены
				if(length == this->body.size()) this->end = true;
				// Если все данные собраны, формируем чанки
				if(this->end){
					// Запоминаем сырые данные
					if(!this->extGzip) this->rody = this->body;
					// Иначе выполняем дешифровку данных
					else this->rody = decompress_gzip(this->body);
					// Создаем чанки для несжатых данных
					if(!this->intGzip) createChunk(this->body.data(), length);
					// Если это не gzip
					else if(!this->extGzip && this->intGzip){
						// Выполняем сжатие тела
						string data = compress_gzip(this->body);
						// Создаем тело в сжатом виде
						if(!data.empty()) this->body = data;
						// Если сжатие не удалось тогда не сжимаем данные
						else this->intGzip = false;
						// Создаем чанки в сжатом виде
						createChunk(this->body.data(), this->body.size());
					}
				}
			}
		}
	}
	// Выводим результат
	return readbytes;
}
/**
 * data Метод получения сырых данных
 * @return сырые данные
 */
const u_char * HttpData::HttpBody::data(){
	// Если данные заполнены
	if(isEnd()){
		// Получаем размер структуры
		const size_t size = this->rody.size();
		// Получаем данные карты размеров
		const u_char * map = reinterpret_cast <const u_char *> (&size);
		// Выполняем копирование карты размеров
		copy(map, map + sizeof(size), back_inserter(this->raw));
		// Выполняем копирование данных тела
		copy(this->rody.begin(), this->rody.end(), back_inserter(this->raw));
	}
	// Выводим сформированные данные
	return this->raw.data();
}
/**
 * getBody Метод получения тела запроса
 * @param  chunked чанкованием
 * @return         данные тела запроса
 */
const string HttpData::HttpBody::getBody(const bool chunked){
	// Создаем чанк
	string result;
	// Если это чанкование
	if(chunked){
		// Переходим по всему массиву чанков
		for(auto it = this->chunks.begin(); it != this->chunks.end(); it++){
			// Формируем строку чанка
			string chunksize = (it->hsize + "\r\n");
			// Добавляем в массив данных, полученный размер чанка
			result.append(chunksize);
			// Добавляем в массив данных, данные чанка
			result.append(it->content);
			// Добавляем завершающую строку
			result.append("\r\n");
		}
		// Добавляем завершающие данные
		result.append("0\r\n\r\n");
	// Выводим результат такой как он есть
	} else result = this->body;
	// Выводим результат
	return result;
}
/**
 * getRawBody Метод получения тела данных в чистом виде
 * @return данные тела запроса
 */
const string HttpData::HttpBody::getRawBody(){
	// Выводим результат
	return this->rody;
}
/**
 * getChunks Метод получения списка чанков
 */
vector <HttpData::HttpBody::Chunk> HttpData::HttpBody::getChunks(){
	// Выводим результат
	return this->chunks;
}
/**
 * HttpBody Конструктор
 * @param chunkSize  максимальный размер каждого чанка (в байтах)
 * @param levelGzip  уровень сжатия
 * @param intGzip    активация режима внутреннего сжатия
 * @param extGzip    активация режима внешнего сжатия
 */
HttpData::HttpBody::HttpBody(const size_t chunkSize, const u_int levelGzip, const bool intGzip, const bool extGzip){
	// Очищаем все данные
	clear();
	// Запоминаем размер чанка
	this->chunkSize = chunkSize;
	// Запоминаем уровень сжатия
	this->levelGzip = levelGzip;
	// Запоминаем режим сжатия
	this->intGzip = intGzip;
	this->extGzip = extGzip;
}
/**
 * ~HttpBody Деструктор
 */
HttpData::HttpBody::~HttpBody(){
	// Очищаем все данные
	clear();
	// Удаляем сырые данные
	vector <u_char> ().swap(this->raw);
	// Удаляем объект чанков
	vector <Chunk> ().swap(this->chunks);
}
/**
 * genDataConnect Метод генерации данных для подключения
 */
void HttpData::genDataConnect(){
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e("\\b([A-Za-z]+)\\s+([\\s\\S]+)\\s+([A-Za-z]+)\\/([\\d\\.]+)", regex::ECMAScript | regex::icase);
	// Выполняем поиск протокола
	regex_search(this->http, match, e);
	// Если данные найдены
	if(!match.empty()){
		// Запоминаем метод запроса
		this->method = Anyks::toCase(match[1].str());
		// Запоминаем путь запроса
		this->path = match[2].str();
		// Запоминаем протокол запроса
		this->protocol = Anyks::toCase(match[3].str());
		// Запоминаем версию протокола
		this->version = match[4].str();
		// Извлекаем данные хоста
		string host = getHeader("host");
		// Извлекаем данные авторизации
		string auth = getHeader("proxy-authorization");
		// Получаем заголовок Proxy-Connection
		string proxy_connection = getHeader("proxy-connection");
		// Если постоянное соединение не установлено
		if((this->options & OPT_SMART) && !proxy_connection.empty())
			// Добавляем заголовок подключения
			setHeader("Connection", proxy_connection);
		// Если хост найден
		if(!host.empty()){
			// Выполняем получение параметров подключения
			Connect gcon = getConnection(this->path);
			// Выполняем получение параметров подключения
			Connect scon = getConnection(host);
			// Запоминаем порт
			if((scon.port.compare(gcon.port) != 0)
			&& (gcon.port.compare("80") == 0)){
				// Запоминаем протокол
				this->protocol = scon.protocol;
				// Уделяем предпочтение 443 порту
				this->port = scon.port;
			// Запоминаем порт такой какой он есть
			} else if(Anyks::checkPort(gcon.port)) {
				// Запоминаем протокол
				this->protocol = gcon.protocol;
				// Запоминаем порт
				this->port = gcon.port;
			// Устанавливаем значение порта и протокола по умолчанию
			} else {
				// Запоминаем протокол
				this->protocol = "http";
				// Устанавливаем http порт
				this->port = "80";
			}
			// Создаем полный адрес запроса
			string fulladdr1 = scon.protocol + string("://") + scon.host;
			string fulladdr2 = fulladdr1 + "/";
			string fulladdr3 = fulladdr1 + string(":") + scon.port;
			string fulladdr4 = fulladdr3 + "/";
			// Определяем путь
			if(((this->protocol.compare("http") == 0)
			|| (this->protocol.compare("https") == 0))
			&& ((Anyks::toCase(this->method).compare("connect") != 0)
			&& ((Anyks::toCase(this->path).compare(Anyks::toCase(host)) == 0)
			|| (Anyks::toCase(this->path).compare(Anyks::toCase(fulladdr1)) == 0)
			|| (Anyks::toCase(this->path).compare(Anyks::toCase(fulladdr2)) == 0)
			|| (Anyks::toCase(this->path).compare(Anyks::toCase(fulladdr3)) == 0)
			|| (Anyks::toCase(this->path).compare(Anyks::toCase(fulladdr4)) == 0)))) this->path = "/";
			// Выполняем удаление из адреса доменного имени
			else this->path = gcon.path;
			// Запоминаем хост
			this->host = scon.host;
		}
		// Если авторизация найдена
		if(!auth.empty()){
			// Устанавливаем правило регулярного выражения
			regex e("\\b([A-Za-z]+)\\s+([\\s\\S]+)", regex::ECMAScript | regex::icase);
			// Выполняем поиск протокола
			regex_search(auth, match, e);
			// Если данные найдены
			if(!match.empty()){
				// Запоминаем тип авторизации
				this->auth = Anyks::toCase(match[1].str());
				// Если это тип авторизация basic, тогда выполняем декодирования данных авторизации
				if(this->auth.compare("basic") == 0){
					// Создаем объект base64
					Base64 base64;
					// Выполняем декодирование логина и пароля
					const string dauth = base64.decode(match[2].str());
					// Устанавливаем правило регулярного выражения
					regex e("\\b([\\s\\S]+)\\:([\\s\\S]+)", regex::ECMAScript | regex::icase);
					// Выполняем поиск протокола
					regex_search(dauth, match, e);
					// Если данные найдены
					if(!match.empty()){
						// Запоминаем логин
						this->login = match[1].str();
						// Запоминаем пароль
						this->password = match[2].str();
					}
				// Запоминаем данные авторизации если это не стандартная авторизация
				} else this->login = match[2].str();
			}
		}
	}
}
/**
 * createRequest Функция создания ответа сервера
 * @param code код ответа
 */
void HttpData::createRequest(const u_short code){
	// Если код существует
	if(this->response.count(code)){
		// Устанавливаем статус
		setStatus(code);
		// Очищаем тело вложений
		this->body.clear();
		// Устанавливаем версию протокола
		this->version = "1.1";
		// Формируем запрос
		this->http = (
			string("HTTP/") + this->version + string(" ") + to_string(code)
			+ string(" ") + this->response[code].text
		);
		// Добавляем запрос в заголовки
		string headers = (this->http + "\r\n");
		// Добавляем заголовки
		headers.append(this->response[code].headers);
		// Создаем заголовки
		this->headers.create(headers.c_str());
		// Если тело существует
		if(!this->response[code].body.empty()){
			// Создаем тело
			this->body.addData(
				this->response[code].body.c_str(),
				this->response[code].body.size(),
				this->response[code].body.size()
			);
		}
	}
}
/**
 * createHeadResponse Функция получения сформированного заголовков ответа
 * @return собранные заголовки ответа
 */
const string HttpData::createHeadResponse(){
	// Определяем тип прокси
	bool smart = (this->options & OPT_SMART);
	// Определяем разрешено ли выводить название агента
	bool agent = (this->options & OPT_AGENT);
	// Замена заголовка Via
	bool via = false;
	// Формируем запрос
	string response = (this->http + string("\r\n"));
	// Тип подключения
	string connection = getHeader("connection");
	// Если заголовок не найден тогда устанавливаем по умолчанию
	// if(connection.empty()) connection = "close";
	// Добавляем остальные заголовки
	for(auto it = this->headers.cbegin(); it != this->headers.cend(); ++it){
		// Определяем заголовок
		string head = it->head;
		// Определяем значение
		string value = it->value;
		// Фильтруем заголовки
		if(agent && (Anyks::toCase(head).compare("via") == 0)){
			// Добавляем стандартный заголовок проксирования
			value += (string(", ") + this->version + string(" ")
			+ this->appName + string(" (") + string(APP_NAME)
			+ string("/") + this->appVersion + string(")"));
			// Запоминаем что заменили заголовок
			via = true;
		}
		// Если указанные заголовки не найдены
		if((Anyks::toCase(head).compare("connection") != 0)
		&& (Anyks::toCase(head).compare("proxy-authorization") != 0)
		&& (!smart || (smart && (Anyks::toCase(head).compare("proxy-connection") != 0)))){
			// Добавляем оставшиеся заголовки
			response.append(head + string(": ") + value + string("\r\n"));
		}
	}
	// Устанавливаем название прокси
	if(agent){
		// Добавляем название агента
		response.append(string("Proxy-Agent: ") + this->appName + string(" (")
		+ string(APP_NAME) + string("/") + this->appVersion + string(")\r\n"));
		// Добавляем стандартный заголовок проксирования
		if(!via) response.append(string("Via: ") + this->version + string(" ")
		+ this->appName + string(" (") + string(APP_NAME)
		+ string("/") + this->appVersion + string(")\r\n"));
	}
	// Получаем заголовок Proxy-Connection
	const string proxy_connection = getHeader("proxy-connection");
	// Если это dumb прокси
	if(!smart && proxy_connection.empty()) response.append(string("Proxy-Connection: ") + connection + string("\r\n"));
	// Если это smart прокси
	else if(smart && !proxy_connection.empty()) connection = proxy_connection;
	// Добавляем тип подключения
	if(!connection.empty()) response.append(string("Connection: ") + connection + string("\r\n"));
	// Запоминаем конец запроса
	response.append(string("\r\n"));
	// Выводим результат
	return response;
}
/**
 * createHeadRequest Функция получения сформированного заголовков запроса
 * @return собранные заголовки ответа
 */
const string HttpData::createHeadRequest(){
	// Определяем тип прокси
	bool smart = (this->options & OPT_SMART);
	// Определяем разрешено ли сжатие
	bool gzip = (this->options & OPT_GZIP);
	// Определяем разрешено ли выводить название агента
	bool agent = (this->options & OPT_AGENT);
	// Определяем разрешено ли постоянное подключение
	bool keepalive = (this->options & OPT_KEEPALIVE);
	// Определяем нужно ли попытаться обходить заблокированные сайты
	bool deblock = (this->options & OPT_DEBLOCK);
	// Замена заголовка Via
	bool via = false;
	// Создаем строку запроса
	string request = (
		Anyks::toCase(this->method, true)
		+ string(" ") + this->path
		+ string(" ") + string("HTTP/")
		+ this->version + string("\r\n")
	);
	// Если нужно попытаться обойти заблокированные сайты
	if(deblock){
		// Устанавливаем заголовок Host
		request.append(
			string("Host: ") + this->host
			+ string(":") + this->port + string("\r\n")
		);
	// Устанавливаем заголовок Host
	} else request.append(string("Host: ") + this->host + string("\r\n"));
	// Получаем UserAgent
	string useragent = getHeader("user-agent");
	// Добавляем useragent
	if(!useragent.empty()) request.append(string("User-Agent: ") + useragent + string("\r\n"));
	// Добавляем остальные заголовки
	for(auto it = this->headers.cbegin(); it != this->headers.cend(); ++it){
		// Определяем заголовок
		string head = it->head;
		// Определяем значение
		string value = it->value;
		// Фильтруем заголовки
		if(agent && (Anyks::toCase(head).compare("via") == 0)){
			// Добавляем стандартный заголовок проксирования
			value += (string(", ") + this->version + string(" ")
			+ this->appName + string(" (") + string(APP_NAME)
			+ string("/") + this->appVersion + string(")"));
			// Запоминаем что заменили заголовок
			via = true;
		}
		// Фильтруем заголовки
		if((Anyks::toCase(head).compare("host") != 0)
		&& (Anyks::toCase(head).compare("user-agent") != 0)
		&& (Anyks::toCase(head).compare("connection") != 0)
		&& (Anyks::toCase(head).compare("proxy-authorization") != 0)
		&& (gzip || (!gzip && (Anyks::toCase(head).compare("accept-encoding") != 0)))
		&& (!smart || (smart && (Anyks::toCase(head).compare("proxy-connection") != 0)))){
			// Добавляем оставшиеся заголовки
			request.append(head + string(": ") + value + string("\r\n"));
		}
	}
	// Устанавливаем название прокси
	if(agent){
		// Добавляем название агента
		request.append(string("Proxy-Agent: ") + this->appName + string(" (")
		+ string(APP_NAME) + string("/") + this->appVersion + string(")\r\n"));
		// Добавляем стандартный заголовок проксирования
		if(!via) request.append(string("Via: ") + this->version + string(" ")
		+ this->appName + string(" (") + string(APP_NAME)
		+ string("/") + this->appVersion + string(")\r\n"));
	}
	// Если постоянное подключение запрещено
	if(!keepalive) setHeader("Connection", "close");
	// Получаем параметры подключения
	string connection = getHeader("connection");
	// Добавляем заголовок connection
	if(!connection.empty()){
		// Устанавливаем заголовок подключения
		request.append(string("Connection: ") + connection + string("\r\n"));
		// Если это dumb прокси
		if(!smart){
			// Проверяем есть ли заголовок соединения прокси
			string pc = getHeader("proxy-connection");
			// Добавляем заголовок закрытия подключения
			if(pc.empty()) request.append(string("Proxy-Connection: ") + connection + string("\r\n"));
		}
	}
	// Запоминаем конец запроса
	request.append(string("\r\n"));
	// Выводим результат
	return request;
}
/**
 * createHeadResponse Метод модификации заголовков в строке
 * @param  str строка с данными заголовков
 * @return     модифицированная строка заголовков
 */
const string HttpData::modifyHeaderString(const string str){
	// Копируем данные заголовков
	string headers = str;
	// Если данные существуют
	if(!headers.empty()){
		// Заголовок connection
		string connection;
		// Определяем тип прокси
		bool smart = (this->options & OPT_SMART);
		// Определяем разрешено ли выводить название агента
		bool agent = (this->options & OPT_AGENT);
		// Если вывод агента разрешен
		if(agent){
			// Результат работы регулярного выражения
			smatch match;
			// Устанавливаем правило регулярного выражения
			regex e("\\r\\nVia\\s*\\:\\s*([^\\r\\n]+)\\r\\n", regex::ECMAScript | regex::icase);
			// Выполняем поиск заголовка
			regex_search(headers, match, e);
			// Заголовок Via
			string via, proxy;
			// Если данные найдены
			if(!match.empty()){
				// Извлекаем значение заголовка Via
				via = match[1].str();
				// Добавляем данные агента
				via.append(string(", ") + this->version + string(" ")
				+ this->appName + string(" (") + string(APP_NAME)
				+ string("/") + this->appVersion + string(")"));
			// Если заголовок via не найден
			} else {
				// Добавляем стандартный заголовок проксирования
				via.append(this->version + string(" ")
				+ this->appName + string(" (") + string(APP_NAME)
				+ string("/") + this->appVersion + string(")"));
			}
			// Добавляем название агента
			proxy.append(this->appName + string(" (")
			+ string(APP_NAME) + string("/") + this->appVersion + string(")"));
			// Добавляем заголовок Via
			addHeaderToString("Via", via, headers);
			// Добавляем заголовок Proxy-Agent
			addHeaderToString("Proxy-Agent", proxy, headers);
		}
		// Результат работы регулярного выражения
		smatch match;
		// Устанавливаем правило регулярного выражения
		regex e("\\r\\nConnection\\s*\\:\\s*([^\\r\\n]+)\\r\\n", regex::ECMAScript | regex::icase);
		// Выполняем поиск заголовка
		regex_search(headers, match, e);
		// Если данные найдены
		if(!match.empty()) connection = match[1].str();
		// Если это dumb прокси
		if(!smart) addHeaderToString("Proxy-Connection", connection, headers);
		// Если это smart прокси
		else {
			// Устанавливаем правило регулярного выражения
			regex e("\\r\\nProxy-Connection\\s*\\:\\s*([^\\r\\n]+)\\r\\n", regex::ECMAScript | regex::icase);
			// Выполняем поиск заголовка
			regex_search(headers, match, e);
			// Если данные найдены
			if(!match.empty()) connection = match[1].str();
		}
		// Добавляем тип подключения
		if(!connection.empty()) addHeaderToString("Connection", connection, headers);
	}
	// Выводим результат
	return headers;
}
/**
 * getConnection Функция извлечения данных подключения
 * @param  str строка запроса
 * @return     объект с данными запроса
 */
HttpData::Connect HttpData::getConnection(const string str){
	// Полученные данные подключения
	Connect data;
	// Результат работы регулярного выражения
	smatch match;
	// Копируем параметры запроса
	string query = str;
	// Устанавливаем правило регулярного выражения
	regex ep("\\b([A-Za-z]+):\\/{2}", regex::ECMAScript | regex::icase);
	// Выполняем поиск протокола
	regex_search(query, match, ep);
	// Если протокол найден
	if(!match.empty()) data.protocol = Anyks::toCase(match[1].str());
	// Устанавливаем правило для поиска
	regex eh(
		// Стандартная запись домена aming.su
		"([\\w\\.\\-]+\\.[\\w\\-]+|"
		// Стандартная запись IPv4 127.0.0.1
		"\\d{1,3}(?:\\.\\d{1,3}){3}|"
		// Стандартная запись IPv6 [2001:0db8:11a3:09d7:1f34:8a2e:07a0:765d]
		"\\[[A-Fa-f\\d\\:]{2,39}\\])(?:\\:(\\d+))?",
		regex::ECMAScript | regex::icase
	);
	// Выполняем поиск домена и порта
	regex_search(str, match, eh);
	// Если протокол найден
	if(!match.empty()){
		// Запоминаем хост
		data.host = Anyks::toCase(match[1].str());
		// Если порт найден, тогда запоминаем его
		if(match.size() == 3) data.port = match[2].str();
	}
	// Устанавливаем номер порта в зависимости от типа протокола
	if(data.port.empty() && ((data.protocol.compare("https") == 0)
	|| (this->method.compare("connect") == 0))) data.port = "443";
	// Если просто порт не существует
	else if(data.port.empty()) data.port = "80";
	// Если протокол не существует
	if(data.protocol.empty()){
		// Устанавливаем версию протокола в зависимости от порта
		if(data.port.compare("443") == 0) data.protocol = "https";
		// Если это не защищенное подключение значит это http подключение
		else data.protocol = "http";
	}
	// Создаем регулярное выражение
	regex ed("^(?:http[s]?\\:\\/\\/)?(?:[\\w\\-\\.]+\\.[\\w\\-]+)(?:\\:\\d+)?(\\/)", regex::ECMAScript | regex::icase);
	// Формируем путь запроса
	data.path = regex_replace(str, ed, "$1");
	// Выводим результат
	return data;
}
/**
 * isIntGzip Метод проверки активации режима сжатия
 * @return результат проверки
 */
const bool HttpData::isIntGzip(){
	// Сообщаем активирован ли режим сжатия
	return this->intGzip;
}
/**
 * isExtGzip Метод проверки активации режима внешнего сжатия
 * @return результат проверки
 */
const bool HttpData::isExtGzip(){
	// Сообщаем активирован ли режим сжатия
	return this->extGzip;
}
/**
 * isUpgrade Метод проверки желания сервера сменить протокол
 * @return результат проверки
 */
const bool HttpData::isUpgrade(){
	// Сообщаем является ли запрос, желанием смены протокола
	return (!getHeader("upgrade").empty()
	&& (Anyks::toCase(getHeader("connection")).find("upgrade") != string::npos));
}
/**
 * isConnect Метод проверяет является ли метод, методом connect
 * @return результат проверки на метод connect
 */
const bool HttpData::isConnect(){
	// Сообщаем является ли метод, методом connect
	return (this->method.compare("connect") == 0);
}
/**
 * isClose Метод проверяет должно ли быть закрыто подключение
 * @return результат проверки на закрытие подключения
 */
const bool HttpData::isClose(){
	// Сообщаем должно ли быть закрыто подключение
	return (Anyks::toCase(getHeader("connection")).find("close") != string::npos);
}
/**
 * isHttps Метод проверяет является ли подключение защищенным
 * @return результат проверки на защищенное подключение
 */
const bool HttpData::isHttps(){
	// Сообщаем является ли продключение защищенным
	return (this->protocol.compare("https") == 0);
}
/**
 * isAlive Метод определения нужно ли держать соединение для прокси
 * @return результат проверки
 */
const bool HttpData::isAlive(){
	// Получаем тип подключения
	string connection = Anyks::toCase(getHeader("connection"));
	// Если это версия протокола 1.1 и подключение установлено постоянное для прокси
	if(getVersion() > 1){
		// Выводим результат провери
		return (connection.find("close") == string::npos);
	// Или если указано явно что это постоянное подключение
	} else return (connection.find("keep-alive") != string::npos);
}
/**
 * isEmpty Если данные не созданы
 * @return результат проверки
 */
const bool HttpData::isEmpty(){
	// Выводим данные о существовании заголовков
	return this->headers.empty();
}
/**
 * isEndHeaders Метод получения данных о заполненности заголовков
 */
const bool HttpData::isEndHeaders(){
	// Выводим данные о заполненности заголовков
	return this->headers.isEnd();
}
/**
 * isEndBody Метод определения заполненности тела ответа данными
 * @return результат проверки
 */
const bool HttpData::isEndBody(){
	// Выводим результат проверки на заполненность тела ответа
	return this->body.isEnd();
}
/**
 * compressIsAllowed Метод проверки активации режима сжатия данных на уровне прокси сервера
 * @param  userAgent агент браузера если существует
 * @return           результат проверки
 */
const bool HttpData::compressIsAllowed(const string userAgent){
	// Флаг установки режима сжатия
	bool gzip = false;
	// Если контент пришел не сжатым а сжатие требуется
	if(this->gzipParams
	&& (this->options & OPT_PGZIP)
	&& isEndHeaders()
	&& getHeader("content-encoding").empty()){
		// Определяем метод запроса
		const string method = getMethod();
		// Определяем метод запроса, разрешено только GET и POST
		if((method.compare("get") != 0)
		&& (method.compare("post") != 0)) return gzip;
		// Если это режим сжатия, тогда отправляем завершающие данные
		if(!isIntGzip()){
			// Получаем статус запроса
			const u_int status = getStatus();
			// Проверяем значение статуса, если это редиректы то запрещаем сжатие
			if(((status > 99) && (status < 200))
			|| ((status > 203) && (status < 400))) return gzip;
			// Получаем размер тела
			const string clength = getHeader("content-length");
			// Получаем тип данных чанкованием
			const string chunked = getHeader("transfer-encoding");
			// Получаем размер контента
			size_t contentLength = (!clength.empty() ? ::atoi(clength.c_str()) : 0);
			// Если размер контента не найден тогда проверяем на чанкование
			if(!contentLength && (!chunked.empty()
			&& (chunked.find("chunked") != string::npos))) contentLength = 2;
			// Если размер контента не найден но это закрытие подключения
			else if(!contentLength && isClose()) contentLength = 1;
			// Если размер контента не найден тогда выходим
			if(!contentLength) return gzip;
			// Результат работы регулярного выражения
			smatch match;
			// Флаг установки режима сжатия
			gzip = true;
			// Получаем наличие заголовка Via
			string via = getHeader("via");
			// Получаем тип файла
			string cmime = getHeader("content-type");
			// Устанавливаем правило регулярного выражения
			regex e(this->gzipParams->regex, regex::ECMAScript | regex::icase);
			// Получаем версию протокола
			for(auto it = this->gzipParams->vhttp.begin(); it != this->gzipParams->vhttp.end(); it++){
				// Поверяем на соответствие версии
				if(getVersion() == float(::atof(it->c_str()))){
					// Запоминаем что протокол проверку прошел
					gzip = true;
					// Выходим
					break;
				// Запрещаем сжатие
				} else gzip = false;
			}
			// Если сжатие разрешено
			if(gzip){
				// Проверяем на тип данных
				for(auto it = this->gzipParams->types.begin(); it != this->gzipParams->types.end(); it++){
					// Выполняем проверку на тип данных
					if((it->compare("*") == 0) || (cmime.find(* it) != string::npos)){
						// Запоминаем что протокол проверку прошел
						gzip = true;
						// Выходим
						break;
					// Запрещаем сжатие
					} else gzip = false;
				}
				// Если сжатие разрешено
				if(gzip && !via.empty()){
					// Readme: http://nginx.org/ru/docs/http/ngx_http_gzip_module.html
					// Запрещаем сжатие
					gzip = false;
					// Переходим по всем параметрам
					for(auto it = this->gzipParams->proxied.begin(); it != this->gzipParams->proxied.end(); it++){
						// Получаем параметр
						const string param = * it;
						// Запрещаем сжатие для всех проксированных запросов, игнорируя остальные параметры;
						if(param.compare("off") == 0){
							// Запрещаем сжатие
							gzip = false;
							// Выходим
							break;
						// Разрешаем сжатие для всех проксированных запросов;
						} else if(param.compare("any") == 0) {
							// Разрешаем сжатие
							gzip = true;
							// Выходим
							break;
						// Разрешаем сжатие, если в заголовке ответа есть поле “Expires” со значением, запрещающим кэширование;
						} else if(param.compare("expired") == 0) {
							// Получаем наличие заголовка Expires
							if(!getHeader("expires").empty()) gzip = true;
							// Если проверка не пройдена
							else {
								// Запрещаем сжатие
								gzip = false;
								// Выходим
								break;
							}
						// Разрешаем сжатие, если в заголовке ответа есть поле “Cache-Control” с параметром “no-cache”;
						} else if(param.compare("no-cache") == 0) {
							// Получаем наличие заголовка Cache-Control
							string cc = getHeader("cache-control");
							// Если заголовок существует
							if(!cc.empty() && (Anyks::toCase(cc).find(param) != string::npos)) gzip = true;
							// Если проверка не пройдена
							else {
								// Запрещаем сжатие
								gzip = false;
								// Выходим
								break;
							}
						// Разрешаем сжатие, если в заголовке ответа есть поле “Cache-Control” с параметром “no-store”;
						} else if(param.compare("no-store") == 0) {
							// Получаем наличие заголовка Cache-Control
							string cc = getHeader("cache-control");
							// Если заголовок существует
							if(!cc.empty() && (Anyks::toCase(cc).find(param) != string::npos)) gzip = true;
							// Если проверка не пройдена
							else {
								// Запрещаем сжатие
								gzip = false;
								// Выходим
								break;
							}
						// Разрешаем сжатие, если в заголовке ответа есть поле “Cache-Control” с параметром “private”;
						} else if(param.compare("private") == 0) {
							// Получаем наличие заголовка Cache-Control
							string cc = getHeader("cache-control");
							// Если заголовок существует
							if(!cc.empty() && (Anyks::toCase(cc).find(param) != string::npos)) gzip = true;
							// Если проверка не пройдена
							else {
								// Запрещаем сжатие
								gzip = false;
								// Выходим
								break;
							}
						// Разрешаем сжатие, если в заголовке ответа нет поля “Last-Modified”;
						} else if(param.compare("no_last_modified") == 0) {
							// Получаем наличие заголовка Last-Modified
							if(getHeader("last-modified").empty()) gzip = true;
							// Если проверка не пройдена
							else {
								// Запрещаем сжатие
								gzip = false;
								// Выходим
								break;
							}
						// Разрешаем сжатие, если в заголовке ответа нет поля “ETag”;
						} else if(param.compare("no_etag") == 0) {
							// Получаем наличие заголовка ETag
							if(getHeader("etag").empty()) gzip = true;
							// Если проверка не пройдена
							else {
								// Запрещаем сжатие
								gzip = false;
								// Выходим
								break;
							}
						// Разрешаем сжатие, если в заголовке запроса есть поле “Authorization”;
						} else if(param.compare("auth") == 0) {
							// Получаем наличие заголовка Authorization
							if(!getHeader("authorization").empty()) gzip = true;
							// Если проверка не пройдена
							else {
								// Запрещаем сжатие
								gzip = false;
								// Выходим
								break;
							}
						}
					}
				}
				// Проверяем размер контента на его минимальную длину
				if(gzip && (contentLength > 2)){
					// Проверяем соответствует ли размер
					if(this->gzipParams->length >= contentLength) gzip = false;
				}
				// Если сжатие разрешено
				if(gzip && !userAgent.empty()){
					// Выполняем проверку
					regex_search(userAgent, match, e);
					// Если проверка не пройдена тогда запрещаем сжатие
					if(!match.empty()) gzip = false;
				}
				// Если запрещено выводить заголовок Vary
				if(gzip && this->gzipParams->vary){
					// Считываем заголовок
					string vary = getHeader("vary");
					// Проверяем наличие
					if(!vary.empty() && (Anyks::toCase(vary)
					.find("accept-encoding") != string::npos)) rmHeader("vary");
				}
			}
		}
	}
	// Выводим результат работы метода
	return gzip;
}
/**
 * rmHeaderInString Метод удаления заголовка из строки
 * @param  header  заголовок
 * @param  headers список заголовков
 * @return         результат работы
 */
const bool HttpData::rmHeaderInString(const string header, string &headers){
	// Результат работы функции
	bool result = false;
	// Если данные существуют
	if(!headers.empty() && !header.empty()){
		// Результат работы регулярного выражения
		smatch match;
		// Устанавливаем правило регулярного выражения
		regex e(
			"(?:(?:(?:OPTIONS|GET|HEAD|POST|PUT|PATCH|DELETE|TRACE|CONNECT)"
			"\\s+[^\\r\\n\\s]+\\s+[A-Za-z]+\\/[\\d\\.]+\\r\\n)|(?:[A-Za-z]+"
			"\\/[\\d\\.]+\\s+\\d+(?:\\s+[^\\r\\n]+)?\\r\\n))(?:[\\w\\-]+\\s*\\:"
			"\\s*[^\\r\\n]+\\r\\n)+\\r\\n",
			regex::ECMAScript | regex::icase
		);
		// Выполняем поиск протокола
		regex_search(headers, match, e);
		// Если данные найдены
		if(!match.empty()){
			// Запоминаем данные заголовков
			string headers1 = match[0].str(), headers2 = match[0].str();
			// Создаем регулярное выражение очистки заголовка от запрещенных символов
			regex he("[^\\w\\-]+", regex::ECMAScript | regex::icase);
			// Исправляем название заголовка
			const string head = regex_replace(header, he, "");
			// Создаем регулярное выражение
			regex re(head + string("\\s*\\:\\s*[^\\r\\n]+\\r\\n"), regex::ECMAScript | regex::icase);
			// Выполняем удаление указанного заголовка
			headers2 = regex_replace(headers2, re, "");
			// Получаем позицию заголовков
			size_t pos = headers.find(headers1);
			// Выполняем замену в строке заголовков
			if(pos != string::npos) headers.replace(pos, headers1.length(), headers2);
			// Определяем результат работы функции
			result = (headers1.compare(headers2) != 0);
		}
	}
	// Выводим результат
	return result;
}
/**
 * addHeaderToString Метод добавления заголовка к строке заголовков
 * @param  header  заголовок
 * @param  value   значение заголовка
 * @param  headers список заголовков
 * @return         результат работы
 */
const bool HttpData::addHeaderToString(const string header, const string value, string &headers){
	// Результат работы функции
	bool result = false;
	// Если данные существуют
	if(!headers.empty() && !value.empty() && !header.empty()){
		// Результат работы регулярного выражения
		smatch match;
		// Устанавливаем правило регулярного выражения
		regex e(
			"(?:(?:(?:OPTIONS|GET|HEAD|POST|PUT|PATCH|DELETE|TRACE|CONNECT)"
			"\\s+[^\\r\\n\\s]+\\s+[A-Za-z]+\\/[\\d\\.]+\\r\\n)|(?:[A-Za-z]+"
			"\\/[\\d\\.]+\\s+\\d+(?:\\s+[^\\r\\n]+)?\\r\\n))(?:[\\w\\-]+\\s*\\:"
			"\\s*[^\\r\\n]+\\r\\n)+\\r\\n",
			regex::ECMAScript | regex::icase
		);
		// Выполняем поиск протокола
		regex_search(headers, match, e);
		// Если данные найдены
		if(!match.empty()){
			// Запоминаем данные заголовков
			string headers1 = match[0].str(), headers2 = match[0].str();
			// Создаем регулярное выражение очистки заголовка от запрещенных символов
			regex he("[^\\w\\-]+", regex::ECMAScript | regex::icase);
			// Создаем регулярное выражение добавления заголовка
			regex ae("\\r\\n\\r\\n", regex::ECMAScript | regex::icase);
			// Исправляем название заголовка
			const string head = regex_replace(header, he, "");
			// Создаем регулярное выражение удаления заголовка
			regex re(head + string("\\s*\\:\\s*[^\\r\\n]+\\r\\n"), regex::ECMAScript | regex::icase);
			// Выполняем удаление указанного заголовка
			headers2 = regex_replace(headers2, re, "");
			// Выполняем модификацию указанного заголовка
			headers2 = regex_replace(headers2, ae, (string("\r\n") + head + string(": ") + value + string("\r\n\r\n")));
			// Получаем позицию заголовков
			size_t pos = headers.find(headers1);
			// Выполняем замену в строке заголовков
			if(pos != string::npos) headers.replace(pos, headers1.length(), headers2);
			// Определяем результат работы функции
			result = (headers1.compare(headers2) != 0);
		}
	}
	// Выводим результат
	return result;
}
/**
 * setRedirect Метод создания запроса редиректа
 * @param response объект ответа
 * @return         реузльтат установки редиректа
 */
const bool HttpData::setRedirect(HttpData &response){
	// Результат установки редиректа
	bool result = false;
	// Если данные переданы верные
	if(response.isEndHeaders()){
		// Получаем статус запроса
		const u_int status = response.getStatus();
		// Получаем адрес запроса
		const string request = response.getHeader("location");
		// Если адрес запроса существует
		if(((status > 299) && (status < 400)) && !request.empty()){
			// Выполняем получение параметров подключения
			Connect connect = getConnection(request);
			// Проверяем протокол
			if(connect.protocol.compare("http") == 0){
				// Получаем данные хоста
				const string host = getHost();
				// Получаем данные порта
				const u_int port = getPort();
				// Получаем данные порта подключения
				const u_int cport = ::atoi(connect.port.c_str());
				// Создаем реферала
				string referer = getProtocol();
				// Если хост, порт и протокол не совпадают
				if((host.compare(connect.host) != 0)
				|| (port != cport)
				|| (referer.compare(connect.protocol) != 0)){
					// Добавляем разделитель протокола
					referer.append("://");
					// Добавляем хост
					referer.append(host);
					// Добавляем порт если он не дефолтный
					if(port != 80) referer.append(string(":") + to_string(port));
					// Добавляем путь запроса
					referer.append(getPath());
					// Устанавливаем реферала
					setHeader("Referer", referer);
				// Устанавливаем путь запроса, если параметры коннекта совпадают
				} else setHeader("Referer", getPath());
				// Устанавливаем порт запроса
				setPort(cport);
				// Устанавливаем путь запроса
				setPath(connect.path);
				// Устанавливаем протокол запроса
				setProtocol(connect.protocol);
				// Если хост не является частью пути
				if(connect.path.find(connect.host) == string::npos){
					// Устанавливаем хост запроса
					setHost(connect.host);
				}
				// Запоминаем что редирект установлен
				result = true;
			}
		}
	}
	// Выводим результат
	return result;
}
/**
 * parse Метод парсинга данных
 * @param buffer  буфер с входящими запросами
 * @param size    размер входящих данных
 * @param name    название приложения
 * @param options опции http парсера
 * @return        размер обработанных байт
 */
const size_t HttpData::parse(const char * buffer, const size_t size, const string name, const u_short options){
	// Результат работы метода
	size_t maxsize = 0;
	// Если данные существуют
	if(size){
		// Результат работы регулярного выражения
		smatch match;
		// Создаем строку для поиска
		string str(buffer, size);
		// Устанавливаем правило регулярного выражения
		regex e(
			"^([^\\r\\n\\s]*)((?:(?:(?:OPTIONS|GET|HEAD|POST|PUT|PATCH|DELETE|TRACE|CONNECT)"
			"\\s+[^\\r\\n\\s]+\\s+[A-Za-z]+\\/[\\d\\.]+\\r\\n)|(?:[A-Za-z]+"
			"\\/[\\d\\.]+\\s+\\d+(?:\\s+[^\\r\\n]+)?\\r\\n))(?:[\\w\\-]+\\s*\\:"
			"\\s*[^\\r\\n]+\\r\\n)+\\r\\n)",
			regex::ECMAScript | regex::icase
		);
		// Выполняем поиск протокола
		regex_search(str, match, e);
		// Если данные найдены
		if(!match.empty()){
			// Запоминаем первые символы
			string badchars = match[1].str();
			// Увеличиваем значение общих найденных символов
			maxsize += badchars.size();
			// Получаем данные заголовков
			string headers = match[2].str();
			// Добавляем размер заголовков
			maxsize += headers.size();
			// Создаем объект
			create(name, options);
			// Выполняем инициализацию объекта
			setData(headers.c_str(), headers.size());
			// Добавляем вложенные данные
			size_t sizeBody = setEntitybody(buffer + maxsize, size - maxsize);
			// Если размер данных тела не получен
			if(!sizeBody || isEndBody()) maxsize += sizeBody;
			// Обнуляем размер обработанных данных
			else maxsize = 0;
		}
	}
	// Выводим результат
	return maxsize;
}
/**
 * getBodySize Метод получения размера тела http данных
 * @return размер тела данных
 */
const size_t HttpData::getBodySize(){
	// Выводим размер данных
	return this->body.getBodySize();
}
/**
 * getRawBodySize Метод получения размера тела http данных в чистом виде
 * @return размер тела данных
 */
const size_t HttpData::getRawBodySize(){
	// Выводим размер блока данных в чистом виде
	return this->body.getRawBody().size();
}
/**
 * setEntitybody Метод добавления данных вложения
 * @param buffer буфер с данными вложения
 * @param size   размер буфера
 * @return       количество добавленных данных
 */
const size_t HttpData::setEntitybody(const char * buffer, const size_t size){
	// Если данные переданы
	if(size && isEndHeaders()){
		// Размер вложений
		size_t length = 0;
		// Проверяем есть ли размер вложений
		string cl = getHeader("content-length");
		// Определяем размер вложений
		if(!cl.empty() && Anyks::isNumber(cl)) length = ::atoi(cl.c_str());
		// Если вложения не найдены
		else {
			// Проверяем есть ли чанкование
			string ch = getHeader("transfer-encoding");
			// Если это чанкование
			if(!ch.empty() && (ch.find("chunked") != string::npos)) length = 2;
			// Если это не автоотключение тогда копируем данные так как они есть
			else if(isClose()){
				// Результат работы регулярного выражения
				smatch match;
				// Устанавливаем правило регулярного выражения
				regex e(
					"^(?:OPTIONS|GET|HEAD|POST|PUT|PATCH|DELETE|TRACE|CONNECT)"
					"\\s+[^\\r\\n\\s]+\\s+[A-Za-z]+\\/[\\d\\.]+",
					regex::ECMAScript | regex::icase
				);
				// Выполняем поиск протокола
				regex_search(this->http, match, e);
				// Если протокол найден
				if(match.empty()) length = 1;
			}
		}
		// Добавляем данные тела
		return this->body.addData(buffer, size, length);
	}
	// Сообщаем что данные добавлены полностью
	return size;
}
/**
 * size Метод получения размеров дампа
 * @return размер дампа
 */
const size_t HttpData::size(){
	// Если размер данные не существует, выполняем генерацию данных
	if(this->raw.empty()) data();
	// Выводим результат
	return this->raw.size();
}
/**
 * getPort Метод получения порта запроса
 * @return порт удаленного ресурса
 */
const u_int HttpData::getPort(){
	// Выводим значение переменной
	return (!this->port.empty() ? ::atoi(this->port.c_str()) : 80);
}
/**
 * getStatus Метод получения статуса запроса
 * @return статус запроса
 */
const u_int HttpData::getStatus(){
	// Выводим значение переменной
	return this->status;
}
/**
 * getVersion Метод получения версии протокола запроса
 * @return версия протокола запроса
 */
const float HttpData::getVersion(){
	// Выводим значение переменной
	return (!this->version.empty() ? ::atof(this->version.c_str()) : 1.0);
}
/**
 * getHttp Метод получения http запроса
 * @return http запрос
 */
const string HttpData::getHttp(){
	// Выводим значение переменной
	return this->http;
}
/**
 * getMethod Метод получения метода запроса
 * @return метод запроса
 */
const string HttpData::getMethod(){
	// Выводим значение переменной
	return this->method;
}
/**
 * getHost Метод получения хоста запроса
 * @return хост запроса
 */
const string HttpData::getHost(){
	// Выводим значение переменной
	return this->host;
}
/**
 * getPath Метод получения пути запроса
 * @return путь запроса
 */
const string HttpData::getPath(){
	// Выводим значение переменной
	return this->path;
}
/**
 * getProtocol Метод получения протокола запроса
 * @return протокол запроса
 */
const string HttpData::getProtocol(){
	// Выводим значение переменной
	return this->protocol;
}
/**
 * getAuth Метод получения метода авторизации запроса
 * @return метод авторизации
 */
const string HttpData::getAuth(){
	// Выводим значение переменной
	return this->auth;
}
/**
 * getLogin Метод получения логина авторизации запроса
 * @return логин авторизации
 */
const string HttpData::getLogin(){
	// Выводим значение переменной
	return this->login;
}
/**
 * getPassword Метод получения пароля авторизации запроса
 * @return пароль авторизации
 */
const string HttpData::getPassword(){
	// Выводим значение переменной
	return this->password;
}
/**
 * getUseragent Метод получения юзерагента запроса
 * @return юзерагент
 */
const string HttpData::getUseragent(){
	// Выводим значение переменной
	return getHeader("user-agent");
}
/**
 * getHeader Метод извлекает данные заголовка по его ключу
 * @param  key ключ заголовка
 * @return     строка с данными заголовка
 */
const string HttpData::getHeader(const string key){
	// Выводим данные заголовков
	return this->headers.getHeader(key).value;
}
/**
 * getResponseHeaders Метод получения заголовков http ответа
 * @return сформированные заголовки ответа
 */
const string HttpData::getResponseHeaders(){
	// Получаем данные ответа
	return createHeadResponse();
}
/**
 * getRequestHeaders Метод получения заголовков http запроса
 * @return сформированные заголовки запроса
 */
const string HttpData::getRequestHeaders(){
	// Получаем данные запроса
	return createHeadRequest();
}
/**
 * getBody Метод получения данных тела http
 * @param  chunked метод чанкование
 * @return         строка с данными тела
 */
const string HttpData::getBody(const bool chunked){
	// Данные тела
	string body;
	// Если данные тела получены
	if(isEndBody()){
		// Если не активно сжатие, снимает режим сжатия и здесь
		if(!this->body.isIntCompress()
		&& !this->body.isExtCompress()) this->unsetGzip();
		// Если это чанкование
		if(chunked){
			// Удаляем из заголовков, заголовок размера
			rmHeader("content-length");
			// Устанавливаем заголовок что данные в виде чанков
			setHeader("Transfer-Encoding", "chunked");
		// Если это извлечение данных конкретного размера
		} else {
			// Удаляем из заголовков, заголовок передачи данных чанками
			rmHeader("transfer-encoding");
			// Устанавливаем размер входящих данных
			setHeader("Content-Length", to_string(getBodySize()));
		}
		// Запоминаем результат
		body = this->body.getBody(chunked);
	}
	// Выводим результат
	return body;
}
/**
 * getResponseData Метод получения http данных ответа
 * @param  chunked метод чанкование
 * @return строка с данными тела
 */
const string HttpData::getResponseData(const bool chunked){
	// Результирующий объект данных
	string result;
	// Получаем данные тела
	string body = getBody(chunked);
	// Получаем данные заголовков
	string headers = getResponseHeaders();
	// Если заголовки и тело существуют
	if(!headers.empty()){
		// Выполняем добавление заголовков в результат
		result.append(headers);
		// Копируем данные тела
		if(!body.empty() && isEndBody()) result.append(body);
		// Если тело существует но заполненно не полностью, очищаем вывод данных
		else if(!body.empty()) result.clear();
	}
	// Выводим результат
	return result;
}
/**
 * getRequestData Метод получения http данных запроса
 * @return строка с данными тела
 */
const string HttpData::getRequestData(){
	// Результирующий объект данных
	string result;
	// Получаем данные тела
	string body = getBody(false);
	// Получаем данные заголовков
	string headers = getRequestHeaders();
	// Если заголовки и тело существуют
	if(!headers.empty()){
		// Выполняем добавление заголовков в результат
		result.append(headers);
		// Копируем данные тела
		if(!body.empty() && isEndBody()) result.append(body);
		// Если тело существует но заполненно не полностью, очищаем вывод данных
		else if(!body.empty()) result.clear();
	}
	// Выводим результат
	return result;
}
/**
 * getRawResponseData Метод получения http данных ответа в чистом виде
 * @return строка с данными ответа
 */
const string HttpData::getRawResponseData(){
	// Результирующая строка данных
	string result;
	// Если данные тела получены
	if(isEndBody()){
		// Получаем данные тела
		string body = this->body.getRawBody();
		// Удаляем из заголовков, заголовок передачи данных чанками
		rmHeader("transfer-encoding");
		// Устанавливаем размер входящих данных
		setHeader("Content-Length", to_string(body.size()));
		// Получаем данные заголовков
		string headers = getResponseHeaders();
		// Создаем регулярное выражение
		regex ce("Content\\-Encoding\\s*\\:[^\\r\\n]*gzip[^\\r\\n]*\\r\\n", regex::ECMAScript | regex::icase);
		// Выполняем удаление заголовка
		headers = regex_replace(headers, ce, "");
		// Создаем регулярное выражение
		regex et("ETag\\s*\\:\\s*W\\/", regex::ECMAScript | regex::icase);
		// Выполняем замену заголовка
		headers = regex_replace(headers, et, "ETag: ");
		// Если заголовки и тело существуют
		if(!headers.empty()){
			// Выполняем добавление заголовков в результат
			result.append(headers);
			// Копируем данные тела
			if(!body.empty()) result.append(body);
		}
	}
	// Выводим результат
	return result;
}
/**
 * getRawRequestData Метод получения http данных запроса в чистом виде
 * @return строка с данными запроса
 */
const string HttpData::getRawRequestData(){
	// Результирующая строка данных
	string result;
	// Если данные тела получены
	if(isEndBody()){
		// Получаем данные тела
		string body = this->body.getRawBody();
		// Удаляем из заголовков, заголовок передачи данных чанками
		rmHeader("transfer-encoding");
		// Устанавливаем размер входящих данных
		setHeader("Content-Length", to_string(body.size()));
		// Получаем данные заголовков
		string headers = getRequestHeaders();
		// Создаем регулярное выражение
		regex ce("Content\\-Encoding\\s*\\:[^\\r\\n]*gzip[^\\r\\n]*\\r\\n", regex::ECMAScript | regex::icase);
		// Выполняем удаление заголовка
		headers = regex_replace(headers, ce, "");
		// Создаем регулярное выражение
		regex et("ETag\\s*\\:\\s*W\\/", regex::ECMAScript | regex::icase);
		// Выполняем замену заголовка
		headers = regex_replace(headers, et, "ETag: ");
		// Если заголовки и тело существуют
		if(!headers.empty()){
			// Выполняем добавление заголовков в результат
			result.append(headers);
			// Копируем данные тела
			if(!body.empty()) result.append(body);
		}
	}
	// Выводим результат
	return result;
}
/**
 * data Метод создания дампа
 * @return сформированный блок дампа
 */
const u_char * HttpData::data(){
	// Если данные получены
	if(isEndBody() && isEndHeaders()){
		// Объект размерности данных
		Dump sizes = {
			sizeof(this->status),
			sizeof(this->options),
			this->http.size(),
			this->auth.size(),
			this->path.size(),
			this->host.size(),
			this->port.size(),
			this->login.size(),
			this->method.size(),
			this->appName.size(),
			this->version.size(),
			this->protocol.size(),
			this->password.size(),
			this->appVersion.size(),
			this->headers.size(),
			this->body.size()
		};
		// Получаем размер структуры
		const size_t size = sizeof(sizes);
		// Получаем данные карты размеров
		const u_char * map = reinterpret_cast <const u_char *> (&sizes);
		// Выполняем копирование карты размеров
		copy(map, map + size, back_inserter(this->raw));
		// Получаем данные статуса
		const u_char * status = reinterpret_cast <const u_char *> (&this->status);
		// Выполняем копирование статуса
		copy(status, status + sizes.status, back_inserter(this->raw));
		// Получаем данные настроек
		const u_char * options = reinterpret_cast <const u_char *> (&this->options);
		// Выполняем копирование данных настроек
		copy(options, options + sizes.options, back_inserter(this->raw));
		// Выполняем копирование данных http запроса
		copy(this->http.begin(), this->http.end(), back_inserter(this->raw));
		// Выполняем копирование данных типа авторизации
		copy(this->auth.begin(), this->auth.end(), back_inserter(this->raw));
		// Выполняем копирование данных пути запроса
		copy(this->path.begin(), this->path.end(), back_inserter(this->raw));
		// Выполняем копирование данных хоста
		copy(this->host.begin(), this->host.end(), back_inserter(this->raw));
		// Выполняем копирование данных порта
		copy(this->port.begin(), this->port.end(), back_inserter(this->raw));
		// Выполняем копирование данных логина
		copy(this->login.begin(), this->login.end(), back_inserter(this->raw));
		// Выполняем копирование данных метода запроса
		copy(this->method.begin(), this->method.end(), back_inserter(this->raw));
		// Выполняем копирование данных названия приложения
		copy(this->appName.begin(), this->appName.end(), back_inserter(this->raw));
		// Выполняем копирование данных версии протокола
		copy(this->version.begin(), this->version.end(), back_inserter(this->raw));
		// Выполняем копирование данных протокола
		copy(this->protocol.begin(), this->protocol.end(), back_inserter(this->raw));
		// Выполняем копирование данных пароля
		copy(this->password.begin(), this->password.end(), back_inserter(this->raw));
		// Выполняем копирование данных версии приложения
		copy(this->appVersion.begin(), this->appVersion.end(), back_inserter(this->raw));
		// Получаем данные заголовков
		const u_char * headers = this->headers.data();
		// Выполняем копирование данных заголовков
		copy(headers, headers + sizes.headers, back_inserter(this->raw));
		// Получаем данные тела
		const u_char * body = this->body.data();
		// Выполняем копирование данных тела
		copy(body, body + sizes.body, back_inserter(this->raw));
	}
	// Выводим сформированные данные
	return this->raw.data();
}
/**
 * set Метод установки сырых данных
 * @param data сырые данные
 * @param size размер сырых данных
 */
void HttpData::set(const u_char * data, size_t size){
	// Если данные существуют
	if(size){
		// Получаем размер структуры
		size_t size_map = sizeof(Dump);
		// Если размер карты меньше общего размера
		if(size_map < size){
			// Очищаем блок данных
			clear();
			// Размеры полученных данных
			size_t size_it = size_map;
			// Извлекаем данные карты размеров
			for(size_t i = 0, j = 0; i < size_map; i += sizeof(size_t), j++){
				// Размер полученных данных
				size_t size_data;
				// Извлекаем размер данных
				memcpy(&size_data, data + i, sizeof(size_t));
				// Если данные верные
				if(size_data && ((size_data + size_it) <= size)){
					// Определяем тип извлекаемых данных
					switch(j){
						// Если это статус запроса
						case 0: Anyks::cpydata(data, size_data, size_it, &this->status); break;
						// Если это настройки
						case 1: Anyks::cpydata(data, size_data, size_it, &this->options); break;
						// Если это http запрос
						case 2: Anyks::cpydata(data, size_data, size_it, this->http); break;
						// Если это тип авторизации
						case 3: Anyks::cpydata(data, size_data, size_it, this->auth); break;
						// Если это путь запроса
						case 4: Anyks::cpydata(data, size_data, size_it, this->path); break;
						// Если это хост запроса
						case 5: Anyks::cpydata(data, size_data, size_it, this->host); break;
						// Если это порт запроса
						case 6: Anyks::cpydata(data, size_data, size_it, this->port); break;
						// Если это логин пользователя
						case 7: Anyks::cpydata(data, size_data, size_it, this->login); break;
						// Если это метод запроса
						case 8: Anyks::cpydata(data, size_data, size_it, this->method); break;
						// Если это название приложения
						case 9: Anyks::cpydata(data, size_data, size_it, this->appName); break;
						// Если это версия протокола
						case 10: Anyks::cpydata(data, size_data, size_it, this->version); break;
						// Если это протокол запроса
						case 11: Anyks::cpydata(data, size_data, size_it, this->protocol); break;
						// Если это пароль пользователя
						case 12: Anyks::cpydata(data, size_data, size_it, this->password); break;
						// Если это версия приложения
						case 13: Anyks::cpydata(data, size_data, size_it, this->appVersion); break;
						// Если это заголовки запроса
						case 14: {
							// Выделяем динамически память
							u_char * buffer = new u_char [size_data];
							// Извлекаем данные адреса
							memcpy(buffer, data + size_it, size_data);
							// Запоминаем результат
							this->headers.set(buffer, size_data);
							// Определяем смещение
							size_it += size_data;
							// Удаляем полученные данные
							delete [] buffer;
						} break;
						// Если это тело запроса
						case 15: {
							// Выделяем динамически память
							u_char * buffer = new u_char [size_data];
							// Извлекаем данные адреса
							memcpy(buffer, data + size_it, size_data);
							// Добавляем размер тела, необходимо для того, чтобы включить сжатие
							// P.S. больше ни на что не влияет, так как при выводе данных этот заголовок генерируется заново
							setHeader("Content-Length", to_string(size_data));
							// Выполняем проверку сжатия, если разрешено то устанавливаем
							if(compressIsAllowed()) setGzip();
							// Выполняем инициализацию тела
							initBody();
							// Запоминаем результат
							this->body.set(buffer, size_data);
							// Определяем смещение
							size_it += size_data;
							// Удаляем полученные данные
							delete [] buffer;
						} break;
					}
				}
			}
		}
	}
}
/**
 * clear Метод очистки структуры
 */
void HttpData::clear(){
	// Обнуляем статус запроса
	this->status = 0;
	// Устанавливаем что режим сжатия отключен
	this->intGzip = false;
	this->extGzip = false;
	this->http.clear();
	this->auth.clear();
	this->method.clear();
	this->path.clear();
	this->protocol.clear();
	this->version.clear();
	this->host.clear();
	this->port.clear();
	this->login.clear();
	this->password.clear();
	// Очищаем карту заголовков
	this->headers.clear();
	// Очищаем тело
	this->body.clear();
	// Очищаем сырые данные
	this->raw.clear();
}
/**
 * initBody Метод инициализации объекта тела
 */
void HttpData::initBody(){
	// Получаем уровень сжатия
	int level = (this->gzipParams ? this->gzipParams->level : Z_DEFAULT_COMPRESSION);
	// Получаем размер чанков
	size_t chunk = (this->gzipParams ? this->gzipParams->chunk : MOD_GZIP_ZLIB_CHUNK);
	// Выполняем создание объекта body
	HttpBody body = HttpBody(chunk, level, this->intGzip, this->extGzip);
	// Запоминаем строку body
	this->body = body;
}
/**
 * rmHeader Метод удаления заголовка
 * @param key название заголовка
 */
void HttpData::rmHeader(const string key){
	// Если параметры пришли верные
	if(!key.empty()) this->headers.remove(key);
}
/**
 * unsetGzip Метод снятия режима сжатия gzip
 */
void HttpData::unsetGzip(){
	// Если один из режимов активирован
	if(this->intGzip || this->extGzip){
		// Получаем данные заголовка etag
		string etag = this->getHeader("etag");
		// Если Etag существует
		if(!etag.empty() && (etag.find("W/") != string::npos)){
			// Изменяем заголовок Etag
			this->setHeader("ETag", etag.replace(0, 2, ""));
		}
		// Удаляем ненужные заголовки
		this->rmHeader("content-encoding");
		// Запоминаем что режим gzip активирован
		this->intGzip = false;
		this->extGzip = false;
	}
}
/**
 * setGzip Метод установки режима сжатия gzip
 * @param intGzip активация внутреннего режима сжатия
 * @param extGzip активация внешнего режима сжатия
 */
void HttpData::setGzip(const bool intGzip, const bool extGzip){
	// Если один из режимов активирован
	if(intGzip || extGzip){
		// Получаем данные заголовка etag
		string etag = this->getHeader("etag");
		// Если Etag существует
		if(!etag.empty() && (etag.find("W/") == string::npos)){
			// Изменяем заголовок Etag
			this->setHeader("ETag", string("W/") + etag);
		}
		// Устанавливаем заголовок что данные придут сжатые
		this->setHeader("Content-Encoding", "gzip");
		// Удаляем ненужные заголовки
		this->rmHeader("accept-ranges");
		// Запоминаем что режим gzip активирован
		this->intGzip = intGzip;
		this->extGzip = extGzip;
	}
}
/**
 * setBodyEnd Метод установки завершения сбора данных тела
 */
void HttpData::setBodyEnd(){
	// Сообщаем что данные все собраны
	this->body.setEnd();
}
/**
 * setHeader Метод добавления нового заголовка
 * @param key   ключ
 * @param value значение
 */
void HttpData::setHeader(const string key, const string value){
	// Если параметры пришли верные
	if(!key.empty() && !value.empty()){
		// Устанавливаем заголовок
		this->headers.append(key, value);
	}
}
/**
 * setData Метод добавления данных
 * @param  buffer  буфер с http данными
 * @param  size    размер http данных
 */
void HttpData::setData(const char * buffer, const size_t size){
	// Проверяем существуют ли данные
	if(size){
		// Очищаем полученные данные
		clear();
		// Результат работы регулярного выражения
		smatch match;
		// Создаем строку с данными
		string str(buffer, size);
		// Устанавливаем правило регулярного выражения
		regex e(
			"^((?:(?:OPTIONS|GET|HEAD|POST|PUT|PATCH|DELETE|TRACE|CONNECT)"
			"\\s+[^\\r\\n\\s]+\\s+[A-Za-z]+\\/([\\d\\.]+)\\r\\n)|(?:[A-Za-z]+"
			"\\/([\\d\\.]+)\\s+(\\d+)(?:\\s+[^\\r\\n]+)?\\r\\n))((?:[\\w\\-]+\\s*\\:"
			"\\s*[^\\r\\n]+\\r\\n)+\\r\\n)",
			regex::ECMAScript | regex::icase
		);
		// Выполняем поиск протокола
		regex_search(str, match, e);
		// Если данные найдены
		if(!match.empty()){
			// Определяем версию протокола
			string version = match[2].str();
			// Если версия не существует получаем второе значение
			if(version.empty()) version = match[3].str();
			// Запоминаем версию протокола
			this->version = version;
			// Получаем строку запроса
			this->http = match[1].str();
			// Запоминаем http запрос
			this->http = Anyks::trim(this->http);
			// Запоминаем статус запроса
			this->status = ::atoi(match[4].str().c_str());
			// Создаем объект с заголовками
			this->headers.create(match[0].str().c_str());
		}
		// Генерируем данные подключения
		genDataConnect();
	}
}
/**
 * setGzipParams Метод установки параметров сжатия gzip
 * @param params параметры сжатия
 */
void HttpData::setGzipParams(void * params){
	// Если параметры переданы
	if(params) this->gzipParams = reinterpret_cast <Gzip *> (params);
}
/**
 * setOptions Метод установки настроек прокси сервера
 * @param options данные для установки
 */
void HttpData::setOptions(const u_short options){
	// Запоминаем данные
	this->options = options;
}
/**
 * setMethod Метод установки метода запроса
 * @param str строка с данными для установки
 */
void HttpData::setMethod(const string str){
	// Запоминаем данные
	this->method = str;
}
/**
 * setHost Метод установки хоста запроса
 * @param str строка с данными для установки
 */
void HttpData::setHost(const string str){
	// Запоминаем данные
	this->host = str;
}
/**
 * setPort Метод установки порта запроса
 * @param number номер порта для установки
 */
void HttpData::setPort(const u_int number){
	// Запоминаем данные
	this->port = to_string(number);
}
/**
 * setStatus Метод установки статуса запроса
 * @param number номер статуса для установки
 */
void HttpData::setStatus(const u_int number){
	// Запоминаем данные
	this->status = number;
}
/**
 * setPath Метод установки пути запроса
 * @param str строка с данными для установки
 */
void HttpData::setPath(const string str){
	// Запоминаем данные
	this->path = str;
}
/**
 * setProtocol Метод установки протокола запроса
 * @param str строка с данными для установки
 */
void HttpData::setProtocol(const string str){
	// Запоминаем данные
	this->protocol = str;
}
/**
 * setVersion Метод установки версии протокола запроса
 * @param number номер версии протокола
 */
void HttpData::setVersion(const float number){
	// Запоминаем данные
	this->version = to_string(number);
	// Если это всего один символ тогда дописываем ноль
	if(this->version.length() == 1) this->version += ".0";
}
/**
 * setAuth Метод установки метода авторизации запроса
 * @param str строка с данными для установки
 */
void HttpData::setAuth(const string str){
	// Запоминаем данные
	this->auth = str;
}
/**
 * setUseragent Метод установки юзерагента запроса
 * @param str строка с данными для установки
 */
void HttpData::setUseragent(const string str){
	// Запоминаем данные
	setHeader("User-Agent", str);
}
/**
 * setClose Метод установки принудительного отключения после запроса
 */
void HttpData::setClose(){
	// Запоминаем данные
	setHeader("Connection", "close");
}
/**
 * addHeader Метод добавления нового заголовка
 * @param buffer буфер с данными заголовка
 */
void HttpData::addHeader(const char * buffer){
	// Если данные существуют
	if(strlen(buffer)){
		// Результат работы регулярного выражения
		smatch match;
		// Устанавливаем правило регулярного выражения
		regex e(
			"((?:(?:OPTIONS|GET|HEAD|POST|PUT|PATCH|DELETE|TRACE|CONNECT)"
			"\\s+[^\\r\\n\\s]+\\s+[A-Za-z]+\\/([\\d\\.]+))|"
			"(?:[A-Za-z]+\\/([\\d\\.]+)\\s+(\\d+)(?:\\s+[A-Za-z\\s\\-]+)?))|"
			"(?:([\\w\\-]+)\\s*\\:\\s*([^\\r\\n]+))",
			regex::ECMAScript | regex::icase
		);
		// Запоминаем входящую строку
		string str = buffer;
		// Выполняем поиск протокола
		regex_search(str, match, e);
		// Если данные найдены
		if(!match.empty()){
			// Если найдены заголовки
			if(match[1].str().empty()){
				// Получаем данные заголовков
				string key = match[5].str();
				string val = match[6].str();
				// Добавляем новый заголовок
				this->headers.append(key, val);
				// Если сжатие активировано
				if((Anyks::toCase(key).compare("content-encoding") == 0)
				&& (val.find("gzip") != string::npos)) this->extGzip = true;
			// Запоминаем результат запроса или ответа
			} else {
				// Очищаем все заголовки
				clear();
				// Определяем версию протокола
				string version = match[2].str();
				// Если версия не существует получаем второе значение
				if(version.empty()) version = match[3].str();
				// Запоминаем http запрос
				this->http = match[1].str();
				// Запоминаем версию протокола
				this->version = version;
				// Запоминаем статус запроса
				this->status = ::atoi(match[4].str().c_str());
			}
		}
	// Если данные пришли пустые значит они существуют и это завершение заголовков
	} else if(buffer) this->headers.setEnd();
}
/**
 * largeRequest Метод генерации ответа (слишком большой размер файла)
 */
void HttpData::largeRequest(){
	// Устанавливаем тело с данными
	createRequest(509);
}
/**
 * brokenRequest Метод генерации ответа (неудачного отправленного запроса)
 */
void HttpData::brokenRequest(){
	// Устанавливаем тело с данными
	createRequest(510);
}
/**
 * faultConnect Метод генерации ответа (неудачного подключения к удаленному серверу)
 */
void HttpData::faultConnect(){
	// Устанавливаем тело с данными
	createRequest(502);
}
/**
 * pageNotFound Метод генерации ответа (страница не найдена)
 */
void HttpData::pageNotFound(){
	// Устанавливаем тело с данными
	createRequest(404);
}
/**
 * faultAuth Метод генерации ответа (неудачной авторизации)
 */
void HttpData::faultAuth(){
	// Устанавливаем тело с данными
	createRequest(403);
}
/**
 * requiredAuth Метод генерации ответа (запроса ввода логина и пароля)
 */
void HttpData::requiredAuth(){
	// Устанавливаем тело с данными
	createRequest(407);
}
/**
 * authSuccess Метод генерации ответа (подтверждения авторизации)
 */
void HttpData::authSuccess(){
	// Устанавливаем тело с данными
	createRequest(200);
}
/**
 * init Метод создания объекта
 * @param name    название приложения
 * @param options опции http парсера
 */
void HttpData::create(const string name, const u_short options){
	// Очищаем полученные данные
	clear();
	// Запоминаем название приложения
	this->appName = name;
	// Запоминаем параметры http парсера
	this->options = options;
	// Запоминаем версию приложения
	this->appVersion = APP_VERSION;
}
/**
 * HttpData Конструктор
 * @param name    название приложения
 * @param options опции http парсера
 */
HttpData::HttpData(const string name, const u_short options){
	// Создаем шаблоны ответов
	this->response.insert(pair <u_short, Http> (100, {"Continue", "\r\n", ""}));
	this->response.insert(pair <u_short, Http> (200, {"Connection established", "\r\n", "Ok"}));
	this->response.insert(pair <u_short, Http> (400, {
		"Bad Request",
		"Proxy-Connection: close\r\n"
		"Connection: close\r\n"
		"Content-type: text/html; charset=utf-8\r\n"
		"\r\n",
		"<html><head><title>400 Bad Request</title></head>\r\n"
		"<body><h2>400 Bad Request</h2></body></html>\r\n"
	}));
	this->response.insert(pair <u_short, Http> (403, {
		"Forbidden",
		"Proxy-Connection: close\r\n"
		"Connection: close\r\n"
		"Content-type: text/html; charset=utf-8\r\n"
		"\r\n",
		"<html><head><title>403 Access Denied</title></head>\r\n"
		"<body><h2>403 Access Denied</h2><h3>Access control list denies you to access this resource</body></html>\r\n"
	}));
	this->response.insert(pair <u_short, Http> (404, {
		"Not Found",
		"Proxy-Connection: close\r\n"
		"Connection: close\r\n"
		"Content-type: text/html; charset=utf-8\r\n"
		"\r\n",
		"<html><head><title>404 Not Found</title></head>\r\n"
		"<body><h2>404 Not Found</h2><h3>Page not found</body></html>\r\n"
	}));
	this->response.insert(pair <u_short, Http> (407, {
		"Proxy Authentication Required",
		"Proxy-Authenticate: Basic realm=\"proxy\"\r\n"
		"Proxy-Connection: close\r\n"
		"Connection: close\r\n"
		"Content-type: text/html; charset=utf-8\r\n"
		"\r\n",
		"<html><head><title>407 Proxy Authentication Required</title></head>\r\n"
		"<body><h2>407 Proxy Authentication Required</h2>\r\n"
		"<h3>Access to requested resource disallowed by administrator or you need valid username/password to use this resource</h3>\r\n"
		"</body></html>\r\n"
	}));
	this->response.insert(pair <u_short, Http> (500, {
		"Internal Error",
		"Proxy-Connection: close\r\n"
		"Connection: close\r\n"
		"Content-type: text/html; charset=utf-8\r\n"
		"\r\n",
		"<html><head><title>500 Internal Error</title></head>\r\n"
		"<body><h2>500 Internal Error</h2><h3>Internal proxy error during processing your request</h3></body></html>\r\n"
	}));
	this->response.insert(pair <u_short, Http> (502, {
		"Bad Gateway",
		"Proxy-Connection: close\r\n"
		"Connection: close\r\n"
		"Content-type: text/html; charset=utf-8\r\n"
		"\r\n",
		"<html><head><title>502 Bad Gateway</title></head>\r\n"
		"<body><h2>502 Bad Gateway</h2><h3>Failed to connect server</h3></body></html>\r\n"
	}));
	this->response.insert(pair <u_short, Http> (503, {
		"Service Unavailable",
		"Proxy-Connection: close\r\n"
		"Connection: close\r\n"
		"Content-type: text/html; charset=utf-8\r\n"
		"\r\n",
		"<html><head><title>503 Service Unavailable</title></head>\r\n"
		"<body><h2>503 Service Unavailable</h2><h3>Recursion detected</h3></body></html>\r\n"
	}));
	this->response.insert(pair <u_short, Http> (509, {
		"Bandwidth Limit Exceeded",
		"Proxy-Connection: close\r\n"
		"Connection: close\r\n"
		"Content-type: text/html; charset=utf-8\r\n"
		"\r\n",
		"<html><head><title>509 Bandwidth Limit Exceeded</title></head>\r\n"
		"<body><h2>509 Bandwidth Limit Exceeded</h2><h3>Data or File too large size</h3></body></html>\r\n"
	}));
	this->response.insert(pair <u_short, Http> (510, {
		"Not Extended",
		"Proxy-Connection: close\r\n"
		"Connection: close\r\n"
		"Content-type: text/html; charset=utf-8\r\n"
		"\r\n",
		"<html><head><title>510 Not Extended</title></head>\r\n"
		"<body><h2>510 Not Extended</h2><h3>Required action is not extended by proxy server</h3></body></html>\r\n"
	}));
	// Выполняем создание объекта
	create(name, options);
}
/**
 * ~HttpData Деструктор
 */
HttpData::~HttpData(){
	// Очищаем полученные данные
	clear();
	// Очищаем память выделенную для вектора
	vector <u_char> ().swap(this->raw);
}
/**
 * isHttp Метод проверки на то http это или нет
 * @param  buffer буфер входящих данных
 * @return        результат проверки
 */
const bool Http::isHttp(const string buffer){
	// Если буфер существует
	if(!buffer.empty()){
		// Создаем новый буфер
		char buf[4];
		// Шаблон основных комманд
		char cmds[8][4] = {"get", "hea", "pos", "put", "pat", "del", "tra", "con"};
		// Копируем нужное количество символов
		strncpy(buf, buffer.c_str(), 3);
		// Устанавливаем завершающий символ
		buf[3] = '\0';
		// Переходим по всему массиву команд
		for(u_short i = 0; i < 8; i++) if(Anyks::toCase(buf).compare(cmds[i]) == 0) return true;
	}
	// Сообщаем что это не http
	return false;
}
/**
 * parse Функция извлечения данных из буфера
 * @param buffer буфер с входящими запросами
 * @param size   размер входящих данных
 * @return       количество обработанных байтов
 */
const size_t Http::parse(const char * buffer, const size_t size){
	// Выполняем парсинг http запроса
	HttpData httpData;
	// Выполняем парсинг входящих данных
	size_t bytes = httpData.parse(buffer, size, this->name, this->options);
	// Добавляем в массив объект подключения
	if(bytes) this->httpData.push_back(httpData);
	// Выводим результат
	return bytes;
}
/**
 * modify Функция модифицирования ответных данных
 * @param data ссылка на данные полученные от сервера
 */
void Http::modify(vector <char> &data){
	// Получаем данные ответа
	const char * headers = data.data();
	// Создаем объект запроса
	HttpData httpData;
	// Создаем объект
	httpData.create(this->name, this->options);
	// Выполняем инициализацию объекта
	httpData.setData(headers, data.size());
	// Если данные распарсены
	if(httpData.isEndHeaders()){
		// Если завершение заголовка найдено
		size_t pos = (strstr(headers, "\r\n\r\n") - headers) + 4;
		// Получаем данные запроса
		string last(data.data() + pos, data.size() - pos);
		// Получаем данные заголовков
		string query = httpData.getResponseHeaders();
		// Объединяем блоки
		query.append(last);
		// Заменяем первоначальный блок с данными
		data.assign(query.begin(), query.end());
	}
}
/**
 * Http::clear Метод очистки всех полученных данных
 */
void Http::clear(){
	// Очищаем массив с объектами запросов
	this->httpData.clear();
}
/**
 * create Метод создания объекта
 * @param name    строка содержащая название прокси сервера
 * @param options параметры прокси сервера
 */
void Http::create(const string name, const u_short options){
	// Если имя передано то запоминаем его
	this->name = name;
	// Запоминаем тип прокси сервера
	this->options = options;
	// Устанавливаем версию системы
	this->version = APP_VERSION;
}
/**
 * Http Конструктор
 * @param name    строка содержащая название прокси сервера
 * @param options параметры прокси сервера
 */
Http::Http(const string name, const u_short options){
	// Выполняем создание объекта
	create(name, options);
}
/**
 * ~Http Деструктор
 */
Http::~Http(){
	// Очищаем полученные данные
	clear();
	// Очищаем память выделенную для вектора
	vector <HttpData> ().swap(this->httpData);
}