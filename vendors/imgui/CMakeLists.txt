project(imgui)

set(SRC_CPP 
    "imgui/imgui.cpp"
    "imgui/imgui_demo.cpp"
    "imgui/imgui_draw.cpp"
    "imgui/imgui_widgets.cpp"
    "imgui/imgui_tables.cpp"
    "imgui/misc/cpp/imgui_stdlib.cpp"
)

set(H_CPP 
    "imgui/imgui_internal.h"
    "imgui/misc/cpp/imgui_stdlib.h"
)

set(PUBLIC_HEADERS 
    "imgui/imgui.h"
)

# Add the backend for the graphics according to the OS
if(APPLE AND NOT FORCE_OPENGL)
    list(APPEND SRC_CPP
        "imgui/backends/imgui_impl_metal.mm"
        "imgui/backends/imgui_impl_osx.mm"
    )
    list(APPEND PUBLIC_HEADERS
        "imgui/backends/imgui_impl_metal.h"
        "imgui/backends/imgui_impl_osx.h"
    )
else()
    list(APPEND SRC_CPP
        "imgui/backends/imgui_impl_opengl3.cpp"
        "imgui/backends/imgui_impl_glfw.cpp"
    )
    list(APPEND PUBLIC_HEADERS
        "imgui/backends/imgui_impl_opengl3.h"
        "imgui/backends/imgui_impl_glfw.h"
    )
endif()

add_library(${PROJECT_NAME} ${SRC_CPP} ${H_CPP} ${PUBLIC_HEADER})

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        IMGUI_IMPL_OPENGL_LOADER_GL3W=1
        # IMGUI_DISABLE_OBSOLETE_FUNCTIONS # to check for obsolete functions
        # IMGUI_USER_CONFIG="${PROJECT_SOURCE_DIR}/path/to/imconfig.h" # to use your own imconfig.h
)
target_include_directories(${PROJECT_NAME}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends"
)
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        glfw
)
set_target_properties(${PROJECT_NAME}
    PROPERTIES
        PUBLIC_HEADER "${PUBLIC_HEADERS}"
)