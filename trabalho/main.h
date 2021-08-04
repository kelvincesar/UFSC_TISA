// Inclusion guard, to prevent multiple includes of the same header
#ifndef MAIN_H
#define MAIN_H

// # Libs:
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>
#include <time.h>

#include "./libs/udp_client.h"
#include "./libs/pi.h"

// # Debug flags
//#define DEBUG_TIME_TEMP
//#define DEBUG_TIME_NIVEL
// # Buffer
#define BUFFER_SIZE     (10)
#define BUFFER_FILE_NAME "./caldeira_log_temperatura.log"

// # Parâmetros da caldeira:
#define TEMP_FAULT      (30)    // Temperatura que gera falha

#define S_AGUA 			(4184) 	// Calor específico da água [Joule/Kg.Celsius]
#define P_AGUA 			(1000)	// Peso específico da água [kg/m3]
#define AREA_BASE_TANQ	(4)		// Área da base do tanque [m2]
#define R_ISOLAMENTO	(0.001)	// Resistência térmica do isolamento (2mm madeira) [Grau / (J/s)] 
#define CONST_CAP_TERM	(S_AGUA * P_AGUA * AREA_BASE_TANQ)	// Constante da capacitância térmica da água

// # Parâmetros do controle:
// Temperatura:
#define T_KI		(0.1)           // Constante KI
#define T_KP		(10000*1000)    // Constante KP
#define Q_MAX		(999999.999)	// Valor máximo da variável Q (testado na caldeira)
#define Q_MIN		(0)			    // Valor mínimo da variável Q
#define T_PERIOD	(0.050)		    // Tempo do controlador de temperatura em segundos (50 ms)

// Nível:
#define H_KI		(1)             // Constante KI
#define H_KP		(1000)          // Constante KP
#define H_MAX		(3.0)			// Altura máxima do tanque
#define H_MIN		(0.1)		    // Altura mínima do tanque
#define VAZAO_MIN   (-100)		    // Vazão mínima permitida da água do tanque (na saida Nf)
#define VAZAO_MAX	(100)		    // Vazão máxima permitida da água do tanque (na entrada Ni)
#define Q_PERIOD 	(0.070)		    // Tempo do controlador de nível em segundos (70 ms)

// # Definições gerais:
#define NSEC_PER_SEC    (1000000000) // Número de nanosegundos em um segundo

#define TO_RED(string) "\x1b[31m" string "\x1b[0m"
#define TO_BLUE(string) "\x1b[35m" string "\x1b[0m"
#define COLOR_RED 	  "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_RESET   "\x1b[0m"
#define COLOR_WHITE	  "\x1b[0;37m"


// # Armazenamento dos dados da caldeira
typedef struct {
	float Ta;	// Temperatura do ar ambiente em volta do recipiente[°C] 
	float T;	// Temperatura da água no interior do recipiente [°C] 
	float Ti;	// Temperatura da água na entrada do recipiente [°C] 
	float Q;	// Valor de Q [J/s ou W];
	float Qt;	// Fluxo de calor total fornecido para o recipiente;
	float Qe;	// Fluxo de calor através do isolamento do recipiente [J/s] 
	float Qi;	// Fluxo de calor inserido pela água que entra no recipiente [J/s]
	float Qa;	// Fluxo de calor inserido pela água aquecida [J/s]
	float C;	// Capacitância térmica da água no recipiente [Joule/Celsius]
	float H;	// Altura da coluna de água dentro do recipiente [m] 
	float Ni;	// Fluxo de água de entrada [kg/s]
	float Nf;	// Fluxo de água de saída [kg/s]
	float Na; 	// Fluxo de água aquecida a 80C de entrada controlada [kg/s]
	float No;	// Fluxo de água de saída do recipiente [kg/s]
	int flag_resfriamento_agua;	// Indica que está tentando fazer resfriamento com circulação de água;
	int flag_aquecimento_agua;  // Indica que está tentando fazer aquecimento com circulação de água quente;
	int flag_troca_calor_const; // Indica que está alterando o nível porém mantendo a troca de calor constante;
} Caldeira;

// # Geral

// Estrutura usada para converter string em float e validar o valor
typedef struct {
    float value;
    int is_valid;
} StringToFloat; 

// Buffer duplo
typedef struct {
	float buffer0[BUFFER_SIZE];
	float buffer1[BUFFER_SIZE];
	int selected_buffer;
	int index;
	int gravar;
} BufferDuplo;


// End of the inclusion guard
#endif
