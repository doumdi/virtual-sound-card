/**
 * Test JACK2 Library Availability
 * 
 * This test verifies that JACK2 libraries are available and can be linked.
 * It does NOT require a running JACK server.
 */

#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_JACK
#include <jack/jack.h>

int main(void)
{
	printf("Testing JACK2 library availability...\n");
	
	/* Test that we can call JACK API functions */
	printf("  JACK library version: %d.%d.%d\n",
	       jack_get_version_major(),
	       jack_get_version_minor(),
	       jack_get_version_micro());
	
	/* Note: We don't try to connect to a JACK server here */
	/* as that would require a running server */
	
	printf("\nJACK2 library test PASSED\n");
	printf("JACK2 is available and can be used\n");
	
	return 0;
}

#else

int main(void)
{
	printf("Testing JACK2 library availability...\n");
	printf("  JACK2 library is not available (not compiled with JACK support)\n");
	printf("\nThis is OK - JACK2 is optional\n");
	printf("To enable JACK2 support:\n");
	printf("  Linux:   Install libjack-jackd2-dev\n");
	printf("  macOS:   brew install jack\n");
	printf("  Windows: Download from https://jackaudio.org/\n");
	
	return 0;
}

#endif
