#include <windows.h>

HANDLE abrePortaSerial(int comPort);
void fechaPortaSerial(HANDLE);
char *enviaComando(char *comando, HANDLE hComm, char *buffer, int bufferSize);