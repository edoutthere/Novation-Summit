// Summit Dump
//
// Utility to print the contents of the .syx patch file saved from the Novation 
// Summit/Peak synth.  The output is to the console and is in text.
//
// The .syx file is a system exclusive file, it contains no midi notes or events
// The .syx file can have 1 or 2 patches in it
//   the patches are structured the same
// I read the entire file into memory and then use pointers to address the bytes in
// the patch.  I took this approach rather than create one mega-struct because in
// some cases, the fields are scattered around the patch and are non-contiguous.
// There is a summary file called ByLocation.txt that has the control byte assignments
// in increasing address order.
// Note that the addressing in this program starts at the start of the patch.
// I did this because there can be 2 patches in a .syx file, the summit can load
// 2 separate patches (the bi-timbral thing).  This util can dump these milti-patches 
// too.
// 
// Dont be scared off/disappointed that this is a C++ file, there is no actual C++ here
// Thats just the way visual studio created the empty source file!
// 
// TODO:
//   find out if genre is in use and how to set it
//   work out the coding for the OSC Coarse Freq
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

#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>

#include "waveMore.h"
#include "rateSync.h"


#define		PATCH_SIZE			512
#define		PATCH_NAME_OFFSET	395		// 0x18B

// These are the major sections of the synth, most settings are grouped starting
// at these addresses
// these offsets are from the start of the patch, not the start of the file
// add 16 to convert the patch offset to the file offset
#define		VOICE_OFFSET			0x20
#define		OSC_COMMON_OFFSET		0x29
#define		OSC1_OFFSET				0x2E
#define		OSC2_OFFSET				0x43
#define		OSC3_OFFSET				0x58
#define		MIXER_OFFSET			0x6D
#define		FX_GLOBAL_OFFSET		0x79
#define		FILTER_OFFSET			0x7B
#define		AMP_ENV_OFFSET			0x90
#define		MOD_ENV_OFFSET			0x98
#define		LFO1_OFFSET				0xA9
#define		LFO2_OFFSET				0xB7
#define		DISTORTION_OFFSET		0xC5
#define		EFFECT_OFFSET			0xC7
#define		CHORUS_OFFSET			0xCA
#define		REVERB_OFFSET			0xD7
#define		DELAY_OFFSET			0xE3
#define		ARP_OFFSET				0xEB
#define		MOD_MATRIX_OFFSET		0xFA
#define		FX_MOD_MATRIX_OFFSET	0x13A
#define		LFO3_OFFSET				0x15A
#define		LFO4_OFFSET				0x15D
#define		TUNING_TABLE_OFFSET		0x160
#define		FM_OFFSET				0x166
#define		LFO_34_SEL_OFFSET		0x172
#define		LFO3_SYNC_OFFSET		0x173
#define		LFO4_SYNC_OFFSET		0x174
#define		AMP_ENV_LOOP_OFFSET		0x175
#define		FILTER_ENV_LOOP_OFFSET	0x176
#define		AMP_ENV_DELAY_OFFSET	0x178
#define		FILTER_ENV_DELAY_OFFSET	0x179


// these are the breakout functions for the major sections
void oscillator_common(unsigned char* );
void oscillator(unsigned char*, int );
void mixer(unsigned char* );
void fm(unsigned char* );
void filter(unsigned char* );
void amp_env(unsigned char*);
void mod_env(unsigned char*, unsigned char*);
void lfo1_2(unsigned char *, int);
void lfo3_4(unsigned char *, int, int);
void fx_global(unsigned char* );
void chorus(unsigned char* );
void delay(unsigned char* );
void reverb(unsigned char* );
void voice(unsigned char* );
void arp(unsigned char* );
void mod_matrix(unsigned char* );
void mod_matrix_print(unsigned char*, int);
void fx_mod_matrix(unsigned char* );

int infile;
unsigned char* filebuf;
struct stat fstatus;

