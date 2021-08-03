/**
 * UFSC - DAS410037 Técnicas de implementação de Sistemas Automatizados
 * Trabalho 02 - Trabalho do controlador de sistemas contínuos concorrente
 * Alunos: 
 * 	- Kelvin César de Andarade
 *	- Lucas Buzetto Tsuchiya 
 */

#include "main.h"

// Threads do sistema
pthread_t thread_update_reads;
pthread_t thread_refresh_screen;
pthread_t thread_read_new_references;
pthread_t thread_control_temp;
pthread_t thread_control_h;


pthread_mutex_t mutex_tela = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_buffer = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_buffer_cheio = PTHREAD_COND_INITIALIZER;

// Armazena dos dados da caldeira
Caldeira caldeira;

BufferDuplo buffer_temp = {.selected_buffer = 0, .index = 0, .gravar = -1};


// Referências;
float t_ref = -1.0f;
float h_ref = -1.0f;

// Função usada para inserir um novo valor no buffer
void buffer_insert(float value, BufferDuplo *buffer){
	pthread_mutex_lock(&mutex_buffer);
	if(buffer->selected_buffer == 0){
		buffer->buffer0[buffer->index] = value;
	}else{
		buffer->buffer1[buffer->index] = value;
	}
	buffer->index++;
	if(buffer->index >= BUFFER_SIZE){
		buffer->gravar = buffer->selected_buffer;
		buffer->selected_buffer = (buffer->selected_buffer + 1) % 2;
		buffer->index = 0;
		pthread_cond_signal(&cond_buffer_cheio);
	}
	pthread_mutex_unlock(&mutex_buffer);
}

// * Função utilizada para converter um valor de string para float
void string_to_float (StringToFloat *number, char *str) {
	int len;
	int ret = sscanf(str, "%f %n", &(number->value), &len);
	number->is_valid = (ret && len==strlen(str));
}

// Thread para exibir os dados na tela
void refresh_screen (){
	int falha_temperatura = 0;
	sleep(2);
	while (1){
		// Verifica se a temperatura ultrapassou o nível de alarme
		if (caldeira.T > TEMP_FAULT && falha_temperatura == 0){
			falha_temperatura = 1;
		} else if(falha_temperatura == 1 && caldeira.T <= TEMP_FAULT){
			falha_temperatura = 0;
		}

		// Exibe os dados no terminal
		pthread_mutex_lock(&mutex_tela);
		system("clear");

		if (falha_temperatura == 1){
			printf(COLOR_RED "-----------------------------------------------------------------------\n");
			printf(COLOR_RED "* [ALARME] A temperatura está acima do valor recomendado de %d °C\n", TEMP_FAULT);
			printf(COLOR_RED "-----------------------------------------------------------------------\n");
		}
		printf(COLOR_CYAN "\nParâmetros de controle:\n");	
		printf(COLOR_WHITE"* Setpoint de temperatura: %.2f °C\n", t_ref);	
		printf(COLOR_WHITE"* Setpoint de nível: %.2f m\n", h_ref);	
		printf(COLOR_CYAN "\nInformação dos sensores lidos da caldeira:\n");
		printf(COLOR_WHITE"* Temperatura da água no interior (T): %f °C\n", caldeira.T);
		printf(COLOR_WHITE"* Nível da caldeira (H): %f m\n\n", caldeira.H);
		printf(COLOR_WHITE"* Temperatura da água de entrada  (Ti): %f °C\n", caldeira.Ti);
		printf(COLOR_WHITE"* Temperatura exterior da caldeira (Ta): %f °C\n\n", caldeira.Ta);

		printf(COLOR_WHITE"* Fluxo de calor inserido pela água de entrada (Qi): %f J/s\n", caldeira.Qi);	
		printf(COLOR_WHITE"* Fluxo de calor inserido pela água quente  (Qa): %f J/s\n", caldeira.Qa);	
		printf(COLOR_WHITE"* Fluxo de calor através do isolante (Qe): %f J/s\n", caldeira.Qe);	
		printf(COLOR_WHITE"* Fluxo de calor do aquecedor (Q): %f J/s\n", caldeira.Q);	
		printf(COLOR_WHITE"* Fluxo de calor total: %f J/s\n\n", caldeira.Qt);	

		printf(COLOR_WHITE"* Fluxo de entrada de água da caldeira (Ni): %f kg/s\n", caldeira.Ni);
		printf(COLOR_WHITE"* Fluxo de entrada de água aquecida 80°C (Na): %f kg/s\n", caldeira.Na);
		printf(COLOR_WHITE"* Fluxo de saída de água da caldeira (No): %f kg/s\n", caldeira.No);
		printf(COLOR_WHITE"* Fluxo de saída de água para esgoto (Nf): %f kg/s\n", caldeira.Nf);

		printf(COLOR_CYAN "\nComandos disponíveis: \n");
		printf(TO_BLUE("[ENTER]") COLOR_WHITE " - Para definir novos valores de setpoint\n");	
		pthread_mutex_unlock(&mutex_tela);
		sleep(1);
	}
}


