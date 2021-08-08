# DAS410037 Técnicas de implementação de Sistemas Automatizados
Repositório para armazenamento dos exercícios da matéria de mestrado Técnicas de implementação de Sistemas Automatizados

Também foi utilizado para o trabalho final de controle de uma caldeira simulada.


# Trabalho final:

Os arquivos do trabalho final estão na pasta "trabalho";

- Para compilação  utilize:
`gcc -lm -pthread -o controle_caldeira ./libs/udp_client.c ./libs/pi.c main.c`

- Para execução:
`./controle_caldeira <ip_caldeira> <porta_caldeira>`

- Exemplo:
`./controle_caldeira 172.29.144.1 5000`

- Para execução do simulador da caldeira:
`java -jar aquecedor2008_1.jar 5000`