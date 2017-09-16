TEMPLATE = app
TARGET = simplebrowser
QT += webenginewidgets
CONFIG += c++11
PRECOMPILED_HEADER += utf8.h
HEADERS += \
    browser.h \
    browserwindow.h \
    tabwidget.h \
    urllineedit.h \
    webview.h \
    webpage.h \
    webpopupwindow.h \
    starplug.h \
    utf8.h \
    stardialogresult.h \
    stardialogexam.h

SOURCES += \
    browser.cpp \
    browserwindow.cpp \
    main.cpp \
    tabwidget.cpp \
    urllineedit.cpp \
    webview.cpp \
    webpage.cpp \
    webpopupwindow.cpp \
    starplug.cpp \
    stardialogresult.cpp \
    stardialogexam.cpp

FORMS += \
    certificateerrordialog.ui \
    passworddialog.ui \
    starplug.ui \
    stardialogresult.ui \
    stardialogexam.ui

RESOURCES += data/simplebrowser.qrc \
    r.qrc

DISTFILES +=

DESTDIR = ../simplebrowser
