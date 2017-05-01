#-------------------------------------------------
#
# Project created by QtCreator 2014-11-26T01:57:21
#
#-------------------------------------------------

QT       += core gui opengl svg

greaterThan(QT_MAJOR_VERSION, 5): QT += widgets

TARGET = QtLensSimulator
TEMPLATE = app

unix {
    # Qt 4.8 on linux requires the -std=c++0x option
    QMAKE_CXXFLAGS += -std=c++11
}

INCLUDEPATH += glm raytracer

SOURCES += main.cpp\
        mainwindow.cpp \
    lensprofileview.cpp \
    lens.cpp \
    raytracer/camera.cpp \
    raytracer/image.cpp \
    raytracer/light.cpp \
    raytracer/object.cpp \
    raytracer/plane.cpp \
    raytracer/ray.cpp \
    raytracer/raytracer.cpp \
    raytracer/scene.cpp \
    raytracer/sceneparser.cpp \
    raytracer/shape.cpp \
    raytracer/sphere.cpp \
    raytracer/triangle.cpp

HEADERS  += mainwindow.h \
    lensprofileview.h \
    lens.h \
    raytracer/utility.h \
    raytracer/camera.h \
    raytracer/image.h \
    raytracer/light.h \
    raytracer/object.h \
    raytracer/plane.h \
    raytracer/ray.h \
    raytracer/raytracer.h \
    raytracer/scene.h \
    raytracer/sceneparser.h \
    raytracer/shadinginfo.h \
    raytracer/shape.h \
    raytracer/sphere.h \
    raytracer/triangle.h

FORMS    += mainwindow.ui
