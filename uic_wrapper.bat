@echo off
SetLocal EnableDelayedExpansion
(set PATH=d:\QtSource\qt-everywhere-opensource-src-5.7.0\qtbase\lib;!PATH!)
if defined QT_PLUGIN_PATH (
    set QT_PLUGIN_PATH=D:\QtSource\qt-everywhere-opensource-src-5.7.0\qtbase\plugins;!QT_PLUGIN_PATH!
) else (
    set QT_PLUGIN_PATH=D:\QtSource\qt-everywhere-opensource-src-5.7.0\qtbase\plugins
)
d:\QtSource\qt-everywhere-opensource-src-5.7.0\qtbase\bin\uic.exe %*
EndLocal
