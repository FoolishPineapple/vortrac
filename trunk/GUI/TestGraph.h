/*
 * TestGraph.h
 * VORTRAC
 *
 * Created by Lisa Mauger on 8/9/05
 *  Copyright 2005 University Corporation for Atmospheric Research.
 *  All rights reserved.
 *
 *
 */

#ifndef TESTGRAPH_H
#define TESTGRAPH_H

#include <QWidget>
#include "DataObjects/VortexData.h"

class TestGraph:public QWidget
{
Q_OBJECT
public:
TestGraph(QWidget *parent = 0);
  void examplePlotNumbers(QList<VortexData> *VortexPointer,
			  QList<VortexData> *dropPointer, 
			  int number_of_VortexData);
  // This function allows us to plot sample data points;
  // All the data points viewed on the graph are generated within this function

  public slots:
    void examplePlot();

  signals:
  void listChanged(QList<VortexData> *glist);
  void dropListChanged(QList<VortexData> *gDropList);
};

#endif