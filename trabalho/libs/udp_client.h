// Inclusion guard, to prevent multiple includes of the same header
#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>


#define FALHA -1

#define UDP_SUCCESS (1)
#define UDP_ERROR   (0)

// # Lista de comandos para comunicação com a caldeira
#define GET_Ta 	(1)     // Requisita valor da temperatura ambiente
#define GET_T	(2)     // Requisita valor da temperatura interna
#define GET_Ti	(3)     // Requisita valor da temperatura da água de entrada
#define GET_No	(4)     // Requisita fluxo de saída de água
#define GET_H	(5)     // Requisita nível do tanque
#define SET_Ni	(6)     // Define fluxo de entrada de água
#define SET_Q	(7)     // Define fluxo de calor no aquecedor  
#define SET_Na 	(8)     // Define fluxo de água quente
#define SET_Nf 	(9)     // Define fluxo de água para o esgoto;





// # Definição das funções do arquivo

int cria_socket_local(void);
void cria_endereco_destino(char *destino, int porta_destino);
void envia_mensagem(int socket_local, char *mensagem);
int recebe_mensagem(int socket_local, char *buffer, int TAM_BUFFER);

/** Retorna tamanho do comando enviado para a caldeira.
 * @param Number valor que será enviado;
 * @param cmd  comando enviado;
 * @returns Tamanho em bytes do comando.
 */
int command_size (float value, char *cmd);

/** Realiza a leitura de algum parâmetro da caldeira.
 * @param socket socket utilizado para comunicação;
 * @param cmd_index index do comando que será enviado (1 até 5)
 * @param leitura ponteiro onde a leitura realizada deve ser retornada;
 * @returns Retorna 1 quando a leitura for realizada com sucesso.
 */
int udp_read_data (int socket, uint cmd_index, float *leitura);

/** Realiza a escrita de algum parâmetro da caldeira.
 * @param socket socket utilizado para comunicação;
 * @param cmd_index index do comando que será enviado (6 até 9);
 * @param value valor que será escrito;
 * @returns Retorna 1 quando a leitura for realizada com sucesso.
 */
int udp_write_data (int socket, uint cmd_index, float value);

// End of the inclusion guard
#endif