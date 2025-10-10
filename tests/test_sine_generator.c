/**
 * Test for Sine Wave Generator
 */

#include "sine_generator.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define TEST_SAMPLE_RATE 48000.0
#define TEST_FREQUENCY 440.0
#define TEST_AMPLITUDE 0.5
#define TEST_SAMPLES 1000
#define EPSILON 0.001

int main(void)
{
    sine_generator_t gen;
    float buffer[TEST_SAMPLES];
    int passed = 1;
    
    printf("Testing sine wave generator...\n");
    
    // Initialize generator
    sine_generator_init(&gen, TEST_FREQUENCY, TEST_SAMPLE_RATE, TEST_AMPLITUDE);
    printf("  Generator initialized: freq=%.1f Hz, sr=%.1f Hz, amp=%.2f\n", 
           gen.frequency, gen.sample_rate, gen.amplitude);
    
    // Generate samples
    sine_generator_process_f32(&gen, buffer, TEST_SAMPLES);
    printf("  Generated %d samples\n", TEST_SAMPLES);
    
    // Verify first sample is approximately 0 (starts at phase 0)
    if (fabs(buffer[0]) > EPSILON) {
        printf("  FAIL: First sample should be near 0, got %.6f\n", buffer[0]);
        passed = 0;
    } else {
        printf("  PASS: First sample is near 0 (%.6f)\n", buffer[0]);
    }
    
    // Verify amplitude bounds
    for (int i = 0; i < TEST_SAMPLES; i++) {
        if (fabs(buffer[i]) > TEST_AMPLITUDE + EPSILON) {
            printf("  FAIL: Sample %d exceeds amplitude bounds: %.6f\n", i, buffer[i]);
            passed = 0;
            break;
        }
    }
    if (passed) {
        printf("  PASS: All samples within amplitude bounds\n");
    }
    
    // Test frequency change
    sine_generator_set_frequency(&gen, 880.0);
    if (fabs(gen.frequency - 880.0) > EPSILON) {
        printf("  FAIL: Frequency change failed\n");
        passed = 0;
    } else {
        printf("  PASS: Frequency changed to %.1f Hz\n", gen.frequency);
    }
    
    // Test amplitude change
    sine_generator_set_amplitude(&gen, 0.8);
    if (fabs(gen.amplitude - 0.8) > EPSILON) {
        printf("  FAIL: Amplitude change failed\n");
        passed = 0;
    } else {
        printf("  PASS: Amplitude changed to %.2f\n", gen.amplitude);
    }
    
    // Test reset
    sine_generator_reset(&gen);
    if (fabs(gen.phase) > EPSILON) {
        printf("  FAIL: Reset failed\n");
        passed = 0;
    } else {
        printf("  PASS: Phase reset to 0\n");
    }
    
    // Test 16-bit integer generation
    int16_t buffer_i16[TEST_SAMPLES];
    sine_generator_process_i16(&gen, buffer_i16, TEST_SAMPLES);
    if (abs(buffer_i16[0]) > 100) {
        printf("  FAIL: 16-bit first sample should be near 0\n");
        passed = 0;
    } else {
        printf("  PASS: 16-bit generation works (first sample: %d)\n", buffer_i16[0]);
    }
    
    // Test 32-bit integer generation
    sine_generator_reset(&gen);  // Reset phase before testing
    int32_t buffer_i32[TEST_SAMPLES];
    sine_generator_process_i32(&gen, buffer_i32, TEST_SAMPLES);
    if (abs(buffer_i32[0]) > 100000) {
        printf("  FAIL: 32-bit first sample should be near 0\n");
        passed = 0;
    } else {
        printf("  PASS: 32-bit generation works (first sample: %d)\n", buffer_i32[0]);
    }
    
    printf("\n");
    if (passed) {
        printf("All tests PASSED\n");
        return 0;
    } else {
        printf("Some tests FAILED\n");
        return 1;
    }
}
