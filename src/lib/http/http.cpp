/* HTTP ПАРСЕР ANYKS */
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
 * isNumber Функция проверки является ли строка числом
 * @param  str строка для проверки
 * @return     результат проверки
 */
bool isNumber(const string &str){
	return !str.empty() && find_if(str.begin(), str.end(), [](char c){
		return !isdigit(c);
	}) == str.end();
}
/**
 * rtrim Функция усечения указанных символов с правой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & rtrim(string &str, const char * t = " \t\n\r\f\v"){
	str.erase(str.find_last_not_of(t) + 1);
	return str;
}
/**
 * ltrim Функция усечения указанных символов с левой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & ltrim(string &str, const char * t = " \t\n\r\f\v"){
	str.erase(0, str.find_first_not_of(t));
	return str;
}
/**
 * trim Функция усечения указанных символов с правой и левой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & trim(string &str, const char * t = " \t\n\r\f\v"){
	return ltrim(rtrim(str, t), t);
}
/**
 * checkPort Функция проверки на качество порта
 * @param  port входная строка якобы содержащая порт
 * @return      результат проверки
 */
bool checkPort(string str){
	// Если строка существует
	if(!str.empty()){
		// Преобразуем строку в цифры
		if(::isNumber(str)){
			// Получаем порт
			u_int port = ::atoi(str.c_str());
			// Проверяем диапазон портов
			if((port > 0) && (port < 65536)) return true;
		}
	}
	// Сообщаем что ничего не нашли
	return false;
}
/**
 * replace Функция замены указанных фраз в строке
 * @param s строка в которой происходит замена
 * @param f искомая строка
 * @param r строка на замену
 */
void replace(string &s, const string f, const string r){
	// Переходим по всем найденным элементам и заменяем в них искомые фразы
	for(string::size_type n = 0; (n = s.find(f, n)) != string::npos; ++n){
		// Заменяем искомую фразу указанной
		s.replace(n, f.length(), r);
	}
}
/**
 * split Функция разделения строк на составляющие
 * @param str   строка для поиска
 * @param delim разделитель
 * @param v     результирующий вектор
 */
void HttpHeaders::split(const string &str, const string delim, vector <string> &v){
	string::size_type i = 0;
	string::size_type j = str.find(delim);
	u_int len = delim.length();
	// Выполняем разбиение строк
	while(j != string::npos){
		v.push_back(str.substr(i, j - i));
		i = ++j + (len - 1);
		j = str.find(delim, j);
		if(j == string::npos) v.push_back(str.substr(i, str.length()));
	}
}
/**
 * getHeader Метод извлекает данные заголовка по его ключу
 * @param  key ключ заголовка
 * @return     строка с данными заголовка
 */
HttpHeaders::Header HttpHeaders::getHeader(string key){
	// Переходим по всему массиву и ищем там нужный нам заголовок
	for(u_int i = 0; i < this->headers.size(); i++){
		// Если заголовок найден
		if(::toCase(this->headers[i].head)
		.compare(::toCase(key)) == 0){
			// Выводим результат
			return this->headers[i];
		}
	}
	// Сообщаем что ничего не найдено
	return {"", ""};
}
/**
 * clear Метод очистки данных
 */
void HttpHeaders::clear(){
	// Очищаем данные http заголовков
	this->headers.clear();
}
/**
 * remove Метод удаления заголовка по ключу
 * @param key ключ заголовка
 */
