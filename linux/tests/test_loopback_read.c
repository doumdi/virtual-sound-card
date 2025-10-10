/**
 * Test Loopback Read
 * 
 * Reads audio from ALSA loopback device and verifies it contains a sine wave
 * This program should be run while sine_generator_app is playing
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <alsa/asoundlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE 48000
#define CHANNELS 2
#define BUFFER_SIZE 1024
#define READ_DURATION 2
#define EXPECTED_FREQUENCY 440.0
#define FREQUENCY_TOLERANCE 5.0

/**
 * Simple zero-crossing frequency detector
 */
static double detect_frequency(int16_t *buffer, size_t num_samples,
			       int sample_rate)
{
	int zero_crossings = 0;
	int16_t prev_sample = buffer[0];

	for (size_t i = 1; i < num_samples; i++) {
		int16_t curr_sample = buffer[i];
		if ((prev_sample < 0 && curr_sample >= 0) ||
		    (prev_sample >= 0 && curr_sample < 0)) {
			zero_crossings++;
		}
		prev_sample = curr_sample;
	}

	/* Frequency = (zero crossings / 2) / duration */
	double duration = (double)num_samples / sample_rate;
	double frequency = (zero_crossings / 2.0) / duration;

	return frequency;
}

/**
 * Check if signal has reasonable amplitude
 */
