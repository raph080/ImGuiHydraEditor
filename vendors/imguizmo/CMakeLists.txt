project(imguizmo)

file(GLOB SRC_CPP
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  "ImGuizmo/*.cpp"
)

file(GLOB PUBLIC_HEADERS
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  "ImGuizmo/*.h"
)

add_library(${PROJECT_NAME} ${SRC_CPP} ${PUBLIC_HEADER})

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        IMGUI_IMPL_OPENGL_LOADER_GL3W=1
        # IMGUI_DISABLE_OBSOLETE_FUNCTIONS # to check for obsolete functions
        # IMGUI_USER_CONFIG="${PROJECT_SOURCE_DIR}/path/to/imconfig.h" # to use your own imconfig.h
)
target_include_directories(${PROJECT_NAME}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo"
        imgui
)
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        glfw
        imgui
)
set_target_properties(${PROJECT_NAME}
    PROPERTIES
        PUBLIC_HEADER "${PUBLIC_HEADERS}"
)

install(
    TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)