const assert = require('assert');
const gateway = 'proxy://zdD786KeuS:k.frolovv@gmail.com@176.107.177.120:44441/'; // Прокси для редиректа
const http = require('http');
const net = require('net');
const url = require('url');
const server = http.createServer((request, response) => {
	console.log("request1", request.url);
	const ph = url.parse(request.url);
	const gw = url.parse(gateway);
	const options = {
		port:		parseInt(gw.port),
		hostname:	gw.hostname,
		method:		request.method,
		path:		request.url,
		headers:	request.headers || {}
	};
	if(gw.auth) options.headers['Proxy-Authorization'] = 'Basic ' + new Buffer(gw.auth).toString('base64');
	// console.log(options)
	const gatewayRequest = http.request(options);
	gatewayRequest.on('error', err => {
		console.log('[error]', err);
		response.end();
	});
	gatewayRequest.on('response', gatewayResponse => {
		if(gatewayResponse.statusCode === 407) {
			console.log('[error] AUTH REQUIRED');
			process.exit();
		}
		gatewayResponse.on('data', chunk => response.write(chunk, 'binary'));
		gatewayResponse.on('end', () => response.end());
		response.writeHead(gatewayResponse.statusCode, gatewayResponse.headers);
	})
	request.on('data', chunk => gatewayRequest.write(chunk, 'binary'));
	request.on('end', () => gatewayRequest.end());
	gatewayRequest.end();
}).on('connect', (request, socketRequest, head) => {
	console.log("request2", request.url);
	const ph = url.parse('http://' + request.url);
	const gw = url.parse(gateway);
	const options = {
		port:		gw.port,
		hostname:	gw.hostname,
		method:		'CONNECT',
		path:		ph.hostname + ':' + (ph.port || 80),
		headers:	request.headers || {}
	};
	if(gw.auth) options.headers['Proxy-Authorization'] = 'Basic ' + new Buffer(gw.auth).toString('base64');
	// console.log(options)
	const gatewayRequest = http.request(options);
	gatewayRequest.on('error', err => {
		console.log('[error]', err);
		process.exit();
	});
	gatewayRequest.on('connect', (res, socket, head) => {
		assert.equal(res.statusCode, 200);
		assert.equal(head.length, 0);
		socketRequest.write("HTTP/" + request.httpVersion + " 200 Connection established\r\n\r\n");
		// Туннелирование к хосту
		socket.on('data', chunk => socketRequest.write(chunk, 'binary'));
		socket.on('end', () => socketRequest.end());
		socket.on('error', () => {
			// Сказать клиенту, что произошла ошибка
			socketRequest.write("HTTP/" + request.httpVersion + " 500 Connection error\r\n\r\n");
			socketRequest.end();
		})
		// Туннелирование к клиенту
		socketRequest.on('data', chunk => socket.write(chunk, 'binary'));
		socketRequest.on('end', () => socket.end());
		socketRequest.on('error', () => socket.end());
	}).end();
}).listen(8080);