#===============================================================================
#
# Qt Project: Resources Monitor for Desktop
#
#===============================================================================

QT += core gui widgets network

include( $${PWD}/../../../_nayk/nayk_common.pri )

# Sources and Headers ==========================================================

SOURCES += \
    canvas.cpp \
    common.cpp \
    currentlocationobject.cpp \
    customscene.cpp \
    main.cpp \
    monitorobject.cpp \
    settings.cpp \
    utclineobject.cpp \
    utctextobject.cpp \
    worldmapobject.cpp

HEADERS += \
    canvas.h \
    common.h \
    currentlocationobject.h \
    customscene.h \
    monitorobject.h \
    settings.h \
    utclineobject.h \
    utctextobject.h \
    worldmapobject.h

# Resources ====================================================================

RESOURCES += \
    main.qrc

win32:RC_FILE = main.rc

# translations =================================================================

TRANSLATIONS += \
    $${PWD}/translations/main_ru.ts

main_tr.commands = lrelease $${PWD}/translations/main_ru.ts -qm $${TRANSLATIONS_DIR}/main_ru.qm

PRE_TARGETDEPS += \
    main_tr

QMAKE_EXTRA_TARGETS += \
    main_tr

# Libs =========================================================================

INCLUDEPATH *= \
    $${COMMON_LIBS_DIR}/include

win32 {
    NAYK_LIB_VER = 1
}

LIBS *= \
    -L$${COMMON_LIBS_DIR} \
    -lnayk_core$${NAYK_LIB_VER} \
    -lnayk_widgets$${NAYK_LIB_VER} \
    -lnayk_graph$${NAYK_LIB_VER} \
    -lnayk_hardware$${NAYK_LIB_VER} \
    -lnayk_network$${NAYK_LIB_VER} \
