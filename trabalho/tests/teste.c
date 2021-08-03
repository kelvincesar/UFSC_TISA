#include <string.h>
#include <stdlib.h>
#include <stdio.h>


int command_size (float value, char *cmd){
	// Tamanho do valor enviado;
	int cmd_len = snprintf(0, 0, "%.2f", value);
	// Incrementa o tamanho do comando;
	cmd_len += strlen(cmd);
	return cmd_len;
}
void main (){
	float amount = 123.21231231231;
	char service[4];

	// Inicializa service com bytes nulos
    memset(service, '\0', sizeof(service));
	strcpy(service, "ani");
	printf("service %s\n", service);
	
	uint len = command_size(amount, service);
	printf("Lenght: %d\n", len);

	// Reserva armazenamento para o comando
	char *cmd = (char *)malloc(len);
	// Gera string com o comando (concatena cmd + valor)
	snprintf(cmd, len, "%s%.2f", service, amount);
	printf("Comando %s \n", cmd);

	// Libera o espaço usado pelo malloc
	free(cmd);
	
}
