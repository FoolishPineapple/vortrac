/*
 *  AnalysisThread.h
 *  VORTRAC
 *
 *  Created by Michael Bell on 6/17/05.
 *  Copyright 2005 University Corporation for Atmospheric Research.
 *  All rights reserved.
 *
 */

#ifndef ANALYSISTHREAD_H
#define ANALYSISTHREAD_H

#include <QMutex>
#include <QSize>
#include <QThread>
#include <QWaitCondition>
#include <QList>
#include "Radar/RadarData.h"
#include "IO/Message.h"
#include "Config/Configuration.h"
#include "DataObjects/VortexList.h"
#include "DataObjects/GriddedFactory.h"
#include "SimplexThread.h"
#include "VortexThread.h"
#include "Pressure/PressureList.h"

class AnalysisThread : public QThread
{
  Q_OBJECT
    
 public:
     AnalysisThread(QObject *parent = 0);
     ~AnalysisThread();
     void analyze(RadarData *dataVolume, Configuration *configPtr);
     void setConfig(Configuration *configPtr);
     void setVortexList(VortexList *archivePtr);
     void setSimplexList(SimplexList *archivePtr);
     void setPressureList(PressureList *archivePtr);
     void setSimplexThread(SimplexThread *threadPtr);
     void setVortexThread(VortexThread *threadPtr);
     void setNumVolProcessed(const float& num);
     void setAnalyticRun(const bool& runOnce);
	 
 public slots:
     void catchLog(const Message& message);
     void foundCenter();
     void foundWinds();
     void abortThread();
 
 protected:
     void run();
 
 signals:
     void doneProcessing();
     void log(const Message& message);
     void newVCP(const int);
     void newCappi(const GriddedData*);
     void newCappiInfo(const float& x, const float& y, const float& rmwEstimate, const float& sMin, const float& sMax,
		       const float& vMax,  const float& userLat, const float& userLon, const float& lat, const float& lon);
     void abortChanged(volatile bool *abortStatus);
	 
 private:
     QMutex mutex;
     QWaitCondition waitForData;
     QWaitCondition waitForCenter;
     QWaitCondition waitForWinds;
     volatile bool abort;
     Configuration *configData;
     RadarData *radarVolume;
     GriddedFactory gridFactory;
     GriddedData *_gridData;
     SimplexThread *simplexThread;
     VortexThread *vortexThread;
     VortexList *vortexList;
     SimplexList *simplexList;
     PressureList *pressureList;

     // is this supposed to be type vortexList -LM
     void archiveAnalysis();
     float vortexLat, vortexLon;
     int numVolProcessed;
     bool analyticRun;
    
};

#endif