// Thread para leitura das novas referências digitadas pelo usuário.
void read_new_references (){

	char teclado[100];			 // Armazena o que o usuário digita
	char *x; 					 // Recebe o retorno do fgets
	StringToFloat new_reference; // Usado para validar e converter a string para float

	while (1){
		x = fgets(teclado, 100, stdin);
		// Se apertou alguma coisa
		if(x != NULL){

			pthread_mutex_lock(&mutex_tela);	
			system("clear");
			printf(COLOR_YELLOW "Definindo uma novo setpoint para as variáveis de controle:\n");
			printf(COLOR_WHITE "* Referências atuais: \n");
			printf("\t- Temperatura: %.2f °C\n", t_ref);
			printf("\t- Nível da caldeira: %.2f m\n\n", h_ref);
			
			printf("* Digite um novo valor para referência de temperatura (°C):");
			fgets(teclado, 100, stdin);
			string_to_float(&new_reference, teclado);
			// Validação do valor digitado
			if(new_reference.is_valid == 1){
				t_ref = new_reference.value;
			} else {
				printf(TO_RED("[ERROR]") " - Valor digitado não é válido! \n");
			}

			printf("* Digite um novo valor para referência de nível (m):");
			fgets(teclado, 100, stdin);
			string_to_float(&new_reference, teclado);
			// Validação do valor digitado
			if(new_reference.is_valid == 1){
				if(new_reference.value <= H_MAX && new_reference.value >= H_MIN) {
					h_ref = new_reference.value;
				} else {
					printf(TO_RED("[ERROR]") " - Valor digitado está fora dos limites permitidos! (Máx: %d Mín: %d) \n", H_MAX, H_MIN);
				}
			} else {
				printf(TO_RED("[ERROR]") " - Valor digitado não é válido! \n");
			}

			printf(COLOR_WHITE "Novas referências definidas: \n");
			printf("\t- Temperatura: %f °C\n", t_ref);
			printf("\t- Nível da caldeira: %f m\n\n", h_ref);
			sleep(5);
			pthread_mutex_unlock(&mutex_tela);
		}
	}

}