void HttpHeaders::remove(const string key){
	// Присваиваем значения строк
	string ckey = key;
	// Убираем пробелы
	ckey = ::trim(ckey);
	// Ищем такой ключ
	for(size_t i = 0; i < this->headers.size(); i++){
		// Если найшли наш ключ
		if(::toCase(this->headers[i].head)
		.compare(::toCase(key)) == 0){
			// Удаляем указанный заголовок
			this->headers.erase(this->headers.begin() + i);
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
void HttpHeaders::append(const string key, const string val){
	// Присваиваем значения строк
	string ckey = key;
	string cval = val;
	// Убираем пробелы
	ckey = ::trim(ckey);
	cval = ::trim(cval);
	// Удаляем сначала исходный заголовок
	remove(ckey);
	// Запоминаем найденны параметры
	this->headers.push_back({ckey, cval});
}
/**
 * create Метод создания объекта http заголовков
 * @param buffer буфер с текстовыми данными
 */
bool HttpHeaders::create(const char * buffer){
	// Очищаем заголовки
	clear();
	// Запоминаем буфер входящих данных
	string str = buffer;
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e(
		"^((?:(?:(?:OPTIONS|GET|HEAD|POST|PUT|PATCH|DELETE|TRACE|CONNECT)"
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
		str = match[1].str();
		// Массив строк
		vector <string> strings;
		// Выполняем разбиение строк
		split(str, "\r\n", strings);
		// Если строки найдены
		if(!strings.empty()){
			// Переходим по всему массиву строк
			for(u_int i = 1; i < strings.size(); i++){
				// Результат работы регулярного выражения
				smatch match;
				// Устанавливаем правило регулярного выражения
				regex e("\\b([\\w\\-]+)\\s*\\:\\s*([\\s\\S]+)", regex::ECMAScript | regex::icase);
				// Выполняем поиск протокола
				regex_search(strings[i], match, e);
				// Если заголовок найден
				if(!match.empty() && (match.size() == 3)){
					// Добавляем данные заголовка
					this->append(match[1].str(), match[2].str());
				}
			}
			// Если данные получены
			return this->headers.size();
		}
	}
	// Сообщаем что ничего не вышло
	return false;
}
/**
 * empty Метод определяет наличие данных
 * @return проверка о наличи данных
 */
bool HttpHeaders::empty(){
	// Выводим существования данных http заголовков
	return this->headers.empty();
}
/**
 * size Метод получения размера
 * @return данные размера
 */
const size_t HttpHeaders::size(){
	// Выводим размер http заголовков
	return this->headers.size();
}
/**
 * cbegin Метод получения начального итератора
 * @return начальный итератор
 */
vector <HttpHeaders::Header>::const_iterator HttpHeaders::cbegin() const noexcept {
	// Выводим начальный итератор
	return this->headers.cbegin();
}
/**
 * cend Метод получения конечного итератора
 * @return конечный итератор
 */
vector <HttpHeaders::Header>::const_iterator HttpHeaders::cend() const noexcept {
	// Выводим конечный итератор
	return this->headers.cend();
}
/**
 * ~HttpHeaders Деструктор
 */
HttpHeaders::~HttpHeaders(){
	// Удаляем данные http заголовков
	vector <Header> ().swap(this->headers);
}
/**
 * operator = Оператор присваивания
 * @param chunk сторонний объект чанка
 * @return      указатель на текущий объект
 */
/*
HttpBody::Chunk & HttpBody::Chunk::operator = (HttpBody::Chunk chunk){
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
 * get Метод получения данных чанка
 * @return данные чанка
 */
const char * HttpBody::Chunk::get(){
	// Выводим данные чанка
	return this->content.data();
}
/**
 * init Метод инициализации чанка
 * @param data данные для присваивания
 * @param size размер данных
 */
void HttpBody::Chunk::init(const char * data, const size_t size){
	// Если данные существуют
	if(data && size){
		// Выполняем копирование данных
		this->content.assign(data, data + size);
		// Копируем размер данных
		this->size = this->content.size();
		// Создаем поток
		stringstream stream;
		// Заполняем поток данными
		stream << std::hex << this->size;
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
HttpBody::Chunk::Chunk(const char * data, const size_t size){
	// Выполняем инициализацию чанка
	init(data, size);
}
/**
 * ~Chunk Деструктор
 */
HttpBody::Chunk::~Chunk(){
	// Удаляем объект чанков
	vector <char> ().swap(this->content);
}
/**
 * size Метод определения размера данных
 * @param  chunked чанкованием
 * @return         размер тела
 */
const size_t HttpBody::size(bool chunked){
	// Выводим размер тела
	size_t size = 0;
	// Если это чанкование
	if(chunked){
		// Переходим по всем чанкам и считаем размеры
		for(size_t i = 0; i < this->chunks.size(); i++) size += this->chunks[i].size;
	// Иначе просто считаем размер блока
	} else size = this->body.size();
	// Выводим результат
	return size;
}
/**
 * compressData Метод сжатия данных
 * @param  buffer буфер с данными
 * @param  size   размер передаваемых данных
 * @return        данные сжатого чанка
 */
HttpBody::Chunk HttpBody::compressData(const char * buffer, const size_t size){
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
	if(deflateInit(&zs, this->compress) == Z_OK){
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
	// Создаем чанк
	Chunk chunk(data.data(), data.size());
	// Выводим результат
	return chunk;
}
/**
 * createChunk Метод создания чанка
 * @param buffer буфер с данными
 * @param size   размер передаваемых данных
 */
void HttpBody::createChunk(const char * buffer, const size_t size){
	// Копируемый размер данных
	size_t copySize = size;
	// Количество обработанных байт
	size_t used = 0;
	// Выполняем создание чанков до тех пор пока данные существуют
	while(copySize){
		// Определяем размер копируемых данных
		size_t size = (copySize < this->maxSize ? copySize : this->maxSize);
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
void HttpBody::clear(){
	// Сбрасываем количество чанков
	this->count = 0;
	// Сбрасываем заполненность данных
	this->end = false;
	// Сбрасываем режимы сжатия
	this->intGzip = false;
	this->extGzip = false;
	// Сбрасываем полученные чанки
	this->chunks.clear();
	// Очищаем тело
	this->body.clear();
}
/**
 * setMaxSize Метод установки размера чанков
 * @param size размер чанков в байтах
 */
void HttpBody::setMaxSize(const size_t size){
	// Устанавливаем максимальный размер чанков в байтах
	this->maxSize = size;
}
/**
 * setCompress Метод установки типа сжатия
 * @param compress тип сжатия
 */
void HttpBody::setCompress(const u_int compress){
	// Устанавливаем тип сжатия
	this->compress = compress;
}
/**
 * setEnd Метод установки завершения передачи данных
 * (активируется при отключении сервера от прокси, все это нужно для протокола HTTP1.0 при Connection = close)
 */
void HttpBody::setEnd(){
	// Запоминаем что все данные получены
	this->end = true;
	// Создаем чанки для несжатых данных
	if(!this->intGzip) createChunk(this->body.data(), this->body.size());
	// Если это не gzip
	else if(!this->extGzip && this->intGzip){
		// Выполняем сжатие тела
		Chunk chunk = compressData(this->body.data(), this->body.size());
		// Создаем тело в сжатом виде
		if(chunk.size) this->body.assign(chunk.get(), chunk.get() + chunk.size);
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
bool HttpBody::isEnd(){
	// Выводим результат проверки заполненности данных
	return this->end;
}
/**
 * isIntCompress Метод проверки на активацию внутреннего сжатия
 * @return результат проверки
 */
bool HttpBody::isIntCompress(){
	// Выводим результат проверки активации внутреннего сжатия
	return this->intGzip;
}
/**
 * isExtCompress Метод проверки на активацию внешнего сжатия
 * @return результат проверки
 */
bool HttpBody::isExtCompress(){
	// Выводим результат проверки активации внешнего сжатия
	return this->extGzip;
}
/**
 * addData Метод добавления данных тела
 * @param  buffer буфер с данными
 * @param  size   размер передаваемых данных
 * @param  length тип данных (0 - по умолчанию, 1 - чанки, все остальные - по размеру)
 * @param  strict жесткие правила проверки (при установки данного флага, данные принимаются только в точном соответствии)
 * @return        количество обработанных байт
 */
const size_t HttpBody::addData(const char * buffer, const size_t size, size_t length, bool strict){
	// Количество прочитанных байт
	size_t readbytes = 0;
	// Если данные тела еще не заполнены
	if(size){
		// Определяем тип данных
		switch(length){
			// Обработка по умолчанию
			case 1: {
				// Увеличиваем количество использованных байт
				readbytes = size;
				// Добавляем данные тела
				copy(buffer, buffer + readbytes, back_inserter(this->body));
			} break;
			// Обрабатываем чанкованием
			case 2: {
				// Выполняем поиск завершения передачи чанков
				char * ptr = strstr(buffer, "\r\n0\r\n\r\n");
				// Определяем позицию
				if(ptr){
					// Определяем позицию конца чанков
					size_t pos = size_t(ptr - buffer);
					// Запоминаем сколько байт мы прочитали
					readbytes = (pos + 7);
					// Запоминаем что все данные получены
					this->end = true;
				// Если это не конец, запоминаем сколько байт мы прочитали
				} else readbytes = size;
				// Иначе добавляем все что есть
				copy(buffer, buffer + readbytes, back_inserter(this->body));
				// Если все данные получены, выполняем преобразование тела
				if(this->end){
					// Смещение
					size_t offset = 0;
					// Новый буфер данных
					vector <char> body;
					// Получаем буфер для поиска
					const char * bodyBuffer = this->body.data();
					// Выполняем обработку до тех пор пока все не обработаем
					while(true){
						// Выполняем поиск завершения передачи чанков
						char * ptr = strstr(bodyBuffer + offset, "\r\n");
						// Если нашли переход
						if(ptr){
							// Размер чанка
							size_t ichunk = 0;
							// Определяем позицию конца чанков
							size_t pos = size_t(ptr - (bodyBuffer + offset));
							// Получаем размер данных в 16-й системе
							string hsize(bodyBuffer + offset, pos);
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
								// Извлекаем размер чанка
								copy(bodyBuffer + offset, bodyBuffer + offset + ichunk, back_inserter(body));
								// Если сжатие не установлено
								if(!this->intGzip){
									// Создаем чанк
									Chunk chunk(bodyBuffer + offset, ichunk);
									// Добавляем чанк
									this->chunks.push_back(chunk);
								}
								// Увеличиваем смещение
								offset += ichunk;
							// Смещаемся на два байта
							} else offset += 2;
						// Если чанки не найдены тогда выходим
						} else break;
					}
					// Если это не gzip
					if(!this->extGzip && this->intGzip){
						// Выполняем сжатие тела
						Chunk chunk = compressData(body.data(), body.size());
						// Создаем тело в сжатом виде
						if(chunk.size) this->body.assign(chunk.get(), chunk.get() + chunk.size);
						// Если сжатие не удалось тогда не сжимаем данные
						else {
							// Запоминаем не сжатые данные
							this->body.assign(body.data(), body.data() + body.size());
							// Отключаем сжатие
							this->intGzip = false;
						}
						// Создаем чанки в сжатом виде
						createChunk(this->body.data(), this->body.size());
					// Просто сохраняем данные тела
					} else this->body.assign(body.data(), body.data() + body.size());
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
				copy(buffer, buffer + readbytes, back_inserter(this->body));
				// Определяем все ли данные заполнены
				if(length == this->body.size()) this->end = true;
				// Если все данные собраны, формируем чанки
				if(this->end){
					// Создаем чанки для несжатых данных
					if(!this->intGzip) createChunk(this->body.data(), length);
					// Если это не gzip
					else if(!this->extGzip && this->intGzip){
						// Выполняем сжатие тела
						Chunk chunk = compressData(this->body.data(), this->body.size());
						// Создаем тело в сжатом виде
						if(chunk.size) this->body.assign(chunk.get(), chunk.get() + chunk.size);
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
 * getBody Метод получения тела запроса
 * @param  chunked чанкованием
 * @return         данные тела запроса
 */
HttpBody::Chunk HttpBody::getBody(bool chunked){
	// Создаем чанк
	Chunk chunk;
	// Если это чанкование
	if(chunked){
		// Создаем вектор с данными
		vector <char> data;
		// Определяем количество чанков
		size_t size = this->chunks.size();
		// Переходим по всему массиву чанков
		for(size_t i = 0; i < size; i++){
			// Формируем завершающую строку
			string chunkend = "\r\n";
			// Формируем строку чанка
			string chunksize = (this->chunks[i].hsize + "\r\n");
			// Добавляем в массив данных, полученный размер чанка
			copy(chunksize.begin(), chunksize.end(), back_inserter(data));
			// Добавляем в массив данных, данные чанка
			copy(this->chunks[i].get(), this->chunks[i].get() + this->chunks[i].size, back_inserter(data));
			// Добавляем завершающую строку
			copy(chunkend.begin(), chunkend.end(), back_inserter(data));
			// Если это конец обработки, добавляем завершающие данные
			if(i == (size - 1)){
				// Формируем закрывающий символ
				string endchunk = "0\r\n\r\n";
				// Добавляем в массив данных, завершающий размер чанка
				copy(endchunk.begin(), endchunk.end(), back_inserter(data));
			}
		}
		// Создаем чанк
		chunk.init(data.data(), data.size());
	// Выводим результат такой как он есть
	} else chunk.init(this->body.data(), this->body.size());
	// Выводим результат
	return chunk;
}
/**
 * getChunks Метод получения списка чанков
 */
vector <HttpBody::Chunk> HttpBody::getChunks(){
	// Выводим результат
	return this->chunks;
}
/**
 * HttpBody Конструктор
 * @param maxSize  максимальный размер каждого чанка (в байтах)
 * @param compress метод сжатия
 * @param intGzip  активация режима внутреннего сжатия
 * @param extGzip  активация режима внешнего сжатия
 */
HttpBody::HttpBody(const size_t maxSize, const u_int compress, bool intGzip, bool extGzip){
	// Очищаем все данные
	clear();
	// Запоминаем размер чанка
	this->maxSize = maxSize;
	// Запоминаем метод сжатия
	this->compress = compress;
	// Запоминаем режим сжатия
	this->intGzip = intGzip;
	this->extGzip = extGzip;
}
/**
 * ~HttpBody Деструктор
 */
HttpBody::~HttpBody(){
	// Очищаем все данные
	clear();
	// Удаляем объект тела
	vector <char> ().swap(this->body);
	// Удаляем объект чанков
	vector <Chunk> ().swap(this->chunks);
}
/**
 * clear Метод очистки данных
 */
void HttpQuery::clear(){
	// Очищаем все переменные
	this->code		= 0;
	this->offset	= 0;
	this->result.clear();
}
/**
 * data Метод получения данных запроса
 * @return данные запроса
 */
const char * HttpQuery::data(){
	// Выводим данные http запроса
	return (!this->result.empty() ? this->result.data() : "");
}
/**
 * size Метод получения размера
 * @return данные размера
 */
const size_t HttpQuery::size(){
	// Выводим размер http запроса
	return this->result.size();
}
/**
 * empty Метод определяет наличие данных
 * @return проверка о наличи данных
 */
bool HttpQuery::empty(){
	// Выводим существования данных http запроса
	return this->result.empty();
}
/**
 * HttpQuery Конструктор
 * @param code       код сообщения
 * @param mess       данные сообщения
 * @param entitybody вложенные данные
 */
HttpQuery::HttpQuery(short code, string mess, vector <char> entitybody){
	// Очищаем вектор
	this->result.clear();
	// Если строка существует
	if(!mess.empty()){
		// Устанавливаем код сообщения
		this->code = code;
		// Копируем в вектор сам запрос
		this->result.assign(mess.begin(), mess.end());
		// Если данные существуют
		if(!entitybody.empty()){
			// Копируем в результирующий вектор данные вложения
			copy(entitybody.begin(), entitybody.end(), back_inserter(this->result));
		}
	}
}
/**
 * ~HttpQuery Деструктор
 */
HttpQuery::~HttpQuery(){
	// Очищаем все переменные
	this->code		= 0;
	this->offset	= 0;
	vector <char> ().swap(this->result);
}
/**
 * getHttpRequest Метод получения сформированного http запроса только с добавлением заголовков
 * @return сформированный http запрос
 */
vector <char> HttpData::getHttpRequest(){
	// Определяем тип прокси
	bool smart = (this->options & OPT_SMART);
	// Определяем разрешено ли выводить название агента
	bool agent = (this->options & OPT_AGENT);
	// Формируем запрос
	string request = (this->http + string("\r\n"));
	// Тип подключения
	string connection = this->headers.getHeader("connection").value;
	// Если заголовок не найден тогда устанавливаем по умолчанию
	if(connection.empty()) connection = "close";
	// Добавляем остальные заголовки
	for(auto it = this->headers.cbegin(); it != this->headers.cend(); ++it){
		// Фильтруем заголовки
		if((::toCase(it->head).compare("connection") != 0)
		&& (::toCase(it->head).compare("proxy-authorization") != 0)
		&& (!smart || (smart && (::toCase(it->head).compare("proxy-connection") != 0)))){
			// Добавляем оставшиеся заголовки
			request.append(it->head + string(": ") + it->value + string("\r\n"));
		}
	}
	// Устанавливаем название прокси
	if(agent) request.append(string("Proxy-Agent: ") + this->appName + string("/") + this->appVersion + string("\r\n"));
	// Если это dumb прокси
	if(!smart){
		// Проверяем есть ли заголовок соединения прокси
		string proxy_connection = this->headers.getHeader("proxy-connection").value;
		// Добавляем заголовок закрытия подключения
		if(proxy_connection.empty()) request.append(string("Proxy-Connection: ") + connection + string("\r\n"));
	// Если это smart прокси
	} else {
		// Получаем заголовок Proxy-Connection
		string proxy_connection = this->headers.getHeader("proxy-connection").value;
		// Если постоянное соединение не установлено
		if(!proxy_connection.empty()) connection = proxy_connection;
	}
	// Добавляем тип подключения
	request.append(string("Connection: ") + connection + string("\r\n"));
	// Запоминаем конец запроса
	request.append(string("\r\n"));
	// Создаем результат
	vector <char> result(request.begin(), request.end());
	// Выводим результат
	return result;
}
/**
 * createHead Функция получения сформированного заголовка запроса
 */
void HttpData::createHead(){
	// Определяем тип прокси
	bool smart = (this->options & OPT_SMART);
	// Определяем разрешено ли сжатие
	bool gzip = (this->options & OPT_GZIP);
	// Определяем разрешено ли выводить название агента
	bool agent = (this->options & OPT_AGENT);
	// Определяем разрешено ли постоянное подключение
	bool keepalive = (this->options & OPT_KEEPALIVE);
	// Очищаем заголовок
	this->request.clear();
	// Создаем строку запроса
	this->request.append(
		::toCase(this->method, true)
		+ string(" ") + this->path
		+ string(" ") + string("HTTP/")
		+ this->version + string("\r\n")
	);
	/*
	// Устанавливаем заголовок Host:
	this->request.append(
		string("Host: ") + this->host
		+ string(":") + this->port + string("\r\n")
	);
	*/
	// Устанавливаем заголовок Host:
	this->request.append(string("Host: ") + this->host + string("\r\n"));
	// Добавляем useragent
	if(!this->useragent.empty()) this->request.append(string("User-Agent: ") + this->useragent + string("\r\n"));
	// Добавляем остальные заголовки
	for(auto it = this->headers.cbegin(); it != this->headers.cend(); ++it){
		// Фильтруем заголовки
		if((::toCase(it->head).compare("host") != 0)
		&& (::toCase(it->head).compare("user-agent") != 0)
		&& (::toCase(it->head).compare("connection") != 0)
		&& (::toCase(it->head).compare("proxy-authorization") != 0)
		&& (gzip || (!gzip && (::toCase(it->head).compare("accept-encoding") != 0)))
		&& (!smart || (smart && (::toCase(it->head).compare("proxy-connection") != 0)))){
			// Добавляем оставшиеся заголовки
			this->request.append(it->head + string(": ") + it->value + string("\r\n"));
		}
	}
	// Устанавливаем название прокси
	if(agent) this->request.append(string("Proxy-Agent: ") + this->appName + string("/") + this->appVersion + string("\r\n"));
	// Если постоянное подключение запрещено
	if(!keepalive) this->connection = "close";
	// Добавляем заголовок connection
	if(!this->connection.empty()){
		// Устанавливаем заголовок подключения
		this->request.append(string("Connection: ") + this->connection + string("\r\n"));
		// Если это dumb прокси
		if(!smart){
			// Проверяем есть ли заголовок соединения прокси
			string pc = this->headers.getHeader("proxy-connection").value;
			// Добавляем заголовок закрытия подключения
			if(pc.empty()) this->request.append(string("Proxy-Connection: ") + this->connection + string("\r\n"));
		}
	}
	// Запоминаем конец запроса
	this->request.append(string("\r\n"));
}
/**
 * getConnection Функция извлечения данных подключения
 * @param  str строка запроса
 * @return     объект с данными запроса
 */
HttpData::Connect HttpData::getConnection(string str){
	// Полученные данные подключения
	Connect data;
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex ep("\\b([A-Za-z]+):\\/{2}", regex::ECMAScript | regex::icase);
	// Выполняем поиск протокола
	regex_search(str, match, ep);
	// Если протокол найден
	if(!match.empty() && (match.size() == 2))
		// Запоминаем версию протокола
		data.protocol = ::toCase(match[1].str());
	// Устанавливаем правило для поиска
	regex eh(
		// Стандартная запись домена anyks.com
		"\\b([\\w\\.\\-]+\\.[A-Za-z]+|"
		// Стандартная запись IPv4 127.0.0.1
		"\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}|"
		// Стандартная запись IPv6 [2001:0db8:11a3:09d7:1f34:8a2e:07a0:765d]
		"\\[[\\w\\:]+\\])(?:\\:(\\d+))?",
		regex::ECMAScript | regex::icase
	);
	// Выполняем поиск домена и порта
	regex_search(str, match, eh);
	// Если протокол найден
	if(!match.empty() && (match.size() > 1)){
		// Запоминаем хост
		data.host = ::toCase(match[1].str());
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
	// Выводим результат
	return data;
}
/**
 * clear Метод очистки структуры
 */
void HttpData::clear(){
	// Обнуляем размер
	this->length = 0;
	// Обнуляем статус запроса
	this->status = 0;
	// Устанавливаем что режим сжатия отключен
	this->intGzip = false;
	this->extGzip = false;
	// Устанавливаем что заголовки не заполнены
	this->fullHeaders = false;
	// Очищаем строки
	this->query.clear();
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
	this->useragent.clear();
	this->connection.clear();
	this->request.clear();
	// Очищаем карту заголовков
	this->headers.clear();
	// Очищаем тело
	this->body.clear();
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
	if(!match.empty() && (match.size() == 5)){
		// Запоминаем метод запроса
		this->method = ::toCase(match[1].str());
		// Запоминаем путь запроса
		this->path = match[2].str();
		// Запоминаем протокол запроса
		this->protocol = ::toCase(match[3].str());
		// Запоминаем версию протокола
		this->version = match[4].str();
		// Извлекаем данные хоста
		string host = this->headers.getHeader("host").value;
		// Извлекаем данные авторизации
		string auth = this->headers.getHeader("proxy-authorization").value;
		// Получаем заголовок Proxy-Connection
		string proxy_connection = this->headers.getHeader("proxy-connection").value;
		// Получаем данные юзерагента
		this->useragent = this->headers.getHeader("user-agent").value;
		// Получаем данные заголовока коннекта
		this->connection = ::toCase(this->headers.getHeader("connection").value);
		// Если постоянное соединение не установлено
		if((this->options & OPT_SMART) && !proxy_connection.empty()) this->connection = proxy_connection;
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
			} else if(::checkPort(gcon.port)) {
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
			&& ((::toCase(this->method).compare("connect") != 0)
			&& ((::toCase(this->path).compare(::toCase(host)) == 0)
			|| (::toCase(this->path).compare(::toCase(fulladdr1)) == 0)
			|| (::toCase(this->path).compare(::toCase(fulladdr2)) == 0)
			|| (::toCase(this->path).compare(::toCase(fulladdr3)) == 0)
			|| (::toCase(this->path).compare(::toCase(fulladdr4)) == 0)))) this->path = "/";
			// Выполняем удаление из адреса доменного имени
			else if(strstr(this->path.c_str(), fulladdr1.c_str())){
				// Запоминаем текущий путь
				string tmp_path = this->path;
				// Вырезаем домер из пути
				tmp_path = tmp_path.replace(0, fulladdr1.length(), "");
				// Если путь существует
				if(!tmp_path.empty()) this->path = tmp_path;
			}
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
			if(!match.empty() && (match.size() == 3)){
				// Запоминаем тип авторизации
				this->auth = ::toCase(match[1].str());
				// Если это тип авторизация basic, тогда выполняем декодирования данных авторизации
				if(this->auth.compare("basic") == 0){
					// Выполняем декодирование логина и пароля
					string dauth = base64_decode(match[2].str());
					// Устанавливаем правило регулярного выражения
					regex e("\\b([\\s\\S]+)\\:([\\s\\S]+)", regex::ECMAScript | regex::icase);
					// Выполняем поиск протокола
					regex_search(dauth, match, e);
					// Если данные найдены
					if(!match.empty() && (match.size() == 3)){
						// Запоминаем логин
						this->login = match[1].str();
						// Запоминаем пароль
						this->password = match[2].str();
					}
				}
			}
		}
	}
	// Генерируем параметры для запроса
	createHead();
}
/**
 * isIntGzip Метод проверки активации режима сжатия
 * @return результат проверки
 */
bool HttpData::isIntGzip(){
	// Сообщаем активирован ли режим сжатия
	return this->intGzip;
}
/**
 * isExtGzip Метод проверки активации режима внешнего сжатия
 * @return результат проверки
 */
bool HttpData::isExtGzip(){
	// Сообщаем активирован ли режим сжатия
	return this->extGzip;
}
/**
 * isConnect Метод проверяет является ли метод, методом connect
 * @return результат проверки на метод connect
 */
bool HttpData::isConnect(){
	// Сообщаем является ли метод, методом connect
	return (this->method.compare("connect") == 0);
}
/**
 * isClose Метод проверяет должно ли быть закрыто подключение
 * @return результат проверки на закрытие подключения
 */
bool HttpData::isClose(){
	// Сообщаем должно ли быть закрыто подключение
	return (this->connection.compare("close") == 0);
}
/**
 * isHttps Метод проверяет является ли подключение защищенным
 * @return результат проверки на защищенное подключение
 */
bool HttpData::isHttps(){
	// Сообщаем является ли продключение защищенным
	return (this->protocol.compare("https") == 0);
}
/**
 * isAlive Метод определения нужно ли держать соединение для прокси
 * @return результат проверки
 */
bool HttpData::isAlive(){
	// Если это версия протокола 1.1 и подключение установлено постоянное для прокси
	if(getVersion() > 1){
		// Проверяем указан ли заголовок отключения
		if(this->connection.compare("close") == 0) return false;
		// Иначе сообщаем что подключение должно жить долго
		return true;
	// Проверяем указан ли заголовок удержания соединения
	} else if(this->connection.compare("keep-alive") == 0) return true;
	// Сообщаем что подключение жить не должно
	return false;
}
/**
 * getFullHeaders Метод получения данных о заполненности заголовков
 */
bool HttpData::getFullHeaders(){
	// Выводим данные о заполненности заголовков
	return this->fullHeaders;
}
/**
 * size Метод получения размера запроса
 * @return размер запроса
 */
const size_t HttpData::size(){
	// Выводим размер запроса
	return this->length;
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
	return this->useragent;
}
/**
 * getQuery Метод получения буфера запроса
 * @return буфер запроса
 */
const string HttpData::getQuery(){
	// Выводим значение переменной
	return this->query;
}
/**
 * getResponseHeaders Метод получения заголовков http ответа
 * @return сформированные заголовки ответа
 */
const string HttpData::getResponseHeaders(){
	// Получаем сформированный ответ
	vector <char> data = this->getHttpRequest();
	// Получаем данные запроса
	return this->responseHeaders.assign(data.begin(), data.end());
}
/**
 * getHeader Метод извлекает данные заголовка по его ключу
 * @param  key ключ заголовка
 * @return     строка с данными заголовка
 */
const string HttpData::getHeader(string key){
	// Выводим данные заголовков
	return this->headers.getHeader(key).value;
}
/**
 * getResponseBody Метод получения данных тела http запроса
 * @param  chunked метод чанкование
 * @return         объект с данными тела
 */
HttpBody::Chunk HttpData::getResponseBody(bool chunked){
	// Если не активно сжатие, снимает режим сжатия и здесь
	if(!this->body.isIntCompress()
	&& !this->body.isExtCompress()) this->unsetGzip();
	// Если это чанкование
	if(chunked){
		// Удаляем из заголовков, заголовок размера
		this->rmHeader("content-length");
		// Устанавливаем заголовок что данные в виде чанков
		this->setHeader("Transfer-Encoding", "chunked");
	// Если это извлечение данных конкретного размера
	} else {
		// Удаляем из заголовков, заголовок передачи данных чанками
		this->rmHeader("transfer-encoding");
		// Устанавливаем размер входящих данных
		this->setHeader("Content-Length", to_string(this->body.size()));
	}
	// Выполняем генерацию результирующего запроса
	createHead();
	// Выводим результат
	return this->body.getBody(chunked);
}
/**
 * isEndBody Метод определения заполненности тела ответа данными
 * @return результат проверки
 */
bool HttpData::isEndBody(){
	// Выводим результат проверки на заполненность тела ответа
	return this->body.isEnd();
}
/**
 * getResponseData Метод получения http данных ответа
 * @return объект с данными
 */
vector <char> HttpData::getResponseData(){
	// Результирующий объект данных
	vector <char> result;
	// Получаем данные тела
	HttpBody::Chunk body = getResponseBody(getVersion() > 1);
	// Получаем данные заголовков
	string headers = getResponseHeaders();
	// Выполняем добавление заголовков в результат
	copy(headers.begin(), headers.end(), back_inserter(result));
	// Копируем данные тела
	copy(body.get(), body.get() + body.size, back_inserter(result));
	// Выводим результат
	return result;
}
/**
 * setBodyData Метод добавления данных тела
 * @param  buffer буфер с данными
 * @param  size   размер передаваемых данных
 * @param  length тип данных (0 - по умолчанию, 1 - чанки, все остальные - по размеру)
 * @return        количество обработанных байт
 */
const size_t HttpData::setBodyData(const char * buffer, const size_t size, size_t length){
	// Выполняем добавление данных тела
	return this->body.addData(buffer, size, length);
}
/**
 * setEntitybody Метод добавления данных вложения
 * @param buffer буфер с данными вложения
 * @param size   размер буфера
 */
bool HttpData::setEntitybody(const char * buffer, size_t size){
	// Если данные в объекте существуют
	if(this->length){
		// Размер вложений
		u_int body_size = 0;
		// Проверяем есть ли размер вложений
		string cl = this->headers.getHeader("content-length").value;
		// Если найден размер вложений
		// Определяем размер вложений
		if(!cl.empty() && ::isNumber(cl)) body_size = (::atoi(cl.c_str()) + this->length);
		// Если вложения не найдены
		else {
			// Проверяем есть ли чанкование
			string ch = this->headers.getHeader("transfer-encoding").value;
			// Если это чанкование
			if(!ch.empty() && (ch.find("chunked") != string::npos)){
				// Выполняем поиск подстроки
				const char * pch = strstr(buffer + this->length, "0\r\n\r\n");
				// Если конец передачи данных мы нашли
				// Определяем размер вложений
				if(pch) body_size = ((pch - buffer) - this->length + 5);
				// Если конец не найден
				else {
					// Ищем дальше
					pch = strstr(buffer + this->length, "0\r\n");
					// Если конец передачи данных мы нашли
					// Определяем размер вложений
					if(pch) body_size = ((pch - buffer) - this->length + 3);
				}
			// Сообщаем что мы закончили
			} else return true;
		}
		// Если данные вложений есть тогда устанавливаем их и выходим
		if(body_size <= size){
			// Извлекаем указанные данные
			this->entitybody.assign(buffer + this->length, buffer + body_size);
			// Увеличиваем максимальный размер данных
			this->length += body_size;
			// Сообщаем что мы закончили
			return true;
		}
	}
	// Сообщаем что ничего не найдено
	return false;
}
/**
 * initBody Метод инициализации объекта тела
 */
void HttpData::initBody(){
	// Выполняем создание объекта body
	HttpBody body = HttpBody(4096, Z_DEFAULT_COMPRESSION, this->intGzip, this->extGzip);
	// Запоминаем объект body
	this->body = body;
}
/**
 * rmHeader Метод удаления заголовка
 * @param key название заголовка
 */
void HttpData::rmHeader(const string key){
	// Если параметры пришли верные
	if(!key.empty()){
		// Удаляем заголовок
		this->headers.remove(key);
		// Выполняем генерацию результирующего запроса
		createHead();
	}
}
/**
 * setGzip Метод установки режима сжатия gzip
 * @param intGzip активация внутреннего режима сжатия
 * @param extGzip активация внешнего режима сжатия
 */
void HttpData::setGzip(bool intGzip, bool extGzip){
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
 * setHeader Метод добавления нового заголовка
 * @param key   ключ
 * @param value значение
 */
void HttpData::setHeader(const string key, const string value){
	// Если параметры пришли верные
	if(!key.empty() && !value.empty()){
		// Устанавливаем заголовок
		this->headers.append(key, value);
		// Выполняем генерацию результирующего запроса
		createHead();
	}
}
/**
 * setOptions Метод установки настроек прокси сервера
 * @param options данные для установки
 */
void HttpData::setOptions(u_short options){
	// Запоминаем данные
	this->options = options;
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setMethod Метод установки метода запроса
 * @param str строка с данными для установки
 */
void HttpData::setMethod(const string str){
	// Запоминаем данные
	this->method = str;
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setHost Метод установки хоста запроса
 * @param str строка с данными для установки
 */
void HttpData::setHost(const string str){
	// Запоминаем данные
	this->host = str;
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setPort Метод установки порта запроса
 * @param number номер порта для установки
 */
void HttpData::setPort(u_int number){
	// Запоминаем данные
	this->port = to_string(number);
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setPath Метод установки пути запроса
 * @param str строка с данными для установки
 */
void HttpData::setPath(const string str){
	// Запоминаем данные
	this->path = str;
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setProtocol Метод установки протокола запроса
 * @param str строка с данными для установки
 */
void HttpData::setProtocol(const string str){
	// Запоминаем данные
	this->protocol = str;
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setVersion Метод установки версии протокола запроса
 * @param number номер версии протокола
 */
void HttpData::setVersion(float number){
	// Запоминаем данные
	this->version = to_string(number);
	// Если это всего один символ тогда дописываем ноль
	if(this->version.length() == 1) this->version += ".0";
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setAuth Метод установки метода авторизации запроса
 * @param str строка с данными для установки
 */
void HttpData::setAuth(const string str){
	// Запоминаем данные
	this->auth = str;
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setUseragent Метод установки юзерагента запроса
 * @param str строка с данными для установки
 */
void HttpData::setUseragent(const string str){
	// Запоминаем данные
	this->useragent = str;
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setClose Метод установки принудительного отключения после запроса
 */
void HttpData::setClose(){
	// Запоминаем данные
	this->connection = "close";
	// Выполняем генерацию результирующего запроса
	createHead();
}
/**
 * setFullHeaders Метод установки конца ввода данных заголовков
 */
void HttpData::setFullHeaders(){
	// Устанавливаем завершающие символы запроса
	this->query += "\r\n";
	// Получаем длину массива заголовков
	this->length = this->query.length();
	// Запоминаем что заголовки заполены полностью
	this->fullHeaders = true;
}
/**
 * addHeader Метод добавления нового заголовка
 * @param buffer буфер с данными заголовка
 */
void HttpData::addHeader(const char * buffer){
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
			// Запоминаем первые символы
			this->query.append(key + string(": ") + val + "\r\n");
			// Если сжатие активировано
			if((::toCase(key).compare("content-encoding") == 0)
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
			// Запоминаем первые символы
			this->query = (this->http + "\r\n");
		}
		// Получаем длину массива заголовков
		this->length = this->query.length();
	}
}
/**
 * createRequest Функция создания ответа сервера
 * @param  index   индекс в массиве ответа
 * @param  request номер ответа
 * @return         объект с данными ответа
 */
HttpQuery HttpData::createRequest(u_short index, u_short request){
	// Устанавливаем дефолтное название прокси
	string defname = "ProxyAnyks/1.0";
	// Определяем позицию дефолтного названия
	size_t pos = this->html[index].find(defname);
	// Результирующая строка
	string result;
	// Если это домен
	if(pos != string::npos){
		// Заменяем дефолтное название на указанное
		result = this->html[index]
		.replace(pos, defname.length(), this->appName + string("/") + this->appVersion);
	}
	// Выводим шаблон сообщения о неудачном отправленном запросе
	result = this->html[index];
	// Данные для вывода
	HttpQuery data(request, result);
	// Выводим результат
	return data;
}
/**
 * brokenRequest Метод получения ответа (неудачного отправленного запроса)
 * @return ответ в формате html
 */
HttpQuery HttpData::brokenRequest(){
	// Выводим результат
	return createRequest(9, 501);
}
/**
 * faultConnect Метод получения ответа (неудачного подключения к удаленному серверу)
 * @return ответ в формате html
 */
HttpQuery HttpData::faultConnect(){
	// Выводим результат
	return createRequest(6, 502);
}
/**
 * pageNotFound Метод получения ответа (страница не найдена)
 * @return ответ в формате html
 */
HttpQuery HttpData::pageNotFound(){
	// Выводим результат
	return createRequest(4, 404);
}
/**
 * faultAuth Метод получения ответа (неудачной авторизации)
 * @return ответ в формате html
 */
HttpQuery HttpData::faultAuth(){
	// Выводим результат
	return createRequest(5, 403);
}
/**
 * requiredAuth Метод получения ответа (запроса ввода логина и пароля)
 * @return ответ в формате html
 */
HttpQuery HttpData::requiredAuth(){
	// Выводим результат
	return createRequest(2, 407);
}
/**
 * authSuccess Метод получения ответа (подтверждения авторизации)
 * @return ответ в формате html
 */
HttpQuery HttpData::authSuccess(){
	// Выводим результат
	return createRequest(0, 200);
}
/**
 * getRequest Метод получения сформированного http запроса
 * @return сформированный http запрос
 */
HttpQuery HttpData::getRequest(){
	// Данные для вывода
	HttpQuery data(200, this->request, this->entitybody);
	// Выводим результат
	return data;
}
/**
 * init Метод инициализации класса
 * @param  str     строка http запроса
 * @param  name    название приложения
 * @param  version версия приложения
 * @param  options опции http парсера
 * @return         данные http запроса
 */
void HttpData::init(const string str, const string name, const string version, const u_short options){
	// Проверяем существуют ли данные
	if(!str.empty()){
		// Очищаем полученные данные
		clear();
		// Запоминаем название приложения
		this->appName = name;
		// Запоминаем версию приложения
		this->appVersion = version;
		// Запоминаем параметры http парсера
		this->options = options;
		// Результат работы регулярного выражения
		smatch match;
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
			this->http = ::trim(this->http);
			// Запоминаем статус запроса
			this->status = ::atoi(match[4].str().c_str());
			// Запоминаем первые символы
			this->query = match[0].str();
			// Создаем объект с заголовками
			this->headers.create(this->query.c_str());
			// Получаем длину массива заголовков
			this->length = this->query.length();
			// Запоминаем что заголовки заполнены полностью
			this->fullHeaders = true;
		}
		// Генерируем данные подключения
		genDataConnect();
	}
}
/**
 * HttpData Конструктор
 * @param name    название приложения
 * @param options опции http парсера
 */
HttpData::HttpData(const string name, const u_short options){
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
 * ~HttpData Деструктор
 */
HttpData::~HttpData(){
	// Очищаем полученные данные
	clear();
	// Очищаем память выделенную для вектора
	vector <char> ().swap(this->entitybody);
}
/**
 * isHttp Метод проверки на то http это или нет
 * @param  buffer буфер входящих данных
 * @return        результат проверки
 */
bool Http::isHttp(const string buffer){
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
		for(int i = 0; i < 8; i++) if(::toCase(buf).compare(cmds[i]) == 0) return true;
	}
	// Сообщаем что это не http
	return false;
}
/**
 * parse Функция извлечения данных из буфера
 * @param buffer буфер с входящими запросами
 * @param size   размер входящих данных
 * @param flag   обрабатывать весь блок данных
 */
size_t Http::parse(const char * buffer, size_t size, bool flag){
	// Определяем максимальный размер данных
	size_t maxsize = 0;
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
		maxsize += badchars.length();
		// Выполняем парсинг http запроса
		HttpData httpData;
		// Выполняем инициализацию объекта
		httpData.init(match[2].str(), this->name, this->version, this->options);
		// Добавляем вложенные данные
		if(httpData.setEntitybody(buffer, size)){
			// Добавляем в массив объект подключения
			this->httpData.push_back(httpData);
			// Увеличиваем общий размер
			maxsize += httpData.size();
			// Следующий размер данных
			size_t nsize = strlen(buffer + maxsize);
			// Если есть еще данные
			if(flag && (nsize >= 34) && (nsize <= (size - maxsize))){
				// Выполняем интерпретацию следующей порции данных
				return parse(buffer + maxsize, size - maxsize, flag);
			// Выводим размер так как он есть
			} else return maxsize;
		}
	}
	// Сообщаем что ничего не найдено
	return 0;
}
/**
 * modify Функция модифицирования ответных данных
 * @param data ссылка на данные полученные от сервера
 */
void Http::modify(vector <char> &data){
	// Получаем данные ответа
	const char * headers = data.data();
	// Создаем объект запроса
	HttpData httpQuery;
	// Выполняем инициализацию объекта
	httpQuery.init(headers, this->name, this->version, this->options);
	// Если данные распарсены
	if(httpQuery.size()){
		// Если завершение заголовка найдено
		u_int pos = (strstr(headers, "\r\n\r\n") - headers) + 4;
		// Получаем данные запроса
		vector <char> last, query = httpQuery.getHttpRequest();
		// Заполняем последние данные структуры
		last.assign(data.begin() + pos, data.end());
		// Объединяем блоки
		copy(last.begin(), last.end(), back_inserter(query));
		// Заменяем первоначальный блок с данными
		data = query;
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
 * Http Конструктор
 * @param name    строка содержащая название прокси сервера
 * @param options параметры прокси сервера
 */
Http::Http(const string name, const u_short options){
	// Если имя передано то запоминаем его
	this->name = name;
	// Запоминаем тип прокси сервера
	this->options = options;
	// Устанавливаем версию системы
	this->version = APP_VERSION;
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