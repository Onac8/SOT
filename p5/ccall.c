#include <stdlib.h>
#include <err.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/dir.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

//CFILE-------------------------------------------------------------------------
//Search C files and return int(0) + file_name(without .c) on success
int cfile (char *c, char *buf){
	char t[] = ".c";
	char *trash;
  int len;

	trash = strrchr(c,'.'); //'.' not found
	if (trash == NULL){
		return -1;
	}

	if (strcmp (trash, t) == 0){
    len = strlen(c) - strlen(trash); //file_name size
    strncpy(buf, c, len);
    strcat(buf, "\0"); // 'example.c' --> 'example' return
		return 0;
	}
	return -1;
}

//COMP--------------------------------------------------------------------------
//Compila y enlaza ficheros .c
int comp(char *path){
  char *env;
  char buf[100];
  DIR *d;
  struct dirent *de;
  int pid, sts;
  int flag = 0;

  d = opendir(path);
  if (d == NULL){
    err(1, "opendir '%s'", path);
  }

  env = getenv("CFLAGS");
  if (env != NULL){
    if (strcmp(env, "-Wall") == 0){
      flag = 1;
      printf("%s\n", env);
    }
  }


  while((de = readdir(d)) != NULL){
    if (de->d_type == DT_REG){ /*Fichero convencional*/
      if (cfile(de->d_name, buf) == 0){ //-> success -> fork | else while again
        pid = fork();
        switch(pid){
        case -1:
          err(1, "fork failed");
          break;

        case 0:
          if (chdir(path) < 0){ //changing working directory to compile
            err(1,"chdir");
          }

          if (flag){
            execl("/usr/bin/gcc", "gcc", env, "-o", buf, de->d_name, NULL);
          }else{
            execl("/usr/bin/gcc", "gcc", "-o", buf, de->d_name, NULL);
          }
          err(1, "execl failed");
          break;

        default:
          while(wait(&sts) != pid)
            ;
          if (sts != 0) {
            printf("%s: NO COMPILA\n", de->d_name);
          }else{
            printf("%s: COMPILA\n", de->d_name);
          }
        }
      }
    }
  }

  if (closedir(d) < 0){
    err(1, "closedir");
  }
  return 0;
}



//MAIN--------------------------------------------------------------------------
int
main(int argc, char *argv[])
{
  char *path;
  char cwd[100];

  //Usage testing
  argc--;
  if (argc == 0){ //Current work directory
    if (getcwd(cwd, sizeof(cwd)) == NULL){
  		err(1, "getcwd");
  	}
    comp(cwd);
  }else if (argc == 1){
    path = strdup(argv[1]);
    comp(path);
    free(path);
  }else{
    errx(1, "Usage: directory_with_files (or current directory if no arguments given)");
  }

  exit(0);
}
