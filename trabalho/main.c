/**
 * UFSC - DAS410037 Técnicas de implementação de Sistemas Automatizados
 * Trabalho 02 - Trabalho do controlador de sistemas contínuos concorrente
 * Alunos: 
 * 	- Kelvin César de Andarade
 *	- Lucas Buzetto Tsuchiya 
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "./libs/udp_client.h"
#include "./libs/pi.h"

// Lista de comandos para comunicação com a caldeira
#define GET_Ta 	(1)
#define GET_T	(2)
#define GET_Ti	(3)
#define GET_No	(4)
#define GET_H	(5)
#define SET_Ni	(6)
#define SET_Q	(7)
#define SET_Na 	(8)
#define SET_Nf 	(9)

// Variáveis de controle
#define T_KI 	(0)
#define T_KP	(2)
#define Q_MAX 	(999999.9)
#define Q_MIN	(0)

#define H_KI	(0)
#define H_KP	(2)
#define H_MAX 	(3)
#define H_MIN	(0.1)

// Data struct 
typedef struct {
	float Ta;	// temperatura do ar ambiente em volta do recipiente[°C] 
	float T;	// temperatura da água no interior do recipiente [°C] 
	float Ti;	// temperatura da água que entra no recipiente [°C] 
	float No;	// fluxo de água de saída do recipiente [Kg/segundo] 
	float H;	// altura da coluna de água dentro do recipiente [m] 
} Caldeira;
// Armazenamento dos dados da caldeira
Caldeira caldeira;

// Referências;
float t_ref;
float h_ref;


void temp_control (struct sockaddr_in address){
	// Variáveis da comunicação
	// Criação do socket
	int socket = cria_socket_local();
	int udp_hanndler = 0;

	// Variáveis do PI
	struct Controlador_PI PI_Temp;
	PI_Init(PI_Temp, T_KP, T_KI, Q_MAX, Q_MIN, 0.05);


	float new_q = 0;

	if (t_ref != NULL){
		// Leitura da temperatura atual da caldeira
		udp_hanndler = udp_read_data(socket, address, GET_T, &caldeira.T);
		if (udp_hanndler == UDP_SUCCESS){
			// Atualiza o controlador PI
			new_q = PI_Update(PI_Temp, t_ref, caldeira.T);

			// Envia novo valor de Q para a caldeira;
			udp_hanndler = udp_write_data(socket, address, SET_Q, new_q);
		}
	}
}

int main (int argc, char *argv[]) {
	if (argc < 3) { 
		fprintf(stderr,"* Uso: controle_caldeira --host --port \n");
		fprintf(stderr,"* Onde:\n");
		fprintf(stderr,"	--host: endereço IP da caldeira \n");
		fprintf(stderr,"	--port: porta da caldeira \n");
		fprintf(stderr,"* Exemplo de uso:\n");
		fprintf(stderr,"   controle_caldeira 172.27.224.1 5000\n");
		exit(FALHA);
	}




	// Criação do socket
	int socket = cria_socket_local();

	// Gera struct com endereço host:port
	int porta_destino = atoi(argv[2]);
	struct sockaddr_in endereco_destino = cria_endereco_destino(argv[1], porta_destino);
	
	int udp_hanndler = 0;
	
	udp_hanndler = udp_write_data(socket, endereco_destino, SET_Q, 1200);

	printf("Mensagem enviada. Resultado: %d\n", udp_hanndler);


	udp_hanndler = udp_read_data(socket, endereco_destino, GET_T, &caldeira.T);

	printf("\nMensagem lida. Resultado (%d) = T_value: %f \n", udp_hanndler, caldeira.T);
}
