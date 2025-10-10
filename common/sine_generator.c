/**
 * Sine Wave Generator Implementation
 */

#include "sine_generator.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void sine_generator_init(sine_generator_t *gen, 
                        double frequency, 
                        double sample_rate,
                        double amplitude)
{
    gen->phase = 0.0;
    gen->frequency = frequency;
    gen->sample_rate = sample_rate;
    gen->amplitude = amplitude;
}

void sine_generator_process_f32(sine_generator_t *gen, 
                               float *buffer, 
                               size_t num_samples)
{
    double phase_increment = 2.0 * M_PI * gen->frequency / gen->sample_rate;
    
    for (size_t i = 0; i < num_samples; i++) {
        buffer[i] = (float)(gen->amplitude * sin(gen->phase));
        gen->phase += phase_increment;
        
        // Wrap phase to avoid numerical issues with large values
        if (gen->phase >= 2.0 * M_PI) {
            gen->phase -= 2.0 * M_PI;
        }
    }
}

void sine_generator_process_i16(sine_generator_t *gen, 
                               int16_t *buffer, 
                               size_t num_samples)
{
    double phase_increment = 2.0 * M_PI * gen->frequency / gen->sample_rate;
    const double scale = 32767.0; // Max value for 16-bit signed integer
    
    for (size_t i = 0; i < num_samples; i++) {
        double sample = gen->amplitude * sin(gen->phase);
        buffer[i] = (int16_t)(sample * scale);
        gen->phase += phase_increment;
        
        // Wrap phase to avoid numerical issues with large values
        if (gen->phase >= 2.0 * M_PI) {
            gen->phase -= 2.0 * M_PI;
        }
    }
}

void sine_generator_process_i32(sine_generator_t *gen, 
                               int32_t *buffer, 
                               size_t num_samples)
{
    double phase_increment = 2.0 * M_PI * gen->frequency / gen->sample_rate;
    const double scale = 2147483647.0; // Max value for 32-bit signed integer
    
    for (size_t i = 0; i < num_samples; i++) {
        double sample = gen->amplitude * sin(gen->phase);
        buffer[i] = (int32_t)(sample * scale);
        gen->phase += phase_increment;
        
        // Wrap phase to avoid numerical issues with large values
        if (gen->phase >= 2.0 * M_PI) {
            gen->phase -= 2.0 * M_PI;
        }
    }
}

void sine_generator_set_frequency(sine_generator_t *gen, double frequency)
{
    gen->frequency = frequency;
}

void sine_generator_set_amplitude(sine_generator_t *gen, double amplitude)
{
    gen->amplitude = amplitude;
}

void sine_generator_reset(sine_generator_t *gen)
{
    gen->phase = 0.0;
}
