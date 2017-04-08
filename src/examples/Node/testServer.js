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
			case "/anyks": {
				// Устанавливаем заголовки
				client.setHeader('Location', "http://vipmake.com/files/default/css/css_ZEbgsW3PQ8CSHuVoHPvFefbGZwNfubouBXxfMYrp_F0.css");
				// Сообщаем что страница не найдена
				client.writeHead(301, {"Content-Type": "text/plain; charset=utf-8"});
				// Закрываем подключение
				client.end();
			} break;
			// Тест 4 (Etag)
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
			// Тест 5 (Etag)
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
			// Тест 6 (Etag)
			case "/etag6": {
				// Устанавливаем заголовки
				client.setHeader('Last-modified', 'Tue, 28 Mar 2017 22:19:47 GMT');
				client.setHeader('Expires', 'Tue, 28 Mar 2017 22:19:47 GMT');
				// Сообщаем что страница не найдена
				client.writeHead(200, {"Content-Type": "text/plain; charset=utf-8"});
				client.write("Ok\n");
				client.end();
			} break;
			// Тест 7 (Etag)
			case "/etag7": {
				// Устанавливаем заголовки
				client.setHeader('Last-modified', 'Tue, 28 Mar 2017 22:19:47 GMT');
				client.setHeader('Etag', '123');
				client.setHeader('Expires', 'Tue, 28 Mar 2017 22:19:47 GMT');
				// Сообщаем что страница не найдена
				client.writeHead(200, {"Content-Type": "text/plain; charset=utf-8"});
				client.write("Ok\n");
				client.end();
			} break;
			// Тест 8 (Cache-control)
			case "/cc1": {
				// Устанавливаем заголовки
				client.setHeader('Last-modified', 'Tue, 28 Mar 2017 22:19:47 GMT');
				client.setHeader('Etag', '123');
				client.setHeader('Expires', 'Tue, 28 Mar 2017 22:19:47 GMT');
				client.setHeader('Cache-control', 'max-age=60');
				// Сообщаем что страница не найдена
				client.writeHead(200, {"Content-Type": "text/plain; charset=utf-8"});
				client.write("Ok\n");
				client.end();
			} break;
			// Тест 9 (Cache-control)
			case "/cc2": {
				// Устанавливаем заголовки
				client.setHeader('Age', '120');
				client.setHeader('Last-modified', 'Tue, 28 Mar 2017 22:19:47 GMT');
				client.setHeader('Etag', '123');
				client.setHeader('Expires', 'Tue, 28 Mar 2017 22:19:47 GMT');
				client.setHeader('Date', 'Tue, 28 Mar 2017 22:19:47 GMT'); // -3 часа от московского
				client.setHeader('Cache-control', 'public, max-age=60');
				// Сообщаем что страница не найдена
				client.writeHead(200, {"Content-Type": "text/plain; charset=utf-8"});
				client.write("Ok\n");
				client.end();
			} break;
			// Тест 10 (Cache-control)
			case "/cc3": {
				// Устанавливаем заголовки
				client.setHeader('Last-modified', 'Tue, 28 Mar 2017 22:19:47 GMT');
				client.setHeader('Etag', '123');
				client.setHeader('Expires', 'Tue, 28 Mar 2017 22:19:47 GMT');
				client.setHeader('Date', 'Tue, 28 Mar 2017 22:19:47 GMT'); // -3 часа от московского
				client.setHeader('Cache-control', 'public, max-age=60, s-maxage=120');
				// Сообщаем что страница не найдена
				client.writeHead(200, {"Content-Type": "text/plain; charset=utf-8"});
				client.write("Ok\n");
				client.end();
			} break;
			// Тест 11 (Cache-control)
			case "/cc4": {
				// Устанавливаем заголовки
				client.setHeader('Last-modified', 'Tue, 28 Mar 2017 22:19:47 GMT');
				client.setHeader('Etag', '123');
				client.setHeader('Expires', 'Tue, 28 Mar 2017 22:19:47 GMT');
				client.setHeader('Date', 'Tue, 28 Mar 2017 22:19:47 GMT'); // -3 часа от московского
				client.setHeader('Cache-control', 'public, max-age=60, s-maxage=120');
				// Сообщаем что страница не найдена
				client.writeHead(200, {"Content-Type": "text/plain; charset=utf-8"});
				client.write("Ok\n");
				client.end();
			} break;
			// Тест 12 (Cache-control)
			case "/cc5": {
				// Устанавливаем заголовки
				client.setHeader('Last-modified', 'Tue, 28 Mar 2017 22:19:47 GMT');
				client.setHeader('Etag', '123');
				client.setHeader('Expires', 'Tue, 28 Mar 2017 22:19:47 GMT');
				client.setHeader('Date', 'Tue, 28 Mar 2017 22:19:47 GMT'); // -3 часа от московского
				client.setHeader('Cache-control', 'public, max-age=60, s-maxage=120, must-revalidate');
				// Сообщаем что страница не найдена
				client.writeHead(200, {"Content-Type": "text/plain; charset=utf-8"});
				client.write("Ok\n");
				client.end();
			} break;
			// Тест 13 (Cache-control)
			case "/cc6": {
				// Устанавливаем заголовки
				client.setHeader('Last-modified', 'Tue, 28 Mar 2017 22:19:47 GMT');
				client.setHeader('Etag', '123');
				client.setHeader('Expires', 'Tue, 28 Mar 2017 22:19:47 GMT');
				client.setHeader('Date', 'Tue, 28 Mar 2017 22:19:47 GMT'); // -3 часа от московского
				client.setHeader('Cache-control', 'max-age=60, s-maxage=120, must-revalidate, proxy-revalidate');
				// Сообщаем что страница не найдена
				client.writeHead(200, {"Content-Type": "text/plain; charset=utf-8"});
				client.write("Ok\n");
				client.end();
			} break;
			// Тест 14 (Cache-control)
			case "/cc7": {
				// Устанавливаем заголовки
				client.setHeader('Last-modified', 'Tue, 28 Mar 2017 22:19:47 GMT');
				client.setHeader('Etag', '123');
				client.setHeader('Expires', 'Tue, 28 Mar 2017 22:19:47 GMT');
				client.setHeader('Date', 'Tue, 28 Mar 2017 22:19:47 GMT'); // -3 часа от московского
				client.setHeader('Cache-control', 'public, max-age=60, s-maxage=120, no-cache');
				// Сообщаем что страница не найдена
				client.writeHead(200, {"Content-Type": "text/plain; charset=utf-8"});
				client.write("Ok\n");
				client.end();
			} break;
			// Тест 15 (Cache-control)
			case "/cc8": {
				// Устанавливаем заголовки
				client.setHeader('Last-modified', 'Tue, 28 Mar 2017 22:19:47 GMT');
				client.setHeader('Etag', '123');
				client.setHeader('Expires', 'Tue, 28 Mar 2017 22:19:47 GMT');
				client.setHeader('Date', 'Tue, 28 Mar 2017 22:19:47 GMT'); // -3 часа от московского
				client.setHeader('Cache-control', 'private, max-age=60, s-maxage=120');
				// Сообщаем что страница не найдена
				client.writeHead(200, {"Content-Type": "text/plain; charset=utf-8"});
				client.write("Ok\n");
				client.end();
			} break;
			// Тест 16 (Cache-control)
			case "/cc9": {
				// Устанавливаем заголовки
				client.setHeader('Last-modified', 'Tue, 28 Mar 2017 22:19:47 GMT');
				client.setHeader('Etag', '123');
				client.setHeader('Expires', 'Tue, 28 Mar 2017 22:19:47 GMT');
				client.setHeader('Date', 'Tue, 28 Mar 2017 22:19:47 GMT'); // -3 часа от московского
				client.setHeader('Cache-control', 'no-store, max-age=60, s-maxage=120');
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