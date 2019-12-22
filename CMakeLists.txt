cmake_minimum_required(VERSION 3.5)

project(qexplorer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(LibArchive REQUIRED)

if(${LibArchive_FOUND})
    include_directories(${LibArchive_INCLUDE_DIRS})
ENDIF(${LibArchive_FOUND})

add_executable(qexplorer
  main.cpp
  mainwindow.cpp
  mainwindow.h
  mainwindow.ui
  archiver.cpp
  archiver.h
)

target_link_libraries(qexplorer PRIVATE Qt5::Widgets ${LibArchive_LIBRARIES})
