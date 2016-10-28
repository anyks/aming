const http = require('http');
const net = require('net');
const url = require('url');
const server = http.createServer((request, response) => {
	console.log("request1", request.url);
	const ph = url.parse(request.url);
	const options = {
		port:		ph.port,
		hostname:	ph.hostname,
		method:		request.method,
		path:		ph.path,
		headers:	request.headers
	};
	const proxyRequest = http.request(options);
	proxyRequest.on('response', proxyResponse => {
		proxyResponse.on('data', chunk => response.write(chunk, 'binary'));
		proxyResponse.on('end', () => response.end());
		response.writeHead(proxyResponse.statusCode, proxyResponse.headers);
	});
	request.on('data', chunk => proxyRequest.write(chunk, 'binary'));
	request.on('end', () => proxyRequest.end());
}).on('connect', (request, socketRequest, head) => {
	console.log("request2", request.url);
	const ph = url.parse('http://' + request.url);
	const socket = net.connect(ph.port, ph.hostname, () => {
		socket.write(head);
		// Сказать клиенту, что соединение установлено
		socketRequest.write("HTTP/" + request.httpVersion + " 200 Connection established\r\n\r\n");
	});
	// Туннелирование к хосту
	socket.on('data', chunk => socketRequest.write(chunk));
	socket.on('end', () => socketRequest.end());
	socket.on('error', () => {
		// Сказать клиенту, что произошла ошибка
		socketRequest.write("HTTP/" + request.httpVersion + " 500 Connection error\r\n\r\n");
		socketRequest.end();
	});
	// Туннелирование к клиенту
	socketRequest.on('data', chunk => socket.write(chunk));
	socketRequest.on('end', () => socket.end());
	socketRequest.on('error', () => socket.end());
}).listen(8080);