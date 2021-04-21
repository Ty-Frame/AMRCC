QT       += core gui \
            location \
            charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    chartpopulator.cpp \
    chartsettingsdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    mapsettingsdialog.cpp \
    shothistoryindex.cpp \
    shotinfodialog.cpp \
    shotqueuemanager.cpp

HEADERS += \
    chartpopulator.h \
    chartsettingsdialog.h \
    mainwindow.h \
    mapsettingsdialog.h \
    shothistoryindex.h \
    shotinfo.h \
    shotinfodialog.h \
    shotqueuemanager.h

FORMS += \
    chartsettingsdialog.ui \
    mainwindow.ui \
    mapsettingsdialog.ui \
    shothistoryindex.ui \
    shotinfodialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    LockIcon.png

RESOURCES += \
    Resources.qrc
