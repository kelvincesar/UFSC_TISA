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


// Definição das funções do arquivo:

int cria_socket_local(void);
struct sockaddr_in cria_endereco_destino(char *destino, int porta_destino);
void envia_mensagem(int socket_local, struct sockaddr_in endereco_destino, char *mensagem);
int recebe_mensagem(int socket_local, char *buffer, int TAM_BUFFER);

/** Retorna tamanho do comando enviado para a caldeira.
 * @param Number valor que será enviado;
 * @param cmd  comando enviado;
 * @returns Tamanho em bytes do comando.
 */
int command_size (float value, char *cmd);

/** Realiza a leitura de algum parâmetro da caldeira.
 * @param socket socket utilizado para comunicação;
 * @param address endereço do host;
 * @param cmd_index index do comando que será enviado (1 até 5)
 * @param leitura ponteiro onde a leitura realizada deve ser retornada;
 * @returns Retorna 1 quando a leitura for realizada com sucesso.
 */
int udp_read_data (int socket, struct sockaddr_in address, uint cmd_index, float *leitura);

/** Realiza a escrita de algum parâmetro da caldeira.
 * @param socket socket utilizado para comunicação;
 * @param address endereço do host;
 * @param cmd_index index do comando que será enviado (6 até 9);
 * @param value valor que será escrito;
 * @returns Retorna 1 quando a leitura for realizada com sucesso.
 */
int udp_write_data (int socket, struct sockaddr_in address, uint cmd_index, float value);

// End of the inclusion guard
#endif