# Compatibility build; CMakeLists.txt is the canonical build and test entry point.
QT += core gui widgets svg
CONFIG += c++17
TARGET = QtLensSimulator
TEMPLATE = app
INCLUDEPATH += . core glm raytracer
SOURCES += main.cpp mainwindow.cpp lensprofileview.cpp lens.cpp \
    core/lensanalysis.cpp raytracer/ray.cpp raytracer/plane.cpp raytracer/shape.cpp
HEADERS += mainwindow.h lensprofileview.h lens.h core/optics.h core/lensanalysis.h \
    raytracer/ray.h raytracer/plane.h raytracer/shape.h
win32-msvc*:QMAKE_CXXFLAGS += /std:c++17 /utf-8
