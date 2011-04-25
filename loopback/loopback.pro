HEADERS       = \
    qtcpclientconnection.h \
    dialog.h \
    mainwindow.h \
    qclientmanager.h \
    screenshot.h \
    directory.h \
    qremotedirmodel.h
SOURCES       = \
                main.cpp \
    qtcpclientconnection.cpp \
    qclientmanager.cpp \
    mainwindow.cpp \
    screenshot.cpp \
    directory.cpp \
    qremotedirmodel.cpp
QT           += network

# install
target.path = $$[QT_INSTALL_EXAMPLES]/network/loopback
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS loopback.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/network/loopback
INSTALLS += target sources

symbian: {
    TARGET.CAPABILITY = NetworkServices
    include($$PWD/../../symbianpkgrules.pri)
}
maemo5: include($$PWD/../../maemo5pkgrules.pri)

FORMS += \
    mainwindow.ui \
    directory.ui
