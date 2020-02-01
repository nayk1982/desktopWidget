#===============================================================================
#
# Qt Project: Resources Monitor for Desktop
#
#===============================================================================

QT += core gui widgets network
CONFIG += hardware

include( $${PWD}/../../_nayk/nayk.pri )

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

# Output dir ===================================================================

CONFIG(release, debug|release) {
    win32: DESTDIR = $${PWD}/../_distrib/win_$${QMAKE_HOST.arch}/bin
    else: unix:!android: DESTDIR  = $${PWD}/../_distrib/linux_$${QMAKE_HOST.arch}/bin
}

# translations =================================================================

TRANSLATIONS += \
    $${PWD}/translations/main_ru.ts \
    $${PWD}/../../_nayk/resources/translations/nayk_common_ru.ts \
    $${PWD}/../../_nayk/resources/translations/nayk_widget_ru.ts


main_tr.commands = lrelease $${PWD}/translations/main_ru.ts -qm $${PWD}/translations/main_ru.qm
nayk_common_tr.commands = lrelease $${PWD}/../../_nayk/resources/translations/nayk_common_ru.ts -qm $${PWD}/translations/nayk_common_ru.qm
nayk_widget_tr.commands = lrelease $${PWD}/../../_nayk/resources/translations/nayk_widget_ru.ts -qm $${PWD}/translations/nayk_widget_ru.qm

PRE_TARGETDEPS += \
    main_tr \
    nayk_common_tr \
    nayk_widget_tr

QMAKE_EXTRA_TARGETS += \
    main_tr \
    nayk_common_tr \
    nayk_widget_tr
