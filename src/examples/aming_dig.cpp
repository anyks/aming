/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/08/2017 16:52:48
*  copyright:  © 2017 anyks.com
*/



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/i/n/./h/>/
/#/i/n/c/l/u/d/e/ /</a/r/p/a
#include <resolv.h>

int main (int argc, char *argv[])
{
    u_char nsbuf[4096];
    char dispbuf[4096];
    ns_msg msg;
    ns_rr rr;
    int i, j, l;

    if (argc < 2) {
        printf ("Usage: %s <domain>[...]\n", argv[0]);
        exit (1);
    }

    for (i = 1; i < argc; i++) {
        l = res_query (argv[i], ns_c_any, ns_t_a, nsbuf, sizeof (nsbuf)); 
        if (l < 0) {
            perror (argv[i]);
        } else {
#ifdef USE_PQUERY

            res_pquery (&_res, nsbuf, l, stdout);
#else

            ns_initparse (nsbuf, l, &msg);
            printf ("%s :\n", argv[i]);
            l = ns_msg_count (msg, ns_s_an);
            for (j = 0; j < l; j++) {
                ns_parserr (&msg, ns_s_an, j, &rr);
                ns_sprintrr (&msg, &rr, nullptr, nullptr, dispbuf, sizeof (dispbuf));
                printf ("%s\n", dispbuf);
            }
#endif
        }
    }
    exit (0);
}