CONFIG +=	warn_on \
			debug
CONFIG -=	qt
QT -= gui core

TEMPLATE = app
TARGET = simpletest
DESTDIR = bin
INCLUDEPATH += ../

MOC_DIR	= temp/$$TARGET/moc
UI_DIR = temp/$$TARGET/ui
OBJECTS_DIR = temp/$$TARGET/obj

SOURCES +=	simpletest.cpp \
			../bplustree.cpp 


