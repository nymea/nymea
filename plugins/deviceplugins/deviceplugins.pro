TEMPLATE = subdirs
SUBDIRS += elro     \
    intertechno     \
#    meisteranker    \
    wifidetector    \
    conrad          \
    mock            \
    openweathermap  \
    lircd           \
    googlemail      \
    wakeonlan       \

boblight {
    SUBDIRS += boblight
}
