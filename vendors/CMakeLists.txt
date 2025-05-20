add_subdirectory(imgui)
add_subdirectory(imguizmo)
add_subdirectory(imguicolortextedit)
add_subdirectory(imguifiledialog)

if((NOT APPLE) OR FORCE_OPENGL)
    add_subdirectory(glew)
    add_subdirectory(glfw)
endif()