static int check_amplitude(int16_t *buffer, size_t num_samples)
{
	int32_t sum = 0;
	int32_t sum_sq = 0;

	for (size_t i = 0; i < num_samples; i++) {
		sum += buffer[i];
		sum_sq += (int32_t)buffer[i] * buffer[i];
	}

	double mean = (double)sum / num_samples;
	double rms = sqrt((double)sum_sq / num_samples);

	/* RMS should be significant (not silent) */
	if (rms < 1000) {
		fprintf(stderr, "Signal too quiet (RMS: %.2f)\n", rms);
		return 0;
	}

	/* Mean should be close to zero (no DC offset) */
	if (fabs(mean) > 1000) {
		fprintf(stderr, "Signal has DC offset (mean: %.2f)\n", mean);
		return 0;
	}

	printf("Signal amplitude OK (RMS: %.2f, mean: %.2f)\n", rms, mean);
	return 1;
}

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	snd_pcm_t *pcm_handle;
	snd_pcm_hw_params_t *params;
	int16_t *buffer;
	int16_t *mono_buffer;
	int err;
	unsigned int sample_rate = SAMPLE_RATE;
	snd_pcm_uframes_t frames = BUFFER_SIZE;
	int test_passed = 1;

	printf("Loopback Read Test\n");
	printf("==================\n");
	printf("Reading from virtual sound card...\n");
	printf("Expected frequency: %.2f Hz\n", EXPECTED_FREQUENCY);
	printf("Duration: %d seconds\n", READ_DURATION);
	printf("\n");

	/* Open PCM device for capture */
	err = snd_pcm_open(&pcm_handle, "hw:Loopback,1,0",
			   SND_PCM_STREAM_CAPTURE, 0);
	if (err < 0) {
		fprintf(stderr,
			"Error opening PCM device hw:Loopback,1,0: %s\n",
			snd_strerror(err));
		fprintf(stderr, "Make sure the snd-aloop module is loaded:\n");
		fprintf(stderr, "  sudo modprobe snd-aloop\n");
		fprintf(stderr,
			"Also ensure sine_generator_app is running in another terminal.\n");
		return 1;
	}

	/* Allocate hardware parameters object */
	snd_pcm_hw_params_alloca(&params);

	/* Fill it with default values */
	snd_pcm_hw_params_any(pcm_handle, params);

	/* Set hardware parameters */
	snd_pcm_hw_params_set_access(pcm_handle, params,
				     SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE);
	snd_pcm_hw_params_set_channels(pcm_handle, params, CHANNELS);
	snd_pcm_hw_params_set_rate_near(pcm_handle, params, &sample_rate, 0);
	snd_pcm_hw_params_set_period_size_near(pcm_handle, params, &frames, 0);

	/* Write parameters to device */
	err = snd_pcm_hw_params(pcm_handle, params);
	if (err < 0) {
		fprintf(stderr, "Error setting HW params: %s\n",
			snd_strerror(err));
		snd_pcm_close(pcm_handle);
		return 1;
	}

	/* Start capture */
	err = snd_pcm_prepare(pcm_handle);
	if (err < 0) {
		fprintf(stderr, "Error preparing PCM: %s\n",
			snd_strerror(err));
		snd_pcm_close(pcm_handle);
		return 1;
	}

	/* Allocate buffers */
	buffer = malloc(frames * CHANNELS * sizeof(int16_t));
	mono_buffer = malloc(frames * sizeof(int16_t));
	if (!buffer || !mono_buffer) {
		fprintf(stderr, "Error allocating buffers\n");
		snd_pcm_close(pcm_handle);
		return 1;
	}

	/* Read audio for specified duration */
	int total_frames = READ_DURATION * sample_rate;
	int frames_read = 0;
	size_t mono_samples_collected = 0;
	size_t total_mono_samples = total_frames;
	int16_t *all_mono_samples = malloc(total_mono_samples * sizeof(int16_t));

	if (!all_mono_samples) {
		fprintf(stderr, "Error allocating analysis buffer\n");
		free(buffer);
		free(mono_buffer);
		snd_pcm_close(pcm_handle);
		return 1;
	}

	printf("Reading audio...\n");

	while (frames_read < total_frames) {
		/* Read samples from device */
		err = snd_pcm_readi(pcm_handle, buffer, frames);
		if (err == -EPIPE) {
			/* Buffer overrun */
			fprintf(stderr, "Buffer overrun\n");
			snd_pcm_prepare(pcm_handle);
			continue;
		} else if (err < 0) {
			fprintf(stderr, "Error reading from PCM device: %s\n",
				snd_strerror(err));
			break;
		}

		/* Extract mono channel (left channel) */
		for (int i = 0; i < err && mono_samples_collected < total_mono_samples; i++) {
			all_mono_samples[mono_samples_collected++] = buffer[i * CHANNELS];
		}

		frames_read += err;

		/* Print progress */
		if (frames_read % (sample_rate / 4) == 0) {
			float progress = (float)frames_read / total_frames * 100.0f;
			printf("\rProgress: %.1f%%", progress);
			fflush(stdout);
		}
	}

	printf("\rProgress: 100.0%%\n");
	printf("Read complete. Analyzing...\n\n");

	/* Analyze the captured audio */
	printf("=== Analysis Results ===\n");

	/* Check amplitude */
	if (!check_amplitude(all_mono_samples, mono_samples_collected)) {
		test_passed = 0;
	}

	/* Detect frequency */
	double detected_freq = detect_frequency(all_mono_samples,
						mono_samples_collected,
						sample_rate);
	printf("Detected frequency: %.2f Hz\n", detected_freq);

	/* Verify frequency is close to expected */
	double freq_diff = fabs(detected_freq - EXPECTED_FREQUENCY);
	if (freq_diff > FREQUENCY_TOLERANCE) {
		fprintf(stderr,
			"FAIL: Frequency mismatch (expected %.2f ± %.2f Hz, got %.2f Hz)\n",
			EXPECTED_FREQUENCY, FREQUENCY_TOLERANCE, detected_freq);
		test_passed = 0;
	} else {
		printf("PASS: Frequency within tolerance (%.2f Hz)\n",
		       freq_diff);
	}

	printf("\n");

	/* Cleanup */
	free(all_mono_samples);
	free(buffer);
	free(mono_buffer);
	snd_pcm_close(pcm_handle);

	if (test_passed) {
		printf("=== TEST PASSED ===\n");
		return 0;
	} else {
		printf("=== TEST FAILED ===\n");
		return 1;
	}
}
