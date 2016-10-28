const http = require('http');
const url = require('url');
const server = http.createServer((request, response) => {
	console.log("request", request.url);
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
}).listen(8080);