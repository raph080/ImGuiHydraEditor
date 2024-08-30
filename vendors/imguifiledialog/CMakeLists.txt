project(imguifiledialog)

file(GLOB SRC_CPP
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  "ImGuiFileDialog/*.cpp"
)

file(GLOB PUBLIC_HEADERS
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  "ImGuiFileDialog/*.h"
)

add_library(${PROJECT_NAME} ${SRC_CPP} ${PUBLIC_HEADER})

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        IMGUI_IMPL_OPENGL_LOADER_GL3W=1
)
target_include_directories(${PROJECT_NAME}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/ImGuiFileDialog"
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