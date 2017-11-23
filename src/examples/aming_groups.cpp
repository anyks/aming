/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/




#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>

int
main(int argc, char *argv[])
{
   int j, ngroups;
   int *groups;
   struct passwd *pw;
   struct group *gr;

   if (argc != 3) {
       fprintf(stderr, "Usage: %s <user> <ngroups>\n", argv[0]);
       exit(EXIT_FAILURE);
   }


   char * login = getlogin();

   printf("Current Login = %s\n", login);

   struct passwd * pw2 = NULL;

    

   while((pw2 = getpwent()) != NULL){
    printf("Login = %s, Password = %s, GID = %d, UID = %d, INFO = %s, DIR = %s, SHELL = %s\n", pw2->pw_name, pw2->pw_passwd, pw2->pw_gid, pw2->pw_uid, pw2->pw_gecos, pw2->pw_dir, pw2->pw_shell);
   }


  


   ngroups = atoi(argv[2]);

   groups = new int[ngroups];
   if (groups == NULL) {
       perror("malloc");
       exit(EXIT_FAILURE);
   }

    

   pw = getpwnam(argv[1]);
   if (pw == NULL) {
       perror("getpwnam");
       delete [] groups;
       exit(EXIT_SUCCESS);
   }

    

   if (getgrouplist(argv[1], pw->pw_gid, groups, &ngroups) == -1) {
       fprintf(stderr, "getgrouplist() returned -1; ngroups = %d\n",
               ngroups);
       delete [] groups;
       exit(EXIT_FAILURE);
   }

    

    

   fprintf(stderr, "ngroups = %d\n", ngroups);
   for (j = 0; j < ngroups; j++) {
       printf("%d", groups[j]);
       gr = getgrgid(groups[j]);
       if (gr != NULL)
           printf(" (%s)", gr->gr_name);
       printf("\n");
   }

   delete [] groups;

   exit(EXIT_SUCCESS);
} 