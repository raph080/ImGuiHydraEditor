cmake_minimum_required(VERSION 3.12)

project(ImGuiHydraEditor VERSION 0.0.1 DESCRIPTION "ImGui Hydra Editor")

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(pxr REQUIRED)

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
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        usdImagingGL
        libglew_static
        glfw
        imgui
        imguizmo
        imguicolortextedit
        imguifiledialog
)

if(WIN32)
    target_compile_definitions(${PROJECT_NAME}
        PRIVATE
            NOMINMAX
            _USE_MATH_DEFINES
    )
endif()

install(
    TARGETS ${PROJECT_NAME}
)

