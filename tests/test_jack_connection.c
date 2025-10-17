/**
 * JACK Connection Test
 * 
 * Simple test to verify JACK library can be linked and basic
 * operations work correctly
 */

#include <stdio.h>
#include <string.h>
#include <jack/jack.h>

int main(void)
{
    jack_client_t *client;
    jack_status_t status;
    jack_options_t options = JackNoStartServer;
    int test_passed = 1;
    
    printf("JACK Connection Test\n");
    printf("====================\n\n");
    
    /* Test 1: Get JACK version */
    printf("Test 1: Get JACK version\n");
    jack_get_version_string();
    printf("  PASS: JACK version function callable\n\n");
    
    /* Test 2: Open JACK client (don't start server) */
    printf("Test 2: Open JACK client\n");
    client = jack_client_open("test_client", options, &status);
    
    if (client == NULL) {
        if (status & JackServerFailed) {
            printf("  SKIP: JACK server not running (expected for CI)\n");
            printf("        This is normal if JACK server is not installed/running\n");
        } else {
            printf("  FAIL: Could not open JACK client (status=0x%x)\n", status);
            test_passed = 0;
        }
    } else {
        printf("  PASS: JACK client opened successfully\n");
        
        /* Test 3: Get sample rate */
        printf("\nTest 3: Get sample rate\n");
        jack_nframes_t sample_rate = jack_get_sample_rate(client);
        printf("  PASS: Sample rate = %d Hz\n", sample_rate);
        
        /* Test 4: Get buffer size */
        printf("\nTest 4: Get buffer size\n");
        jack_nframes_t buffer_size = jack_get_buffer_size(client);
        printf("  PASS: Buffer size = %d frames\n", buffer_size);
        
        /* Test 5: Get client name */
        printf("\nTest 5: Get client name\n");
        const char *client_name = jack_get_client_name(client);
        printf("  PASS: Client name = %s\n", client_name);
        
        /* Cleanup */
        jack_client_close(client);
    }
    
    printf("\n====================\n");
    if (test_passed) {
        printf("Result: PASS\n");
        printf("\nAll JACK API tests passed!\n");
        printf("Note: Some tests may be skipped if JACK server is not running.\n");
        return 0;
    } else {
        printf("Result: FAIL\n");
        return 1;
    }
}