// lotsa LUTs
// note that there are include files with additional LUT value as well
const char *patch_category[15] = {"None","Arp","Bass","Bell","Classic","DrumPerc","Keyboard","Lead","Motion","Pad","Poly","SFX","String","User 1","User 2"};
int osc_foot[4] = {16,8,4,2};
const char *osc_waveshape[5] = {"sin","tri","saw","sq","more"};
const char *osc_shapeSource[] = {"manual", "mod env 1", "LFO1"};
const char *fm_source[] = {"manual", "mod env 2", "LFO2"};
const char *filter_shape[] = {"LP", "BP", "HP", "dual"};
const char *lfo_range[] = {"lo","hi","sync"};
const char *lfo_wave[] = {"tri","saw","sq","sh"};
const char *lfo_fade_mode[] = {"FadeIn","FadeOut","GateIn","GateOut"};
const char *chorus_type[] = {"2 tap","4 tap","ensemble"};
const char *reverb_size[] = {"size 1","size2","size 3"};
const char *voice_type[] = {"Mono","Mono LG","Mono 2","Poly","Poly 2"};
const char *arp_type[] = {"up","down","up-down1","up-down2","played","random","chord"};
const char *mod_matrix_src[] = {"Direct  ","ModWheel","AftTouch","ExprPED1","BrthPED2","Velocity","Keyboard","Lfo1+   ","Lfo1+/- ","Lfo2+   ","Lfo2+/- ","AmpEnv  ","ModEnv1 ","ModEnv 2","Animate1","Animate2","CV +/-  ","Lfo3+   ","Lfo3+/- ","Lfo4+   ","Lfo4+/- ","BndWhl+ ","BndWhl- "};
const char *mod_matrix_dest[] = {"O123Ptch","Osc1Ptch","Osc2Ptch","Osc3Ptch","Osc1VSnc","Osc2VSnc","Osc3VSnc","Osc1Shpe","Osc2Shpe","Osc3Shpe","Osc1 Lev","Osc2 Lev","Osc3 Lev","NoiseLev","Ring Lev","VcaLevel","Filt Drv","FiltDist","FiltFreq","Filt Res","Lfo1Rate","Lfo2Rate","AmpEnv A","AmpEnv D","AmpEnv R","ModEnv1A","ModEnv1D","ModEnv1R","ModEnv2A","ModEnv2D","ModEnv2R","FM O1>O2","FM O2>O3","FM O3>O1","FM Ns>O1","O3>FiltF","Ns>FiltF","FfreqSep"};
const char *FltShpDual[] = {"LP > HP","LP > BP","HP > BP","LP + HP","LP + BP","HP + BP","LP + LP","BP + BP","HP + HP"};
const char *AudioInput[] = {"off","prefilt","postfilt"};
const char *arp_sync_rate[] = {"8 beats", "6 beats", "5 + 1/3", "4 beats", "3 beats", "2 + 2/3", "2nd 	","4th D 	","1 + 1/3", "4th 	","8th D 	","4th T 	","8th 	","16th D ","8th T 	","16th 	","16th T ","32nd 	","32nd T"};
const char *fx_mod_matrix_src[] = {"Direct  ","ModWheel","AftTouch","ExprPED1","BrthPED2","Velocity","Keyboard","Animate1","Animate2","CV +/-	 ","Lfo3+	 ","Lfo3+/- ","Lfo4+	 ","Lfo4+/- ","BndWhl+ ","BndWhl- "};
const char *fx_mod_matrix_dest[] = {"DistLev  ","Chor Lev ","Chor Rate","Chor Dep ","Chor FB  ","Del Lev  ","Del Time ","Del FB	  ","Rev Lev  ","Rev Time ","Rev LPF  ","Rev HPF  "};
const char *fx_global_routing[] = {"Parallel", "D->R->C", "D->C->R", "R->D->C", "R->C->D","C->D->R", "C->R->D"};

