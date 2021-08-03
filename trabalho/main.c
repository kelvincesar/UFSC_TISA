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
#include <math.h>
#include <fcntl.h>


#include "./libs/udp_client.h"
#include "./libs/pi.h"

// Threads do sistema
pthread_t thread_update_reads;
pthread_t thread_refresh_screen;
pthread_t thread_read_new_references;
pthread_t thread_control_temp;
pthread_t thread_control_h;

pthread_mutex_t mutex_tela = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_buffer = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_buffer_cheio = PTHREAD_COND_INITIALIZER;


#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_RESET   "\x1b[0m"
#define COLOR_WHITE	  "\x1b[0;37m"

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


#define TEMP_FAULT (2)		// Temperatura que gera falha
// Variáveis de controle
#define T_KI		(100)
#define T_KP		(10000)
#define Q_MAX		(999999.9)	// Valor máximo da variável Q (testado na caldeira)
#define Q_MIN		(0)			// Valor mínimo da variável Q
#define T_PERIOD	(0.050)		// Tempo do controlador de temperatura em segundos (50 ms)


#define H_KI		(1)
#define H_KP		(100)
#define H_MAX		(3)			// Altura máxima do tanque
#define H_MIN		(0.1)		// Altura mínima do tanque
#define VAZAO_MIN   (-100)		// Vazão mínima permitida da água do tanque (na saida Nf)
#define VAZAO_MAX	(100)		// Vazão máxima permitida da água do tanque (na entrada Ni)
#define Q_PERIOD 	(0.070)		// Tempo do controlador de nível em segundos (70 ms)

#define BUFFER_SIZE 100
// Data struct 
typedef struct {
	float Ta;	// temperatura do ar ambiente em volta do recipiente[°C] 
	float T;	// temperatura da água no interior do recipiente [°C] 
	float Ti;	// temperatura da água na entrada do recipiente [°C] 
	float No;	// fluxo de água de saída do recipiente [kg/s] 
	float H;	// altura da coluna de água dentro do recipiente [m] 
	float Q;	// Valor de Q [J/s ou W];
	float Ni;	// Valor da vazão de entrada [kg/s];
	float Nf;	// Valor da vazão de saída [kg/s];
} Caldeira;

// Armazenamento dos dados da caldeira
Caldeira caldeira;
//Buffer duplo
typedef struct {
	float buffer0[BUFFER_SIZE];
	float buffer1[BUFFER_SIZE];
	int emsuso;
	int index;
	int gravar;
} BufferDuplo;

BufferDuplo buffer_temp;
buffer_temp.emsuso = 0;
buffer_temp.index = 0;
buffer_temp.gravar = -1;

// Referências;
float t_ref = -1.0f;
float h_ref = -1.0f;

#define NSEC_PER_SEC    (1000000000) /* The number of nsecs per sec. */
void refresh_screen (){
	int falha_temperatura = 0;
	sleep(2);
	while (1){
		if (caldeira.T > TEMP_FAULT && falha_temperatura == 0){
			falha_temperatura = 1;
		} else if(falha_temperatura == 1 && caldeira.T <= TEMP_FAULT){
			falha_temperatura = 0;
		}
		pthread_mutex_lock(&mutex_tela);
		system("clear");

		if (falha_temperatura == 1){
			printf(COLOR_RED "-----------------------------------------------------------------------\n");
			printf(COLOR_RED "* [ALARME] A temperatura está acima do valor recomendado de %d °C\n", TEMP_FAULT);
			printf(COLOR_RED "-----------------------------------------------------------------------\n");
		}
		printf(COLOR_CYAN "\nInformação dos sensores lidos da caldeira:\n");
		printf(COLOR_WHITE"* Temperatura da água no interior (T): %f °C\n", caldeira.T);
		printf(COLOR_WHITE"* Temperatura da água de entrada  (Ti): %f °C\n", caldeira.Ti);
		printf(COLOR_WHITE"* Temperatura exterior da caldeira (Ta): %f °C\n", caldeira.Ta);
		printf(COLOR_WHITE"* Fluxo da água na saída da caldeira (No): %f kg/s\n", caldeira.No);
		printf(COLOR_WHITE"* Nível da caldeira (H): %f m\n", caldeira.H);
		pthread_mutex_unlock(&mutex_tela);
		sleep(1);
	}
}

