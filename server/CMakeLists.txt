include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/libhive
)

set(hive_SRCS
    main.cpp
    hivecore.cpp
    jsonrpcserver.cpp
    tcpserver.cpp
    radio433.cpp
)

add_executable(hive ${hive_SRCS})
target_link_libraries(hive libhive)

qt5_use_modules(hive Network)
