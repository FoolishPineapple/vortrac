/*
 *  LevelII.cpp
 *  VORTRAC
 *
 *  Created by Michael Bell on 7/19/05.
 *  Copyright 2005 University Corporation for Atmospheric Research.
 *  All rights reserved.
 *
 */

#include "LevelII.h"
#include "RadarQC.h"

LevelII::LevelII(const QString &radarname, const float &lat, const float &lon, const QString &filename) 
	: RadarData(radarname, lat, lon, filename)
{

  numSweeps = 0;
  numRays = 0;
  volHeader = new nexrad_vol_scan_title;
  radarHeader = new digital_radar_data_header;
  msgHeader = new nexrad_message_header;
  Sweeps = new Sweep[20];
  Rays = new Ray[7500];
  swap_bytes = false;
  
}

LevelII::~LevelII()
{
  delete volHeader;
  //delete radarHeader;
  //delete msgHeader;
  //delete [] ref_data;
  //delete [] vel_data;
  //delete [] sw_data;
  	
}

bool LevelII::readVolume()
{

  // Read in a level II file
  // Virtual function
	return false;
}

void LevelII::addSweep(Sweep* newSweep)
{

  // Add a Sweep to the volume
  // Already allocated memory for the sweep in the LevelII constructor
  //Sweep *newSweep = new Sweep;
  numSweeps += 1;
  newSweep->setSweepIndex( numSweeps );
  newSweep->setFirstRay(numRays);
  newSweep->setElevation( (radarHeader->elevation) * NEX_FIXED_ANGLE );
  newSweep->setUnambig_range( (radarHeader->unamb_range_x10) / 10.0 );
  newSweep->setNyquist_vel( (radarHeader->nyquist_vel_x100) / 100 );
  newSweep->setFirst_ref_gate( radarHeader->ref_gate1 );
  newSweep->setFirst_vel_gate( radarHeader->vel_gate1 );
  newSweep->setRef_gatesp( radarHeader->ref_gate_width );
  newSweep->setVel_gatesp( radarHeader->vel_gate_width );
  newSweep->setRef_numgates( radarHeader->ref_num_gates );
  newSweep->setVel_numgates( radarHeader->vel_num_gates );
  newSweep->setVcp( radarHeader->vol_coverage_pattern );
  
  //return *newSweep;
}

void LevelII::addRay(Ray* newRay)
{

  // Add a new ray to the sweep
  //Ray *newRay = new Ray();
  numRays++;
  newRay->setSweepIndex( (numSweeps - 1) );
  newRay->setTime( radarHeader->milliseconds_past_midnight );
  newRay->setDate( radarHeader->julian_date );
  newRay->setAzimuth( (radarHeader->azimuth) * NEX_FIXED_ANGLE );
  newRay->setElevation( (radarHeader->elevation) * NEX_FIXED_ANGLE );
  newRay->setVelResolution( radarHeader->velocity_resolution );
  newRay->setRayIndex( radarHeader->radial_num );
  newRay->setRefData( ref_data );
  newRay->setVelData( vel_data );
  newRay->setSwData( sw_data );
  newRay->setUnambig_range( float(radarHeader->unamb_range_x10) / 10.0 );
  newRay->setNyquist_vel( float(radarHeader->nyquist_vel_x100) / 100 );
  newRay->setFirst_ref_gate( radarHeader->ref_gate1 );
  newRay->setFirst_vel_gate( radarHeader->vel_gate1 );
  newRay->setRef_gatesp( radarHeader->ref_gate_width );
  newRay->setVel_gatesp( radarHeader->vel_gate_width );
  newRay->setRef_numgates( radarHeader->ref_num_gates );
  newRay->setVel_numgates( radarHeader->vel_num_gates );
  newRay->setVcp( radarHeader->vol_coverage_pattern );

  //return *newRay;
}

float* LevelII::decode_ref(const char *buffer, short int numGates)
{

  // Decode each byte
  float* const refArray = new float[numGates];
  for (short int i = 0; i <= numGates - 1; i++) {
    unsigned char encoded = (unsigned char)buffer[i];
    if (encoded == 0) {
      refArray[i] = -999;
    } else if (encoded == 1) {
      // Ambiguous, set to bad for now
      refArray[i] = -999;
    } else {
      refArray[i] = (((float)encoded - 2.)/2.) - 32.0;
    }
  }

  return refArray;

}

float* LevelII::decode_vel(const char *buffer, short int numGates,
			   short int velRes)
{

  // Decode each byte
  float* const velArray = new float[numGates];
  for (int i = 0; i <= numGates - 1; i++) {
    unsigned char encoded = (unsigned char)buffer[i];
    if (encoded == 0) {
      velArray[i] = -999;
    } else if (encoded == 1) {
      velArray[i] = -999;
    } else {
      if (velRes == 2) {
	velArray[i] = (((float)encoded - 2.)/2.) - 63.5;
      } else {
	velArray[i] = ((float)encoded - 2.) - 127.0;
      }
    }
  }

  return velArray;

}