// Thread para leitura dos sensores da caldeira
void leitura_sensores_caldeira (){
	// Variáveis da comunicação
	int socket = cria_socket_local();

	// Variáveis de tempo
	struct timespec t;					// Armazena horário
	struct timespec t_exec;				// Tempo ao final da execução;
	int ns_period = 0.010 * 1.0E9;		// Converte o periodo de segundo para nano segundo (10 ms)
	int exec_ns;
	clock_gettime(CLOCK_MONOTONIC, &t);	// Leitura do horário atual
	t.tv_sec++;							// Inicia o controle após um segundo
	printf(COLOR_GREEN "* Thread de amostragem da caldeira inicializada\n");

	while(1){
		// Aguarda até a próxima execução
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
		// Requisita a leitura dos sensores
		udp_read_data(socket, GET_T,  &caldeira.T);
		udp_read_data(socket, GET_H,  &caldeira.H);
		udp_read_data(socket, GET_Ta, &caldeira.Ta);
		udp_read_data(socket, GET_Ti, &caldeira.Ti);
		udp_read_data(socket, GET_No, &caldeira.No);

		// Cálculo dos valores de fluxo de calor
		caldeira.Qa = caldeira.Na * S_AGUA * (80 - caldeira.T);
		caldeira.Qi = caldeira.Ni * S_AGUA * (caldeira.Ti - caldeira.T);
		caldeira.Qe = -(caldeira.T - caldeira.Ta) / R_ISOLAMENTO;
		caldeira.Qt = caldeira.Qa + caldeira.Qi + caldeira.Qe + caldeira.Q;

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

// Thread para controle da temperatura da caldeira
void temp_control (){
	// Variáveis da comunicação
	int socket = cria_socket_local();
	int udp_hanndler = 0;

	// Intancia as variáveis do controlador PI
	Controlador_PI PI_Temp;
	PI_Init(&PI_Temp, T_KP, T_KI, Q_MAX, Q_MIN, T_PERIOD);

	// Variáveis de controle
	float new_q = 0;	//Armazena novo valor de Q para a caldeira;

	// Variáveis de tempo
	struct timespec t;					// Armazena horário de execução.
	struct timespec t_exec;				// Tempo ao final da execução;

	#ifdef DEBUG_TIME_TEMP
		struct timespec t_old;			// Usado para debug
	#endif

	int ns_period = T_PERIOD * 1.0E9;	// Converte o periodo de segundo para nano segundo
	int exec_ns;						// Tempo de execução em nano segundos;
	clock_gettime(CLOCK_MONOTONIC, &t);	// Leitura do horário atual
	t.tv_sec += 2;						// Inicia o controle após dois segundos
	printf(COLOR_GREEN "* Thread de controle da temperatura inicializada\n");
	while(1){
		// Aguarda até a próxima execução
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);

		// Se houver referência de temperatura
		if (t_ref != -1.0f){
			// Atualiza o controlador PI
			new_q = PI_Update(&PI_Temp, t_ref, caldeira.T);
			// Envia novo valor de Q para a caldeira;
			udp_hanndler = udp_write_data(socket, SET_Q, new_q);

			caldeira.Q = new_q;
		}
		
		// Computa o tempo utilizado para execução
		clock_gettime(CLOCK_MONOTONIC, &t_exec);	// Leitura do horário atual
		exec_ns = (t_exec.tv_sec - t.tv_sec) * 1.0E9;
		exec_ns += abs(t_exec.tv_nsec - t.tv_nsec);
		
		#ifdef DEBUG_TIME_TEMP
			t_old = t;
		#endif

		// Calcula a próxima execução removendo o tempo utilizado na execuçao do controle.
		t.tv_nsec += (ns_period - exec_ns);
		// Correção das variáveis de segundo e nano segundo
		while (t.tv_nsec >= NSEC_PER_SEC) {
			t.tv_nsec -= NSEC_PER_SEC;
			t.tv_sec++;
		}

		#ifdef DEBUG_TIME_TEMP 
			printf("temp_control - Executou em %f ms. Periodo: %f\n", exec_ns / 1E6, (t.tv_sec-t_old.tv_sec)*1000 + (t.tv_nsec-t_old.tv_nsec) / 1E6);
		#endif
	}

}

// Função para incrementar o nível da caldeira
void incrementa_nivel (int socket, float vazao){
	// Define vazão para a válvula de entrada
	udp_write_data(socket, SET_Ni, vazao);
	udp_write_data(socket, SET_Nf, 0);
	caldeira.Ni = vazao;
	caldeira.Nf = 0;
}
// Função para decrementar o nível da caldeira
void decrementa_nivel (int socket, float vazao){
	// Define vazão para a válvula de saída
	udp_write_data(socket, SET_Ni, 0);
	udp_write_data(socket, SET_Nf, vazao);
	caldeira.Ni = 0;
	caldeira.Nf = vazao;
}

// Thread para controle de nível da caldeira
void nivel_control (){
	// Variáveis da comunicação
	int socket = cria_socket_local();
	int udp_hanndler = 0;

	// Variáveis do PI
	Controlador_PI PI_Nivel;
	PI_Init(&PI_Nivel, H_KP, H_KI, VAZAO_MAX, VAZAO_MIN, Q_PERIOD);

	// Variáveis de controle
	float new_vazao = 0;				//Armazena novo valor de vazao

	// Variáveis de tempo
	struct timespec t;					// Tempo da próxima execução.
	struct timespec t_exec;				// Tempo ao final da execução;

	#ifdef DEBUG_TIME_NIVEL
		struct timespec t_old;			// Usado para debug
	#endif

	int ns_period = Q_PERIOD * 1.0E9;	// Converte o periodo de segundo para nano segundo
	int exec_ns;						// Armazena os nanosegundos de execução do código

	clock_gettime(CLOCK_MONOTONIC, &t);	// Leitura do horário atual
	t.tv_sec += 2;						// Inicia o controle após dois segundos

	printf(COLOR_GREEN "* Thread de controle do nível inicializada\n");
	while(1){
		// Aguarda até a próxima execução
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);

		if (h_ref != -1.0f){

			// Atualiza o controlador PI
			new_vazao = PI_Update(&PI_Nivel, h_ref, caldeira.H);

			// Com base na saída, define o atuador
			if(new_vazao > 0){
				new_vazao = fabs(new_vazao);
				incrementa_nivel(socket, new_vazao);

			} else if(new_vazao < 0){
				new_vazao = fabs(new_vazao);
				decrementa_nivel(socket, new_vazao);
			} else {
				// Não faz nada com o nível do tanque...
				udp_hanndler = udp_write_data(socket, SET_Ni, 0);
				udp_hanndler = udp_write_data(socket, SET_Nf, 0);
				caldeira.Ni = 0;
				caldeira.Nf = 0;
			}
		}
		
		// Computa o tempo utilizado para execução
		clock_gettime(CLOCK_MONOTONIC, &t_exec);	// Leitura do horário atual
		exec_ns = (t_exec.tv_sec - t.tv_sec) * 1.0E9;
		exec_ns += abs(t_exec.tv_nsec - t.tv_nsec);


		#ifdef DEBUG_TIME_NIVEL
			t_old = t;			// Utilizado para depuração do periodo da thread
		#endif

		// Calcula a próxima execução removendo o tempo utilizado na execuçao do controle.
		t.tv_nsec += (ns_period - exec_ns);
		// Correção das variáveis de segundo e nano segundo
		while (t.tv_nsec >= NSEC_PER_SEC) {
				t.tv_nsec -= NSEC_PER_SEC;
				t.tv_sec++;
		}
	
		#ifdef DEBUG_TIME_NIVEL
			printf("nivel_control - Executou em %f ms. Periodo: %f\n", exec_ns / 1E6, (t.tv_sec-t_old.tv_sec)*1000 + (t.tv_nsec-t_old.tv_nsec) / 1E6);
		#endif
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

	// Atualiza endereço do host
	cria_endereco_destino(argv[1], porta_destino);

	t_ref = 25;
	h_ref = 1.25;
	caldeira.Q = -1;
	caldeira.T = -1;
	// Criação das threads de controle;
	pthread_create(&thread_update_reads, NULL, (void *) leitura_sensores_caldeira, NULL);
	pthread_create(&thread_refresh_screen, NULL, (void *) refresh_screen, NULL);
	pthread_create(&thread_read_new_references, NULL, (void *) read_new_references, NULL);

	pthread_create(&thread_control_temp, NULL, (void *) temp_control, NULL);
	pthread_create(&thread_control_h,    NULL, (void *) nivel_control, NULL);
	
	pthread_join(thread_update_reads, NULL);
	pthread_join(thread_refresh_screen, NULL);
	pthread_join(thread_read_new_references, NULL);
	pthread_join(thread_control_temp, NULL);
	pthread_join(thread_control_h, NULL);


	/* Testes de comunicação:
	// Criação do socket
	int socket = cria_socket_local();
	int udp_hanndler = 0;
	
	udp_hanndler = udp_write_data(socket, SET_Q, 1200);
	printf("Mensagem enviada. Resultado: %d\n", udp_hanndler);


	udp_hanndler = udp_read_data(socket, GET_T, &caldeira.T);
	printf("\nMensagem lida. Resultado (%d) = T_value: %f \n", udp_hanndler, caldeira.T);
	*/
}
