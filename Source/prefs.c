// prefs.c

// NOTE THAT NONE OF THIS CODE IS ENDIAN-SAVVY.
// PREFERENCES FILES WILL NOT TRANSFER BETWEEN PLATFORMS.

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "prefs.h"

#include "hiscore.h"
#include "keyselect.h"
#include "main.h"
#include "music.h"
#include "soundfx.h"

#define kPrefsMaxSize 65536

PrefList prefList[] = {
	{ "mod ", &musicOn,   sizeof( bool       ) },
	{ "sfx ", &soundOn,   sizeof( bool       ) },
	{ "keys", playerKeys, sizeof( playerKeys ) },
	{ "high", scores,     sizeof( scores     ) },
	{ "cmbx", &best,      sizeof( best       ) }
};

#define kPrefListSize (sizeof(prefList)/sizeof(prefList[0]))

static unsigned char* FindPrefsLine( unsigned char *prefsText, long prefsLength, unsigned char *const searchCode, long dataQuantity );

void LoadPrefs( void )
{
	char *prefDir = SDL_GetPrefPath("", "CandyCrisis");
	if (prefDir == NULL) return;

	int dirFd = open(prefDir, O_RDONLY);
	if (dirFd < 0) goto cleanup_prefpath;

	int prefFd = openat(dirFd, "prefs", O_RDONLY);
	if (prefFd < 0) goto close_dir;

	FILE *prefFile = fdopen(prefFd, "r");
	if (prefFile == NULL) goto close_pref_fd;

	fseek(prefFile, 0, SEEK_END);
	long size = ftell(prefFile);
	fseek(prefFile, 0, SEEK_SET);

	unsigned char *prefData = calloc(1, size);
	if (prefData == NULL) goto close_pref;

	int bytesRead = fread(prefData, 1, size, prefFile);
	if (bytesRead != size) goto free_buf;

	for ( int count = 0; count < kPrefListSize; count++ ) {
		unsigned char *infoAt = FindPrefsLine(
			prefData, size, prefList[count].itemName, prefList[count].size
		);

		if ( infoAt ) {
			unsigned char *dataAt = prefList[count].itemPointer;
			int digitsLeft = prefList[count].size;

			while ( digitsLeft-- ) {
				unsigned char info  = ((*infoAt >= 'A')? (*infoAt - 'A' + 0xA): (*infoAt - '0')) << 4;
				infoAt++;
				info |= ((*infoAt >= 'A')? (*infoAt - 'A' + 0xA): (*infoAt - '0'));
				infoAt++;

				*dataAt++ = info;
			}
		}
	}

free_buf:
	free(prefData);
close_pref:
	// this also closes the fd, so skip that call
	fclose(prefFile);
	goto close_dir;
close_pref_fd:
	close(prefFd);
close_dir:
	close(dirFd);
cleanup_prefpath:
	SDL_free(prefDir);
	return;
}

/* Finds a specific line in the prefs. */

static unsigned char* FindPrefsLine( unsigned char *prefsText, long prefsLength, unsigned char *const searchCode, long dataQuantity )
{
	unsigned char *prefsAt, *check, *endCheck;

	for( prefsAt = prefsText; prefsAt < (prefsText+prefsLength-3); prefsAt++ )
	{
		if ( memcmp(prefsAt, searchCode, sizeof(((PrefList){0}).itemName)) == 0 ) {
			prefsAt += 6;

			// perform sizing check

			dataQuantity *= 2; // hexadecimal bytes are 2 chars

			if( ((prefsAt + dataQuantity) - prefsText) > prefsLength ) return NULL; // prefs block ended too early

			check = prefsAt;
			endCheck = check + dataQuantity;
			while( check < endCheck )
			{
				if( (*check < '0' || *check > '9') && (*check < 'A' || *check > 'F') )
				{
					return NULL; // incorrect size, too short
				}

				check++;
			}

			if( (*endCheck >= '0' && *endCheck <= '9') || (*endCheck >= 'A' && *endCheck <= 'F') )
			{
				return NULL; // incorrect size, too long
			}

			return prefsAt;
		}
	}

	return NULL;
}

/* Saves out preferences into a file. */

void SavePrefs( void )
{
	char *prefDir = SDL_GetPrefPath("", "CandyCrisis");
	if (prefDir == NULL) return;

	int dirFd = open(prefDir, O_RDONLY);
	if (dirFd < 0) goto cleanup_prefpath;

	int prefFd = openat(dirFd, "prefs", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	if (prefFd < 0) goto close_dir;

	FILE *prefFile = fdopen(prefFd, "w");
	if (prefFile == NULL) goto close_pref_fd;

	for ( int count = 0; count < kPrefListSize; count++ ) {
		fprintf( prefFile, "%.4s: ", prefList[count].itemName);

		unsigned char *dataAt = prefList[count].itemPointer;
		for ( int size = 0; size < prefList[count].size; size++ ) {
			fprintf( prefFile, "%02X", *dataAt );
			dataAt++;
		}

		fputc( '\n', prefFile );
	}

	// this also closes the fd, so skip that call
	fclose(prefFile);
	goto close_dir;
close_pref_fd:
	close(prefFd);
close_dir:
	close(dirFd);
cleanup_prefpath:
	SDL_free(prefDir);
	return;
}
