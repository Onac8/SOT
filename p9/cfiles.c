#include <stdlib.h>
#include <err.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/dir.h>
#include <dirent.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>

pthread_mutex_t lock;
char *keyword;
char *cnt_path;


//Check if cfiles.cnt exists----------------------------------------------------
int cnt_exist(const char *path){
  struct stat path_stat;
  stat(path, &path_stat);
  return S_ISREG(path_stat.st_mode); //non-zero if the file is reg_file
}

//INCR_COUNTER------------------------------------------------------------------
void incr_counter(){
  char buf[100];
  int fd, nr, counter;

  fd = open(cnt_path, O_RDWR);
  if (fd < 0){
    err(1, "open");
  }


  //Inicio region critica
  if(flock(fd, LOCK_EX) != 0){
    err(1, "lock");
  }
  nr = read(fd, buf, sizeof buf);
  if (nr < 0) {
    close(fd);
    err(1, "read %s", cnt_path);
  }
  buf[nr] = 0;
  counter = atoi(buf);
  counter++;
  snprintf(buf, sizeof buf, "%d", counter); /*int en un string*/
  lseek(fd, 0, 0); //fijamos offset a 0 para reescribir
  if (write(fd, buf, strlen(buf)) < 0) {
    close(fd);
    err(1, "write");
  }
  if(flock(fd, LOCK_UN) != 0){
    err(1, "lock");
  }
  //Fin region critica


  if (close(fd) < 0){
    err(1, "close");
  }
}


//LINE_SEARCH-------------------------------------------------------------------
//Devuelve 1 si encuentra la palabra en la linea, 0 si no la encuentra
int line_search(char *line, char *word){
  char *token;

  token = strtok (line," ,.-\n");
  while (token != NULL){
    if (strcmp(token, word) == 0){
      return 1;
    }
    token = strtok (NULL, " ,.-\n");
  }
  return 0;
}


//THREAD------------------------------------------------------------------------
static void* tmain(void *a){
  int fd, success;
  char *arg_p;
  char line[256];
  FILE* file;

  arg_p = a;

  //Open--------------------------
  file = fopen(arg_p, "r");
  if (file == NULL){
    err(1, "fopen");
  }

  fd = fileno(file); //*FILE to fd
  if (fd < 0){
    err(1, "fileno");
  }
  //------------------------------

  while(fgets(line, sizeof(line), file) != NULL){ // cuidao, nos quedamos con \n tambien
    pthread_mutex_lock(&lock);
    success = line_search(line, keyword);
    pthread_mutex_unlock(&lock);

    if(success){ //palabra coincidente
      printf("Keyword [%s] found in %s -> increasing cfiles.cnt\n", keyword, arg_p);
      incr_counter();//sumamos a contador en fichero
      break;
    }
  }

  if (fclose(file) != 0){
    err(1, "fclose");
  }
  return NULL;
}


//MAIN--------------------------------------------------------------------------
int
main (int argc, char *argv[]){
  char cntfile_path[100];
  char *home_dir;
  struct passwd *user;
  pthread_t thr[3];
  void *sts[3];
  int i, fd;

  keyword = argv[1]; //Puntero a palabra a buscar
  argc--;
  // Usage testing
  if (argc < 2){
    errx(1, "Usage: {keyword}  {file_path_1}  {file_path_2}  {...}");
  }

  //Creating home path
  user = getpwuid(getuid());
  home_dir = user-> pw_dir;
	if (user == NULL) {
	   err(1, "getpwuid");
	}
  strcpy(cntfile_path, home_dir);
  strcat(cntfile_path, "/cfiles.cnt");
  cnt_path = cntfile_path; //Variable global


  //cfiles.cnt exists?------------------------------
  if (cnt_exist(cnt_path) == 0) { // = 0 if cfiles.cnt doesn't exists
    fd = creat(cnt_path, 0644); //creating and open cfiles.cnt
    if (fd < 0){
      err(1, "open");
    }
    if (write (fd, "0\0", 1) < 0){
      err(1, "write");
    }
    if (close(fd) < 0){
      err(1, "close");
    }
  }
  //------------------------------------------------


  if(pthread_mutex_init(&lock, NULL) != 0) {
    err(1, "mutex_init");
  }
  for(i = 0; i < argc-1; i++){ //tantos threads como paths haya
    if(pthread_create(thr+i, NULL, tmain, argv[i+2]) != 0) {
      err(1, "thread");
    }
  }

  //Waiting threads ends
  for(i = 0 ; i < argc-1 ; i++) {
    pthread_join(thr[i], sts+i);
    free(sts[i]);
  }
  if(pthread_mutex_destroy(&lock) != 0){
    err(1, "mutex_destroy");
  }
  exit(0);
}
