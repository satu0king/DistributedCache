cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(RE VERSION 0.1)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# # should we use our own math functions
# option(USE_MYMATH "Use tutorial provided math implementation" ON)

# configure a header file to pass some of the CMake settings
# to the source code
# configure_file(TutorialConfig.h.in TutorialConfig.h)

# add the MathFunctions library
# if(USE_MYMATH)
#   add_subdirectory(MathFunctions)
#   list(APPEND EXTRA_LIBS MathFunctions)
# endif()

add_subdirectory(src)

# add the executable
# add_executable(app src/application.cpp)


# target_link_libraries(Tutorial PUBLIC ${EXTRA_LIBS})

# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
# target_include_directories(Tutorial PUBLIC
#                            "${PROJECT_BINARY_DIR}"
#                            )
