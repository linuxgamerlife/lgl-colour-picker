cmake_minimum_required(VERSION 3.20)
project(lgl-colour-picker VERSION 1.0.2 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)
include(CTest)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wall> $<$<COMPILE_LANGUAGE:CXX>:-Wextra>
                        $<$<COMPILE_LANGUAGE:CXX>:-Wpedantic> $<$<COMPILE_LANGUAGE:CXX>:-Werror>)
endif()

option(LGL_COLOUR_PICKER_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
if(LGL_COLOUR_PICKER_ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-4.0>=4.20)
pkg_check_modules(ADWAITA REQUIRED IMPORTED_TARGET libadwaita-1>=1.6)
pkg_check_modules(JSON_GLIB REQUIRED IMPORTED_TARGET json-glib-1.0)
find_program(GLIB_COMPILE_RESOURCES glib-compile-resources REQUIRED)

set(RESOURCE_XML ${PROJECT_SOURCE_DIR}/data/colour-picker.gresource.xml)
set(RESOURCE_C ${CMAKE_CURRENT_BINARY_DIR}/colour-picker-resources.c)
set(RESOURCE_DEPS ${PROJECT_SOURCE_DIR}/data/style.css ${PROJECT_SOURCE_DIR}/HELP.md
    ${PROJECT_SOURCE_DIR}/packaging/icons/256x256/lgl-colour-picker.png)
add_custom_command(OUTPUT ${RESOURCE_C}
    COMMAND ${GLIB_COMPILE_RESOURCES} --generate-source --target=${RESOURCE_C}
            --sourcedir=${PROJECT_SOURCE_DIR}/data --sourcedir=${PROJECT_SOURCE_DIR} ${RESOURCE_XML}
    DEPENDS ${RESOURCE_XML} ${RESOURCE_DEPS} VERBATIM)
set_source_files_properties(${RESOURCE_C} PROPERTIES COMPILE_OPTIONS "-w")

add_library(colour_picker_core STATIC src/core/colour.cpp src/core/colour.h src/core/settings.cpp
    src/core/settings.h src/core/theme.cpp src/core/theme.h)
target_include_directories(colour_picker_core PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(colour_picker_core PUBLIC PkgConfig::GTKMM PkgConfig::JSON_GLIB)

add_executable(lgl-colour-picker ${RESOURCE_C} src/main.cpp src/ui/themewatcher.cpp
    src/ui/themewatcher.h src/ui/pickercontroller.cpp src/ui/pickercontroller.h src/ui/helpwindow.cpp
    src/ui/helpwindow.h src/ui/window.cpp src/ui/window.h)
target_compile_definitions(lgl-colour-picker PRIVATE LGL_COLOUR_PICKER_VERSION="${PROJECT_VERSION}")
target_link_libraries(lgl-colour-picker PRIVATE colour_picker_core PkgConfig::ADWAITA)

install(TARGETS lgl-colour-picker RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES packaging/com.linuxgamerlife.lgl-colour-picker.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES packaging/com.linuxgamerlife.lgl-colour-picker.metainfo.xml
        DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
foreach(size 48 64 128 256)
    install(FILES packaging/icons/${size}x${size}/lgl-colour-picker.png
            DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/${size}x${size}/apps)
endforeach()

if(BUILD_TESTING)
    foreach(test colour settings theme)
        add_executable(${test}_test tests/${test}_test.cpp)
        target_link_libraries(${test}_test PRIVATE colour_picker_core)
        add_test(NAME ${test} COMMAND ${test}_test)
    endforeach()
endif()
