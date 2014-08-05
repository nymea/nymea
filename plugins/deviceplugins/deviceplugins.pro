TEMPLATE = subdirs
SUBDIRS += elro         \
    intertechno         \
#   meisteranker        \
    wifidetector        \
    conrad              \
    mock                \
    openweathermap      \
    lircd               \
    wakeonlan           \
    mailnotification    \
    philipshue          \
    wemo                \


boblight {
    SUBDIRS += boblight
}
