This is the Summit Dump utility

It is a command line utility to dump the summit (and peak) .syx patch files
The output is in text to the console

Windows:  .\SummitDump .\patch.syx
Linux:    ./summitdump ./patch.syx

The windows exe was built using visual studio 19 on windows 10
The linux executable was built on a 64-bit CentOS distro

Example Output:

SummitDump   Built 01/10/2022
Dumping .\single.syx
Reading 527 bytes
  Valid syx file
  Single Patch
____________________________
| Patch 1 [Init Patch      ]
| Patch Category: Bell
| Patch Genre: 0
   __________________________
  | OSC COMMON
            Diverge: 0
              Drift: 0
          Noise LPF: 127
          Noise HPF: 0
               Sync: OFF
       Tuning Table: 0
   __________________________
  | OSC1
                Octave: 8'
           Coarse Freq: 128!
             Fine Freq: 128
      Pitch: Mod ENV 2: 0
           Pitch: LFO2: 0
                  Wave: saw
          Shape Source: manual
                 Shape: 0 (Manual)
                 Shape: 0 (Mod Env 1)
                 Shape: 0 (LFO1)
                 Vsync; 0
              Sawdense; 0
              DenseDet; 64
            fixed note; 0
            Bend Range; 76
   __________________________
  | OSC2
                Octave: 8'
           Coarse Freq: 128!
             Fine Freq: 128
      Pitch: Mod ENV 2: 0
           Pitch: LFO2: 0
                  Wave: saw
          Shape Source: manual
                 Shape: 0 (Manual)
                 Shape: 0 (Mod Env 1)
                 Shape: 0 (LFO1)
                 Vsync; 0
              Sawdense; 0
              DenseDet; 64
            fixed note; 0
            Bend Range; 76
   __________________________
  | OSC3
                Octave: 8'
           Coarse Freq: 128!
             Fine Freq: 128
      Pitch: Mod ENV 2: 0
           Pitch: LFO2: 0
                  Wave: saw
          Shape Source: manual
                 Shape: 0 (Manual)
                 Shape: 0 (Mod Env 1)
                 Shape: 0 (LFO1)
                 Vsync; 0
              Sawdense; 0
              DenseDet; 64
            fixed note; 0
            Bend Range; 76
   __________________________
  | MIXER
          OSC1 Level: 255
          OSC2 Level: 0
          OSC3 Level: 0
         Noise Level: 0
      Ring 1-2 Level: 0
            VCA Gain: 127
   __________________________
  | FM
      OSC 3 > OSC 1
             Source: manual
             Manual: 0
          Mod Env 2: 0
              LFO 2: 0
      OSC 1 > OSC 2
             Source: manual
             Manual: 0
          Mod Env 2: 0
              LFO 2: 0
      OSC 2 > OSC 3
             Source: manual
             Manual: 0
          Mod Env 2: 0
              LFO 2: 0
   __________________________
  | FILTER
           LFO1 Depth: 128
     OSC 3 filter mod: 0
            Key Track: 127
             Selected: mod env1
             Amp Env : 0
           Mod Env 1 : 0
            Resonance: 0
         Filter Slope: 24
                 Freq: 255
                Shape: LP
            Overdrive: 0
   __________________________
  | AMP ENVELOPE
       Attack: 2
        Decay: 90
      Sustain: 127
      Release: 40
         Loop: OFF
        Delay: 0
   __________________________
  | MOD ENVELOPES
      1: Selected
       Attack: 2
        Decay: 75
      Sustain: 35
      Release: 45
     Velocity; 64
      Trigger; 1
    Hold Time; 0
      Repeats; 3
         Loop: off
        Delay; 0
      2:  
       Attack: 2
        Decay: 75
      Sustain: 35
      Release: 45
     Velocity; 64
      Trigger; 1
    Hold Time; 0
      Repeats; 3
         Loop: off
        Delay; 0
   __________________________
  | LFO 1
       Wave type: tri
       Fade time: 0
           Range: lo
            Rate: 128
           Phase: 0
            Slew: 0
           Range: LFO1
            Sync: on
         Repeats: 0
     Common Sync: Legato
     Common Sync: off
   __________________________
  | LFO 2
       Wave type: tri
       Fade time: 0
           Range: lo
            Rate: 128
           Phase: 0
            Slew: 0
           Range: LFO1
            Sync: on
         Repeats: 0
     Common Sync: Legato
     Common Sync: off
   __________________________
  | LFO 3
       Wave type: tri
            Rate: 64
   __________________________
  | LFO 4
       Wave type: tri
            Rate: 64
   __________________________
  | FX GLOBAL Menu
      WetLevel: 127
      DryLevel: 127
       Routing: Parallel

   __________________________
  | EFFECTS BYPASS: OFF
    DISTORTION: 0
   __________________________
  | CHORUS
       Level: 0
        Type: 8th D
        Rate: 0
        ChorDepth: 127
        ChorFback: 0
        LoPass: 7
        HiPass: 64
   __________________________
  | DELAY
      Feedback: 2
          Time: 1
          Sync: ON
     Sync Time: 90
         Level: 0
       LP damp: 0
       HP damp: 60
     L/R ratio: 20
      Slewrate: 0
         Width: 64
   __________________________
  | REVERB
          Time: 90
          Size: size2
         Level: 0
       LP Damp: 50
       HP Damp: 1
       RevSize: 64
       ModDepth: 64
       ModRate: 4
       Lo Pass: 74
       Hi Pass: 0
     Pre Delay: 40
   __________________________
  | VOICE
            Mode: Poly
           Glide: OFF
      Glide time: 60
          Unison: 0
       Unidetune: 25
          Spread: 0
 Keyboard Octave: 64
        Preglide: 64
      PatchLevel: 64
     FiltPostDrv: 0
     FiltDiverge: 0
     FiltShpDual: LP > HP
      FiltShpSep: -64
      AudioInput: off
   __________________________
  | ARP   OFF
          Tempo: 120
      Sync Rate: 16th 	
           Type: up
         Rhythm: 0
         Octave: 1
           Gate: 64
          Swing: 50
      Key Latch: OFF
       Key Sync: OFF
     ArpVelMode: played
   __________________________
  | MOD MATRIX

