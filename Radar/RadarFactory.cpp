/*
 *  RadarFactory.cpp
 *  VORTRAC
 *
 *  Created by Michael Bell on 7/18/05.
 *  Copyright 2005 University Corporation for Atmospheric Research.
 *  All rights reserved.
 *
 */

#include "RadarFactory.h"
#include "thredds_Config.h"
#include <iostream>
#include <QPushButton>
#include <QUrl>
#include <QNetworkRequest>

RadarFactory::RadarFactory(Configuration* radarConfig, QObject *parent)
  : QObject(parent)
{
  this->setObjectName("Radar Factory");
  mainConfig = radarConfig;

  // Will poll for data and return radar objects in a queue
  radarQueue = new QQueue<QString>;

  // Get relevant configuration info
  QDomElement radar = mainConfig->getConfig("radar");
  radarName = mainConfig->getParam(radar,"name");
  radarLat = mainConfig->getParam(radar,"lat").toFloat();
  radarLon = mainConfig->getParam(radar,"lon").toFloat();
  radarAlt = mainConfig->getParam(radar,"alt").toFloat();
  // radarAltitude is given in meters, convert to km
  radarAlt = radarAlt/1000;

  QDate startDate = QDate::fromString(mainConfig->getParam(radar,"startdate"),
				      Qt::ISODate);
  QDate endDate = QDate::fromString(mainConfig->getParam(radar,"enddate"),
				    Qt::ISODate);
  QTime startTime = QTime::fromString(mainConfig->getParam(radar,"starttime"),
				      "hh:mm:ss");
  QTime endTime = QTime::fromString(mainConfig->getParam(radar,"endtime"),
				    "hh:mm:ss");
  startDateTime = QDateTime(startDate, startTime, Qt::UTC);
  endDateTime = QDateTime(endDate, endTime, Qt::UTC);
  
  QString path = mainConfig->getParam(radar,"dir");
  dataPath = QDir(path);

  QString format = mainConfig->getParam(radar,"format");
  if (format == "LDMLEVELII") {
    radarFormat = ldmlevelII;
  } else if (format == "NCDCLEVELII") {
	radarFormat = ncdclevelII;
  } else if (format == "MODEL") {
    radarFormat = model;
  }
  else { 
    // Will implement more later but give error for now
    emit log(Message("Data format not supported"));
  }

  // Connect the signals and slots
  connect(&catalog_manager, SIGNAL(finished(QNetworkReply*)),
	SLOT(getRemoteData(QNetworkReply*)));
  connect(&datafile_manager, SIGNAL(finished(QNetworkReply*)),
	SLOT(saveRemoteData(QNetworkReply*)));

}

RadarFactory::~RadarFactory()
{
  mainConfig = NULL;
  delete mainConfig;
  delete radarQueue;
}

RadarData* RadarFactory::getUnprocessedData()
{
  
  // Get the latest files off the queue and make a radar object

  if (radarQueue->isEmpty()) {
    // We might end up here if we restart a trial that has no new
    // data ... 
    emit log(Message("No new data available for processing"));
    return NULL;
  }

  // Get the files off the queue
  QString fileName = dataPath.filePath(radarQueue->dequeue());

  // Test file to make sure it is not growing
  QFile radarFile(fileName);
  qint64 newFilesize = radarFile.size();
  qint64 prevFilesize = 0;
  while (prevFilesize != newFilesize) {
	prevFilesize = newFilesize;
	sleep(1);
	newFilesize = radarFile.size();
  }
  //sleep(1);
  //emit log(Message(QString("Reading file:"+fileName), 0, this->objectName()));
  sleep(1);
  // Mark it as processed
  fileAnalyzed[fileName] = true;
    
  // Now make a new radar object from that file and send it back
  switch(radarFormat) {
  case ncdclevelII :
    {
      NcdcLevelII *radarData = new NcdcLevelII(radarName, radarLat, 
					       radarLon, fileName);
      radarData->setAltitude(radarAlt);
      return radarData;
      //break;
    }
  case ldmlevelII :
  {
      LdmLevelII *radarData = new LdmLevelII(radarName, radarLat, 
											 radarLon, fileName);
      radarData->setAltitude(radarAlt);
      return radarData;
      // break;
  }  
  case model:
    {
      AnalyticRadar *radarData = new AnalyticRadar(radarName, 
						   radarLat, radarLon,
						   fileName);
      radarData->setAltitude(radarAlt);
      radarData->setConfigElement(mainConfig);
      return radarData;
      //break;
    }
  case dorade:
    {
      // Not yet implemented
      break;
    }
  case netcdf:
    {
      // Not yet implemented
      break;
    }
  }

  // If we get here theres a problem, return a null pointer
  emit log(Message(QString("Problem with radar data Factory"),0,this->objectName(),Yellow));
  return NULL;

}

