CONFIG +=	warn_on \
			debug
CONFIG -=	qt
QT -= gui core

TEMPLATE = app
TARGET = speedtest
DESTDIR = bin
INCLUDEPATH += ../

MOC_DIR	= temp/$$TARGET/moc
UI_DIR = temp/$$TARGET/ui
OBJECTS_DIR = temp/$$TARGET/obj

SOURCES +=	speedtest.cpp \
			../bplustree.cpp 


