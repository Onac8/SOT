#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <unistd.h>
#include <sys/dir.h>
#include <dirent.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>


char *makepath(char *p, char *aux){ //le pasamos nuestra parte del path y lo nuevo
  char *p_aux;
  char x[50];

  strcpy(x, p);
  strcat(x, "/");
  strcat(x, aux);
  p_aux = strdup(x);

  // p_aux = strdup(p);
  // strcat(p_aux, "/");
  // strcat(p_aux, aux);
  return p_aux;
}


void printfich(char *file){
  char buf[1024];
  int fd, nr;

  fd = open (file, O_RDONLY);
  if(fd < 0){
    err(1, "open %s", file);
  }

  printf("%s\n", file); //ruta y nombre del fichero

  for(;;){
    nr = read(fd, buf, sizeof buf);
    if (nr < 0) {
      close(fd);
      err(1, "read %s", file);
    }else if(nr == 0){
      break;
    }
    if (write(1, buf, nr) != nr) {
      close(fd);
      err(1, "write");
    }
  }

  if (close(fd) < 0) {
    err(1, "close %s", file);
  }
}


int listdir (char *path){
  char *path_aux;
  char *path_fich;
  DIR *d;
  struct dirent *de;

  d = opendir(path);
  if (d == NULL){
    err(1, "opendir");
  }

  printf("%s\n", path);    //primer directorio, segundo,...

  while((de = readdir(d)) != NULL){
    if (de->d_type == DT_DIR){ /*Directorio dentro de directorio*/
      if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0){ //para omitir los directorios "." y ".."
        path_aux = makepath(path, de->d_name);
        if (path_aux == NULL){
          err(1,"makepath");
        }
        if (listdir(path_aux) < 0){ /*Recursividad*/
          err(1, "listdir");
        }
        free (path_aux);
      }
    }
    if (de->d_type == DT_REG){ /*Fichero convencional*/
      path_fich = makepath(path, de->d_name);
      if (path_fich == NULL){ //path (.../x.txt, .../reverse.c)
        err(1,"makepath");
      }
      printfich(path_fich);
      free (path_fich);
    }
  }

  if (closedir(d) < 0){
    err(1, "closedir");
  }

  return 0;
}




int
main(int argc, char *argv[])
{
  char dir[100];

  argc--;
  if (argc == 1){
    strcpy(dir, argv[1]);
  }else if (argc > 1){
    err(1, "Max arguments exceeded");
  }else{
    if (getcwd(dir, sizeof(dir)) == NULL){
  		err(1, "getcwd");
  	}
  }

  if (listdir(dir) < 0){
    err(1, "listdir");
  }
	exit(0);
}
