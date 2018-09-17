#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* convert (char *in, int length){
  int i;
  char c;
  for (i=0 ; i <= (length/2)-1 ; i++){
    c = in[i];
    in[i] = in[length-i-1];
    in[length-i-1] = c;
  }
  return in;
}

int
main(int argc, char *argv[])
{
  if (argc < 2){
    fprintf(stderr, "Need arguments to reverse!\n");
    exit(1);
  }else{
    int length;
    int i;
    for (i=1 ; i < argc ; i++){
      length = strlen(argv[i]);
      fprintf(stdout, "%s\n", convert(argv[i], length));
    }
    exit(0);
  }
}
