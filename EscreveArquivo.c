#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CommsHandler.h"

#define BLOCK_START 0xAE
#define BLOCK_END 0xAF
#define LOG_ENTRY_SIZE 11
#define MAX_ROW_SIZE 64

void EscreveArquivo(HANDLE hComm)
{
  char comandoSerial[4] = {BLOCK_START, 'n', 'l', BLOCK_END};

  char *qtdEventosString = malloc(4);
  printf("Solicitando nº de ativações\n\n");
  enviaComando(comandoSerial, hComm, qtdEventosString, 4);

  int qtdEventosMSB = qtdEventosString[1] << 8;
  int qtdEventosLSB = qtdEventosString[2];
  int qtdEventos = qtdEventosMSB + qtdEventosLSB;

  char *bufferAtivacoes[qtdEventos];
  comandoSerial[1] = 'r';

  for (int i = 0; i < qtdEventos; i++)
  {
    printf("Baixando evento %d de %d...\r", i, qtdEventos);
    bufferAtivacoes[i] = malloc(LOG_ENTRY_SIZE);
    enviaComando(comandoSerial, hComm, bufferAtivacoes[i], LOG_ENTRY_SIZE);
  }

  printf("\n\nEventos armazenados\n\n");

  FILE *ponteiroArquivo, *ponteiroArquivoRaw;
  char *rawFilePath = malloc(128);
  char *txtFilePath = malloc(128);
  const char *userPath = getenv("USERPROFILE");

  sprintf(rawFilePath, "%s\\Desktop\\LogAtivacoesRaw.dat", userPath);
  sprintf(txtFilePath, "%s\\Desktop\\LogAtivacoes.csv", userPath);

  printf("Gerando dump binário...\n\n");
  ponteiroArquivoRaw = fopen(rawFilePath, "wb");
  fwrite(bufferAtivacoes, 1, sizeof(bufferAtivacoes), ponteiroArquivoRaw);
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

    memset(row, 0, MAX_ROW_SIZE);

    // Transform Evento
    if (evento == 'B')
    {
      strncat(row, "Bloqueio", MAX_ROW_SIZE);
    }
    else if (evento == 'D')
    {
      strncat(row, "Desbloqueio", MAX_ROW_SIZE);
    }

    strncat(row, ";", MAX_ROW_SIZE);

    // Transform Dispositivo
    if (dispositivo == 'P')
    {
      strncat(row, "Painel", MAX_ROW_SIZE);
    }
    else if (dispositivo == 'C')
    {
      strncat(row, "Controle", MAX_ROW_SIZE);
    }
    else if (dispositivo == 'D')
    {
      strncat(row, "Detecção", MAX_ROW_SIZE);
    }
    else if (dispositivo == 'G')
    {
      strncat(row, "Gerente", MAX_ROW_SIZE);
    }
    else if (dispositivo == 'V')
    {
      strncat(row, "Vigilante", MAX_ROW_SIZE);
    }

    strncat(row, ";", MAX_ROW_SIZE);

    // Transform Hora, Minuto
    char *tmp = "     ";
    sprintf(tmp, "%ulh%ul", (int)hora, (int)minuto);

    strncat(row, tmp, MAX_ROW_SIZE);
    strncat(row, ";", MAX_ROW_SIZE);

    // Transform Dia, Mês, Ano
    tmp = "     ";
    int anoMSB = bufferAtivacoes[i][7] << 8;
    int anoLSB = bufferAtivacoes[i][8];
    int ano = anoMSB + anoLSB;

    sprintf(tmp, "%ul/%ul/%d", (int)dia, (int)mes, ano);

    strncat(row, tmp, MAX_ROW_SIZE);
    strncat(row, "\r\n", MAX_ROW_SIZE);

    fputs(row, ponteiroArquivo);
  }
  printf("Caminho da planilha - %s\n\n", txtFilePath);

  fclose(ponteiroArquivo);
  fechaPortaSerial(hComm);
  free(row);
  free(txtFilePath);
  free(bufferAtivacoes);

  printf("Processo finalizado! É seguro desconectar a placa agora");
}
