/*
 *  GBVTD.h
 *  vortrac
 *
 *  Created by Michael Bell on 5/6/06.
 *  Copyright 2006 University Corporation for Atmospheric Research.
 *  All rights reserved.
 *
 */

#ifndef GBVTD_H
#define GBVTD_H


#include <QString>
#include "DataObjects/Coefficient.h"

#include "VTD.h"

class GBVTD : public VTD
{

public:
  
    GBVTD(QString& initClosure, int& wavenumbers, float*& gaps, float hvvpwind);
    virtual ~GBVTD();

    bool analyzeRing(float& xCenter, float& yCenter, float& radius,
		     float& height, int& numData, float*& ringData,
		     float*& ringAzimuths, Coefficient*& vtdCoeffs,
		     float& stdDev);

    void  setWindCoefficients(float& radius, float& height,
			      int& numCoefficients, float*& FourierCoeffs,
			      Coefficient*& vtdCoeffs);
};

#endif

