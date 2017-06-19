#!/usr/bin/env node

/* RESTful TEST SERVER */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(910) 983-95-90
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
// Оборачиваем в замыкание
(function(){
	// Подключаем модули
	const url	= require('url');
	const http	= require('http');
	/**
	 * init Функция инициализации
	 * @param {Object} req объект запроса
	 */
	const init = function(req){
		// Копируем идентификатор сервера
		const client = this;
		// Выполняем обработку url ссылки
		const requestUrl = url.parse(req.url);
		// Выполняем парсинг адресов
		switch(requestUrl.pathname){
			// Если это запрос для получения данных
			case "/test": {
				// Определяем контент
				const contentType = req.headers['content-type'];
				// Устанавливаем заголовки
				client.setHeader('Foo', 'bar');
				client.setHeader('Set-Cookie', ['foo=bar', 'bar=baz']);
				// Сообщаем что страница не найдена
				client.writeHead(404, {"Content-Type": "text/plain; charset=utf-8"});
				client.write("404 Not Found\n");
				client.end();
			} break;
			// Если это редирект
			case "/aming": {
				// Устанавливаем заголовки
				client.setHeader('Location', "http://vipmake.com/files/default/css/css_ZEbgsW3PQ8CSHuVoHPvFefbGZwNfubouBXxfMYrp_F0.css");
				// Сообщаем что страница не найдена
				client.writeHead(301, {"Content-Type": "text/plain; charset=utf-8"});
				// Закрываем подключение
				client.end();
			} break;
			// Тест 1 (Etag)
			case "/etag1": {
				// Дата последней модификации
				const modifiedServer = 'Tue, 28 Mar 2017 22:19:47 GMT';
				// Получаем дату последней модификации от клиента
				const modifiedClient = req.headers['if-modified-since'];
				// Устанавливаем заголовки
				client.setHeader('Last-modified', modifiedServer);
				// client.setHeader('Date', 'Tue, 28 Mar 2017 22:19:47 GMT'); // -3 часа от московского
				// client.setHeader('Pragma', 'no-cache');
				// Если дата на сервере изменилась
				if(!modifiedClient || ((new Date(modifiedServer)).valueOf() > (new Date(modifiedClient)).valueOf())){
					// Сообщаем что страница не найдена
					client.writeHead(200, {"Content-Type": "text/plain; charset=utf-8"});
					client.write("Ok\n");
				// Если дата не устарела
				} else {
					// Сообщаем что страница не найдена
					client.writeHead(304, {"Content-Type": "text/plain; charset=utf-8"});
					client.write("Not Modified\n");
				}
				// Закрываем подключение
				client.end();
			} break;
			// Тест 2 (Etag)
			case "/etag2": {
				// Дата последней модификации
				const modifiedServer = 'Tue, 28 Mar 2017 22:19:47 GMT';
				// Получаем дату последней модификации от клиента
				const modifiedClient = req.headers['if-modified-since'];
				// Данные eTag
				const etagServer = "123";
				// Получаем данные Etag
				let etagClient = req.headers['if-none-match'];
				// Если etag существует
				if(etagClient) etagClient = etagClient.replace("W/", "");
				// Устанавливаем заголовки
				client.setHeader('Last-modified', modifiedServer);
				client.setHeader('Etag', etagServer);
				client.setHeader('Pragma', 'no-cache');
				// Если дата на сервере или etag соответствует
				if(((new Date(modifiedServer)).valueOf() <= (new Date(modifiedClient)).valueOf()) || (etagServer === etagClient)){
					// Сообщаем что страница не найдена
					client.writeHead(304, {"Content-Type": "text/plain; charset=utf-8"});
					client.write("Not Modified\n");
				// Если данные не соответствуют
				} else {
					// Сообщаем что страница не найдена
					client.writeHead(200, {"Content-Type": "text/plain; charset=utf-8"});
					client.write("Ok\n");
				}
				// Закрываем подключение
				client.end();
			} break;
			// Тест 3 (Etag)
			case "/etag3": {
				// Время смерти кэша
				const expiresCache = 'Sat, 08 Apr 2017 20:52:48 MSK';
				// Дата последней модификации
				const modifiedServer = 'Tue, 28 Mar 2017 22:19:47 GMT';
				// Получаем дату последней модификации от клиента
				const modifiedClient = req.headers['if-modified-since'];
				// Устанавливаем заголовки
				client.setHeader('Last-modified', modifiedServer);
				client.setHeader('Expires', expiresCache);
				// Если дата на сервере или etag соответствует
				if((new Date(modifiedServer)).valueOf() <= (new Date(modifiedClient)).valueOf()){
					// Сообщаем что страница не найдена
					client.writeHead(304, {"Content-Type": "text/plain; charset=utf-8"});
					client.write("Not Modified\n");
				// Если данные не соответствуют
				} else {
					// Сообщаем что страница не найдена
					client.writeHead(200, {"Content-Type": "text/plain; charset=utf-8"});
					client.write("Ok\n");
				}
				// Закрываем подключение
				client.end();
			} break;
			// Тест 4 (Cache-control)
			case "/cc1": {
				// Дата последней модификации
				const modifiedServer = 'Sun, 09 Apr 2017 21:10:48 MSK';
				// Устанавливаем заголовки
				client.setHeader('Last-modified', modifiedServer);
				client.setHeader('Cache-control', 'max-age=60');
				// Сообщаем что страница не найдена
				client.writeHead(200, {"Content-Type": "text/plain; charset=utf-8"});
				client.write("Ok\n");
				client.end();
			} break;
			// Тест 5 (Cache-control)
			case "/cc2": {
				// Дата последней модификации
				const modifiedServer = 'Sun, 09 Apr 2017 21:14:48 MSK';
				// Устанавливаем заголовки
				client.setHeader('Last-modified', modifiedServer);
				client.setHeader('Cache-control', 'public, max-age=60, s-maxage=120');
				// Сообщаем что страница не найдена
				client.writeHead(200, {"Content-Type": "text/plain; charset=utf-8"});
				client.write("Ok\n");
				client.end();
			} break;
			// Тест 6 (Cache-control)
			case "/cc3": {
				// Дата последней модификации
				const modifiedServer = 'Sun, 09 Apr 2017 21:22:48 MSK';
				// Устанавливаем заголовки
				client.setHeader('Last-modified', modifiedServer);
				client.setHeader('Cache-control', 'max-age=60, must-revalidate');
				// Сообщаем что страница не найдена
				client.writeHead(200, {"Content-Type": "text/plain; charset=utf-8"});
				client.write("Ok\n");
				client.end();
			} break;
			// Тест 7 (Cache-control)
			case "/cc4": {
				// Дата последней модификации
				const modifiedServer = 'Sun, 09 Apr 2017 21:26:48 MSK';
				// Устанавливаем заголовки
				client.setHeader('Last-modified', modifiedServer);
				client.setHeader('Cache-control', 'max-age=60, proxy-revalidate');
				// Сообщаем что страница не найдена
				client.writeHead(200, {"Content-Type": "text/plain; charset=utf-8"});
				client.write("Ok\n");
				client.end();
			} break;
			// Тест 8 (Cache-control)
			case "/cc5": {
				// Дата последней модификации
				const modifiedServer = 'Sun, 09 Apr 2017 21:50:48 MSK';
				// Устанавливаем заголовки
				client.setHeader('Last-modified', modifiedServer);
				client.setHeader('Cache-control', 'max-age=60, no-cache');
				// Сообщаем что страница не найдена
				client.writeHead(200, {"Content-Type": "text/plain; charset=utf-8"});
				client.write("Ok\n");
				client.end();
			} break;
			// Тест 9 (Cache-control)
			case "/cc6": {
				// Дата последней модификации
				const modifiedServer = 'Sun, 09 Apr 2017 21:50:48 MSK';
				// Устанавливаем заголовки
				client.setHeader('Last-modified', modifiedServer);
				client.setHeader('Cache-control', 'max-age=60, private');
				// Сообщаем что страница не найдена
				client.writeHead(200, {"Content-Type": "text/plain; charset=utf-8"});
				client.write("Ok\n");
				client.end();
			} break;
			// Тест 10 (Cache-control)
			case "/cc7": {
				// Дата последней модификации
				const modifiedServer = 'Sun, 09 Apr 2017 21:50:48 MSK';
				// Устанавливаем заголовки
				client.setHeader('Last-modified', modifiedServer);
				client.setHeader('Cache-control', 'max-age=60, no-store');
				// Сообщаем что страница не найдена
				client.writeHead(200, {"Content-Type": "text/plain; charset=utf-8"});
				client.write("Ok\n");
				client.end();
			} break;
		}
	};
	// Создаем сервер
	http.createServer((request, response) => init.call(response, request)).listen(3322, "127.0.0.1");
})();