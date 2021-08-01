/**
 *  Funções utilizadas para subir um client UDP e interface com a caldeira simulada
 */

#include "udp_client.h"



/********************************************************************
 *  Funções utilizadas para interface UDP com a caldeira			*
 ********************************************************************/

int cria_socket_local(void) {
	int socket_local;		/* Socket usado na comunicacão */

	socket_local = socket( PF_INET, SOCK_DGRAM, 0);
	if (socket_local < 0) {
		perror("socket");
		return -1;
	}
	return socket_local;
}

struct sockaddr_in cria_endereco_destino(char *destino, int porta_destino){
	struct sockaddr_in servidor; 	/* Endereço do servidor incluindo ip e porta */
	struct hostent *dest_internet;	/* Endereço destino em formato próprio */
	struct in_addr dest_ip;		/* Endereço destino em formato ip numérico */

	if (inet_aton ( destino, &dest_ip ))
		dest_internet = gethostbyaddr((char *)&dest_ip, sizeof(dest_ip), AF_INET);
	else
		dest_internet = gethostbyname(destino);

	if (dest_internet == NULL) {
		fprintf(stderr,"Endereço de rede inválido\n");
		exit(FALHA);
	}

	memset((char *) &servidor, 0, sizeof(servidor));
	memcpy(&servidor.sin_addr, dest_internet->h_addr_list[0], sizeof(servidor.sin_addr));
	servidor.sin_family = AF_INET;
	servidor.sin_port = htons(porta_destino);

	return servidor;
}

void envia_mensagem(int socket_local, struct sockaddr_in endereco_destino, char *mensagem) {
	/* Envia msg ao servidor */

	if (sendto(socket_local, mensagem, strlen(mensagem)+1, 0, (struct sockaddr *) &endereco_destino, sizeof(endereco_destino)) < 0 ){ 
		perror("sendto");
		return;
	}
}


int recebe_mensagem(int socket_local, char *buffer, int TAM_BUFFER) {
	int bytes_recebidos;		/* Número de bytes recebidos */

	/* Espera pela msg de resposta do servidor */
	bytes_recebidos = recvfrom(socket_local, buffer, TAM_BUFFER, 0, NULL, 0);
	if (bytes_recebidos < 0)
	{
		perror("recvfrom");
	}

	return bytes_recebidos;
}



/********************************************************************
 *  Funções para interface de comandos com a caldeira    			*
 ********************************************************************/

/*
A caldeira possui instrumentação embutida e aceita os seguintes comandos:
	"sta0" lê valor de Ta
	"st-0" lê valor de T
	"sti0" lê valor de Ti
	"sno0" lê valor de No
	"sh-0" lê valor de H
	"ani123.4" define valor de Ni como 123.4
	"aq-567.8" define valor de Q como 567.8
	"ana123.4" define valor de Na como 123.4
	"anf123.4" define valor de Nf como 123.4
*/


int command_size (float value, char *cmd){
	// Tamanho do valor enviado;
	int cmd_len = snprintf(0, 0, "%f", value);
	
	// Incrementa o tamanho do comando;
	return (cmd_len+strlen(cmd));
}

int udp_read_data (int socket, struct sockaddr_in address, uint cmd_index, float *leitura){
	// Armazena o nome do serviço que será solicitado
	char service[5];
	// Inicializa service com bytes nulos
    memset(service, '\0', sizeof(service));
	// Define o comando com base no index passado
	switch (cmd_index) {
		case 1: // lê valor de Ta
			strcpy(service, "sta0");
			break;
		case 2: // lê valor de T
			strcpy(service, "st-0");
			break;
		case 3: // lê valor de Ti
			strcpy(service, "sti0");
			break;
		case 4: // lê valor de No
			strcpy(service, "sno0");
			break;
		case 5: // lê valor de H
			strcpy(service, "sh-0");
			break;
		default:
			return UDP_ERROR;
	}
	// Tamanho do comando que será enviado
	uint len = strlen(service);
	// Reserva armazenamento para o comando e resposta
	char *cmd = (char *) malloc(sizeof(char)*len);
	char *response = (char *)malloc(sizeof(char)*len);
	int response_size;

	// Envio do comando de leitura
	envia_mensagem(socket, address, service);

	// Leitura da resposta
	response_size = recebe_mensagem(socket, response, 1000);

	// Compara resposta com comando para saber se foi realizada com sucesso
	int result = (strcmp("000", response) == 0) ? UDP_ERROR : UDP_SUCCESS;


	if(result == UDP_SUCCESS){
		// printf("Leitura realizada com sucesso\n");

		// Aloca espaço temporário para armazenar o valor respondido (sem o cmd)
		char *read_value = (char *) malloc(sizeof(char)*response_size-3);

		// Copia apenas o float value removendo o comando da resposta;
		memcpy(read_value, &response[3], response_size-3);

		//printf("read_value: %s\n", read_value);

		// Armazena o valor lido em float
		*leitura = atof(read_value);

		free(read_value);
	}
	//printf("Comando: %s\n", cmd);
	//printf("Response: %s\n", response);
	
	// Limpa e retorna
	free(cmd);
	free(response);
	return result; 
}

int udp_write_data (int socket, struct sockaddr_in address, uint cmd_index, float value) {

	char service[4];
	// Inicializa service com bytes nulos
    memset(service, '\0', sizeof(service));

	// Define o comando com base no index passado
	switch (cmd_index){
		case 6: // define valor de Ni
			strcpy(service, "ani");
			break;
		case 7: // define valor de Q
			strcpy(service, "aq-");
			break;
		case 8: // define valor de Na
			strcpy(service, "ana");
			break;
		case 9: // define valor de Nf
			strcpy(service, "anf");
			break;
		default:
			return UDP_ERROR;// error
	}
	
	// Tamanho do comando que será enviado
	uint len = command_size(value, service);
	// Reserva armazenamento para o comando e resposta
	char *cmd = (char *) malloc(sizeof(char)*len);
	char *response = (char *)malloc(sizeof(char)*len);
	
	int response_size;

	// Gera string com o comando (concatena cmd + valor)
	snprintf(cmd, len, "%s%f", service, value);

	// Envio do comando
	envia_mensagem(socket, address, cmd);

	// Leitura da resposta
	response_size = recebe_mensagem(socket, response, 1000);

	// Compara resposta com comando para saber se foi enviado e aceito;
	int result = (response_size != len-1 ) ? UDP_ERROR : UDP_SUCCESS;

	//printf("Comando: %s\n", cmd);
	//printf("Response: %s\n", response);
	//printf("Sizes %d and %d\n", len-1, response_size);

	// Limpa e retorna
	free(cmd);
	free(response);
	return result; 
}
