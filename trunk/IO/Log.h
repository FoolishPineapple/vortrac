/*
 * Log.h
 * VORTRAC
 *
 * Created by Michael Bell on 7/22/05
 *  Copyright 2005 University Corporation for Atmospheric Research.
 *  All rights reserved.
 *
 *
 */

#ifndef LOG_H
#define LOG_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDomElement>
#include "Message.h"

class Log : public QWidget
{
  Q_OBJECT

 public:
  Log(QWidget *parent = 0);
  ~Log();
  bool saveLogFile();
  bool saveLogFile(const QString& fileName);
  QString getLogFileName() { return logFileName; }

 public slots:
  void setWorkingDirectory(const QString& newDir);
  void setLogFileName(const QString& newName);
  void catchLog(const Message& logEntry);

    
 signals:
  void newLogEntry(const QString & logEntry);
  void newProgressEntry(int progress);
  void log(const Message& message);

 private:
  QString logFileName;
  QFile *logFile;
  QString workingDirectory;
  int absoluteProgress;
  bool displayLocation;

  bool writeToFile(const QString& message);

};

#endif