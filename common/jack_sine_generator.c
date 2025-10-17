/**
 * JACK2 Sine Wave Generator Application
 * 
 * Cross-platform sine wave generator using JACK Audio Connection Kit
 * Works on Linux, Windows, and macOS with JACK2 installed
 * 
 * Usage: ./jack_sine_generator [frequency] [duration_seconds]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <jack/jack.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DEFAULT_FREQUENCY 440.0
#define DEFAULT_DURATION 10

typedef struct {
    jack_client_t *client;
    jack_port_t *output_port_left;
    jack_port_t *output_port_right;
    double phase;
    double frequency;
    double amplitude;
    int is_running;
    unsigned long frames_processed;
    unsigned long target_frames;
} sine_generator_t;

static sine_generator_t generator;

/**
 * JACK process callback - called by JACK server in real-time thread
 */
static int process_callback(jack_nframes_t nframes, void *arg)
{
    sine_generator_t *gen = (sine_generator_t *)arg;
    jack_default_audio_sample_t *out_left, *out_right;
    unsigned int i;
    
    /* Get output buffers */
    out_left = jack_port_get_buffer(gen->output_port_left, nframes);
    out_right = jack_port_get_buffer(gen->output_port_right, nframes);
    
    /* Check if we've reached target duration */
    if (gen->target_frames > 0 && gen->frames_processed >= gen->target_frames) {
        /* Output silence */
        memset(out_left, 0, sizeof(jack_default_audio_sample_t) * nframes);
        memset(out_right, 0, sizeof(jack_default_audio_sample_t) * nframes);
        gen->is_running = 0;
        return 0;
    }
    
    /* Generate sine wave */
    jack_nframes_t sample_rate = jack_get_sample_rate(gen->client);
    double phase_increment = 2.0 * M_PI * gen->frequency / (double)sample_rate;
    
    for (i = 0; i < nframes; i++) {
        /* Check frame limit */
        if (gen->target_frames > 0 && gen->frames_processed >= gen->target_frames) {
            /* Fill rest with silence */
            while (i < nframes) {
                out_left[i] = 0.0f;
                out_right[i] = 0.0f;
                i++;
            }
            gen->is_running = 0;
            break;
        }
        
        /* Generate sample */
        float sample = (float)(gen->amplitude * sin(gen->phase));
        out_left[i] = sample;
        out_right[i] = sample;
        
        /* Update phase */
        gen->phase += phase_increment;
        if (gen->phase >= 2.0 * M_PI) {
            gen->phase -= 2.0 * M_PI;
        }
        
        gen->frames_processed++;
    }
    
    return 0;
}

/**
 * JACK shutdown callback
 */
static void jack_shutdown_callback(void *arg)
{
    sine_generator_t *gen = (sine_generator_t *)arg;
    gen->is_running = 0;
    fprintf(stderr, "JACK server shutdown\n");
}

/**
 * Signal handler for graceful shutdown
 */
