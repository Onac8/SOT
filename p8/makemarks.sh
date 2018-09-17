#!/bin/sh

#4 args maximo
if test $# -eq 4
then
  echo "#Nombre\t\tEjer1\t\tEjer2\t\tEjer3\t\tEjer4\t\tFinal" >notasfinales.txt
else
  echo usage error: $0 [file1.txt] [file2.txt] [file3.txt] [file4.txt]
  exit 1
fi
#Nombres alumnos
for i in $*
do
  awk '$0 ~ /^#/ {next} {printf ("%s\n",$1)}' $i | sort -u >>names.txt
done
names=`cat names.txt | sort -u`
rm names.txt
#Relleno notasfinales
for i in $names
do
  #Nombres
  echo -n $i >>notasfinales.txt
  for j in $*
  do
    #Esta nombre en cada ejer?
    egrep -q $i $j
    if test $? -eq 1
    then
      #Exit(1) --> Nombre no esta --> Ejercicio no entregado ("-")
      echo -n "\t\t-\t" >>notasfinales.txt
    else
      awk '$0 ~ /'$i'/ {printf ("\t\t%s\t",$2)}' $j >>notasfinales.txt
    fi
  done
  #Buscamos
  awk '$0 ~ /'$i'/{
    i += 1;
    sumNota = sumNota + $2;}
  END {
    if (i <'$#'){
      printf ("\t\tNP\n");
    }else{
      printf ("\t\t%.2f\n",sumNota/'$#');
    }
  }' $* >>notasfinales.txt
done
exit 0
