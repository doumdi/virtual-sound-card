/**
 * Test that generates a WAV file with sine wave
 * This is useful for manual audio verification
 */

#include "sine_generator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define SAMPLE_RATE 48000
#define DURATION_SECONDS 2
#define FREQUENCY 440.0  // A4 note
#define AMPLITUDE 0.5

/* WAV file header structure */
typedef struct {
    char riff[4];           // "RIFF"
    uint32_t file_size;     // File size - 8
    char wave[4];           // "WAVE"
    char fmt[4];            // "fmt "
    uint32_t fmt_size;      // 16 for PCM
    uint16_t audio_format;  // 1 for PCM
    uint16_t num_channels;  // 1 = Mono, 2 = Stereo
    uint32_t sample_rate;   // Sample rate
    uint32_t byte_rate;     // byte_rate = sample_rate * num_channels * bits_per_sample/8
    uint16_t block_align;   // block_align = num_channels * bits_per_sample/8
    uint16_t bits_per_sample; // 16
    char data[4];           // "data"
    uint32_t data_size;     // data size
} wav_header_t;

int write_wav_file(const char *filename, int16_t *samples, size_t num_samples)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        printf("Failed to open file: %s\n", filename);
        return 1;
    }
    
    wav_header_t header;
    memcpy(header.riff, "RIFF", 4);
    memcpy(header.wave, "WAVE", 4);
    memcpy(header.fmt, "fmt ", 4);
    memcpy(header.data, "data", 4);
    
    header.fmt_size = 16;
    header.audio_format = 1; // PCM
    header.num_channels = 1; // Mono
    header.sample_rate = SAMPLE_RATE;
    header.bits_per_sample = 16;
    header.byte_rate = SAMPLE_RATE * header.num_channels * header.bits_per_sample / 8;
    header.block_align = header.num_channels * header.bits_per_sample / 8;
    header.data_size = num_samples * header.num_channels * header.bits_per_sample / 8;
    header.file_size = 36 + header.data_size;
    
    fwrite(&header, sizeof(wav_header_t), 1, fp);
    fwrite(samples, sizeof(int16_t), num_samples, fp);
    
    fclose(fp);
    return 0;
}

int main(void)
{
    sine_generator_t gen;
    size_t num_samples = SAMPLE_RATE * DURATION_SECONDS;
    int16_t *samples;
    int result = 0;
    
    printf("Generating sine wave test file...\n");
    printf("  Frequency: %.1f Hz (A4 note)\n", FREQUENCY);
    printf("  Sample rate: %d Hz\n", SAMPLE_RATE);
    printf("  Duration: %d seconds\n", DURATION_SECONDS);
    printf("  Samples: %zu\n", num_samples);
    
    // Allocate buffer
    samples = (int16_t *)malloc(num_samples * sizeof(int16_t));
    if (!samples) {
        printf("  FAIL: Memory allocation failed\n");
        return 1;
    }
    
    // Initialize generator
    sine_generator_init(&gen, FREQUENCY, SAMPLE_RATE, AMPLITUDE);
    
    // Generate samples
    sine_generator_process_i16(&gen, samples, num_samples);
    printf("  Generated %zu samples\n", num_samples);
    
    // Write WAV file
    const char *filename = "test_sine_440hz.wav";
    if (write_wav_file(filename, samples, num_samples) == 0) {
        printf("  PASS: Created WAV file: %s\n", filename);
        printf("  You can play this file to verify audio output\n");
    } else {
        printf("  FAIL: Failed to write WAV file\n");
        result = 1;
    }
    
    free(samples);
    
    printf("\n");
    if (result == 0) {
        printf("Test PASSED - WAV file created successfully\n");
    } else {
        printf("Test FAILED\n");
    }
    
    return result;
}
