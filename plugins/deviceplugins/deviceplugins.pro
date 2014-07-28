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
    eq-3                \

boblight {
    SUBDIRS += boblight
}
