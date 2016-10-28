var glb = {} // global
glb.ajaxError = false
glb.ajaxCount = 0
$(document).ready(function() {
    glb.hidden = {}
    $('body > input:hidden').each(function() {
        glb.hidden[$(this).attr('name')] = $(this).val()
    }).remove()
    if(typeof init === 'function') init();
})
function ajax(url, data, context, handle) {
    if(typeof context === 'undefined') context = null
    if(typeof handle === 'undefined') handle = function() { ; }
    if(glb.ajaxError) return
    glb.ajaxCount += 1
    $.ajax({
        url: url,
        type: 'POST',
        dataType: 'text',
        async: true,
        cache: false,
        data: data,
        context: context,
        error: function() { glb.ajaxError = true },
        success: function(text) {
            if (glb.ajaxError) return
            glb.ajaxCount -= 1
            var obj = jeval(text)
            if (!obj) return
            if (obj.err) return
            if (obj.redirect) document.location.href = obj.redirect
            if (obj.goto) document.location.href = obj.goto
            handle(obj, this)
        }
    })
}
function jeval(obj) { try { return eval('(' + obj + ')') } catch(e) { e = e ; return null } }
if (!window.console) { window.console = {log : function() { ; }, dir : function() { ; }} }
function mt_rand(min, max) { return Number(Math.floor(Math.random() * (max - min + 1)) + min) }
//
//
//
function m3q_switch(vClass) {
    glb.m3q_switch += 1
    if(glb.m3q_switch > 4) glb.m3q_switch -= 4
    $('img.' + vClass).hide()
    $('img.' + vClass + ':nth-child(' + glb.m3q_switch + ')').show()
}
function table_dropdown(context, criteria) {
    var t, tr = $(context).parents('tr:first')
    tr = tr.next()
    while(true)
        if(!tr.length) break
        else if(!tr.is(criteria)) break
        else { t = tr.next() ; tr.toggle() ; tr = t }
}
function init() {
    if($('img.m3q_switch').length) {
        glb.m3q_switch = 0
        m3q_switch('m3q_switch')
    }
    $('input.textfield.searchtip').keypress(function(e) {
        var t = e.keyCode || e.which
        if (t != 13) return
    })
    $('#btn_search').click(function(e) {
        var selector = $('input.textfield.searchtip')
        var name = selector.attr('name')
        var data = {}
        data[name] = $.trim(selector.val())
        selector.val(data[name])
        if (!data[name]) {
            e.preventDefault()
            return
        }
        if (!/!$/.test(data[name])) return
        e.preventDefault()
        ajax('/search/', data, selector, function(obj, selector) {
            if ('value' in obj) selector.val(obj.value)
        })
    })
    if($('#change_me_to_e').length) $('#change_me_to_e').text('e')
    if($("button.jq_ui_button").length) $("button.jq_ui_button").button().css('margin-right', '0px')
    $("table.enc_t input:password").keypress(function(e){var t=e.keyCode?e.keyCode:e.which;if(t!=13)return;var n=$(this).parents("table:first");var r=n.find("td.encrypted");$(this).parents("tr:first").remove();var i=decrypt(r.text(),$.trim($(this).val()));var s,o=0,u=Number.MAX_VALUE;i=i.split("\n");for(var a in i){s=i[a].split(";").length;if(s>o)o=s;if(s<u)u=s}if(o!=u)return;n.prevAll("table.enc_t").remove();n.nextAll("table.enc_t").remove();glb.my_rnd=mt_rand(0,Number.MAX_VALUE)+"";glb.my_passwd=encrypt($.trim($(this).val()),glb.my_rnd);for(var a in i)i[a]="<td>"+i[a].replace(/;/g,"</td><td>")+"</td>";i='<table class="enc_tt" cellspacing="0" cellpadding="0" style="border-collapse:collapse; width:600px;">'+'<tr><td colspan="3"><input type="text" style="border:1px; width:100%;" /></td></tr>'+"<tr>"+i.join("</tr><tr>")+"</tr></table>";r.html(i);r.parent().show();var f=n.find("table.enc_tt");f.find("tr:gt(0)").hide();f.find("input:text").keypress(function(e){var t=e.keyCode?e.keyCode:e.which;if(t!=13)return;var n=$(this).parents("table:first");n.find("tr:gt(0)").hide();var r=$.trim($(this).val());if(r.indexOf("add:")==0){var i=r.match(/add:(.*)/);r=i[1];r=r.split(";");n.append(n.find("tr:last").clone());var s=n.find("tr:last");s.find("td").text("");for(var o=0;o<s.find("td").length;o++)if(r[o])s.find("td").eq(o).text(r[o]);s.show()}else if(r=="save:ok"){var u=[];n.find("tr:gt(0)").each(function(){var e=[];$(this).find("td").each(function(){e.push($.trim($(this).text()))});u.push(e.join(";"))});u=u.join("\n");u=encrypt(u,decrypt(glb.my_passwd,glb.my_rnd));var a=$.trim(n.parents("table.enc_t:first").find("th:first").text());ajax('/_/',{action:'passwd',th:a,passwd:u},null,function(){document.location.reload()})}else n.find("tr:gt(0) td").each(function(){if($(this).text().toLowerCase().indexOf(r.toLowerCase())!=-1)$(this).parent().show()})}).focus()})
    $('table.t_click:not(.enc_t) th:first-child').css('cursor', 'pointer').click(function() {
        var table = $(this).parents('table:first')
        var hidden = table.find('tr:gt(0):hidden:first')
        if(hidden.length) hidden.toggle()
        else table.find('tr:gt(0)').toggle()
    })
    if($('input[name=hero]').length) {
        $('.herolist ul li a').click(function(e) {e.preventDefault()})
        var hero = $('input[name=hero]').val().split(',')
        for(var i in hero) $('.herolist ul li a[href=#' + hero[i] + ']').click(function (e) {
            var url = $(this).attr('href')
            url = url.replace('#', '')
            if($('input[name=en]').length)
                playPauseSound('/inc/dota/en/' + url + '.mp3')
            else
                playPauseSound('/inc/dota/ru/' + url + '.mp3')
        }).find('img').css('opacity', '1')
    }
}
function run_spoiler(context) {
    $(context).nextAll('div').toggle()
    if($(context).nextAll('div').css('display') != 'none') $(context).html('[&ndash;]')
    else $(context).html('[+]')
}
var myListener = new Object()
myListener.onInit = function() {
    this.position = 0
    this.url = null
}
myListener.onUpdate = function() {;}
function getFlashObject() {return document.getElementById("myFlash")}
function playPauseSound(url) {
    if(url == myListener.url) {
        if(myListener.isPlaying && !glb.is_paused) {
            getFlashObject().SetVariable("method:pause", "")
            glb.is_paused = true
        } else {
            glb.is_paused = false
            getFlashObject().SetVariable("method:play", "")
            getFlashObject().SetVariable("enabled", "true")
        }
    } else {
        glb.is_paused = false
        getFlashObject().SetVariable("method:setUrl", url)
        getFlashObject().SetVariable("method:play", "")
        getFlashObject().SetVariable("enabled", "true")
    }
}
function toggle_spoiler(context) {
    $(context).parent().next().toggle()
}
