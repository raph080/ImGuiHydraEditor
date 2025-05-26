cmake_minimum_required(VERSION 3.12)

project(ImGuiHydraEditor VERSION 0.0.1 DESCRIPTION "ImGui Hydra Editor")

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(pxr REQUIRED)
find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)


add_subdirectory(vendors)

file(GLOB_RECURSE SRC_CPP
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  "src/*.cpp"
)
file(GLOB_RECURSE SRC_H
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  "src/*.h"
)

# Configure RPATH so libraries are found after make install
if(APPLE)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
elseif(UNIX)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib64")
endif()


add_executable(${PROJECT_NAME} ${SRC_CPP} ${SRC_H})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${PXR_LIBRARIES}
        libglew_static
        glfw
        imgui
        imguizmo
        imguicolortextedit
        imguifiledialog
)

include_directories(${PROJECT_NAME} ${PXR_INCLUDE_DIRS})

if(WIN32)
    target_compile_definitions(${PROJECT_NAME}
        PRIVATE
            NOMINMAX
            _USE_MATH_DEFINES
    )
endif()


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