int main(int argc, char **argv)
{
	int i;
	unsigned char* chptr;
	unsigned char* patchPtr;
	int multi;
	char tempString[200];

	if( argc < 2 )
		{
		printf("Usage: SummitDump file\n");
		return(-1);
		}

	printf("SummitDump   Built 01/10/2022\n");
	printf("Dumping %s\n", argv[1]);

	infile = _open( argv[1], O_RDONLY);
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
		printf("Unable to malloc space for the file contents\n");
		return(-2);
	}

	printf("Reading %d bytes\n", fstatus.st_size);
	_read(infile, filebuf, fstatus.st_size);
	_close(infile);

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

	// determine if this file is a single or multi-patch
	if( *(chptr+8) == 0x02 )
		{
		multi = 1;
		printf("  Multi Patch\n");
		}
	else
		{
		multi = 0;
		printf("  Single Patch\n");
		}

	// set the data pointer to the start of the patch
	patchPtr = chptr+0x10;
	for(i=0;i<multi+1;i++ )
		{
		// print the patch number and name
		snprintf( tempString, 17, "%s\n", patchPtr+PATCH_NAME_OFFSET );
		printf("____________________________\n");
		printf("| Patch %d [%s]\n", i+1, tempString);
		printf("| Patch Category: %s\n",patch_category[*(patchPtr+0x20)] );
		printf("| Patch Genre: %X\n",*(patchPtr+0x21) );     // 0-9  Dont kno where this is set...

		oscillator_common(patchPtr+OSC_COMMON_OFFSET);

		// this setting is way out of the OSC setting grouping
		// it is directly addressed
		printf("       Tuning Table: %d\n", *(patchPtr+TUNING_TABLE_OFFSET) );

		oscillator(patchPtr+OSC1_OFFSET, 1);
		oscillator(patchPtr+OSC2_OFFSET, 2);
		oscillator(patchPtr+OSC3_OFFSET, 3);
		mixer(patchPtr+MIXER_OFFSET);
		fm(patchPtr+FM_OFFSET);
		filter(patchPtr+FILTER_OFFSET);
		amp_env(patchPtr+AMP_ENV_OFFSET);
		printf("         Loop: %s\n", *(patchPtr+AMP_ENV_LOOP_OFFSET)?"ON":"OFF");
		printf("        Delay: %d\n", (*(patchPtr+AMP_ENV_DELAY_OFFSET)&0x7F));
		mod_env(patchPtr+MOD_ENV_OFFSET, patchPtr+FILTER_ENV_LOOP_OFFSET );
		lfo1_2(patchPtr+LFO1_OFFSET, 1);
		lfo1_2(patchPtr+LFO2_OFFSET, 2);

		// LFO 3/4 have a sync bool outside the LFOs address range
		lfo3_4(patchPtr+LFO3_OFFSET, 3, *(patchPtr+LFO3_SYNC_OFFSET));
		lfo3_4(patchPtr+LFO4_OFFSET, 4, *(patchPtr+LFO4_SYNC_OFFSET));

	    fx_global(patchPtr+FX_GLOBAL_OFFSET);
		printf("\n   __________________________\n");
		printf("  | EFFECTS BYPASS: %s\n", *(patchPtr+EFFECT_OFFSET)?"ON":"OFF");
		printf("    DISTORTION: %d\n", *(patchPtr+DISTORTION_OFFSET));
		chorus(patchPtr+CHORUS_OFFSET);
		delay(patchPtr+DELAY_OFFSET);
		reverb(patchPtr+REVERB_OFFSET);
		voice(patchPtr+VOICE_OFFSET);
		arp(patchPtr+ARP_OFFSET);
		mod_matrix(patchPtr+MOD_MATRIX_OFFSET);
		fx_mod_matrix(patchPtr+FX_MOD_MATRIX_OFFSET);

		// next patch
		patchPtr += PATCH_SIZE;
		}
}

//-----------------------------------------------------------------------------------------
// Osc common settings
// 
// These are only available through the menu system
//-----------------------------------------------------------------------------------------
void oscillator_common(unsigned char* arg)
{
	printf("   __________________________\n");
	printf("  | OSC COMMON\n");
	printf("            Diverge: %d\n", (*(arg+0)&0x7F));
	printf("              Drift: %d\n", (*(arg+1)&0x7F));
	printf("          Noise LPF: %d\n", (*(arg+2)&0x7F));
	printf("          Noise HPF: %d\n", (*(arg+3)&0x7F));
	printf("               Sync: %s\n", *(arg+4)?"ON":"OFF");
}

