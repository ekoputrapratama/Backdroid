cmake_minimum_required(VERSION 3.15)
project(backdroid VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(QT_QML_GENERATE_QMLLS_INI. ON)
set(QT_MIN_REQUIRED_VERSION "6.10.0")
add_compile_definitions(QT_DISABLE_DEPRECATED_UP_TO=0x050F00)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set_property(GLOBAL PROPERTY TARGET_DIR ${CMAKE_CURRENT_BINARY_DIR})

include(FeatureSummary)

find_package(Python 3.14 REQUIRED)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DBACKDROID_DEBUG) 
  add_definitions(-DQT_DISABLE_DEPRECATED_UP_TO=0x050F00) 
endif()

# Find Qt modules
find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
  Core
  Gui
  Qml
  QmlModels
  QmlWorkerScript
  Network
  Widgets
  Quick
  QuickControls2
  QuickTemplates2
  DBus
  Xml
  Sql
)
find_package(usb-1.0 REQUIRED)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux") 
  find_package(udev REQUIRED)
  # find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS XcbQpa)
elseif(WIN32)
  find_library(USER32 user32)
endif()

include(cmake/BackDroidSetup.cmake)
add_subdirectory(lib/backdroid-common)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DBACKDROID_DEBUG)  
endif()

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}/include
)

set(backdroid_RESOURCES
  qml/backdroid.qrc
)

set(backdroid_SRCS
  src/main.cpp
  src/Backdroid.cpp
  src/Settings.cpp
)

set(backdroid_HDRS
  include/main.h
  include/Backdroid.h
  include/Settings.h
)

# qt5_add_resources (RCC_SOURCES ${bnr_RESOURCES})
get_property(OUTPUT_PATH GLOBAL PROPERTY TARGET_DIR)
# set(LIBRARY_OUTPUT_PATH ${OUTPUT_PATH}/plugins/${PROJECT_NAME})

add_executable(backdroid ${backdroid_SRCS} ${backdroid_HDRS} ${backdroid_RESOURCES})
target_include_directories(backdroid 
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
  ${CMAKE_HOME_DIRECTORY}/lib/backdroid-common/include
  /usr/include/python3.14
  # ${PYTHON_INCLUDE_DIRS}
  # ${Boost_INCLUDE_DIRS}
)
find_package(Qt6 COMPONENTS Core Gui Qml Sql Quick QuickControls2 QuickTemplates2 REQUIRED)
target_link_libraries(backdroid PUBLIC 
  backdroid-common
  Qt6::Core
  Qt6::Gui
  Qt6::Qml
  Qt6::QmlModels
  Qt6::Network
  Qt6::Widgets
  Qt6::Sql
  Qt6::Quick
  Qt6::QuickControls2
  Qt6::QuickTemplates2
  Qt6::Xml
  python3.14
)

# set(bnrd_SRCS
#   src/bnrd.cpp
#   src/BnrServer.cpp
# )
# set(bnrd_HDRS
#   include/bnrd.h
#   include/BnrServer.h
# )

# add_executable(bnrd ${bnrd_SRCS} ${bnrd_HDRS})

# target_link_libraries(bnrd PRIVATE 
#   Qt5::Core
#   Qt5::DBus
#   Qt5::Network
# )
# include_files(bnr src/bnrd.py src/message.py bnr.plugin)
# file(COPY bnr.plugin DESTINATION ${LIBRARY_OUTPUT_PATH})
# file(COPY ${CMAKE_CURRENT_BINARY_DIR}/libbnr.so ${OUTPUT_PATH}/plugins/${PROJECT_NAME})
# install(TARGETS ${PROJECT_NAME}
# 		LIBRARY DESTINATION lib/stella/plugins/${PROJECT_NAME})
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)