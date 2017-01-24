//g++ -std=c++11 -o ./sysctl ./linux_sysctl.cpp

#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/sysctl.h>

int _sysctl(struct __sysctl_args *args );

#define OSNAMESZ 100

int main(void){
	 struct __sysctl_args args;
	 int maxcon;
	 size_t maxconlth;
	 int name[] = {NET_CORE_SOMAXCONN};

	 memset(&args, 0, sizeof(struct __sysctl_args));
	 args.name = name;
	 args.nlen = sizeof(name)/sizeof(name[0]);
	 args.oldval = maxcon;
	 args.oldlenp = &maxconlth;

	 maxconlth = sizeof(maxcon);

	 if (syscall(SYS__sysctl, &args) == -1) {
			 perror("_sysctl");
			 exit(EXIT_FAILURE);
	 }
	 printf("This machine is running %*i\n", maxconlth, maxcon);
	 exit(EXIT_SUCCESS);

	 /*
	 struct __sysctl_args args;
	 char osname[OSNAMESZ];
	 size_t osnamelth;
	 int name[] = { CTL_KERN, KERN_OSTYPE };

	 memset(&args, 0, sizeof(struct __sysctl_args));
	 args.name = name;
	 args.nlen = sizeof(name)/sizeof(name[0]);
	 args.oldval = osname;
	 args.oldlenp = &osnamelth;

	 osnamelth = sizeof(osname);

	 if (syscall(SYS__sysctl, &args) == -1) {
			 perror("_sysctl");
			 exit(EXIT_FAILURE);
	 }
	 printf("This machine is running %*s\n", osnamelth, osname);
	 exit(EXIT_SUCCESS);
	 */
}