bool RadarFactory::hasUnprocessedData()
{
  
  // Check the unprocessed list first, if it has files no need to reread directory yet
  if(!radarQueue->isEmpty()) {
    return true;
  }

  // Otherwise, check the directory for appropriate files

  switch(radarFormat) {
  case ncdclevelII:
    {
      // Should have filenames starting with radar ID
      dataPath.setNameFilters(QStringList(radarName + "*"));
      dataPath.setFilter(QDir::Files);
      dataPath.setSorting(QDir::Name);
      QStringList filenames = dataPath.entryList();

      // Remove any files with extensions
      for (int i = filenames.size();i >= 0; --i)
	if(filenames.value(i).contains('.')) {
	  QString currentFile = filenames.value(i);
	  if(currentFile.contains('/')) {
	    int slashIndex = currentFile.indexOf('/');
	    currentFile.remove(0,slashIndex+1);
	    if(currentFile.contains('.'))
	      filenames.removeAt(i);
	    else
	      continue;
	  }
	  else 
	    filenames.removeAt(i);
	}
      
      // Check to see which are in the time limits
      for (int i = 0; i < filenames.size(); ++i) {
		  QString file = filenames.at(i);
		  QString timepart = file;
		  // Replace the radarname so we just have timestamps
		  timepart.replace(radarName, "");
		  QStringList timestamp = timepart.split("_");
		  QDate fileDate = QDate::fromString(timestamp.at(0), "yyyyMMdd");
		  QTime fileTime = QTime::fromString(timestamp.at(1), "hhmmss");
		  QDateTime fileDateTime = QDateTime(fileDate, fileTime, Qt::UTC);
		  
		  if (fileDateTime >= startDateTime && fileDateTime <= endDateTime) {	
			  // Valid time and radar name, check to see if it has been processed
			  if (!fileAnalyzed[dataPath.filePath(file)]) {
				  // File has not been analyzed, add it to the queue
				  radarQueue->enqueue(file);
			  }
		  }
      }

    break;
    }
  case ldmlevelII:
  {
      // Should have filenames starting with radar ID
      dataPath.setNameFilters(QStringList(radarName + "*"));
      dataPath.setFilter(QDir::Files);
      //dataPath.setSorting(QDir::Time | QDir::Reversed);
	  dataPath.setSorting(QDir::Name);
      QStringList filenames = dataPath.entryList();
      
      // Check to see which are in the time limits
      for (int i = 0; i < filenames.size(); ++i) {
		  QString file = filenames.at(i);
		  QString timepart = file.split("/").last();
		  // Replace the radarname so we just have timestamps
		  timepart.replace(radarName, "");
		  QDate fileDate;
		  QTime fileTime;
		  if (timepart.contains('.')) {
			  // NRL Format
			  QStringList timestamp = timepart.split(".");
			  fileDate = QDate::fromString(timestamp.at(1).left(8), "yyyyMMdd");
			  fileTime = QTime::fromString(timestamp.at(1).right(6), "hhmmss");
		  }  else if (timepart.contains('_')) {
			  // Purdue Format
			  QStringList timestamp = timepart.split("_");
			  fileDate = QDate::fromString(timestamp.at(1), "yyyyMMdd");
			  if (timestamp.size() > 2) {
			    fileTime = QTime::fromString(timestamp.at(2), "hhmm");
			  } else {
			    emit log(Message(QString("Problem with time in level_II filename, this may be a NCDC file"),0,this->objectName(),Red,QString("Radar format mismatch?")));
			  } 
		  }
		  
		  QDateTime fileDateTime = QDateTime(fileDate, fileTime, Qt::UTC);
			
		  QString timeString = fileDateTime.toString(Qt::ISODate);
		  QString start = startDateTime.toString(Qt::ISODate);
		  QString end = endDateTime.toString(Qt::ISODate);
		  if (fileDateTime >= startDateTime && fileDateTime <= endDateTime) {	
			  // Valid time and radar name, check to see if it has been processed
			  if (!fileAnalyzed[dataPath.filePath(file)]) {
				  // File has not been analyzed, add it to the queue
				  radarQueue->enqueue(file);
			  }
		  }
      }
	  
	  break;
  }
	  
  case model:
    {
      // Currently the GUI is set up to take the XML config in the 
      // radar directory parameter when analytic storm is selected as
      // radar format.
      
      // Here we will pass that xml file through the queue to be processed
      
      QString fileName = mainConfig->getParam(mainConfig->getConfig("radar"),
					      "dir");

      //Message::toScreen("Radar Factory:analytic config filename: "+fileName);
      
      if (!fileAnalyzed[fileName]) {
	// File has not been analyzed, add it to the queue
	radarQueue->enqueue(fileName);
      }
      
      break;
    }
    
  case dorade:
    {
      // Not yet implemented
      break;
    }  
  case netcdf:
    {
      // Not yet implemented
      break; 
    } 
    
  }

  // See if we added any new files to the queue
  if(!radarQueue->isEmpty()) {
    return true;
  }
  
  // We made it here so there must be nothing new
  return false;
  
}

