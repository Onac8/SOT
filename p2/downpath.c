#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>



void mayus (char *str){ /*Convert to upper case*/
  char *s = str;
  while (*s) {
    *s = toupper(*s);
    s++;
  }
}


int makepath (char *buf, int len){
	struct passwd *user;
	char *name;
	char path[50];
	char aux[10];
	pid_t p;

	user = getpwuid(getuid());
	name = user->pw_name;
	p = getpid();

	if (user == NULL) {
		return -1;
	}

	if (getcwd(path, sizeof(path)) == NULL){ //path absoluto del fichero
		return -1;
	}

	mayus(name);
	/*Formando el path*/
	strcat(buf, path); /*path absoluto*/
	strcat(buf, "/");
	strcat(buf, name);
	strcat(buf, ".");
	sprintf(aux, "%d", p); /*int en un string*/
	strcat(buf, aux); /*PEPE.1234*/

	return strlen(buf);
}


int
main(int argc, char *argv[])
{
	int len = 30;
	char buffer[len];
	char *buf = buffer;

	if (makepath(buf, len) < 0){
		errx(1, "Can't make absolut path.");
	}

	printf("%s\n", buf);
	exit(0);
}
