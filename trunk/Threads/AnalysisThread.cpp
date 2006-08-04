/*
 *  AnalysisThread.cpp
 *  VORTRAC
 *
 *  Created by Michael Bell on 6/17/05.
 *  Copyright 2005 University Corporation for Atmospheric Research.
 *  All rights reserved.
 *
 */

#include <QtGui>

#include "AnalysisThread.h"
#include "Radar/RadarQC.h"
#include "DataObjects/CappiGrid.h"
#include "DataObjects/VortexData.h"
#include "Message.h"
#include "SimplexThread.h"
#include "HVVP/Hvvp.h"

AnalysisThread::AnalysisThread(QObject *parent)
  : QThread(parent)
{
  Message::toScreen("AnalysisThread Constructor");
  abort = false;
  simplexThread = new SimplexThread;
}

AnalysisThread::~AnalysisThread()
{
  mutex.lock();
  abort = true;
  waitForData.wakeOne();
  mutex.unlock();
  
  // Wait for the thread to finish running if it is still processing
  wait();

}

void AnalysisThread::setConfig(Configuration *configPtr)
{

  configData = configPtr;

}

void AnalysisThread::setVortexList(VortexList *archivePtr)
{

  vortexList = archivePtr;

}

void AnalysisThread::setSimplexList(SimplexList *archivePtr)
{
	
	simplexList = archivePtr;
	
}

void AnalysisThread::abortThread()
{
  Message::toScreen("In AnalysisThread Abort");
  if(simplexThread->isRunning()) {
    Message::toScreen("in AnalysisThread Exit");
    mutex.lock();
    simplexThread->exit();
    //simplexThread = new SimplexThread;
    //delete simplexThread;
    //Message::toScreen("AnalysisThread has mutex locked");
    mutex.unlock();
    //}
    // exit();
  }
  Message::toScreen("Before AnalysisThread Exit");
  this->terminate();
  //  this->exit();
  Message::toScreen("Leaving AnalysisThread Abort");
}

void AnalysisThread::analyze(RadarData *dataVolume, Configuration *configPtr)
{
	// Lock the thread
	QMutexLocker locker(&mutex);

	this->radarVolume = dataVolume;
	configData = configPtr;
	radarVolume->setAltitude(configData->getParam(configData->getConfig("radar"),"alt").toFloat()/1000.0);
	// Sets altitude of radar to radarData (km) from ConfigData (meters)

	// Start or wake the thread
	if(!isRunning()) {
		start();
	} else {
	  //Message::toScreen("found running copy of analysis... waiting....");
		waitForData.wakeOne();
	}
}