float* LevelII::decode_sw(const char *buffer, short int numGates)
{

  // Decode each byte
  float* const swArray = new float[numGates];
  for (int i = 0; i <= numGates - 1; i++) {
    unsigned char encoded = (unsigned char)buffer[i];
    if (encoded == 0) {
      swArray[i] = -999;
    } else if (encoded == 1) {
      swArray[i] = -999;
    } else {
      swArray[i] = (((float)encoded - 2.)/2.) - 63.5;
    }
  }

  return swArray;

}

void LevelII::swapVolHeader()
{

  swab((char *)&volHeader->julian_date, (char *)&volHeader->julian_date, 4);
  swab((char *)&volHeader->milliseconds_past_midnight, 
       (char *)&volHeader->milliseconds_past_midnight, 4);

}

void LevelII::swapRadarHeader()
{

  radarHeader->milliseconds_past_midnight = 
    swap4((char *)&radarHeader->milliseconds_past_midnight); /* (15-16) */
  swab((char *)&radarHeader->julian_date,
       (char *)&radarHeader->julian_date, 2);          /* (17) from 1/1/70 */
  swab((char *)&radarHeader->unamb_range_x10,
       (char *)&radarHeader->unamb_range_x10, 2);      /* (18) km. */
  swab((char *)&radarHeader->azimuth,
       (char *)&radarHeader->azimuth,2);     /* (19) binary angle */
  swab((char *)&radarHeader->radial_num,
       (char *)&radarHeader->radial_num, 2);           /* (20) */
  swab((char *)&radarHeader->radial_status,
       (char *)&radarHeader->radial_status, 2);        /* (21) */
  swab((char *)&radarHeader->elevation,
       (char *)&radarHeader->elevation, 2);   /* (22) binary angle */
  swab((char *)&radarHeader->elev_num,
       (char *)&radarHeader->elev_num, 2);             /* (23) */
  swab((char *)&radarHeader->ref_gate1,
       (char *)&radarHeader->ref_gate1, 2);            /* (24) meters */
  swab((char *)&radarHeader->vel_gate1,
       (char *)&radarHeader->vel_gate1, 2);            /* (25) meters */
  swab((char *)&radarHeader->ref_gate_width,
       (char *)&radarHeader->ref_gate_width, 2);       /* (26) meters */
  swab((char *)&radarHeader->vel_gate_width, 
       (char *)&radarHeader->vel_gate_width, 2);       /* (27) meters */
  swab((char *)&radarHeader->ref_num_gates,
       (char *)&radarHeader->ref_num_gates, 2);        /* (28) */
  swab((char *)&radarHeader->vel_num_gates,
       (char *)&radarHeader->vel_num_gates, 2);        /* (29) */
  swab((char *)&radarHeader->sector_num,
       (char *)&radarHeader->sector_num, 2);           /* (30) */
  // float sys_gain_cal_const;   /* (31-32) */
  swab((char *)&radarHeader->ref_ptr,
       (char *)&radarHeader->ref_ptr, 2);              /* (33) byte count from start of drdh */
  swab((char *)&radarHeader->vel_ptr,
       (char *)&radarHeader->vel_ptr, 2);              /* (34) byte count from start of drdh */
  swab((char *)&radarHeader->sw_ptr,
       (char *)&radarHeader->sw_ptr, 2);               /* (35) byte count from start of drdh */
  swab((char *)&radarHeader->velocity_resolution,
       (char *)&radarHeader->velocity_resolution, 2);  /* (36) */
  swab((char *)&radarHeader->vol_coverage_pattern,
       (char *)&radarHeader->vol_coverage_pattern, 2); /* (37) */
  // swab((char *)&radarHeader->VNV1;                 /* V & V simulator reserved */
  // swab((char *)&radarHeader->VNV2;
  // swab((char *)&radarHeader->VNV3;
  // swab((char *)&radarHeader->VNV4;
  swab((char *)&radarHeader->ref_data_playback,
       (char *)&radarHeader->ref_data_playback, 2);    /* (42) */
  swab((char *)&radarHeader->vel_data_playback,
       (char *)&radarHeader->vel_data_playback, 2);    /* (43) */
  swab((char *)&radarHeader->sw_data_playback,
       (char *)&radarHeader->sw_data_playback, 2);     /* (44) */
  swab((char *)&radarHeader->nyquist_vel_x100,
       (char *)&radarHeader->nyquist_vel_x100, 2);     /* (45)m/s */
  swab((char *)&radarHeader->atmos_atten_factor_x1000,
       (char *)&radarHeader->atmos_atten_factor_x1000, 2); /* (46) dB/km */
  swab((char *)&radarHeader->threshold_parameter, 
       (char *)&radarHeader->threshold_parameter, 2);  /* (47) */
  
}

bool LevelII::machineBigEndian(){

    union {
	unsigned char byte[4];
	int val;
    }word;

    word.val = 0;
    word.byte[3] = 0x01;
    
    return word.val == 1;
}

int short LevelII::swap2(char *ov)		/* swap integer*2 */
{
    union {
	int short newval;
	char nv[2];
    }u;
    u.nv[1] = *ov++; u.nv[0] = *ov++;
    return(u.newval);
}

int long LevelII::swap4(char *ov )		/* swap integer*4 */
{
   union {
     int long newval;
     char nv[4];
   }u;
 
   u.nv[3] = *ov++; u.nv[2] = *ov++; u.nv[1] = *ov++; u.nv[0] = *ov++;

   return(u.newval);
}
