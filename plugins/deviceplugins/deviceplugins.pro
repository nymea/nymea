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
    lgsmarttv           \
    datetime            \
    genericelements     \
    commandlauncher     \
    unitec              \
    leynew              \
    tune                \
    udpcommander        \
    kodi                \



boblight {
    SUBDIRS += boblight
}

contains(DEFINES, BLUETOOTH_LE) {
    SUBDIRS += elgato
}