//-----------------------------------------------------------------------------------------
// Oscillator Section
// 
// Print the oscillator settings
// the arg is set to the start of an osc settings area in memory
// 
//-----------------------------------------------------------------------------------------
void oscillator(unsigned char* arg, int num)
{
	printf("   __________________________\n");
	printf("  | OSC%d\n",num);
	//printf("      ",);
	printf("                Octave: %d'\n",osc_foot[(*arg)-0x3F] );
	//printf("      Coarse Freq: %X %X  %d\n", *(arg+1), *(arg+2), ((*(arg+1)&0x7F)<<1) + (*(arg+2)>>6));  // TODO resolve this value calc/lookup
	printf("           Coarse Freq: %d!\n", ((*(arg+1)&0x7F)<<1) + (*(arg+2)>>6));
	//printf("      Coarse Freq: %2.1f\n", (float)((((*(arg+1)&0x7F)<<1) + (*(arg+2)>>6))-127)/10.0 );
	printf("             Fine Freq: %d\n", ((*(arg+3)&0x7F)<<1) + (*(arg+4)>>6) );
	printf("      Pitch: Mod ENV 2: %d\n", (*(arg+5)&0x7F)-64);
	printf("           Pitch: LFO2: %d\n", (((*(arg+6)&0x7F)<<1) + (*(arg+7)>>6))-128 );
	if( *(arg+8) == 4 )
		printf("                  Wave: more [%s]\n",osc_waveMore[(*(arg+9))-4] );
	else
		printf("                  Wave: %s\n",osc_waveshape[*(arg+8)] );
	printf("          Shape Source: %s\n",osc_shapeSource[*(arg+10)] );
	printf("                 Shape: %d (Manual)\n", (*(arg+11)&0x7F)-64);
	printf("                 Shape: %d (Mod Env 1)\n", (*(arg+12)&0x7F)-64);
	printf("                 Shape: %d (LFO1)\n", (*(arg+13)&0x7F)-64);
	printf("                 Vsync; %d\n", *(arg+14)&0x7F);
	printf("              Sawdense; %d\n", *(arg+15)&0x7F);
	printf("              DenseDet; %d\n", *(arg+16)&0x7F);
	printf("            fixed note; %d\n", *(arg+17)&0x7F);
	printf("            Bend Range; %d\n", *(arg+18)&0x7F);

}

//-----------------------------------------------------------------------------------------
// Mixer Section
// 
// Print the mixer settings
// the arg is set to the start of the settings area in memory
// assumes that all the settings are grouped
// 
//-----------------------------------------------------------------------------------------
void mixer(unsigned char* arg)
{
	printf("   __________________________\n");
	printf("  | MIXER\n");
	//printf("      ",);
	printf("          OSC1 Level: %d\n",(((*arg)&0x7F)<<1) + (*(arg+1)>>6) );
	printf("          OSC2 Level: %d\n",(((*(arg+2))&0x7F)<<1) + (*(arg+3)>>6) );
	printf("          OSC3 Level: %d\n",(((*(arg+4))&0x7F)<<1) + (*(arg+5)>>6) );
	printf("         Noise Level: %d\n",(((*(arg+8))&0x7F)<<1) + (*(arg+9)>>6) );
	printf("      Ring 1-2 Level: %d\n",(((*(arg+6))&0x7F)<<1) + (*(arg+7)>>6) );
	printf("            VCA Gain: %d\n", *(arg+11)&0x7F);

}

