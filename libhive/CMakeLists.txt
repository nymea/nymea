include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(libhive_SRCS
    device.cpp
    deviceclass.cpp
    deviceplugin.cpp
)

add_library(libhive ${libhive_SRCS})

qt5_use_modules(libhive Network)

