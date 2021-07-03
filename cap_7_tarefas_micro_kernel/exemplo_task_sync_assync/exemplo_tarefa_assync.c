#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

void main(void){
	char linha[100];
	char *x; // Recebe o retorno do fgets
	
	// Realiza a leitura das flags de configuração do fgets;
	int flags = fcntl( fileno(stdin), F_GETFL, 0);
	
	// Altera a flag de configuração do fgets para operar de forma assyc;
	fcntl( fileno(stdin), F_SETFL, flags | O_NONBLOCK);

	// Aplicação de leitura de uma frase:
	printf("Digite uma frase: \n");
	x = fgets( linha, 100, stdin);

	// Enquanto for null, nada foi lido do teclado;
	while (x == NULL){
		printf(".");
		x = fgets( linha, 100, stdin);
	}

	printf("Recebido a frase `%s`\n - Possui %ld caracteres.\n", x, strlen(linha) - 1);
	
}