//-----------------------------------------------------------------------------------------
// FM Section
// 
// Novation does not document this section very well
// The support responses did not explain how it worked either
//   they just parrot the inadequate the user manual diagram
// 
// The FM section has 3 sections 
// Each section is assigned to a dedicated osc x modulating osc y
//   The modulator and carrier selection is fixed
// Each section has 3 modulation depth sources
// Each of the selected sources (Manual, Mod Env 2, LFO2) have dedicated level values
//   The default value for each of these 3 is 0, no effect
// These 3 values control how much the modulator modulates the carrier
//   The amount of the modulator that is applied to the carrier is controlled by these
//   three values ADDED UP
//   The sources button only selects which one of these the knob is adjusting
// Manual: the knob sets the fixed amount that controls the amount of modulation
// Mod Env 2: the knob sets the amount of Mod Env 2 that controls the amount of modulation
// LFO 2: the knob sets the amount of LFO 2 that controls the amount of modulation
// These 3 are additive, the 3 settings are summed then controls the level of the
// modulator that is applied to the carrier
// In a more geeker description, the modulator OSC output passes through a voltage controlled
// pot before actually connecting to the modulation devices modulator input.  The 3 modulation
// depth sources are added then that combined output controls the voltage controlled pot
// value.
//-----------------------------------------------------------------------------------------
void fm(unsigned char* arg)
{
	printf("   __________________________\n");
	printf("  | FM\n");
	printf("      OSC 3 > OSC 1\n");
	printf("             Source: %s\n",fm_source[*(arg+0)] );
	printf("             Manual: %d\n", *(arg+1)&0x7F);
	printf("          Mod Env 2: %d\n", *(arg+2)&0x7F);
	printf("              LFO 2: %d\n", *(arg+3)&0x7F);
	printf("      OSC 1 > OSC 2\n");
	printf("             Source: %s\n",fm_source[*(arg+4)] );
	printf("             Manual: %d\n", *(arg+5)&0x7F);
	printf("          Mod Env 2: %d\n", *(arg+6)&0x7F);
	printf("              LFO 2: %d\n", *(arg+7)&0x7F);
	printf("      OSC 2 > OSC 3\n");
	printf("             Source: %s\n",fm_source[*(arg+8)] );
	printf("             Manual: %d\n", *(arg+9)&0x7F);
	printf("          Mod Env 2: %d\n", *(arg+10)&0x7F);
	printf("              LFO 2: %d\n", *(arg+11)&0x7F);
}

//-----------------------------------------------------------------------------------------
// Filter Section
// Note: some filter functions are controlled in the VOICE menu
//-----------------------------------------------------------------------------------------
void filter(unsigned char* arg)
{
	printf("   __________________________\n");
	printf("  | FILTER\n");
	printf("           LFO1 Depth: %d\n", (((*(arg+8))&0x7F)<<1) + (*(arg+9)>>6));
	printf("     OSC 3 filter mod: %d\n", (*(arg+10)&0x7F));
	printf("            Key Track: %d\n", (*(arg+4)&0x7F));
	printf("             Selected: %s\n", *(arg+11)?"mod env1":"amp env");
	printf("             Amp Env : %d\n", (*(arg+12)&0x7F)-64);
	printf("           Mod Env 1 : %d\n", (*(arg+13)&0x7F)-64);
	printf("            Resonance: %d\n", (*(arg+5)&0x7F));
	printf("         Filter Slope: %d\n", *(arg+2)?24:12);
	printf("                 Freq: %d\n",(((*(arg+6))&0x7F)<<1) + (*(arg+7)>>6) );
	printf("                Shape: %s\n",filter_shape[*(arg+3)] );
	printf("            Overdrive: %d\n", (*(arg+0)&0x7F));

}

//-----------------------------------------------------------------------------------------
// AMP Env Section
// 
//-----------------------------------------------------------------------------------------
void amp_env(unsigned char* arg)
{
	printf("   __________________________\n");
	printf("  | AMP ENVELOPE\n");
	printf("       Attack: %d\n", (*(arg+0)&0x7F));
	printf("        Decay: %d\n", (*(arg+1)&0x7F));
	printf("      Sustain: %d\n", (*(arg+2)&0x7F));
	printf("      Release: %d\n", (*(arg+3)&0x7F));
}

