'use strict'

const phantom = require('phantom');
const co = require('co');

co(function*(){
  var instance = yield phantom.create([
      '--proxy=127.0.0.1:5555',
      '--proxy-auth=TNjFZRCZ3C:techhost@agro24.ru',
      '--proxy-type=socks5'
      ])

  var page = yield instance.createPage()
  yield page.open('http://games.ru')
  //yield page.open('http://stex3d.com')
});