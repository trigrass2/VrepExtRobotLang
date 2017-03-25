#-------------------------------------------------
#
# Project created by QtCreator 2016-04-13T14:07:56
#
#-------------------------------------------------

QT       -= gui
QT       += xml
TEMPLATE = lib

DEFINES -= UNICODE
CONFIG += c++14
CONFIG += stl
CONFIG += staticlib
DEFINES += QT_COMPIL

win32 {
    DEFINES += WIN_VREP
    DEFINES += VREPEXT_ROBOTLANG
    INCLUDEPATH += "E:\vss\OutLib\boost_1_57_0"

    CONFIG(debug, debug|release) {
        TARGET = VrepExtRobotLangD
        DESTDIR = "C:/Program Files (x86)/V-REP3/V-REP_PRO_EDU"
        #DLLDESTDIR = "C:/Program Files (x86)/V-REP3/V-REP_PRO_EDU"
    } else {
        TARGET = VrepExtRobotLang
        DESTDIR = "C:/Program Files (x86)/V-REP3/V-REP_PRO_EDU"
        TARGET = VrepExtRobotLang
    }
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

SOURCES += CommonH.cpp \
    CommSubscriberConrete.cpp \
    Kss1500Compiler.cpp \
    Kss1500MotionPlannar.cpp \
    Kss1500MoveRange.cpp \
    Kss1500RobotMotion.cpp \
    RobotLangParserOpr.cpp \
    RobotMessages.cpp \
    RobotLangCommApis.cpp


HEADERS += CommonH.h \
    ICommObserver.h \
    ICommSubscribers.h \
    IRobotMotion.h \
    VrepExtExport.h \
    Kss1500Compiler.h \
    Kss1500MotionPlannar.h \
    Kss1500MoveRange.h \
    Kss1500RobotMotion.h \
    RobotMessages.h \
    RobotLangParserOpr.h \
    RobotLangCommApis.h
unix {
    target.path = /usr/lib
    INSTALLS += target


}

