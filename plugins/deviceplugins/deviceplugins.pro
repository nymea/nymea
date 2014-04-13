TEMPLATE = subdirs
SUBDIRS += elro     \
    intertechno     \
#    meisteranker    \
    wifidetector    \
#    conrad          \
    mock            \
#    weatherground   \
    openweathermap  \

boblight {
    SUBDIRS += boblight
}
