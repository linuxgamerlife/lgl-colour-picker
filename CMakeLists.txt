cmake_minimum_required(VERSION 3.16)
project(lgl-colour-picker VERSION 1.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core DBus Widgets)

qt_add_executable(lgl-colour-picker
    src/main.cpp
    src/colourpicker.cpp
    src/colourpicker.h
    resources.qrc
)

target_link_libraries(lgl-colour-picker PRIVATE Qt6::Core Qt6::DBus Qt6::Widgets)

target_compile_options(lgl-colour-picker PRIVATE
    -Wall -Wextra -Wpedantic
)

install(TARGETS lgl-colour-picker RUNTIME DESTINATION bin)
install(FILES packaging/lgl-colour-picker.desktop
    DESTINATION share/applications)
install(FILES packaging/lgl-colour-picker.metainfo.xml
    DESTINATION share/metainfo)
install(FILES packaging/icons/48x48/lgl-colour-picker.png
    DESTINATION share/icons/hicolor/48x48/apps)
install(FILES packaging/icons/64x64/lgl-colour-picker.png
    DESTINATION share/icons/hicolor/64x64/apps)
install(FILES packaging/icons/128x128/lgl-colour-picker.png
    DESTINATION share/icons/hicolor/128x128/apps)
install(FILES packaging/icons/256x256/lgl-colour-picker.png
    DESTINATION share/icons/hicolor/256x256/apps)
