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


int command_size (float value, char *cmd);
int udp_read_data (int socket, struct sockaddr_in address, uint cmd_index, float *leitura);
int udp_write_data (int socket, struct sockaddr_in address, uint cmd_index, float value);

// End of the inclusion guard
#endif