void RadarFactory::catchLog(const Message& message)
{
  emit log (message);
}

void RadarFactory::updateDataQueue(const VortexList* list)
{
  if(!list->count()>0)
    return;
  int totalFiles = radarQueue->count();
  for(int i = totalFiles-1; i >= 0; i--)
    {
      switch(radarFormat) {
      case ncdclevelII:
	{
	  // Should have filenames starting with radar ID
      
	  // Check to see which are in the time limits
	  QString file = radarQueue->at(i);
	  QString timepart = file;
	  // Replace the radarname so we just have timestamps
	  timepart.replace(radarName, "");
	  QStringList timestamp = timepart.split("_");
	  QDate fileDate = QDate::fromString(timestamp.at(0), "yyyyMMdd");
	  QTime fileTime = QTime::fromString(timestamp.at(1), "hhmmss");
	  QDateTime fileDateTime = QDateTime(fileDate, fileTime, Qt::UTC);
	  
	  if (fileDateTime >= startDateTime && fileDateTime <= endDateTime){
	    // Valid time and radar name, check to see if it 
	    //  has been processed in vortexList
	    for(int j = 0; j < list->size(); j++) {
	      QDateTime processedTime = list->at(j).getTime();
	      if(abs(processedTime.secsTo(fileDateTime)) < 30) {
		if (!fileAnalyzed[dataPath.filePath(file)]) {
		  // File has been analyzed, remove it from the queue
		  radarQueue->removeAt(radarQueue->indexOf(file));
		  fileAnalyzed[dataPath.filePath(file)] = true; 
		}
	      }
	      else {
		if(list->last().getTime() >  fileDateTime)
		  if(!fileAnalyzed[dataPath.filePath(file)]) {
		    radarQueue->removeAt(radarQueue->indexOf(file));
		    fileAnalyzed[dataPath.filePath(file)] = true; 
		  }
	      } 
	    }
	  }
	  break;
	}
      case ldmlevelII:
	{
	  // Should have filenames starting with radar ID
	  // Check to see which are already in vortexList
	  QString file = radarQueue->at(i);
	  QString timepart = file.split("/").last();
	  // Replace the radarname so we just have timestamps
	  timepart.replace(radarName, "");
	  QDate fileDate;
	  QTime fileTime;
	  if (timepart.contains('.')) {
	    // NRL Format
	    QStringList timestamp = timepart.split(".");
	    fileDate = QDate::fromString(timestamp.at(1).left(8), "yyyyMMdd");
	    fileTime = QTime::fromString(timestamp.at(1).right(6), "hhmmss");
	  }  else if (timepart.contains('_')) {
	    // Purdue Format
	    QStringList timestamp = timepart.split("_");
	    fileDate = QDate::fromString(timestamp.at(1), "yyyyMMdd");
	    if (timestamp.size() > 2) {
	      fileTime = QTime::fromString(timestamp.at(2), "hhmm");
	    } else {
	      emit log(Message(QString("Problem with time in level_II filename, this may be a NCDC file"),0,this->objectName(),Red,QString("Radar format mismatch?")));
	    } 
	  }
	  
	  QDateTime fileDateTime = QDateTime(fileDate, fileTime, Qt::UTC);
	  if (fileDateTime >= startDateTime && fileDateTime <= endDateTime){
	    // Valid time and radar name, check to see if it 
	    //  has been processed in vortexList
	    for(int j = 0; j < list->size(); j++) {
	      QDateTime processedTime = list->at(j).getTime();
	      if(abs(processedTime.secsTo(fileDateTime)) < 30) {
		if (!fileAnalyzed[dataPath.filePath(file)]) {
		  // File has been analyzed, remove it from the queue
		  radarQueue->removeAt(radarQueue->indexOf(file));
		  fileAnalyzed[dataPath.filePath(file)] = true; 
		  //  Message::toScreen("Plucked file "+file+" from the list");
		}
	      }
	      else {
		if(list->last().getTime() >  fileDateTime)
		  if(!fileAnalyzed[dataPath.filePath(file)]) {
		    radarQueue->removeAt(radarQueue->indexOf(file));
		    fileAnalyzed[dataPath.filePath(file)] = true; 
		  }
	      }
	    }
	  }
	  break;
	}
	  case model:
	  {
		  // Not yet implemented
		  break;
	  }			  
	  case dorade:
	  {
		  // Not yet implemented
		  break;
	  }
	  case netcdf:
	  {
		  // Not yet implemented
		  break;
	  }
		  
      }
    }
}