//-----------------------------------------------------------------------------------------
// MOD Env Section
// Note: the velocity, monotrig, Hold Time, repeats values are menu items 
//-----------------------------------------------------------------------------------------
void mod_env(unsigned char* arg, unsigned char* extra)
{
	printf("   __________________________\n");
	printf("  | MOD ENVELOPES\n");
	printf("      1: %s\n",*(arg+0)?" ":"Selected");
	printf("       Attack: %d\n", (*(arg+1)&0x7F));
	printf("        Decay: %d\n", (*(arg+2)&0x7F));
	printf("      Sustain: %d\n", (*(arg+3)&0x7F));
	printf("      Release: %d\n", (*(arg+4)&0x7F));
	printf("     Velocity; %d\n", (*(arg+5)&0x7F));
	printf("      Trigger; %d\n", (*(arg+6)&0x7F));
	printf("    Hold Time; %d\n", (*(arg+7)&0x7F));
	printf("      Repeats; %d\n", (*(arg+8)&0x7F));
	printf("         Loop: %s\n", *(extra+0)?"on":"off" );
	printf("        Delay; %d\n", (*(extra+3)&0x7F));

	printf("      2: %s\n",*(arg+0)?"Selected":" ");
	printf("       Attack: %d\n", (*(arg+9)&0x7F));
	printf("        Decay: %d\n", (*(arg+10)&0x7F));
	printf("      Sustain: %d\n", (*(arg+11)&0x7F));
	printf("      Release: %d\n", (*(arg+12)&0x7F));
	printf("     Velocity; %d\n", (*(arg+13)&0x7F));
	printf("      Trigger; %d\n", (*(arg+14)&0x7F));
	printf("    Hold Time; %d\n", (*(arg+15)&0x7F));
	printf("      Repeats; %d\n", (*(arg+16)&0x7F));
	printf("         Loop: %s\n", *(extra+1)?"on":"off" );
	printf("        Delay; %d\n", (*(extra+4)&0x7F));
}



//-----------------------------------------------------------------------------------------
//  LFO 1, 2
//
// Note: address 0xC2 is unassigned
//-----------------------------------------------------------------------------------------
void lfo1_2(unsigned char*arg, int num)
{
	printf("   __________________________\n");
	printf("  | LFO %d\n", num);
	printf("       Wave type: %s\n",lfo_wave[*(arg+4)] );
	printf("       Fade time: %d\n",*(arg+7) );
	printf("           Range: %s\n",lfo_range[*arg] );

	if( *arg == 2 )  // range=sync
		{
		if( ((*(arg+3))&0x7F) <= 0x22 )
			printf("       Rate sync: %s\n", rateSync[ ((*(arg+3))&0x7F) ] );
		else
			printf("       ERROR: LFO Rate Sync value %d is > 34\n", ((*(arg+3))&0x7F) );
		}
	else
		printf("            Rate: %d\n",(((*(arg+1))&0x7F)<<1) + (*(arg+2)>>6) );
	printf("           Phase: %d\n",*(arg+5)*3 );
	printf("            Slew: %d\n",*(arg+6) );
	printf("           Range: %s\n",lfo_fade_mode[*arg+8] );
	printf("            Sync: %s\n", *(arg+9)?"on":"off");
	printf("         Repeats: %d\n",*(arg+10) );
	printf("     Common Sync: %s\n", *(arg+11)?"Retrig":"Legato");
	printf("     Common Sync: %s\n", *(arg+12)?"on":"off");
}

//-----------------------------------------------------------------------------------------
//
// LFO 3 and 4 
// Note: settings are in 2 places
//-----------------------------------------------------------------------------------------
void lfo3_4(unsigned char *arg, int num, int sync)
{
	printf("   __________________________\n");
	printf("  | LFO %d\n", num);
	printf("       Wave type: %s\n",lfo_wave[*(arg+0)] );
	if( sync == 1 )
		{
		if( ((*(arg+2))&0x7F) <= 0x22 )
			printf("       Rate sync: %s\n", rateSync[ ((*(arg+2))&0x7F) ] );
		else
			printf("ERROR: LFO Rate Sync value is > 34:  %d\n", ((*(arg+2))&0x7F) );
		}
	else
		printf("            Rate: %d\n",((*(arg+1))&0x7F ) );
}

void fx_global(unsigned char* arg)
{
	printf("   __________________________\n");
	printf("  | FX GLOBAL Menu\n");
	printf("      WetLevel: %d\n", (*(arg+1)&0x7F));
	printf("      DryLevel: %d\n", (*(arg+0)&0x7F));
	printf("       Routing: %s\n",fx_global_routing[*(arg+79)] );
}


