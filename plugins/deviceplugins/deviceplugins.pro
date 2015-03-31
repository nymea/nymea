TEMPLATE = subdirs
SUBDIRS += elro         \
    intertechno         \
    wifidetector        \
    conrad              \
    mock                \
    openweathermap      \
    lircd               \
    wakeonlan           \
    mailnotification    \
    philipshue          \
    eq-3                \
    wemo                \
    lgsmarttv           \
    datetime            \
    genericelements     \
    commandlauncher     \
    unitec              \
    leynew              \

boblight {
    SUBDIRS += boblight
}

contains(DEFINES, BLUETOOTH_LE) {
    SUBDIRS += elgato
}