Slot 1             Slot 2             Slot 3             Slot 4             
   src1: Direct       src1: Direct       src1: Direct       src1: Direct    
   src2: Direct       src2: Direct       src2: Direct       src2: Direct    
  depth: 0           depth: 0           depth: 0           depth: 0         
   dest: O123Ptch     dest: O123Ptch     dest: O123Ptch     dest: O123Ptch  

Slot 5             Slot 6             Slot 7             Slot 8             
   src1: Direct       src1: Direct       src1: Direct       src1: Direct    
   src2: Direct       src2: Direct       src2: Direct       src2: Direct    
  depth: 0           depth: 0           depth: 0           depth: 0         
   dest: O123Ptch     dest: O123Ptch     dest: O123Ptch     dest: O123Ptch  

Slot 9             SLot 10            SLot 11            SLot 12            
   src1: Direct       src1: Direct       src1: Direct       src1: Direct    
   src2: Direct       src2: Direct       src2: Direct       src2: Direct    
  depth: 0           depth: 0           depth: 0           depth: 0         
   dest: O123Ptch     dest: O123Ptch     dest: O123Ptch     dest: O123Ptch  

SLot 13            SLot 14            SLot 15            SLot 16            
   src1: Direct       src1: Direct       src1: Direct       src1: Direct    
   src2: Direct       src2: Direct       src2: Direct       src2: Direct    
  depth: 0           depth: 0           depth: 0           depth: 0         
   dest: O123Ptch     dest: O123Ptch     dest: O123Ptch     dest: O123Ptch  

   __________________________
  | FX MOD MATRIX

    SLot 1:   Direct   : Direct   > 0 > DistLev  
    SLot 2:   Direct   : Direct   > 0 > DistLev  
    SLot 3:   Direct   : Direct   > 0 > DistLev  
    SLot 4:   Direct   : Direct   > 0 > DistLev  
