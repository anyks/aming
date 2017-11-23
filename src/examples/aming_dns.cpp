/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/


#include <event2/d/n/s/./h/>/
/#/i/n/c/l/u/d/e/ /</e/v/e/n/t/2
#include <event2/e/v/e/n/t/./h/>/
/
/#/i/n/c/l/u/d/e/ /</s/y/s

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <assert.h>

#include <iostream>

int n_pending_requests = 0;
struct event_base *base = nullptr;

struct user_data {
    char *name;  
    int idx;  
};

void callback(int errcode, struct evutil_addrinfo *addr, void *ptr)
{
    
    struct user_data * data = reinterpret_cast <struct user_data *> (ptr);
    const char *name = data->name;
    if (errcode) {
        printf("%d. %s -> %s\n", data->idx, name, evutil_gai_strerror(errcode));
    } else {
        struct evutil_addrinfo *ai;
        printf("%d. %s", data->idx, name);
        if (addr->ai_canonname)
            printf(" [%s]", addr->ai_canonname);
        puts("");
        for (ai = addr; ai; ai = ai->ai_next) {
            char buf[128];
            const char *s = nullptr;
            if (ai->ai_family == AF_INET) {
                struct sockaddr_in *sin = (struct sockaddr_in *)ai->ai_addr;
                s = evutil_inet_ntop(AF_INET, &sin->sin_addr, buf, 128);
            } else if (ai->ai_family == AF_INET6) {
                struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)ai->ai_addr;
                s = evutil_inet_ntop(AF_INET6, &sin6->sin6_addr, buf, 128);
            }
            if (s)
                printf("    -> %s\n", s);
        }
        evutil_freeaddrinfo(addr);
    }
    free(data->name);
    delete data;
    if (--n_pending_requests == 0)
        event_base_loopexit(base, nullptr);
}

 
int main(int argc, char **argv)
{
    int i;
    struct evdns_base *dnsbase;

    if (argc == 1) {
        puts("No addresses given.");
        return 0;
    }
    base = event_base_new();
    if (!base)
        return 1;
    dnsbase = evdns_base_new(base, 1);
    if (!dnsbase)
        return 2;

    if(evdns_base_nameserver_ip_add(dnsbase, "dns1.yandex.net") != 0){
        std::cout << " Not Set 8.8.8.8" << std::endl;
    }
    if(evdns_base_nameserver_ip_add(dnsbase, "8.8.4.4") != 0){
        std::cout << " Not set 8.8.4.4" << std::endl;
    }

    for (i = 1; i < argc; ++i) {
        struct evutil_addrinfo hints;
        struct evdns_getaddrinfo_request *req;
        struct user_data *user_data;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_flags = EVUTIL_AI_CANONNAME;
         
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        if (!(user_data = new struct user_data[sizeof(struct user_data)])){
            perror("malloc");
            exit(1);
        }
        if (!(user_data->name = strdup(argv[i]))) {
            perror("strdup");
            exit(1);
        }
        user_data->idx = i;

        ++n_pending_requests;
        req = evdns_getaddrinfo(
                          dnsbase, argv[i], nullptr  ,
                          &hints, callback, user_data);
        if (req == NULL) {
          printf("    [request for %s returned immediately]\n", argv[i]);
           
        }
    }

    if (n_pending_requests)
      event_base_dispatch(base);

    evdns_base_free(dnsbase, 0);
    event_base_free(base);

    return 0;
}