void read_new_references (){
	char teclado[100];
	char *x; // Recebe o retorno do fgets
	// Realiza a leitura das flags de configuração do fgets;
	int flags_sync = fcntl( fileno(stdin), F_GETFL, 0);
	// Altera a flag de configuração do fgets para operar de forma assyc;
	fcntl( fileno(stdin), F_SETFL, flags_sync | O_NONBLOCK);

	while (1){
		x = fgets(teclado, 100, stdin);
		// Se apertou alguma coisa
		if(x != NULL){
			pthread_mutex_lock(&mutex_tela);	
			printf(COLOR_YELLOW " Definindo uma nova referência para os valores!!\n");
			printf("* Referências atuais: \n");
			printf("	- Temperatura: %f °C\n", t_ref);
			printf("	- Nível da caldeira: %f m\n\n", h_ref);
			fcntl( fileno(stdin), F_SETFL, flags_sync);
			printf("* Digite um novo valor para referência de temperatura (°C):");
			fgets(teclado, 100, stdin);
			t_ref = atof(teclado);
			printf("* Digite um novo valor para referência de nível (m):");
			fgets(teclado, 100, stdin);
			h_ref = atof(teclado);

			printf("\n* Novas referências definidas: \n");
			printf("	- Temperatura: %f °C\n", t_ref);
			printf("	- Nível da caldeira: %f m\n\n", h_ref);
			pthread_mutex_unlock(&mutex_tela);
		}
		sleep(0.5);
	}

}


