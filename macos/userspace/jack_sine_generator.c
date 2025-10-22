/**
 * JACK2 Sine Wave Generator Application (macOS)
 * 
 * Generates a sine wave and plays it through JACK2 audio server
 * Usage: ./jack_sine_generator [frequency] [duration_seconds]
 * 
 * Prerequisites:
 *   - JACK2 for macOS installed (brew install jack or from https://jackaudio.org/)
 *   - JACK server running (QjackCtl or jackd)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <jack/jack.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DEFAULT_FREQUENCY 440.0
#define DEFAULT_DURATION 5

typedef struct {
	double phase;
	double frequency;
	double sample_rate;
	double amplitude;
	int frames_remaining;
	int total_frames;
	volatile int should_exit;
} sine_generator_t;

static sine_generator_t generator;
static jack_port_t *output_port_left;
static jack_port_t *output_port_right;
static jack_client_t *client;

static void sine_generator_init(sine_generator_t *gen, double frequency,
				double sample_rate, double amplitude, int duration)
{
	gen->phase = 0.0;
	gen->frequency = frequency;
	gen->sample_rate = sample_rate;
	gen->amplitude = amplitude;
	gen->total_frames = (int)(duration * sample_rate);
	gen->frames_remaining = gen->total_frames;
	gen->should_exit = 0;
}

static void sine_generator_process_float(sine_generator_t *gen, float *left,
					 float *right, jack_nframes_t nframes)
{
	double phase_increment = 2.0 * M_PI * gen->frequency / gen->sample_rate;

	for (jack_nframes_t i = 0; i < nframes; i++) {
		if (gen->frames_remaining <= 0) {
			left[i] = 0.0f;
			right[i] = 0.0f;
		} else {
			float sample = (float)(gen->amplitude * sin(gen->phase));
			left[i] = sample;
			right[i] = sample;
			
			gen->phase += phase_increment;
			if (gen->phase >= 2.0 * M_PI) {
				gen->phase -= 2.0 * M_PI;
			}
			
			gen->frames_remaining--;
		}
	}
}

static int jack_process_callback(jack_nframes_t nframes, void *arg)
{
	(void)arg;
	
	float *left_buffer = (float *)jack_port_get_buffer(output_port_left, nframes);
	float *right_buffer = (float *)jack_port_get_buffer(output_port_right, nframes);
	
	sine_generator_process_float(&generator, left_buffer, right_buffer, nframes);
	
	if (generator.frames_remaining <= 0) {
		generator.should_exit = 1;
	}
	
	return 0;
}

static void jack_shutdown_callback(void *arg)
{
	(void)arg;
	fprintf(stderr, "JACK server shut down\n");
	exit(1);
}

static void signal_handler(int sig)
{
	(void)sig;
	generator.should_exit = 1;
}

int main(int argc, char *argv[])
{
	jack_status_t status;
	const char **ports;
	double frequency = DEFAULT_FREQUENCY;
	int duration = DEFAULT_DURATION;

	/* Parse command line arguments */
	if (argc >= 2) {
		frequency = atof(argv[1]);
		if (frequency <= 0 || frequency > 20000) {
			fprintf(stderr, "Invalid frequency: %.2f Hz\n", frequency);
			return 1;
		}
	}
	if (argc >= 3) {
		duration = atoi(argv[2]);
		if (duration <= 0 || duration > 60) {
			fprintf(stderr, "Invalid duration: %d seconds\n", duration);
			return 1;
		}
	}

	printf("JACK2 Sine Wave Generator (macOS)\n");
	printf("==================================\n");
	printf("Frequency: %.2f Hz\n", frequency);
	printf("Duration: %d seconds\n", duration);
	printf("\n");

	/* Create JACK client */
	client = jack_client_open("sine_generator", JackNullOption, &status, NULL);
	if (client == NULL) {
		fprintf(stderr, "Failed to open JACK client: 0x%2.0x\n", status);
		if (status & JackServerFailed) {
			fprintf(stderr, "Unable to connect to JACK server\n");
			fprintf(stderr, "Make sure JACK is running:\n");
			fprintf(stderr, "  brew install jack (to install)\n");
			fprintf(stderr, "  jackd -d coreaudio (to start server)\n");
			fprintf(stderr, "  or use QjackCtl GUI\n");
		}
		return 1;
	}

	if (status & JackServerStarted) {
		printf("JACK server started\n");
	}

	if (status & JackNameNotUnique) {
		const char *client_name = jack_get_client_name(client);
		printf("Unique name assigned: %s\n", client_name);
	}

	/* Initialize sine generator */
	sine_generator_init(&generator, frequency, jack_get_sample_rate(client),
			    0.5, duration);

	printf("Sample Rate: %.0f Hz\n", generator.sample_rate);
	printf("Total Frames: %d\n", generator.total_frames);
	printf("\n");

	/* Set process callback */
	jack_set_process_callback(client, jack_process_callback, NULL);

	/* Set shutdown callback */
	jack_on_shutdown(client, jack_shutdown_callback, NULL);

	/* Create output ports */
	output_port_left = jack_port_register(client, "output_left",
					      JACK_DEFAULT_AUDIO_TYPE,
					      JackPortIsOutput, 0);
	output_port_right = jack_port_register(client, "output_right",
					       JACK_DEFAULT_AUDIO_TYPE,
					       JackPortIsOutput, 0);

	if ((output_port_left == NULL) || (output_port_right == NULL)) {
		fprintf(stderr, "Failed to create output ports\n");
		jack_client_close(client);
		return 1;
	}

	/* Activate the client */
	if (jack_activate(client)) {
		fprintf(stderr, "Failed to activate client\n");
		jack_client_close(client);
		return 1;
	}

	/* Connect ports to physical outputs */
	ports = jack_get_ports(client, NULL, NULL,
			       JackPortIsPhysical | JackPortIsInput);
	if (ports == NULL) {
		fprintf(stderr, "No physical playback ports found\n");
		fprintf(stderr, "You may need to connect manually using:\n");
		fprintf(stderr, "  qjackctl (GUI - recommended)\n");
		fprintf(stderr, "  jack_connect sine_generator:output_left system:playback_1\n");
	} else {
		if (jack_connect(client, jack_port_name(output_port_left), ports[0])) {
			fprintf(stderr, "Cannot connect left output port\n");
		}
		if (ports[1] != NULL) {
			if (jack_connect(client, jack_port_name(output_port_right), ports[1])) {
				fprintf(stderr, "Cannot connect right output port\n");
			}
		}
		jack_free(ports);
	}

	/* Set up signal handler */
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	printf("Playing sine wave...\n");
	printf("Press Ctrl+C to stop\n\n");

	/* Wait for playback to complete */
	int last_progress = -1;
	while (!generator.should_exit) {
		usleep(100000); /* 100ms */
		
		int progress = (int)((1.0 - (double)generator.frames_remaining / generator.total_frames) * 100.0);
		if (progress != last_progress && progress <= 100) {
			printf("\rProgress: %d%%", progress);
			fflush(stdout);
			last_progress = progress;
		}
	}

	printf("\rProgress: 100%%\n");
	printf("Playback complete!\n");

	/* Cleanup */
	jack_client_close(client);

	return 0;
}
