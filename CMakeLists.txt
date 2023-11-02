cmake_minimum_required(VERSION 3.12)

project(ImGuiHydraEditor VERSION 0.0.1 DESCRIPTION "ImGui Hydra Editor")

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(USD REQUIRED)
find_package(Python COMPONENTS Interpreter Development REQUIRED)

add_subdirectory(vendors)

file(GLOB_RECURSE SRC_CPP
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  "src/*.cpp"
)
file(GLOB_RECURSE SRC_H
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  "src/*.h"
)

add_executable(${PROJECT_NAME} ${SRC_CPP} ${SRC_H})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        # where the library itself will look for its internal headers
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        # where top-level project will look for the library's public headers
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external>
        # where external projects will look for the library's public headers
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>

        ${USD_INCLUDE_DIR}
        ${Python_INCLUDE_DIRS}
        imgui
        imguizmo
        imguicolortextedit
        imguifiledialog
)


target_link_libraries(${PROJECT_NAME}
    PUBLIC
        ${USD_BOOST_LIBRARIES}
        ${USD_LIBRARIES}
        ${Python_LIBRARIES}
        libglew_static
        glfw
        imgui
        imguizmo
        imguicolortextedit
        imguifiledialog
)

install(
    TARGETS ${PROJECT_NAME}
)