void leitura_sensores_caldeira (struct sockaddr_in *address){
	// Variáveis da comunicação
	int socket = cria_socket_local();
	int udp_hanndler = 0;

	// Variáveis de tempo
	struct timespec t;
	struct timespec t_exec;				// Tempo ao final da execução;
	int ns_period = 0.010 * 1.0E9;		// Converte o periodo de segundo para nano segundo (10 ms)
	int exec_ns;
	clock_gettime(CLOCK_MONOTONIC, &t);	// Leitura do horário atual
	t.tv_sec++;							// Inicia o controle após um segundo
	printf(COLOR_GREEN "* Thread de amostragem da caldeira inicializada\n");

	

	while(1){
		// Aguarda até a próxima execução
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
		udp_read_data(socket, *address, GET_T,	&caldeira.T);
		udp_read_data(socket, *address, GET_H,	&caldeira.H);
		udp_read_data(socket, *address, GET_Ta, &caldeira.Ta);
		udp_read_data(socket, *address, GET_Ti, &caldeira.Ti);
		udp_read_data(socket, *address, GET_No, &caldeira.No);

		

		
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

void buffer_insert(float value, BufferDuplo *buffer){
	pthread_mutex_lock(&mutex_buffer);
	if(buffer->emsuso == 0){
		buffer->buffer0[buffer->index] = value;
	}else{
		buffer->buffer1[buffer->index] = value;
	}
	buffer->index++;
	if(buffer->index >= BUFFER_SIZE){
		buffer->gravar = buffer->emsuso;
		buffer->emsuso = (buffer->emsuso + 1) % 2;
		buffer->index = 0;
		pthread_cond_signal(&cond_buffer_cheio);
	}
	pthread_mutex_unlock(&mutex_buffer);
}



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
	t.tv_sec += 2;						// Inicia o controle após dois segundos
	printf(COLOR_GREEN "* Thread de controle da temperatura inicializada\n");
	while(1){
		// Aguarda até a próxima execução
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);

		if (t_ref != -1.0f){
			// Atualiza o controlador PI
			new_q = PI_Update(&PI_Temp, t_ref, caldeira.T);

			// Envia novo valor de Q para a caldeira;
			udp_hanndler = udp_write_data(socket, *address, SET_Q, new_q);
			//printf("temp_control - Após ler T = %f definida novo H = %f\n", caldeira.T, new_q);

			/*
			if(new_q == 0 && caldeira.Q != new_q){
				printf("Abrindo circulação de água para esfriar tanque!!!!!!!!");
				udp_hanndler = udp_write_data(socket, *address, SET_Ni, 10);
				udp_hanndler = udp_write_data(socket, *address, SET_Nf, 10);
			} else if(new_q > 0 && caldeira.Q == 0){
				udp_hanndler = udp_write_data(socket, *address, SET_Ni, 0);
				udp_hanndler = udp_write_data(socket, *address, SET_Nf, 0);	
			}
			*/
			caldeira.Q = new_q;
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

void incrementa_nivel (int socket, struct sockaddr_in address, float vazao){
	// Define vazão para a válvula de entrada
	udp_write_data(socket, address, SET_Ni, vazao);
	udp_write_data(socket, address, SET_Nf, 0);
	caldeira.Ni = vazao;
	caldeira.Nf = 0;
}
void decrementa_nivel (int socket, struct sockaddr_in address, float vazao){
	// Define vazão para a válvula de saída
	udp_write_data(socket, address, SET_Ni, 0);
	udp_write_data(socket, address, SET_Nf, vazao);
	caldeira.Ni = 0;
	caldeira.Nf = vazao;
}

void nivel_control (struct sockaddr_in *address){
	// Variáveis da comunicação
	int socket = cria_socket_local();
	int udp_hanndler = 0;

	// Variáveis do PI
	Controlador_PI PI_Nivel;
	PI_Init(&PI_Nivel, H_KP, H_KI, VAZAO_MAX, VAZAO_MIN, Q_PERIOD);

	// Variáveis de controle
	float new_vazao = 0;				//Armazena novo valor de vazao

	// Variáveis de tempo
	struct timespec t;
	struct timespec t_exec;				// Tempo ao final da execução;
	int ns_period = Q_PERIOD * 1.0E9;	// Converte o periodo de segundo para nano segundo
	int exec_ns;
	clock_gettime(CLOCK_MONOTONIC, &t);	// Leitura do horário atual
	t.tv_sec += 2;						// Inicia o controle após dois segundos
	printf(COLOR_GREEN "* Thread de controle do nível inicializada\n");
	while(1){
		// Aguarda até a próxima execução
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);

		if (h_ref != -1.0f){

			// Atualiza o controlador PI
			new_vazao = PI_Update(&PI_Nivel, h_ref, caldeira.H);

			if(new_vazao > 0){
				new_vazao = fabs(new_vazao);
				incrementa_nivel(socket, *address, new_vazao);

			} else if(new_vazao < 0){
				new_vazao = fabs(new_vazao);
				decrementa_nivel(socket, *address, new_vazao);
			} else {
				// Não faz nada com o nível do tanque...
				udp_hanndler = udp_write_data(socket, *address, SET_Ni, 0);
				udp_hanndler = udp_write_data(socket, *address, SET_Nf, 0);
				caldeira.Ni = 0;
				caldeira.Nf = 0;
			}

			//printf(COLOR_CYAN "nivel_control - Após ler H = %f definida nova vazão = %f\n", caldeira.H, new_vazao);
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
	printf(COLOR_GREEN "* Iniciando o sistema...\n");
	// Gera struct com endereço host:port
	int porta_destino = atoi(argv[2]);
	struct sockaddr_in endereco_destino = cria_endereco_destino(argv[1], porta_destino);

	t_ref = 25;
	h_ref = 1.25;
	caldeira.Q = -1;
	caldeira.T = -1;
	// Criação das threads de controle;
	pthread_create(&thread_update_reads, NULL, (void *) leitura_sensores_caldeira, (void *) &endereco_destino);
	pthread_create(&thread_refresh_screen, NULL, (void *) refresh_screen, NULL);
	pthread_create(&thread_read_new_references, NULL, (void *) read_new_references, NULL);

	pthread_create(&thread_control_temp, NULL, (void *) temp_control, (void *) &endereco_destino);
	pthread_create(&thread_control_h,    NULL, (void *) nivel_control, (void *) &endereco_destino);
	
	pthread_join(thread_update_reads, NULL);
	pthread_join(thread_refresh_screen, NULL);
	pthread_join(thread_read_new_references, NULL);
	pthread_join(thread_control_temp, NULL);
	pthread_join(thread_control_h, NULL);
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
