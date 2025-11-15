add_library(mrn_plugin_stub STATIC
    plugins_stub/plugin_stub.cpp
)

target_include_directories(mrn_plugin_stub PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)
