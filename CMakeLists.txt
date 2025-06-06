cmake_minimum_required(VERSION 3.12)

project(ImGuiHydraEditor VERSION 0.0.1 DESCRIPTION "ImGui Hydra Editor")

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(X11)
find_package(pxr REQUIRED)
find_package(OpenGL REQUIRED)

add_subdirectory(vendors)

file(GLOB SRC_CPP
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  "src/*.cpp"
  "src/layouts/*.cpp"
  "src/models/*.cpp"
  "src/sceneindices/*.cpp"
  "src/style/*.cpp"
  "src/views/*.cpp"
)

file(GLOB SRC_H
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  "src/*.h"
  "src/layouts/*.h"
  "src/models/*.h"
  "src/sceneindices/*.h"
  "src/style/*.h"
  "src/views/*.h"
  "src/backends/*.h"
)

set(LIBRARIES
    ${PXR_LIBRARIES}
    imgui
    imguizmo
    imguicolortextedit
    imguifiledialog
)

# Add the backend for the graphics according to the OS
if(APPLE AND NOT FORCE_OPENGL)
    list(APPEND SRC_CPP "src/backends/metal.mm")
    list(APPEND LIBRARIES
            "-framework Cocoa"
            "-framework Metal"
            "-framework QuartzCore"
            "-framework GameController"
            "-framework AppKit"
            "-framework MetalKit"
    )
else()
    list(APPEND SRC_CPP "src/backends/opengl.cpp")
    list(APPEND LIBRARIES
        libglew_static
        glfw
    )
endif()

# Configure RPATH so libraries are found after make install
if(APPLE)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
elseif(UNIX)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib64")
elseif(WIN32)
    target_compile_definitions(${PROJECT_NAME}
        PRIVATE
            NOMINMAX
            _USE_MATH_DEFINES
    )
endif()

add_executable(${PROJECT_NAME} ${SRC_CPP} ${SRC_H})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${LIBRARIES}
)

include_directories(${PROJECT_NAME} ${PXR_INCLUDE_DIRS})

# --------------- install section ---------------

install(
    TARGETS ${PROJECT_NAME}
)

IF(EXISTS ${PXR_CMAKE_DIR}/lib)
    INSTALL(
        DIRECTORY ${PXR_CMAKE_DIR}/lib
        DESTINATION ${CMAKE_INSTALL_PREFIX}
    )
endif()

IF(EXISTS ${PXR_CMAKE_DIR}/lib64)
    INSTALL(
        DIRECTORY ${PXR_CMAKE_DIR}/lib64
        DESTINATION ${CMAKE_INSTALL_PREFIX}
    )
endif()

INSTALL(
    DIRECTORY ${PXR_CMAKE_DIR}/plugin
    DESTINATION ${CMAKE_INSTALL_PREFIX}
)