void chorus(unsigned char* arg)
{
	printf("   __________________________\n");
	printf("  | CHORUS\n");
	printf("       Level: %d\n", (*(arg+0)&0x7F));
	printf("        Type: %s\n",chorus_type[*(arg+1)] );
	printf("        Rate: %d\n", (*(arg+2)&0x7F));
	// menu items
	printf("        ChorDepth: %d\n", (*(arg+3)&0x7F));
	printf("        ChorFback: %d\n", (*(arg+4)&0x7F));
	printf("        LoPass: %d\n", (*(arg+5)&0x7F));
	printf("        HiPass: %d\n", (*(arg+6)&0x7F));
}

void delay(unsigned char* arg)
{
	printf("   __________________________\n");
	printf("  | DELAY\n");
	printf("      Feedback: %d\n", (*(arg+6)&0x7F));
	printf("          Time: %d\n", (*(arg+1)&0x7F));
	printf("          Sync: %s\n", *(arg+4)?"ON":"OFF");
	printf("     Sync Time: %d\n", (*(arg+5)&0x7F));
	printf("         Level: %d\n", (*(arg+0)&0x7F));
    // menu items
	printf("       LP damp: %d\n", (*(arg+7)&0x7F));
	printf("       HP damp: %d\n", (*(arg+8)&0x7F));
	printf("     L/R ratio: %d\n", (*(arg+2)&0x7F));
	printf("      Slewrate: %d\n", (*(arg+9)&0x7F));
	printf("         Width: %d\n", (*(arg+3)&0x7F));
}

void reverb(unsigned char* arg)
{
	printf("   __________________________\n");
	printf("  | REVERB\n");
	printf("          Time: %d\n", (*(arg+2)&0x7F));
	printf("          Size: %s\n",reverb_size[*(arg+1)]);
	printf("         Level: %d\n", (*(arg+0)&0x7F));
	printf("       LP Damp: %d\n", (*(arg+3)&0x7F));
	printf("       HP Damp: %d\n", (*(arg+4)&0x7F));
	printf("       RevSize: %d\n", (*(arg+5)&0x7F));	
	printf("       ModDepth: %d\n", (*(arg+6)&0x7F));
	printf("       ModRate: %d\n", (*(arg+7)&0x7F));
	printf("       Lo Pass: %d\n", (*(arg+8)&0x7F));
	printf("       Hi Pass: %d\n", (*(arg+9)&0x7F));
	printf("     Pre Delay: %d\n", (*(arg+10)&0x7F));
}

void voice(unsigned char* arg)
{
	unsigned char *argp;

	printf("   __________________________\n");
	printf("  | VOICE\n");
	printf("            Mode: %s\n",voice_type[*(arg+0)]);
	printf("           Glide: %s\n", *(arg+7)?"ON":"OFF");
	printf("      Glide time: %d\n", (*(arg+5)&0x7F));
	printf("          Unison: %d\n", (*(arg+1)&0x7F));
	printf("       Unidetune: %d\n", (*(arg+2)&0x7F));
	printf("          Spread: %d\n", (*(arg+3)&0x7F));
	printf(" Keyboard Octave: %d\n", (*(arg+4)&0x7F));  // TODO: need lut 61-67
	printf("        Preglide: %d\n", (*(arg+6)&0x7F));

	// voice has several sections in it
	argp = arg + 87;
	printf("      PatchLevel: %d\n", (*(argp)&0x7F));
	printf("     FiltPostDrv: %d\n", (*(argp+5)&0x7F));
	argp = arg + 105;
	printf("     FiltDiverge: %d\n", (*(argp)&0x7F));
	argp = arg + 323;
	printf("     FiltShpDual: %s\n", FltShpDual[ (*(argp)&0xF)] );
	printf("      FiltShpSep: %d\n", (*(arg+1)&0x7F)-64);
	printf("      AudioInput: %s\n", AudioInput[(*(argp+2)&0x03)] );

}

