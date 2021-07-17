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

  unsigned char *qtdEventosString = malloc(4);
  printf("Solicitando nº de ativações\n\n");
  enviaComando(comandoSerial, hComm, qtdEventosString, 4);

  int qtdEventosByteAlto = qtdEventosString[1] << 8;
  int qtdEventosByteBaixo = qtdEventosString[2];
  int qtdEventos = qtdEventosByteAlto | qtdEventosByteBaixo;

  printf("Dados recebidos: %02X %02X %02X %02X \n\n",
         qtdEventosString[0],
         qtdEventosString[1],
         qtdEventosString[2],
         qtdEventosString[3]);

  unsigned char *bufferAtivacoes[qtdEventos];
  comandoSerial[1] = READ_FLAG;

  for (int i = 0; i < qtdEventos; i++)
  {
    printf("Baixando evento %d de %d...\r", i, qtdEventos);
    bufferAtivacoes[i] = malloc(LOG_ENTRY_SIZE);
    enviaComando(comandoSerial, hComm, bufferAtivacoes[i], LOG_ENTRY_SIZE);
  }

  FILE *ponteiroArquivo, *ponteiroArquivoRaw;
  char *rawFilePath = calloc(128, 1);
  char *txtFilePath = calloc(128, 1);
  const char *userPath = getenv("USERPROFILE");

  sprintf(rawFilePath, "%s\\Desktop\\LogAtivacoesRaw.dat", userPath);
  sprintf(txtFilePath, "%s\\Desktop\\LogAtivacoes.csv", userPath);

  printf("\nGerando dump binário...\n\n");
  ponteiroArquivoRaw = fopen(rawFilePath, "w");
  for (int i = 0; i < qtdEventos; i++)
  {
    for (int k = 0; k < LOG_ENTRY_SIZE; k++)
    {
      fputc(bufferAtivacoes[i][k], ponteiroArquivoRaw);
    }
  }
  fclose(ponteiroArquivoRaw);
  printf("Caminho do arquivo binário - %s\n\n", rawFilePath);
  free(rawFilePath);

  printf("Gerando planilha de ativações...\n\n");
  ponteiroArquivo = fopen(txtFilePath, "w");
  char *row;
  for (int i = 0; i < qtdEventos; i++)
  {
    row = calloc(MAX_ROW_SIZE, 1);
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
      strcat(row, "Deteccao");
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

    // Transform Hora, Minuto
    char *tmp = calloc(6, 1);
    sprintf(tmp, "%02dh%02d;", (int)hora, (int)minuto);

    strcat(row, tmp);

    // Transform Dia, Mês, Ano
    free(tmp);
    tmp = calloc(11, 1);
    int anoByteAlto = bufferAtivacoes[i][7] << 8;
    int anoByteBaixo = bufferAtivacoes[i][8];
    int ano = anoByteAlto | anoByteBaixo;

    sprintf(tmp, "%02d/%02d/%d\n", (int)dia, (int)mes, ano);

    strcat(row, tmp);
    fputs(row, ponteiroArquivo);
    free(row);
  }
  printf("\nCaminho da planilha - %s\n\n", txtFilePath);

  fclose(ponteiroArquivo);
  fechaPortaSerial(hComm);
  free(txtFilePath);
  free(bufferAtivacoes);

  printf("Processo finalizado! É seguro desconectar a placa agora");
}