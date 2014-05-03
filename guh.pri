guh_version_major = 0
guh_version_minor = 0
guh_version_patch = 1

coverage {
    message("Building coverage.")
    QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage -O0
    LIBS += -lgcov
    QMAKE_LFLAGS += -fprofile-arcs
}

message("fooooooooo $${guh_version_major} $${MYVAR}")
DEFINES += GUH_VERSION_MAJOR=$$guh_version_major
DEFINES += GUH_VERSION_MINOR=$$guh_version_minor
DEFINES += GUH_VERSION_PATCH=$$guh_version_patch
DEFINES += GUH_VERSION_STRING=\\\"$${guh_version_major}.$${guh_version_minor}.$${guh_version_patch}\\\"
