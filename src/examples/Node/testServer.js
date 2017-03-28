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
			case "/test":
				// Определяем контент
				const contentType = client.getHeader('content-type');
				// Устанавливаем заголовки
				client.setHeader('Foo', 'bar');
				client.setHeader('Set-Cookie', ['foo=bar', 'bar=baz']);
				// Сообщаем что страница не найдена
				client.writeHead(404, {"Content-Type": "text/plain; charset=utf-8"});
				client.write("404 Not Found\n");
				client.end();
			break;
			// Тест 1 (Age)
			case "/age1":
				// Устанавливаем заголовки
				client.setHeader('Age', '60');
				// Сообщаем что страница не найдена
				client.writeHead(200, {"Content-Type": "text/plain; charset=utf-8"});
				client.write("Ok\n");
				client.end();
			break;
			// Тест 2 (Age)
			case "/age2":
				// Устанавливаем заголовки
				client.setHeader('Age', '60');
				client.setHeader('Date', 'Tue, 28 Mar 2017 22:19:47 GMT'); // -3 часа от московского
				// Сообщаем что страница не найдена
				client.writeHead(200, {"Content-Type": "text/plain; charset=utf-8"});
				client.write("Ok\n");
				client.end();
			break;
			// Тест 3 (Age)
			case "/age3":
				// Устанавливаем заголовки
				client.setHeader('Age', '60');
				client.setHeader('Date', 'Tue, 28 Mar 2017 22:19:47 GMT'); // -3 часа от московского
				client.setHeader('Pragma', 'no-cache');
				// Сообщаем что страница не найдена
				client.writeHead(200, {"Content-Type": "text/plain; charset=utf-8"});
				client.write("Ok\n");
				client.end();
			break;
			// Тест 4 (Etag)
			case "/etag1":
				// Дата последней модификации
				const modifiedServer = 'Tue, 28 Mar 2017 22:19:47 GMT';
				// Получаем дату последней модификации от клиента
				const modifiedClient = client.getHeader('If-Modified-Since');
				// Устанавливаем заголовки
				client.setHeader('Last-modified', modifiedServer);
				client.setHeader('Date', 'Tue, 28 Mar 2017 22:19:47 GMT'); // -3 часа от московского
				client.setHeader('Pragma', 'no-cache');
				// Если дата на сервере изменилась
				if((new Date(modifiedServer)).valueOf() > (new Date(modifiedClient)).valueOf()){
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
			break;
			// Тест 5 (Etag)
			case "/etag2":
				// Дата последней модификации
				const modifiedServer = 'Tue, 28 Mar 2017 22:19:47 GMT';
				// Получаем дату последней модификации от клиента
				const modifiedClient = client.getHeader('If-Modified-Since');
				// Данные eTag
				const etagServer = "123";
				// Получаем данные Etag
				const etagClient = client.getHeader('If-None-Match');
				// Устанавливаем заголовки
				client.setHeader('Last-modified', modifiedServer);
				client.setHeader('Etag', '123');
				client.setHeader('Pragma', etagServer);
				// Если дата на сервере изменилась
				if(((new Date(modifiedServer)).valueOf() > (new Date(modifiedClient)).valueOf()) || (etagServer === etagClient)){
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
			break;
			// Тест 6 (Etag)
			case "/etag6":
				// Устанавливаем заголовки
				client.setHeader('Last-modified', 'Tue, 28 Mar 2017 22:19:47 GMT');
				client.setHeader('Expires', 'Tue, 28 Mar 2017 22:19:47 GMT');
				// Сообщаем что страница не найдена
				client.writeHead(200, {"Content-Type": "text/plain; charset=utf-8"});
				client.write("Ok\n");
				client.end();
			break;
			// Тест 7 (Etag)
			case "/etag7":
				// Устанавливаем заголовки
				client.setHeader('Last-modified', 'Tue, 28 Mar 2017 22:19:47 GMT');
				client.setHeader('Etag', '123');
				client.setHeader('Expires', 'Tue, 28 Mar 2017 22:19:47 GMT');
				// Сообщаем что страница не найдена
				client.writeHead(200, {"Content-Type": "text/plain; charset=utf-8"});
				client.write("Ok\n");
				client.end();
			break;
			// Тест 8 (Cache-control)
			case "/cc1":
				// Устанавливаем заголовки
				client.setHeader('Last-modified', 'Tue, 28 Mar 2017 22:19:47 GMT');
				client.setHeader('Etag', '123');
				client.setHeader('Expires', 'Tue, 28 Mar 2017 22:19:47 GMT');
				client.setHeader('Cache-control', 'max-age=60');
				// Сообщаем что страница не найдена
				client.writeHead(200, {"Content-Type": "text/plain; charset=utf-8"});
				client.write("Ok\n");
				client.end();
			break;
			// Тест 9 (Cache-control)
			case "/cc2":
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
			break;
			// Тест 10 (Cache-control)
			case "/cc3":
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
			break;
			// Тест 11 (Cache-control)
			case "/cc4":
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
			break;
			// Тест 12 (Cache-control)
			case "/cc5":
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
			break;
			// Тест 13 (Cache-control)
			case "/cc6":
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
			break;
			// Тест 14 (Cache-control)
			case "/cc7":
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
			break;
			// Тест 15 (Cache-control)
			case "/cc8":
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
			break;
			// Тест 16 (Cache-control)
			case "/cc9":
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
			break;
		}
	};
	// Создаем сервер
	http.createServer((request, response) => init.call(response, request)).listen(3322, "127.0.0.1");
})();