QT += core gui widgets

CONFIG += c++17

VERSION = 1.0.0.0

DESTDIR = $$PWD/bin
LIBS += -L$$PWD/lib
LIBS += -lbass -lbassopus -lbassmidi

SOURCES += $$files($$PWD/src/*.cpp)
HEADERS += $$files($$PWD/include/*.h)
FORMS += $$files($$PWD/resource/*.ui)
