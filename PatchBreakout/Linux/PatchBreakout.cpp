// Summit Patch Breakout (Linux version)
//
// This utility converts the all bank patch file into individual patch files
// The patches that are created are named by thier patch name
//
// Summit patches are stored in MIDI system exclusive "messages".  These are
// used to encapsulate non-midi information, and in the case of the summit
// the patch settings.  You can save a single patch at a time using the
// Components utility from Novation.  You can save an entire bank of patches
// as well.  It will create a syx file that is 67456 bytes in size.
// This file is the input file for this utility.
//
// 01/13/2022   edouththere
//              created
//
// TODO:
//
//  Copyright (C) 2022  Ed Out There  edoutthere@outlook.com
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.
//-------------------------------------------------------------------------------------

// Needed to make VS ignore the "unsafeness" of posix calls
#define _CRT_NONSTDC_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

#define		PATCH_SIZE		527

int isValidFileName( unsigned char *, int );

int infile, patchFile;
unsigned char* filebuf;
struct stat fstatus;


int main(int argc, char **argv)
{
    int patchCount = 0;
    int special = 1;
	unsigned char* chptr;
	unsigned char* patchPtr;
	unsigned char* strptr;
	unsigned char patchName[21];

	if( argc < 2 )
		{
		printf("Usage: PatchBreakout infile\n");
		return(-1);
		}

	printf("PatchBreakout  Built 01/13/2022\n");
	printf("input: %s\n", argv[1]);

	infile = open( argv[1], O_RDONLY);
	if (infile == -1)
	{
		printf("cant open input file %s\n", argv[1]);
		return(-1);
	}

	// read the entire file into memory
	fstat(infile, &fstatus);
	filebuf = (unsigned char*)malloc(fstatus.st_size);
	if (filebuf == NULL)
    	{
		printf("ERROR: Unable to malloc space for the file contents, exiting\n");
		return(-2);
    	}

	printf("  Reading %d bytes\n", fstatus.st_size);
	read(infile, filebuf, fstatus.st_size);
	close(infile);

	chptr = filebuf;  // start of file

	// the file header is the first 16 bytes
	// is this a valid syx file
	if( *chptr == 0xf0 && *(chptr+1) == 0 && *(chptr+fstatus.st_size-1) == 0xF7 )
		printf("  Valid syx file\n");
	else
		{
		printf("  not a valid syx file structure\n");
		return(0);
		}

	// set the data pointer to the start of the input file
	patchPtr = filebuf;
    while(1)
        {
        //printf("::%X\n", (unsigned long)patchPtr );
        // get the name of the current patch
        strncpy( (char *)patchName, (const char *)(patchPtr+16), 16 );

        // patch names are allocated 16 chars in a patch
        // remove the trailing space chars from the name
        strptr = &patchName[15];
        while( *strptr == 0x20 )
            *strptr-- = 0;
        patchName[16] = 0;  // for the patch names that are 16 chars long

        // check that the patch name is a valid file name
        // if it has any funky chars, just call it "special n"
        if( isValidFileName( patchName, strlen( (const char *)patchName ) ) == 0 )
            sprintf( (char *)patchName, "Special %d", special++ );

        // the output files are MIDI system exclusive files
        strcat( (char *)patchName, ".syx" );
        printf("Creating Patch file : %s\n", patchName);
        patchCount++;

    	patchFile = open( (const char *)patchName, O_CREAT|O_RDWR, 438);
    	if (patchFile == -1)
        	{
            printf("%s\n", strerror(errno) );
    		printf("cant create output patch file [%s]\n", patchName);
    		return(-1);
      	    }
       	write(patchFile, patchPtr, PATCH_SIZE);
        printf("write: %s\n", strerror(errno) );
    	close(patchFile);

		// next patch
		patchPtr += PATCH_SIZE;
        if( patchPtr > chptr+fstatus.st_size-1 )
            break;
		}
    printf("Wrote %d patch files\n", patchCount );
}

// function to validate a file name
int isValidFileName( unsigned char *arg, int len )
{
    int i;
    // for each char in the argument, test if it is a valid file name char
    // real strict here, not going into converting patch names...
    for(i=0;i<len;i++)
        {
        if( !(isalnum(arg[i]) || isblank(arg[i])) )
            return(0);
        }
    return(1);

}


