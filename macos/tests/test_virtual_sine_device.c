/**
 * Test Virtual Sine Device Configuration
 * 
 * Basic test to verify the virtual sine device can be built and
 * has the correct configuration options.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Sine generator (copy from virtual_sine_device.c for testing)
typedef struct {
    double phase;
    double frequency;
    double sample_rate;
    double amplitude;
} sine_generator_t;

static void sine_generator_init(sine_generator_t *gen, double frequency,
                                double sample_rate, double amplitude)
{
    gen->phase = 0.0;
    gen->frequency = frequency;
    gen->sample_rate = sample_rate;
    gen->amplitude = amplitude;
}

static void sine_generator_process_f32(sine_generator_t *gen, float *buffer,
                                       size_t num_samples)
{
    double phase_increment = 2.0 * M_PI * gen->frequency / gen->sample_rate;
    
    for (size_t i = 0; i < num_samples; i++) {
        buffer[i] = (float)(gen->amplitude * sin(gen->phase));
        gen->phase += phase_increment;
        
        if (gen->phase >= 2.0 * M_PI) {
            gen->phase -= 2.0 * M_PI;
        }
    }
}

int main(void)
{
    sine_generator_t gen;
    float buffer[100];
    int test_passed = 1;
    
    printf("Virtual Sine Device Configuration Test\n");
    printf("=======================================\n\n");
    
    // Test 1: Initialize sine generator
    printf("Test 1: Initialize sine generator... ");
    sine_generator_init(&gen, 440.0, 48000.0, 0.5);
    if (gen.frequency == 440.0 && gen.sample_rate == 48000.0 && 
        gen.amplitude == 0.5 && gen.phase == 0.0) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
        test_passed = 0;
    }
    
    // Test 2: Generate samples
    printf("Test 2: Generate sine wave samples... ");
    sine_generator_process_f32(&gen, buffer, 100);
    
    // Check that samples are within expected range
    int samples_ok = 1;
    for (int i = 0; i < 100; i++) {
        if (buffer[i] < -1.0f || buffer[i] > 1.0f) {
            samples_ok = 0;
            break;
        }
    }
    
    if (samples_ok) {
        printf("PASS\n");
    } else {
        printf("FAIL (samples out of range)\n");
        test_passed = 0;
    }
    
    // Test 3: Verify sine wave properties
    printf("Test 3: Verify sine wave properties... ");
    sine_generator_init(&gen, 440.0, 48000.0, 1.0);
    sine_generator_process_f32(&gen, buffer, 100);
    
    // Check amplitude (sample 0 should be near 0, sample at 1/4 period should be near 1.0)
    int quarter_period = (int)(48000.0 / 440.0 / 4.0);
    if (quarter_period < 100) {
        float val_at_quarter = buffer[quarter_period];
        if (val_at_quarter > 0.9f && val_at_quarter <= 1.0f) {
            printf("PASS\n");
        } else {
            printf("FAIL (amplitude incorrect: %.3f)\n", val_at_quarter);
            test_passed = 0;
        }
    } else {
        printf("SKIP (buffer too small)\n");
    }
    
    // Test 4: Different frequencies
    printf("Test 4: Different frequencies... ");
    sine_generator_init(&gen, 880.0, 48000.0, 0.5);
    if (gen.frequency == 880.0) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
        test_passed = 0;
    }
    
    // Test 5: Phase wrapping
    printf("Test 5: Phase wrapping... ");
    sine_generator_init(&gen, 440.0, 48000.0, 0.5);
    // Generate enough samples to cause phase wrapping
    for (int i = 0; i < 1000; i++) {
        sine_generator_process_f32(&gen, buffer, 100);
    }
    // Phase should be wrapped and less than 2*PI
    if (gen.phase >= 0.0 && gen.phase < 2.0 * M_PI) {
        printf("PASS\n");
    } else {
        printf("FAIL (phase: %.6f)\n", gen.phase);
        test_passed = 0;
    }
    
    printf("\n");
    
    if (test_passed) {
        printf("=== ALL TESTS PASSED ===\n");
        return 0;
    } else {
        printf("=== SOME TESTS FAILED ===\n");
        return 1;
    }
}