void AnalysisThread::run()
{
  abort = false;
  emit log(Message("Data found, starting analysis...", -1));

       forever {
		// Check to see if we should quit
		if (abort)
		  return;

		// OK, Let's process some radar data
		mutex.lock();
		bool analysisGood = true;
		
		// Read in the radar data
		radarVolume->readVolume();

		// Pass VCP value to display
		emit newVCP(radarVolume->getVCP());
		mutex.unlock();
		if(abort)
		  return;
		mutex.lock();
		//Dealias
		
		if(!radarVolume->isDealiased()){
		  
		  RadarQC dealiaser(radarVolume);

		  connect(&dealiaser, SIGNAL(log(const Message&)), this, 
			  SLOT(catchLog(const Message&)), Qt::DirectConnection);
		  
		  dealiaser.getConfig(configData->getConfig("qc"));
		  
		  if(dealiaser.dealias()) {
		    emit log(Message("Finished Dealias Method", 10));
		    radarVolume->isDealiased(true);
		  } else {
		    emit log(Message("Finished Dealias Method with Failures"));
		    analysisGood = false;
		    // Something went wrong
		  }
		}
		else
		  emit log(Message("RadarVolume is Dealiased"));
		
		// Check the vortex list for a current center
		mutex.unlock();
		if(abort)
		  return;
		mutex.lock();
		if (!vortexList->isEmpty()) {
		  vortexList->timeSort();
		  vortexLat = vortexList->last().getLat();
		  vortexLon = vortexList->last().getLon();
		} else {
		  vortexLat = configData->getConfig("vortex").firstChildElement("lat").text().toFloat();
		  vortexLon = configData->getConfig("vortex").firstChildElement("lon").text().toFloat();
		  
		  // Need to define a filename and time for the Lists
		  QString vortexPath = configData->getConfig("vortex").firstChildElement("dir").text();
		  QString vortexFile = radarVolume->getDateTimeString();
		  vortexFile.replace(QString(":"),QString("_"));
		  QString outFileName = vortexPath + "/" + vortexFile + "vortexList.xml";
		  vortexList->setFileName(outFileName);
		  
		  QString simplexPath = configData->getConfig("center").firstChildElement("dir").text();
		  QString simplexFile = radarVolume->getDateTimeString();
		  simplexFile.replace(QString(":"),QString("_"));
		  outFileName = simplexPath + "/" + simplexFile + "simplexList.xml";
		  simplexList->setFileName(outFileName);
		  
		}
		
		mutex.unlock();
		if(abort)
		  return;
		mutex.lock();

		// Create CAPPI
		emit log(Message("Creating CAPPI..."));

		/* If Analytic Model is running we need to make an analytic
		   gridded data rather than a cappi*/
		
		GriddedData *gridData;

		if(radarVolume->getNumSweeps() < 0) {
		  Configuration *analyticConfig = new Configuration();
		  QDomElement radar = configData->getConfig("radar");
		  float radarLat = configData->getParam(radar,"lat").toFloat();
		  float radarLon = configData->getParam(radar,"lon").toFloat();
		  analyticConfig->read(configData->getParam(radar, "dir"));
		  gridData = gridFactory.makeAnalytic(radarVolume,
					   configData->getConfig("cappi"),
					   analyticConfig, &vortexLat, 
					   &vortexLon, &radarLat, &radarLon);
		}
		else {
		  gridData = gridFactory.makeCappi(radarVolume,
						configData->getConfig("cappi"),
			 			&vortexLat, &vortexLon);
		  //Message::toScreen("AnalysisThread: outside makeCappi");

		}
		
		mutex.unlock();
		if(abort)
		  return;
		mutex.lock();

		/* GriddedData not currently a QObject so this will fail
		   connect(gridData, SIGNAL(log(const Message&)),this,
		   SLOT(catchLog(const Message&)), Qt::DirectConnection); */
		
		// Output Radar data to check if dealias worked
		gridData->writeAsi();
		emit log(Message("Writing ASI"));
		
		// Create data instances to hold the analysis results
		VortexData *vortexData = new VortexData(); 

		mutex.unlock();  // Added this one ... I think...

		// Mutex Investigation.....
		
		mutex.lock();
		if (!abort) {
		  //Find Center 
		  simplexThread = new SimplexThread();
		  connect(simplexThread, SIGNAL(log(const Message&)), this, 
			  SLOT(catchLog(const Message&)), Qt::DirectConnection);
		  connect(simplexThread, SIGNAL(centerFound()), 
			  this, SLOT(foundCenter()), Qt::DirectConnection);
		  //simplexThread->findCenter(configData, gridData, &vortexLat, &vortexLon, simplexList);
		  //waitForCenter.wait(&mutex); 
		}
		else
		  return;
		mutex.unlock();  
		//Message::toScreen("Where....");

		
		// Get environmental wind
		/*
		* rt: Range from radar to circulation center (km).
		*
		* cca: Meteorological azimuth angle of the direction
		*      to the circulation center (degrees from north).
		*
		* rmw: radius of maximum wind measured from the TC circulation
		*      center outward.
		*/

		float rt = 0;
		float cca = 0;
		float rmw = 0;

		Hvvp *envWindFinder = new Hvvp;
		envWindFinder->setRadarData(radarVolume,rt, cca, rmw);
		envWindFinder->findHVVPWinds();
		float missingLink = envWindFinder->getAvAcrossBeamWinds();
		Message::toScreen("Hvvp gives "+QString().setNum(missingLink));
		/*
		// Run core VTD algorithm
		vortexdata->runVTD(configdata.getVTDParams());
		
		// Get current pressure values
		PressureData *pressuredata = new PressureData;
		pressuredata->setPressure(datasource->getPressure());
		
		// Calculate central pressure
		vortexdata->setCentralPressure(pressuredata->getPressure());
		
		// Should have all relevant variables now
		// Update timeline and output
		vortexlist->addVortex(vortexdata->getArchiveData());
		*/
		

		mutex.lock(); // Added this one....

		delete vortexData;
		
		//Message::toScreen("Deleted vortex data.... ???");
		
		if(!analysisGood)
		{
			// Some error occurred, notify the user
			emit log(Message("Radar volume processing error!"));
			emit log(Message("Radar volume processing error!"));
			return;
		} else {
			// Store the resulting analysis in the vortex list
			archiveAnalysis();
			
			// Complete the progress bar and log that we're done
			emit log(Message("Analysis complete!",100));

			// Let the poller know we're done
			emit(doneProcessing());
		}
		mutex.unlock();
		
		// Go to sleep, wait for more data
		mutex.lock();
		if (!abort)
			// Wait until new data is available
			waitForData.wait(&mutex);
		mutex.unlock();
		
	}
	emit log(Message("End of Analysis Thread Run"));
}

void AnalysisThread::archiveAnalysis()
{

}

void AnalysisThread::foundCenter()
{
	waitForCenter.wakeOne();
}

void AnalysisThread::catchLog(const Message& message)
{
  emit log(message);
}
