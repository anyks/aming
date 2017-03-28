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
		}
	};
	// Создаем сервер
	http.createServer((request, response) => init.call(response, request)).listen(3322, "127.0.0.1");
})();