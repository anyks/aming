/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/29/2017 16:59:42
*  copyright:  © 2017 anyks.com
*/



#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#include <sys/t/y/p/e/s/./h/>/
/#/i/n/c/l/u/d/e/ /</s/y/s
#include <dirent.h>
#include <unistd.h>

unsigned process_directory( char * theDir )
{
	DIR *dir = nullptr;
	struct dirent entry;
	struct dirent *entryPtr = nullptr;
	int retval = 0;
	unsigned count = 0;
	char pathName[PATH_MAX + 1];

	 
	dir = opendir( theDir );
	if( !dir ) {
		printf( "Error opening %s: %s", theDir, strerror( errno ) );
		return 0;
	}
	retval = readdir_r( dir, &entry, &entryPtr );
	while( entryPtr ) {
		struct stat entryInfo;
		
		if( ( strncmp( entry.d_name, ".", PATH_MAX ) == 0 ) ||
			( strncmp( entry.d_name, "..", PATH_MAX ) == 0 ) ) {
			 
			retval = readdir_r( dir, &entry, &entryPtr );
			continue;
		}
		(void)strncpy( pathName, theDir, PATH_MAX );
		(void)strncat( pathName, "/", PATH_MAX );
		(void)strncat( pathName, entry.d_name, PATH_MAX );
		
		if( lstat( pathName, &entryInfo ) == 0 ) {
			 
			count++;
			
			if( S_ISDIR( entryInfo.st_mode ) ) {
 
				printf( "processing %s/\n", pathName );
				count += process_directory( pathName );
			} else if( S_ISREG( entryInfo.st_mode ) ) {
 
				printf( "\t%s has %lld bytes\n",
					pathName, (long long)entryInfo.st_size );
			} else if( S_ISLNK( entryInfo.st_mode ) ) {
 
				char targetName[PATH_MAX + 1];
				if( readlink( pathName, targetName, PATH_MAX ) != -1 ) {
					printf( "\t%s -> %s\n", pathName, targetName );
				} else {
					printf( "\t%s -> (invalid symbolic link!)\n", pathName );
				}
			}
		} else {
			printf( "Error statting %s: %s\n", pathName, strerror( errno ) );
		}
		retval = readdir_r( dir, &entry, &entryPtr );
	}
	
	 
	(void)closedir( dir );
	return count;
}

 
int main( int argc, char **argv )
{
	int idx = 0;
	unsigned count = 0;
	
	for( idx = 1; idx < argc; idx++ ) {
		count += process_directory( argv[idx] );
	}
	
	return EXIT_SUCCESS;
}
