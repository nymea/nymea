include(guh.pri)

# Create translation files
lrelease.input = TRANSLATIONS
lrelease.output = $$top_srcdir/${QMAKE_FILE_BASE}.qm
lrelease.commands = $$[QT_INSTALL_BINS]/lupdate ${QMAKE_FILE_IN} -ts $$TRANSLATIONS; $$[QT_INSTALL_BINS]/lrelease ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
lrelease.CONFIG += no_link

QMAKE_EXTRA_COMPILERS += lrelease
PRE_TARGETDEPS += compiler_lrelease_make_all

message("Translations:" $$TRANSLATIONS )

translations.path = /usr/share/guh/translations
translations.files = $$[QT_SOURCE_TREE]/translations/*.qm

INSTALLS += translations
