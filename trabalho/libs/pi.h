// Inclusion guard, to prevent multiple includes of the same header
#ifndef PI_H
#define PI_H

typedef struct {
    // Ganhos do controlador
    float kp;
    float ki;

    // Limites de saida
    float out_max;
    float out_min;

    // Tempo de amostragem em segundos
    float sample_time_sec;

    // Variaveis do controlador
    float integrator;
    float out;
    float prev_error;

} Controlador_PI;

/** Realiza a inicialização do controlador PI.
 * @param pi Estrutura do controlador;
 * @param kp Ganho proporcional;
 * @param ki Ganho integral;
 * @param max Saturador para o valor máximo de saída da variável de controle;
 * @param min Saturador para o valor mínimo de saída da variável de controle;
 * @param sample_time Tempo de amostragem do sensor em segundos.
 */
void PI_Init (Controlador_PI *pi, float kp, float ki, float max, float min, float sample_time);

/** Realiza a atualização do controlador PI e retorna um novo valor para variável de controle.
 * @param pi Estrutura do controlador
 * @param ref Valor de referência desejado para a variável medida;
 * @param measure Valor atual da medição da variável controlada;
 * @returns Retorna um novo valor para a variável de controle.
 */
float PI_Update (Controlador_PI *pi, float ref, float measure);


// End of the inclusion guard
#endif