void arp(unsigned char* arg)
{
	printf("   __________________________\n");
	printf("  | ARP   %s\n", *(arg+8)?"ON":"OFF");
	printf("          Tempo: %d\n",(((*(arg+0))&0x7F)<<1) + (*(arg+1)>>6) );
	printf("      Sync Rate: %s\n",arp_sync_rate[18-(*(arg+2))]);
	printf("           Type: %s\n",arp_type[*(arg+3)]);
	printf("         Rhythm: %d\n", (*(arg+4)&0x7F));
	printf("         Octave: %d\n", (*(arg+5)&0x7F)+1);
	printf("           Gate: %d\n", (*(arg+6)&0x7F));
	printf("          Swing: %d\n", (*(arg+7)&0x7F));
	printf("      Key Latch: %s\n", *(arg+9)?"ON":"OFF");
	printf("       Key Sync: %s\n", *(arg+10)?"ON":"OFF");
	printf("     ArpVelMode: %s\n", *(arg+11)?"rhythm":"played");
}

void mod_matrix(unsigned char* arg)
{
	int i;

	printf("   __________________________\n");
	printf("  | MOD MATRIX\n\n");

	// the settings are displayed in a 4x4 array for compactness
	for(i=0;i<4;i++)
		mod_matrix_print(arg+(i*16), i*4);

}

//-----------------------------------------------------------------------------------------
// MOD Matrix value print
// 
// Function to print a row of mod matrix values
// There are several variable width values and this code handles keeping
// the values grouped in a visually pleasing way
//-----------------------------------------------------------------------------------------
void mod_matrix_print(unsigned char* arg, int slot)
{
	int i, depth;

	for(i=0;i<4;i++)
		{
		if( slot > 7 && i > 0 )
			printf("SLot %d            ", slot+i+1);
		else if( slot > 11 )
			printf("SLot %d            ", slot+i+1);
		else
			printf("Slot %d             ", slot+i+1);
		}
	printf("\n");

	for(i=0;i<4;i++)
		printf("   src1: %s  ",mod_matrix_src[*((arg+0)+(i*4))]);
	printf("\n");

	for(i=0;i<4;i++)
		printf("   src2: %s  ",mod_matrix_src[*((arg+1)+(i*4))]);
	printf("\n");

	// depth string has to be built because the number value can be
	// 1 or 2 digits and there are negative values too
	for(i=0;i<4;i++)
		{
		depth = *((arg+2)+(i*4))&0x7F;
		if( depth<55 )  // negative 2-digit value: 3 chars
			printf("  depth: %d       ", depth-64 );
		else if( depth>=55 && depth<64 )  // negative 1-digit value: 2 chars
			printf("  depth: %d        ", depth-64 );
		if( depth>=64 && depth<=73 )  // positive 1-digit value: 1 char
			printf("  depth: %d         ", depth-64 );
		if( depth>73 )  // positive 2-digit value: 2 chars
			printf("  depth: %d        ", depth-64 );
		}
	printf("\n");

	for(i=0;i<4;i++)
		printf("   dest: %s  ",mod_matrix_dest[*(arg+3+(i*4))]);
	printf("\n\n");
}

void fx_mod_matrix(unsigned char* arg)
{
	int i;

	printf("   __________________________\n");
	printf("  | FX MOD MATRIX\n\n");

	for(i=0;i<4;i++)
		{
		printf("    SLot %d:   ", i+1);
		printf("%s : %s > %d > %s\n",fx_mod_matrix_src[*((arg+0)+(i*4))], fx_mod_matrix_src[*((arg+1)+(i*4))], *((arg+2)+(i*4))-64, fx_mod_matrix_dest[*((arg+3)+(i*4))] );
		}
}

/*  LITTERBOX

// binary values
printf("value: %s\n", *(arg+0)?"ON":"OFF");

// values indexed into a table of strings
printf("value: %s\n",string_table[*(arg+0)] );

// unsigned value 0-127
printf("value: %d\n", (*(arg+0)&0x7F));

// signed value -64 - +63
printf("value: %d\n", (*(arg+0)&0x7F)-64);

// unsigned values 0-255
printf("value: %d\n",(((*arg+0)&0x7F)<<1) + (*(arg+1)>>6) );

// signed value -128 - +127
printf("value: %d\n", (((*(arg+0)&0x7F)<<1) + (*(arg+1)>>6))-128 );

// 2-byte debugging
printf("value: %X  %X\n",*(arg+0), *(arg+1) );

*/