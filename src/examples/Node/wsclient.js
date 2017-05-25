var socket = new WebSocket("ws://127.0.0.1:8081");
socket.onopen = function() {
  alert("Соединение установлено.");
  
  socket.send("Привет");
};

socket.onclose = function(event) {
  if (event.wasClean) {
    alert('Соединение закрыто чисто');
  } else {
    alert('Обрыв соединения'); // например, "убит" процесс сервера
  }
  alert('Код: ' + event.code + ' причина: ' + event.reason);
};

socket.onmessage = function(event) {
  alert("Получены данные " + event.data);
};

socket.onerror = function(error) {
  alert("Ошибка " + error.message);
};

/*
GET / HTTP/1.1
Host: 127.0.0.1:8081
Upgrade: websocket
Connection: Upgrade
Origin: http://127.0.0.1:8081
Sec-WebSocket-Key: Iv8io/9s+lYFgZWcXczP8Q==
Sec-WebSocket-Version: 13
Proxy-Authorization: Basic emRENzg2S2V1UzprLmZyb2xvdnZAZ21haWwuY29t
 */