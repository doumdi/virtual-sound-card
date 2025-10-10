/**
 * Sine Wave Generator for Virtual Sound Card
 * 
 * Provides sine wave generation for testing audio output
 */

#ifndef SINE_GENERATOR_H
#define SINE_GENERATOR_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Sine wave generator context
 */
typedef struct {
    double phase;           /* Current phase (0.0 to 2π) */
    double frequency;       /* Frequency in Hz */
    double sample_rate;     /* Sample rate in Hz */
    double amplitude;       /* Amplitude (0.0 to 1.0) */
} sine_generator_t;

/**
 * Initialize a sine wave generator
 * 
 * @param gen Pointer to generator structure
 * @param frequency Frequency in Hz
 * @param sample_rate Sample rate in Hz
 * @param amplitude Amplitude (0.0 to 1.0)
 */
void sine_generator_init(sine_generator_t *gen, 
                        double frequency, 
                        double sample_rate,
                        double amplitude);

/**
 * Generate sine wave samples (32-bit float)
 * 
 * @param gen Pointer to generator structure
 * @param buffer Output buffer for samples
 * @param num_samples Number of samples to generate
 */
void sine_generator_process_f32(sine_generator_t *gen, 
                               float *buffer, 
                               size_t num_samples);

/**
 * Generate sine wave samples (16-bit integer)
 * 
 * @param gen Pointer to generator structure
 * @param buffer Output buffer for samples
 * @param num_samples Number of samples to generate
 */
void sine_generator_process_i16(sine_generator_t *gen, 
                               int16_t *buffer, 
                               size_t num_samples);

/**
 * Generate sine wave samples (32-bit integer)
 * 
 * @param gen Pointer to generator structure
 * @param buffer Output buffer for samples
 * @param num_samples Number of samples to generate
 */
void sine_generator_process_i32(sine_generator_t *gen, 
                               int32_t *buffer, 
                               size_t num_samples);

/**
 * Set frequency of sine wave
 * 
 * @param gen Pointer to generator structure
 * @param frequency New frequency in Hz
 */
void sine_generator_set_frequency(sine_generator_t *gen, double frequency);

/**
 * Set amplitude of sine wave
 * 
 * @param gen Pointer to generator structure
 * @param amplitude New amplitude (0.0 to 1.0)
 */
void sine_generator_set_amplitude(sine_generator_t *gen, double amplitude);

/**
 * Reset phase to zero
 * 
 * @param gen Pointer to generator structure
 */
void sine_generator_reset(sine_generator_t *gen);

#ifdef __cplusplus
}
#endif

#endif /* SINE_GENERATOR_H */
