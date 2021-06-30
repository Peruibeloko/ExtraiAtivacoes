#include "EscreveArquivo.h"
#include "CommsHandler.h"

#include <stdio.h>

int main(void)
{
  system("chcp 65001>NUL");
  printf("Qual o número da porta serial conectada? ");
  int comPort;
  scanf("%d", &comPort);
  printf("\n");

  HANDLE hComm = abrePortaSerial(comPort);
  if (!hComm)
  {
    return 1;
  }

  EscreveArquivo(hComm);
  fechaPortaSerial(hComm);

  return 0;
}