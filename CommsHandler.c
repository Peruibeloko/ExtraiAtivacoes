#include <windows.h>
#include <stdio.h>

HANDLE abrePortaSerial(int comPort)
{
  HANDLE hComm;
  DCB dcb;

  printf("Abrindo porta serial COM%d\n\n", comPort);
  char *comPath = malloc(16);
  sprintf(comPath, "\\\\.\\COM%d", comPort);

  hComm = CreateFileA(comPath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

  if (hComm == INVALID_HANDLE_VALUE)
  {
    printf("Erro ao abrir porta serial\n\n");
    return NULL;
  }

  printf("Porta aberta com sucesso\n\n");

  RtlZeroMemory(&dcb, sizeof(DCB));
  dcb.DCBlength = sizeof(DCB);

  printf("Obtendo configuração atual da porta serial\n\n");
  BOOL fSuccess = GetCommState(hComm, &dcb);
  if (!fSuccess)
  {
    //  Handle the error.
    printf("Falha ao obter configuração atual da porta serial. Erro %d.\n\n", GetLastError());
    return NULL;
  }

  printf("Configuração recuperada com sucesso\n\n");
  printf("Padronizando em 9600bps 8N1\n\n");
  dcb.BaudRate = CBR_9600;
  dcb.ByteSize = 8;
  dcb.Parity = NOPARITY;
  dcb.StopBits = ONESTOPBIT;

  SetCommState(hComm, &dcb);
  SetCommMask(hComm, EV_RXCHAR);

  printf("Tudo feito, iniciando troca de informações...\n\n");
  return hComm;
}

void fechaPortaSerial(HANDLE hComm)
{
  printf("Fechando porta serial\n\n");
  CloseHandle(hComm);
}

char *enviaComando(char *comando, HANDLE hComm, char *buffer, int bufferSize)
{
  BOOL success;
  DWORD evtFlag;

  success = WriteFile(hComm, comando, 4, NULL, NULL);

  if (!success)
  {
    printf("Falha ao enviar comando %s", comando);
    exit(1);
  }

  while (evtFlag != EV_RXCHAR)
  {
    WaitCommEvent(hComm, &evtFlag, NULL);
  }
  
  ReadFile(hComm, buffer, bufferSize, NULL, NULL);

  return buffer;
}
