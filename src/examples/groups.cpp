// $ g++ -std=c++11 ./examples/groups.cpp -o ./bin/groups
// $ ./bin/groups
// Help: http://man7.org/linux/man-pages/man3/

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

   /*
   struct passwd {
     char   *pw_name;       // username
     char   *pw_passwd;     // user password
     uid_t   pw_uid;        // user ID
     gid_t   pw_gid;        // group ID
     char   *pw_gecos;      // user information
     char   *pw_dir;        // home directory
     char   *pw_shell;      // shell program
    };
    */

   while((pw2 = getpwent()) != NULL){
    printf("Login = %s, Password = %s, GID = %d, UID = %d, INFO = %s, DIR = %s, SHELL = %s\n", pw2->pw_name, pw2->pw_passwd, pw2->pw_gid, pw2->pw_uid, pw2->pw_gecos, pw2->pw_dir, pw2->pw_shell);
   }


  // int getlogin_r(char *buf, size_t bufsize);


   ngroups = atoi(argv[2]);

   groups = new int[ngroups];
   if (groups == NULL) {
       perror("malloc");
       exit(EXIT_FAILURE);
   }

   /* Fetch passwd structure (contains first group ID for user) */

   pw = getpwnam(argv[1]);
   if (pw == NULL) {
       perror("getpwnam");
       delete [] groups;
       exit(EXIT_SUCCESS);
   }

   /* Retrieve group list */

   if (getgrouplist(argv[1], pw->pw_gid, groups, &ngroups) == -1) {
       fprintf(stderr, "getgrouplist() returned -1; ngroups = %d\n",
               ngroups);
       delete [] groups;
       exit(EXIT_FAILURE);
   }

   /* Display list of retrieved groups, along with group names */

   /*
    struct group {
         char   *gr_name;        // group name
         char   *gr_passwd;      // group password
         gid_t   gr_gid;         // group ID
         char  **gr_mem;         // NULL-terminated array of pointers
                                    to names of group members
     };
    */

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