#//////////////////////////////////////////
# тестовая работа к занятиям https://youtu.be/6YgHevU0UxE & https://youtu.be/EFevocY6NHY
#//////////////////////////////////////////

QT       += core gui widgets

TARGET = ogltest13-14
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++17

SOURCES += \
        engine3d/eye.cpp \
        engine3d/light.cpp \
        engine3d/material.cpp \
        engine3d/materiallibrary.cpp \
        engine3d/object3d.cpp \
        engine3d/object3delement.cpp \
        engine3d/object3dgroup.cpp \
        engine3d/skybox.cpp \
        engine3d/transformational.cpp \
        main.cpp \
        oglwidget.cpp

HEADERS += \
        engine3d/eye.h \
        engine3d/light.h \
        engine3d/material.h \
        engine3d/materiallibrary.h \
        engine3d/object3d.h \
        engine3d/object3delement.h \
        engine3d/object3dgroup.h \
        engine3d/skybox.h \
        engine3d/transformational.h \
        oglwidget.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    models.qrc \
    shaders.qrc \
    textures.qrc

win32-g* {
LIBS += -lopengl32
}

win32-msvc* {
LIBS += lopengl32.lib
}

DISTFILES +=
