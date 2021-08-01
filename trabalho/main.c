/**
 * UFSC - DAS410037 Técnicas de implementação de Sistemas Automatizados
 * Trabalho 02 - Trabalho do controlador de sistemas contínuos concorrente
 * Alunos: 
 * 	- Kelvin César de Andarade
 *	- Lucas Buzetto Tsuchiya 
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "./libs/udp_client.h"
#include "./libs/pi.h"

// Threads do sistema
pthread_t thread_control_temp;
pthread_t thread_control_h;



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
#define T_KI		(2)
#define T_KP		(10)
#define Q_MAX		(999999.9)	// Valor máximo da variável Q (testado na caldeira)
#define Q_MIN		(0)			// Valor mínimo da variável Q
#define T_PERIOD	(0.050)		// Tempo do controlador de temperatura em segundos (50 ms)

#define H_KI		(0)
#define H_KP		(2)
#define H_MAX		(3)
#define H_MIN		(0.1)

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
float t_ref = -1.0f;
float h_ref = -1.0f;

#define NSEC_PER_SEC    (1000000000) /* The number of nsecs per sec. */

void temp_control (struct sockaddr_in *address){
	// Variáveis da comunicação
	int socket = cria_socket_local();
	int udp_hanndler = 0;

	// Variáveis do PI
	Controlador_PI PI_Temp;
	PI_Init(&PI_Temp, T_KP, T_KI, Q_MAX, Q_MIN, T_PERIOD);

	// Variáveis de controle
	float new_q = 0;	//Armazena novo valor de Q para a caldeira;

	// Variáveis de tempo
	struct timespec t;
	struct timespec t_exec;				// Tempo ao final da execução;
	int ns_period = T_PERIOD * 1.0E9;	// Converte o periodo de segundo para nano segundo
	int exec_ns;
	clock_gettime(CLOCK_MONOTONIC, &t);	// Leitura do horário atual
	t.tv_sec++;							// Inicia o controle após um segundo

	while(1){
		// Aguarda até a próxima execução
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);

		if (t_ref != -1.0f){
			// Leitura da temperatura atual da caldeira
			udp_hanndler = udp_read_data(socket, *address, GET_T, &caldeira.T);
			if (udp_hanndler == UDP_SUCCESS){
				// Atualiza o controlador PI
				new_q = PI_Update(&PI_Temp, t_ref, caldeira.T);

				// Envia novo valor de Q para a caldeira;
				udp_hanndler = udp_write_data(socket, *address, SET_Q, new_q);

				printf("temp_control - After reading T = %f, sent new Q = %f\n", caldeira.T, new_q);
			}
		}
		
		// Computa o tempo utilizado para execução
		clock_gettime(CLOCK_MONOTONIC, &t_exec);	// Leitura do horário atual
		exec_ns = (t_exec.tv_sec - t.tv_sec) * 1.0E9;
		exec_ns += abs(t_exec.tv_nsec - t.tv_nsec);

		// Calcula a próxima execução removendo o tempo utilizado na execuçao do controle.
		t.tv_nsec += (ns_period - exec_ns);
		// Correção das variáveis de segundo e nano segundo
		while (t.tv_nsec >= NSEC_PER_SEC) {
				t.tv_nsec -= NSEC_PER_SEC;
				t.tv_sec++;
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
	// Gera struct com endereço host:port
	int porta_destino = atoi(argv[2]);
	struct sockaddr_in endereco_destino = cria_endereco_destino(argv[1], porta_destino);

	t_ref = 25;
	// Criação das threads de controle;
	pthread_create(&thread_control_temp, NULL, (void *) temp_control, (void *) &endereco_destino);

	pthread_join(thread_control_temp, NULL);

	/* Testes de comunicação:
	// Criação do socket
	int socket = cria_socket_local();
	int udp_hanndler = 0;
	
	udp_hanndler = udp_write_data(socket, endereco_destino, SET_Q, 1200);
	printf("Mensagem enviada. Resultado: %d\n", udp_hanndler);


	udp_hanndler = udp_read_data(socket, endereco_destino, GET_T, &caldeira.T);
	printf("\nMensagem lida. Resultado (%d) = T_value: %f \n", udp_hanndler, caldeira.T);
	*/
}
