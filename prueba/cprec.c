#include <stdlib.h>
#include <err.h>
#include <unistd.h>
#include <sys/dir.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>


char *makepath(char *p, char *aux){ //le pasamos nuestra parte del path y lo nuevo
  char *p_aux;
  char x[100];

  strcpy(x, p);
  strcat(x, "/");
  strcat(x, aux);
  p_aux = strdup(x);
  return p_aux;
}


//Copy only a file -------------------------------------------------------------
int newfich(char *old_fich, char *new_fich, long per_fich){
  char buf[1024];
  int fd, new_fd, nr;
  int status = -1;

  fd = open(old_fich, O_RDONLY);
  new_fd = open(new_fich, O_CREAT|O_TRUNC|O_WRONLY, per_fich);
  if(fd < 0){
    err(1, "open %s", old_fich);
  }
  if(new_fd < 0){
    err(1, "create %s", new_fich);
  }


  //Copying file
  for(;;){
    nr = read(fd, buf, sizeof buf);
    if (nr < 0) {
      close(fd);
      close(new_fd);
      err(1, "read %s", old_fich);
    }else if(nr == 0){
      break;
    }
    if (write(new_fd, buf, nr) != nr) {
      close(fd);
      close(new_fd);
      err(1, "write");
    }
  }


  if (close(fd) < 0) {
    err(1, "close %s", old_fich);
  }
  if (close(new_fd) < 0) {
    err(1, "close %s", new_fich);
  }
  status = 0;
  return status;
}



//Copy directories and files recursively----------------------------------------
int newdir(char *old_path, char *new_path, long per_dir, long per_fich){
  char *path_aux;
  char *path_aux2;
  char *path_fich;
  char *path_fich2;
  DIR *d;
  struct dirent *de;
  int status = -1;

  d = opendir(old_path);

  if (mkdir(new_path, per_dir) < 0){
    err(1, "mkdir");
  }

  while((de = readdir(d)) != NULL){
    if (de->d_type == DT_DIR){ /*Directorio dentro de directorio*/
      if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0){ //para omitir los directorios "." y ".."
        path_aux = makepath(old_path, de->d_name);
        path_aux2 = makepath(new_path, de->d_name);
        if (path_aux == NULL || path_aux2 == NULL){
          err(1,"makepath");
        }
        if (newdir(path_aux, path_aux2, per_dir, per_fich) < 0){ /*Recursividad*/
          err(1, "newdir");
        }
        free (path_aux);
        free (path_aux2);
      }
    }
    if (de->d_type == DT_REG){ /*Fichero convencional*/
      path_fich = makepath(old_path, de->d_name);
      path_fich2 = makepath(new_path, de->d_name);
      if (path_fich == NULL || path_fich2 == NULL){ //path (.../x.txt, .../reverse.c)
        err(1,"makepath");
      }
      newfich(path_fich, path_fich2, per_fich);
      free (path_fich);
      free (path_fich2);
    }
  }

  if (closedir(d) < 0){
    err(1, "closedir");
  }

  status = 0;
  return status;
}



//Main--------------------------------------------------------------------------
int
main(int argc, char *argv[])
{
  char *path;
  char *path2;
  char *per;
  char *per2;
  long per_dir, per_fich;
  int fd;
  DIR *d;
  DIR *d2;
  int flag = 0; //flag=0 (dir op), flag=1 (fich op)

  argc--;
  if (argc != 4){
    errx(1, "Usage: dir_permission(4 digits)  fich_permission(4 digits)  old_path/fich  new_path/fich");
  }

  //Usage testing
  per = strdup(argv[1]);
  per2 = strdup(argv[2]);
  path = strdup(argv[3]);
  path2 = strdup(argv[4]);
  if(strlen(per) != 4 || strlen(per2) != 4){
    errx(1, "Usage: dir_permission(4 digits)  fich_permission(4 digits)  old_path/fich  new_path/fich");
  }
  per_dir = strtol(per, NULL, 0); //0 base (octal is taken because the first 0)
  per_fich = strtol(per2, NULL, 0);


  //If d -> NULL, maybe its a fich. Else its a directory
  d = opendir(path);
  if (d == NULL){
    //Operation with files in current directory?
    fd = open (path, O_RDONLY);
    if(fd < 0){
      err(1, "open %s", path); //That path/file no exists
    }
    if (close(fd) < 0){
      err(1, "close");
    }else{
      flag = flag + 1; //Operation with fich
    }
  }


  //Destiny path already exists?
  d2 = opendir(path2);
  if (d2 != NULL){
    closedir(d2);
    errx(1, "Destiny path already exists. Please, use another path..."); //CAMBIO DE PERMISOS??
  }else{ //NULL
    //Operation with files in current directory?
    fd = open (path2, O_RDONLY);
    if(fd > 0){ //OK. Else, file doesn't exist or maybe its a path
      if(close(fd) < 0){
        err(1,"close %s", path2);
      }
      errx(1, "Destiny path file already exists. Please, use another path file..."); //CAMBIO DE PERMISOS??
    }
  }


  //Copy of the old fich/dir---------
  if (flag == 0){ //Dir1 + dir2 --> OK
    closedir(d);
    if (newdir(path, path2, per_dir, per_fich) < 0){ //DIRECTORIES
      err(1, "newdir");
    }
  }else if (flag == 1){ //Fich1 + fich2 --> OK
    if (newfich(path, path2, per_fich) < 0){ //FICHS
      err(1, "newfich");
    }
  }

  free(per);
  free(per2);
  free(path);
  free(path2);
  exit(0);
}
