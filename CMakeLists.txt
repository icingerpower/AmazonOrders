cmake_minimum_required(VERSION 3.5)

project(AmazonOrders VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# Compile and install QXlsx
#sudo apt-get install libxkbcommon-dev
#git clone https://github.com/QtExcel/QXlsx.git
#cd QXlsx
#mkdir build
#cd build
#cmake ../QXlsx/ -DCMAKE_PREFIX_PATH=/home/cedric/Qt/6.7.3/gcc_64 -DCMAKE_BUILD_TYPE=Release
#cmake --build .
#cmake --install .
set(Qt_DIR_HINT "${Qt${QT_VERSION_MAJOR}_DIR}")
message("Qt found in path: ${Qt_DIR_HINT}")
find_package(QXlsxQt6 REQUIRED HINTS "${Qt_DIR_HINT}/../../../../lib/cmake/QXlsxQt6")

include(../common/workingdirectory/workingdirectory.cmake)
include(../common/utils/utils.cmake)

set(PROJECT_SOURCES
        main.cpp
        MainWindow.cpp
        MainWindow.h
        MainWindow.ui
        DialogFilterOut.cpp
        DialogFilterOut.h
        DialogFilterOut.ui
        ListOrderModel.h
        ListOrderModel.cpp
        ListPatternTemplates.h
        ListPatternTemplates.cpp
        ListPatternSkus.h
        ListPatternSkus.cpp
        ListPatternNames.h
        ListPatternNames.cpp
        TableInventoryRecommendation.h
        TableInventoryRecommendation.cpp
        OrderCreator.h
        OrderCreator.cpp
        "../common/types/types.h"
        "../common/types/types.cpp"
        ${UTILS_FILES}
        ${WORKING_DIR_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(AmazonOrders
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        DialogMissingSkus.h DialogMissingSkus.cpp DialogMissingSkus.ui
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET AmazonOrders APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(AmazonOrders SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(AmazonOrders
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(AmazonOrders PRIVATE Qt${QT_VERSION_MAJOR}::Widgets QXlsx::QXlsx)

set_target_properties(AmazonOrders PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS AmazonOrders
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(AmazonOrders)
endif()
