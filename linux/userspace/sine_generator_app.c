/**
 * Sine Wave Generator Application
 * 
 * Generates a sine wave and plays it to the ALSA loopback device
 * Usage: ./sine_generator_app [frequency] [duration_seconds]
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

#define DEFAULT_FREQUENCY 440.0
#define DEFAULT_DURATION 5
#define SAMPLE_RATE 48000
#define CHANNELS 2
#define BUFFER_SIZE 1024

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

static void sine_generator_process_i16(sine_generator_t *gen, int16_t *buffer,
				       size_t num_samples)
{
	double phase_increment = 2.0 * M_PI * gen->frequency / gen->sample_rate;
	const double scale = 32767.0;

	for (size_t i = 0; i < num_samples; i++) {
		double sample = gen->amplitude * sin(gen->phase);
		buffer[i] = (int16_t)(sample * scale);
		gen->phase += phase_increment;

		if (gen->phase >= 2.0 * M_PI) {
			gen->phase -= 2.0 * M_PI;
		}
	}
}

int main(int argc, char *argv[])
{
	snd_pcm_t *pcm_handle;
	snd_pcm_hw_params_t *params;
	sine_generator_t gen;
	int16_t *buffer;
	double frequency = DEFAULT_FREQUENCY;
	int duration = DEFAULT_DURATION;
	int err;
	unsigned int sample_rate = SAMPLE_RATE;
	snd_pcm_uframes_t frames = BUFFER_SIZE;

	/* Parse command line arguments */
	if (argc >= 2) {
		frequency = atof(argv[1]);
		if (frequency <= 0 || frequency > 20000) {
			fprintf(stderr, "Invalid frequency: %.2f Hz\n",
				frequency);
			return 1;
		}
	}
	if (argc >= 3) {
		duration = atoi(argv[2]);
		if (duration <= 0 || duration > 60) {
			fprintf(stderr, "Invalid duration: %d seconds\n",
				duration);
			return 1;
		}
	}

	printf("Sine Wave Generator\n");
	printf("===================\n");
	printf("Frequency: %.2f Hz\n", frequency);
	printf("Duration: %d seconds\n", duration);
	printf("Sample Rate: %d Hz\n", sample_rate);
	printf("Channels: %d\n", CHANNELS);
	printf("Buffer Size: %lu frames\n", (unsigned long)frames);
	printf("\n");

	/* Open PCM device for playback */
	err = snd_pcm_open(&pcm_handle, "hw:Loopback,0,0",
			   SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) {
		fprintf(stderr,
			"Error opening PCM device hw:Loopback,0,0: %s\n",
			snd_strerror(err));
		fprintf(stderr, "Make sure the snd-aloop module is loaded:\n");
		fprintf(stderr, "  sudo modprobe snd-aloop\n");
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

	/* Allocate buffer */
	buffer = malloc(frames * CHANNELS * sizeof(int16_t));
	if (!buffer) {
		fprintf(stderr, "Error allocating buffer\n");
		snd_pcm_close(pcm_handle);
		return 1;
	}

	/* Initialize sine generator */
	sine_generator_init(&gen, frequency, sample_rate, 0.5);

	printf("Playing sine wave...\n");

	/* Generate and play audio for specified duration */
	int total_frames = duration * sample_rate;
	int frames_written = 0;

	while (frames_written < total_frames) {
		/* Generate interleaved stereo samples */
		for (unsigned int i = 0; i < frames; i++) {
			int16_t mono_sample;
			sine_generator_process_i16(&gen, &mono_sample, 1);
			/* Duplicate for stereo */
			buffer[i * CHANNELS] = mono_sample;
			buffer[i * CHANNELS + 1] = mono_sample;
		}

		/* Write samples to device */
		err = snd_pcm_writei(pcm_handle, buffer, frames);
		if (err == -EPIPE) {
			/* Buffer underrun */
			fprintf(stderr, "Buffer underrun\n");
			snd_pcm_prepare(pcm_handle);
		} else if (err < 0) {
			fprintf(stderr, "Error writing to PCM device: %s\n",
				snd_strerror(err));
			break;
		}

		frames_written += frames;

		/* Print progress */
		if (frames_written % (sample_rate / 4) == 0) {
			float progress = (float)frames_written / total_frames * 100.0f;
			printf("\rProgress: %.1f%%", progress);
			fflush(stdout);
		}
	}

	printf("\rProgress: 100.0%%\n");
	printf("Playback complete!\n");

	/* Cleanup */
	free(buffer);
	snd_pcm_drain(pcm_handle);
	snd_pcm_close(pcm_handle);

	return 0;
}
