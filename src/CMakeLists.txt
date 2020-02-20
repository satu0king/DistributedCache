add_library(RE_CODE
    application.cpp
)

target_compile_features(RE_CODE PRIVATE cxx_std_17)
target_compile_options(RE_CODE PRIVATE "-Wall" "-Wextra")

find_package(Boost 1.72.0 REQUIRED)
target_link_libraries(RE_CODE PRIVATE Boost)

target_include_directories(RE_CODE PRIVATE ../include)

add_executable(app_server application.cpp basic_data_connector.cpp)