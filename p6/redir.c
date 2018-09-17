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


//GREP
//Filtra las lineas de error de gcc dada una palabra----------------------------
int grep_f(int fd, char *word){
	int pid, sts;

	dup2 (fd, 0);
	close(fd);
	pid = fork();
	switch(pid){
	case -1:
		err(1, "grep failed");

	case 0:
		execl("/bin/grep", "grep", word, NULL);
		err(1, "execl failed");

	default:
		while(wait(&sts) != pid)
			;
		if (sts != 0) {
			return -1;
		}
		return 0;
	}
}


//COMP--------------------------------------------------------------------------
//Compila y enlaza ficheros .c
int comp(char *path, char *word){
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

				int fd[2];
				if (pipe(fd) < 0) {
					err(1, "pipe failed");
				}

				pid = fork();
        switch(pid){
        case -1:
          err(1, "fork failed");

        case 0:
					close(fd[0]);
					dup2(fd[1], 2);
					close(fd[1]);

          if (chdir(path) < 0){ //changing working directory to compile
            err(1,"chdir");
          }

          if (flag){
            execl("/usr/bin/gcc", "gcc", env, "-o", buf, de->d_name, NULL);
          }else{
            execl("/usr/bin/gcc", "gcc", "-o", buf, de->d_name, NULL);
          }
          err(1, "execl failed");

        default:
          while(wait(&sts) != pid)
            ;

					close (fd[1]);
          if (sts != 0) {
            printf("%s: NO COMPILA\n", de->d_name);
						if (strcmp(word, "") != 0){ //Palabra distinto de "";
							if ((grep_f(fd[0], word)) < 0){ //Grepping!!
						 		errx(1, "'%s' pattern not found.", word);
							}
						}
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


//Directory or word?------------------------------------------------------------
int is_directory(const char *path){
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode); //non-zero if the file is a directory
}


//MAIN--------------------------------------------------------------------------
int
main(int argc, char *argv[])
{
  char *path;
	char *word;
  char cwd[100];

  //Usage testing
  argc--;
  if (argc == 0){ //Current work directory
    if (getcwd(cwd, sizeof(cwd)) == NULL){
  		err(1, "getcwd");
  	}
    comp(cwd,"");
  }else if (argc == 1){
    path = strdup(argv[1]);
		if (is_directory(path) == 0){ //Palabra + CWD
			if (getcwd(cwd, sizeof(cwd)) == NULL){
	  		err(1, "getcwd");
	  	}
	    comp(cwd,path);
		}else{ //directorio y NO palabra
    	comp(path, "");
		}
		free(path);
  }else if (argc == 2){
		path = strdup(argv[1]);
		if (is_directory(path) == 0){ //Palabra
			errx(1, "Usage: directory_with_files   word_to_find");
		}
		word = strdup(argv[2]);
		comp(path, word);
		free(path);
		free(word);
	}else{
    errx(1, "Usage: directory_with_files   word_to_find");
  }

  exit(0);
}
