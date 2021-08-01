#include "pi.h"

void PI_Init (Controlador_PI *pi, float kp, float ki, float max, float min, float sample_time){
    // Inicializa as variaveis do controlador PI;
    pi->integrator = 0.0f;
    pi->out = 0.0f;
    pi->prev_error = 0.0f;

    pi->ki = ki;
    pi->kp = kp;
    pi->sample_time_sec = sample_time;
    pi->out_max = max;
    pi->out_min = min;
}



float PI_Update (Controlador_PI *pi, float ref, float measure){
    // Erro do sinal
    float error = ref - measure;

    // Cálculo do ganho proporcional
    float proportional = pi->kp * error;

    // Cálculo do integral;
    pi->integrator += 0.5f * pi->ki * pi->sample_time_sec * (error + pi->prev_error);

    // Calcula os limitadores do integrador
    float int_lim_min, int_lim_max;

    int_lim_max = (pi->out_max > proportional) ? pi->out_max - proportional : 0.0f;
    int_lim_min = (pi->out_min < proportional) ? pi->out_min - proportional : 0.0f;

    // Saturação do integrador
    if (pi->integrator > int_lim_max){
        pi->integrator = int_lim_max;
    } else if (pi->integrator < int_lim_min){
        pi->integrator = int_lim_min;
    }

    // Saída:
    pi->out = proportional + pi->integrator;

    // Saturação da saída
    if (pi->out > pi->out_max) {
        pi->out = pi->out_max;
    } else if (pi->out < pi->out_min) {
        pi->out = pi->out_min;
    }

    // Armazena o erro atual como anterior
    pi->prev_error = error;

    // retorno do novo valor
    return pi->out;
}