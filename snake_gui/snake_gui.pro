QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14

TARGET = snake_gui
TEMPLATE = app


# QT += multimedia (Osa taustamusiikki ominaisuutta)

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dialog.cpp \
    gameboard.cpp \
    main.cpp \
    mainwindow.cpp \
    point.cpp

HEADERS += \
    dialog.hh \
    gameboard.hh \
    mainwindow.hh \
    point.hh

FORMS += \
    dialog.ui \
    mainwindow.ui


DISTFILES += \

    Resources/apple.png \


RESOURCES += \
    mainwindow.qrc
