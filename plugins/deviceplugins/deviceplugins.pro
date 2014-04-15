TEMPLATE = subdirs
SUBDIRS += elro     \
    intertechno     \
#    meisteranker    \
    wifidetector    \
#    conrad          \
    mock            \
#    weatherground   \
    openweathermap  \
    lircd \

boblight {
    SUBDIRS += boblight
}
