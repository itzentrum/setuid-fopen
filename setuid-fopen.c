/**
 * setuid-touch
 *
 * Takes a username as a parameter, looks up their uid and home dir and then
 * calls setuid(that uid) and fopen(their home dir).
 *
 * This is to trigger the automount behaviour on the home dir with the user's
 * own privileges.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main (int argc, char ** argv) {
	// Extract username from parameters
	if (argc != 2) {
		return -1;
	}
	char* username = argv[1];
	
	// Try and find out home dir and uid
	char homedir[512];
	int uid;
	// This is done by calling the dscl utility via popen()

	// Prepare the statements to be executed
	FILE* handle;
	char buffer[512];
	char command1[1024] = "dscl /Search -read /Users/";
	char command2[1024];
	
	strcat (command1, username);
	strcpy (command2, command1);
	strcat (command1, " NFSHomeDirectory | cut -f 2 -d \" \"");
	strcat (command2, " UniqueID | cut -f 2 -d \" \"");

	// Now call the utility to find out the home dir
	handle = popen (command1, "r");
	fread ( buffer, sizeof(buffer), 1, handle);
	if (ferror(handle) != 0) {
		fprintf(stderr, "error while looking up NFSHomeDirectory for user %s\n", username);
		return -2;
	}
	// Make sure to drop the leading newline
	strncpy(homedir, buffer, strlen(buffer)-1);
	strcat(homedir, "");
	pclose(handle);

	// Now call the utility to find out the uid
	handle = popen (command2, "r");
	fread (buffer, sizeof(buffer), 1, handle);
	if (ferror(handle) != 0) {
		fprintf(stderr, "error while looking up UniqueID for user %s\n", username);
		return -3;
	}
	uid = atoi(buffer);
	pclose(handle);


	// Now try to drop privileges
	if ( setuid(uid) ) {
		fprintf(stderr, "error while dropping privileges: %s\n", strerror(errno));
		return -4;
	}

	// Now call fopen() on the user's home dir to trigger automount behaviour
	if ( fopen (homedir, "r") == NULL ) {
		fprintf(stderr, "error while opening homedir: %s\n", strerror(errno));
		return -5;
	}

	return 0;
}