int RadarFactory::getNumProcessed() const
{
  // Returns the number of volumes that RadarFactory has sent to be processed
  // This was added for determining when volumes have been sent but have 
  // not produced vortexData - which would indicated that either the 
  // vortex was not within radar range or that the standard deviation 
  // was simply too high to incorporate the analysis.

  return fileAnalyzed.keys(true).count();
}

void RadarFactory::fetchRemoteData()
{
	// Fetch the catalog of files
	QString server = "http://shelf.rcac.purdue.edu:8080/thredds/";
	QUrl catalog = QUrl(server + radarName + "/catalog.xml");
	QNetworkRequest request(catalog);
	catalog_manager.get(request);	
}

bool RadarFactory::getRemoteData(QNetworkReply *catalog_reply)
{
	QUrl url = catalog_reply->url();
	if (catalog_reply->error()) {
		emit log(Message(QString("Problem downloading THREDDS catalog"),0,this->objectName(),Yellow,QString("Problem with THREDDS")));
	} else {
		// Save then parse the file
		QString filename("catalog.xml");
		QFile file(filename);
		if (!file.open(QIODevice::WriteOnly)) {
			emit log(Message(QString("Problem saving THREDDS catalog"),0,this->objectName(),Yellow,QString("Problem with THREDDS")));
			return false;
		}
		
		file.write(catalog_reply->readAll());
		file.close();
		
		thredds_Config* catalog = new thredds_Config(0, filename);
		if (!catalog->validate()) return false;
		
		QDomElement dataset = catalog->getConfig("dataset");
		QString datafile = catalog->getAttribute(dataset,"dataset", "name");

		// Check to see if this file is already in the directory, or download it
		if (!dataPath.exists(datafile)) {
			QString dataurl = catalog->getAttribute(dataset,"dataset", "urlPath");
			QString server = "http://shelf.rcac.purdue.edu:8080/thredds/fileServer/";
			QUrl fileurl = QUrl(server + dataurl);
			QNetworkRequest request(fileurl);
			datafile_manager.get(request);
		}
	}
	
	catalog_reply->deleteLater();
	return true;
}

bool RadarFactory::saveRemoteData(QNetworkReply *datafile_reply)
{
	QUrl url = datafile_reply->url();
	QString path = url.path();
	QString filename = QFileInfo(path).fileName();
	QFile file(dataPath.absolutePath() + filename);
	if (!file.open(QIODevice::WriteOnly)) {
		emit log(Message(QString("Problem saving remote data"),0,this->objectName(),Yellow,QString("Problem with remote data")));
		return false;
	}
	
	file.write(datafile_reply->readAll());
	file.close();
	
	return true;
}

