#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CommsHandler.h"

#define BLOCK_START 0xAE
#define READ_FLAG 0x72
#define NUMBER_FLAG 0x6E
#define LOG_FLAG 0x6C
#define BLOCK_END 0xAF

#define LOG_ENTRY_SIZE 10
#define MAX_ROW_SIZE 64

void EscreveArquivo(HANDLE hComm)
{
  char comandoSerial[4] = {BLOCK_START, NUMBER_FLAG, LOG_FLAG, BLOCK_END};

  char *qtdEventosString = malloc(4);
  printf("Solicitando nº de ativações\n\n");
  enviaComando(comandoSerial, hComm, qtdEventosString, 4);

  int qtdEventosMSB = qtdEventosString[1] << 8;
  int qtdEventosLSB = qtdEventosString[2];
  int qtdEventos = qtdEventosMSB | qtdEventosLSB;

  printf("Dados recebidos: %02X %02X %02X %02X \n\n",
         qtdEventosString[0],
         qtdEventosString[1],
         qtdEventosString[2],
         qtdEventosString[3]);

  char *bufferAtivacoes[qtdEventos];
  comandoSerial[1] = READ_FLAG;

  for (int i = 0; i < qtdEventos; i++)
  {
    printf("Baixando evento %d de %d...\r", i, qtdEventos);
    bufferAtivacoes[i] = malloc(LOG_ENTRY_SIZE);
    enviaComando(comandoSerial, hComm, bufferAtivacoes[i], LOG_ENTRY_SIZE);
  }

  printf("\n\nEventos recebidos: \n\n");

  for (int i = 0; i < qtdEventos; i++)
  {
    printf("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
           bufferAtivacoes[i][0],
           bufferAtivacoes[i][1],
           bufferAtivacoes[i][2],
           bufferAtivacoes[i][3],
           bufferAtivacoes[i][4],
           bufferAtivacoes[i][5],
           bufferAtivacoes[i][6],
           bufferAtivacoes[i][7],
           bufferAtivacoes[i][8],
           bufferAtivacoes[i][9]);
  }

  FILE *ponteiroArquivo, *ponteiroArquivoRaw;
  char *rawFilePath = malloc(128);
  char *txtFilePath = malloc(128);
  const char *userPath = getenv("USERPROFILE");

  sprintf(rawFilePath, "%s\\Desktop\\LogAtivacoesRaw.dat", userPath);
  sprintf(txtFilePath, "%s\\Desktop\\LogAtivacoes.csv", userPath);

  printf("\nGerando dump binário...\n\n");
  ponteiroArquivoRaw = fopen(rawFilePath, "wb");
  for (int i = 0; i < qtdEventos; i++)
  {
    fputs(bufferAtivacoes[i], ponteiroArquivoRaw);
  }
  fclose(ponteiroArquivoRaw);
  printf("Caminho do arquivo binário - %s\n\n", rawFilePath);
  free(rawFilePath);

  printf("Gerando planilha de ativações...\n\n");
  ponteiroArquivo = fopen(txtFilePath, "w");
  char *row = calloc(1, MAX_ROW_SIZE);
  for (int i = 0; i < qtdEventos; i++)
  {
    char evento = bufferAtivacoes[i][1];
    char dispositivo = bufferAtivacoes[i][2];
    char hora = bufferAtivacoes[i][3];
    char minuto = bufferAtivacoes[i][4];
    char dia = bufferAtivacoes[i][5];
    char mes = bufferAtivacoes[i][6];

    // Transform Evento
    if (evento == 0x42)
    {
      strcat(row, "Bloqueio");
    }
    else if (evento == 0x44)
    {
      strcat(row, "Desbloqueio");
    }

    strcat(row, ";");
    printf("%s\r", row);

    // Transform Dispositivo
    if (dispositivo == 0x50)
    {
      strcat(row, "Painel");
    }
    else if (dispositivo == 0x43)
    {
      strcat(row, "Controle");
    }
    else if (dispositivo == 0x44)
    {
      strcat(row, "Detecção");
    }
    else if (dispositivo == 0x47)
    {
      strcat(row, "Gerente");
    }
    else if (dispositivo == 0x56)
    {
      strcat(row, "Vigilante");
    }

    strcat(row, ";");
    printf("%s\r", row);

    // Transform Hora, Minuto
    char *tmp = "     ";
    sprintf(tmp, "%dh%d", (int)hora, (int)minuto);

    strcat(row, tmp);
    strcat(row, ";");
    printf("%s\r", row);

    // Transform Dia, Mês, Ano
    tmp = "     ";
    int anoMSB = bufferAtivacoes[i][7] << 8;
    int anoLSB = bufferAtivacoes[i][8];
    int ano = anoMSB | anoLSB;

    sprintf(tmp, "%d/%d/%d", (int)dia, (int)mes, ano);

    strcat(row, tmp);
    strcat(row, "\r\n");
    printf("%s\r\n", row);

    fputs(row, ponteiroArquivo);
  }
  printf("\nCaminho da planilha - %s\n\n", txtFilePath);

  fclose(ponteiroArquivo);
  fechaPortaSerial(hComm);
  free(row);
  free(txtFilePath);
  free(bufferAtivacoes);

  printf("Processo finalizado! É seguro desconectar a placa agora");
}
