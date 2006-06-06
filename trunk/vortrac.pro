######################################################################
# Automatically generated by qmake (2.00a) Thu Jul 21 17:39:59 2005
######################################################################

TEMPLATE = app
DEPENDPATH += . Config Grids GUI IO Radar
INCLUDEPATH += . GUI Config IO Radar

# Input
HEADERS += DataObjects/PressureData.h \
           Threads/AnalysisThread.h \
           Threads/PollThread.h \
           Threads/SimplexThread.h \
           DataObjects/VortexData.h \
           DataObjects/SimplexData.h \
           DataObjects/VortexList.h \
           DataObjects/SimplexList.h \
           DataObjects/Coefficient.h \
           DataObjects/Center.h \
           Config/Configuration.h \
           DataObjects/AnalyticGrid.h \
           DataObjects/CappiGrid.h \
           DataObjects/GriddedData.h \
           DataObjects/GriddedFactory.h \
           GUI/ConfigTree.h \
           GUI/ConfigurationDialog.h \
           GUI/MainWindow.h \
           GUI/AnalysisPage.h \
           GUI/GraphFace.h \
           GUI/KeyPicture.h \
           GUI/TestGraph.h \
           GUI/AbstractPanel.h \ 
           GUI/panels.h \
           GUI/DiagnosticPanel.h \
           GUI/RadarListDialog.h \
           GUI/StopLight.h \
           HVVP/Hvvp.h \
           IO/Message.h \
           IO/Log.h \
           Radar/RadarFactory.h \
           Radar/LevelII.h \
           Radar/AnalyticRadar.h \
           Radar/nexh.h \
           Radar/RadarQC.h \
           Radar/RadarData.h \
           Radar/Ray.h \
           Radar/Sweep.h \
           VTD/GBVTD.h \
           Math/Matrix.h \
           ChooseCenter.h 
SOURCES += main.cpp \
           DataObjects/PressureData.cpp \
           Threads/AnalysisThread.cpp \
           Threads/PollThread.cpp \
           Threads/SimplexThread.cpp \
           DataObjects/VortexData.cpp \
           DataObjects/SimplexData.cpp \
           DataObjects/VortexList.cpp \
           DataObjects/SimplexList.cpp \
           DataObjects/Coefficient.cpp \
           DataObjects/Center.cpp \
           Config/Configuration.cpp \
           DataObjects/AnalyticGrid.cpp \
           DataObjects/CappiGrid.cpp \
           DataObjects/GriddedData.cpp \
           DataObjects/GriddedFactory.cpp \
           GUI/ConfigTree.cpp \
           GUI/ConfigurationDialog.cpp \
           GUI/MainWindow.cpp \
           GUI/AnalysisPage.cpp \
           GUI/GraphFace.cpp \ 
           GUI/KeyPicture.cpp \
           GUI/TestGraph.cpp \
           GUI/AbstractPanel.cpp \
           GUI/panels.cpp \
           GUI/DiagnosticPanel.cpp \
           GUI/RadarListDialog.cpp \
           GUI/StopLight.cpp \
           HVVP/Hvvp.cpp \
           IO/Message.cpp \
           IO/Log.cpp \
           Radar/RadarFactory.cpp \
           Radar/LevelII.cpp \
           Radar/AnalyticRadar.cpp\
           Radar/RadarQC.cpp \
           Radar/RadarData.cpp \
           Radar/Ray.cpp \
           Radar/Sweep.cpp \
           VTD/GBVTD.cpp \
           Math/Matrix.cpp \
           ChooseCenter.cpp 
F95 = gfortran
g95.output = ${QMAKE_FILE_BASE}.o
g95.commands = $$F95 -c -g  -O0 ${QMAKE_FILE_NAME} \
  -o ${QMAKE_FILE_OUT} ${INCPATH}
g95.input    = FORTRAN_SOURCES
FORTRAN_SOURCES = 
QMAKE_EXTRA_UNIX_COMPILERS += g95 
LIBS += -lgfortran -lg2c
RESOURCES += vortrac.qrc
QT += xml