static void signal_handler(int sig)
{
    generator.is_running = 0;
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
            fprintf(stderr, "Valid range: 0.1 - 20000 Hz\n");
            return 1;
        }
    }
    if (argc >= 3) {
        duration = atoi(argv[2]);
        if (duration < 0 || duration > 3600) {
            fprintf(stderr, "Invalid duration: %d seconds\n", duration);
            fprintf(stderr, "Valid range: 0 - 3600 seconds (0 = infinite)\n");
            return 1;
        }
    }
    
    printf("JACK2 Sine Wave Generator\n");
    printf("=========================\n");
    printf("Frequency: %.2f Hz\n", frequency);
    if (duration > 0) {
        printf("Duration: %d seconds\n", duration);
    } else {
        printf("Duration: infinite (press Ctrl+C to stop)\n");
    }
    printf("\n");
    
    /* Initialize generator structure */
    memset(&generator, 0, sizeof(generator));
    generator.frequency = frequency;
    generator.amplitude = 0.5;
    generator.is_running = 1;
    generator.phase = 0.0;
    generator.frames_processed = 0;
    
    /* Open JACK client */
    generator.client = jack_client_open("sine_generator", JackNullOption, &status);
    if (generator.client == NULL) {
        fprintf(stderr, "Failed to open JACK client\n");
        if (status & JackServerFailed) {
            fprintf(stderr, "Unable to connect to JACK server\n");
            fprintf(stderr, "\nPlease ensure JACK server is running:\n");
            fprintf(stderr, "  Linux:   jackd -d alsa\n");
            fprintf(stderr, "  macOS:   jackd -d coreaudio\n");
            fprintf(stderr, "  Windows: jackd -d portaudio\n");
            fprintf(stderr, "\nOr use QjackCtl GUI to start JACK server\n");
        }
        return 1;
    }
    
    if (status & JackNameNotUnique) {
        const char *client_name = jack_get_client_name(generator.client);
        printf("Warning: Client name already in use, assigned: %s\n", client_name);
    }
    
    /* Get sample rate */
    jack_nframes_t sample_rate = jack_get_sample_rate(generator.client);
    printf("JACK Sample Rate: %d Hz\n", sample_rate);
    printf("JACK Buffer Size: %d frames\n", jack_get_buffer_size(generator.client));
    printf("\n");
    
    /* Calculate target frames if duration specified */
    if (duration > 0) {
        generator.target_frames = (unsigned long)duration * sample_rate;
    }
    
    /* Set callbacks */
    jack_set_process_callback(generator.client, process_callback, &generator);
    jack_on_shutdown(generator.client, jack_shutdown_callback, &generator);
    
    /* Create output ports */
    generator.output_port_left = jack_port_register(generator.client, "output_left",
                                                     JACK_DEFAULT_AUDIO_TYPE,
                                                     JackPortIsOutput, 0);
    generator.output_port_right = jack_port_register(generator.client, "output_right",
                                                      JACK_DEFAULT_AUDIO_TYPE,
                                                      JackPortIsOutput, 0);
    
    if (generator.output_port_left == NULL || generator.output_port_right == NULL) {
        fprintf(stderr, "Failed to create output ports\n");
        jack_client_close(generator.client);
        return 1;
    }
    
    /* Activate client */
    if (jack_activate(generator.client)) {
        fprintf(stderr, "Failed to activate JACK client\n");
        jack_client_close(generator.client);
        return 1;
    }
    
    printf("JACK client activated\n");
    printf("Output ports: sine_generator:output_left, sine_generator:output_right\n");
    printf("\n");
    
    /* Auto-connect to system playback ports if available */
    ports = jack_get_ports(generator.client, NULL, NULL,
                          JackPortIsPhysical | JackPortIsInput);
    if (ports != NULL) {
        int connected = 0;
        
        if (ports[0] != NULL) {
            if (jack_connect(generator.client, 
                           jack_port_name(generator.output_port_left), 
                           ports[0]) == 0) {
                printf("Connected output_left to %s\n", ports[0]);
                connected++;
            }
        }
        
        if (ports[1] != NULL) {
            if (jack_connect(generator.client,
                           jack_port_name(generator.output_port_right),
                           ports[1]) == 0) {
                printf("Connected output_right to %s\n", ports[1]);
                connected++;
            }
        } else if (ports[0] != NULL && connected == 1) {
            /* Mono output - connect both to the same port */
            if (jack_connect(generator.client,
                           jack_port_name(generator.output_port_right),
                           ports[0]) == 0) {
                printf("Connected output_right to %s (mono)\n", ports[0]);
                connected++;
            }
        }
        
        if (connected == 0) {
            printf("Warning: Could not auto-connect to playback ports\n");
            printf("Use a JACK patchbay (qjackctl) to manually connect\n");
        }
        
        jack_free(ports);
    } else {
        printf("Warning: No physical playback ports found\n");
        printf("Use a JACK patchbay (qjackctl) to manually connect\n");
    }
    
    printf("\n");
    printf("Generating sine wave... ");
    if (duration > 0) {
        printf("(running for %d seconds)\n", duration);
    } else {
        printf("(press Ctrl+C to stop)\n");
    }
    
    /* Setup signal handler */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    /* Main loop - wait until stopped */
    while (generator.is_running) {
        sleep(1);
        
        /* Print progress */
        if (duration > 0) {
            float progress = (float)generator.frames_processed / generator.target_frames * 100.0f;
            if (progress <= 100.0f) {
                printf("\rProgress: %.1f%%", progress);
                fflush(stdout);
            }
        }
    }
    
    if (duration > 0) {
        printf("\rProgress: 100.0%%\n");
    } else {
        printf("\n");
    }
    
    printf("Stopping...\n");
    
    /* Cleanup */
    jack_deactivate(generator.client);
    jack_client_close(generator.client);
    
    printf("JACK client closed\n");
    printf("Playback complete!\n");
    
    return 0;
}
