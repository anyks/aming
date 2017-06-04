// $ g++ -std=c++11 ./examples/dir.cpp -o ./bin/dir
// $ ./bin/dir

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

unsigned process_directory( char * theDir )
{
	DIR *dir = NULL;
	struct dirent entry;
	struct dirent *entryPtr = NULL;
	int retval = 0;
	unsigned count = 0;
	char pathName[PATH_MAX + 1];

	/* открыть указанный каталог, если возможно. */
	dir = opendir( theDir );
	if( dir == NULL ) {
		printf( "Error opening %s: %s", theDir, strerror( errno ) );
		return 0;
	}
	retval = readdir_r( dir, &entry, &entryPtr );
	while( entryPtr != NULL ) {
		struct stat entryInfo;
		
		if( ( strncmp( entry.d_name, ".", PATH_MAX ) == 0 ) ||
			( strncmp( entry.d_name, "..", PATH_MAX ) == 0 ) ) {
			/* Short-circuit the . and .. entries. */
			retval = readdir_r( dir, &entry, &entryPtr );
			continue;
		}
		(void)strncpy( pathName, theDir, PATH_MAX );
		(void)strncat( pathName, "/", PATH_MAX );
		(void)strncat( pathName, entry.d_name, PATH_MAX );
		
		if( lstat( pathName, &entryInfo ) == 0 ) {
			/* вызов stat() был успешным, так что продолжаем. */
			count++;
			
			if( S_ISDIR( entryInfo.st_mode ) ) {
/* каталог */
				printf( "processing %s/\n", pathName );
				count += process_directory( pathName );
			} else if( S_ISREG( entryInfo.st_mode ) ) {
/* обычный файл */
				printf( "\t%s has %lld bytes\n",
					pathName, (long long)entryInfo.st_size );
			} else if( S_ISLNK( entryInfo.st_mode ) ) {
/* символическая ссылка */
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
	
	/* закрытие каталога и возвращение числа элементов. */
	(void)closedir( dir );
	return count;
}

/* readdir_demo main()
 * 
 * проходит через указанные каталоги 
 * и передает их в метод process_directory().
 */
int main( int argc, char **argv )
{
	int idx = 0;
	unsigned count = 0;
	
	for( idx = 1; idx < argc; idx++ ) {
		count += process_directory( argv[idx] );
	}
	
	return EXIT_SUCCESS;
}
