#include "backend.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

#include "pxr/imaging/hgiGL/texture.h"
#include "pxr/imaging/hgi/texture.h"

#include <cstdint>
#include <iostream>

static GLFWwindow* window = nullptr;


int InitBackend(const char* title, int width, int height)
{
    // Initialize glfw
    if (!glfwInit()) return -1;

    #if defined(__APPLE__)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE,
                   GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    // Create a GLFW window
    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // init imgui
#if defined(__APPLE__)
    const char* glsl_version = "#version 150";
#else
    const char* glsl_version = "#version 130";
#endif

    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui_ImplGlfw_InitForOpenGL(window, true);

    glEnable(GL_DEPTH_TEST);

    return 0;
}

void ShutdownBackend()
{
    // terminate imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(NULL);

    // terminale glfw
    glfwDestroyWindow(window);
    glfwTerminate();
}

void RunBackend(void (*callback)())
{
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        callback();

        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
}

void *GetPointerToTextureBackend(pxr::HdRenderBuffer* buffer, pxr::Hgi* hgi)
{
    // Note: there is more straightforward to get texture pointer.
    // This method ensure OpenGL texture is created even if
    // default hgi != hgiGL (e.g. on Mac).

    float* pixelData = (float*)buffer->Map();
    int width = buffer->GetWidth();
    int height = buffer->GetHeight();
    pxr::HdFormat hdFormat = buffer->GetFormat();

    GLenum internalFormat, format, type;

    switch (hdFormat) {
        case pxr::HdFormatUNorm8Vec4:
            internalFormat = GL_RGBA8;
            format = GL_RGBA;
            type = GL_UNSIGNED_BYTE;
            break;
        case pxr::HdFormatFloat32Vec4:
            internalFormat = GL_RGBA32F;
            format = GL_RGBA;
            type = GL_FLOAT;
            break;
        case pxr::HdFormatFloat16Vec4:
            internalFormat = GL_RGBA16F;
            format = GL_RGBA;
            type = GL_HALF_FLOAT;
            break;
        case pxr::HdFormatUNorm8:
            internalFormat = GL_R8;
            format = GL_RED;
            type = GL_UNSIGNED_BYTE;
            break;
        default:
            std::cerr << "Unhandled HdFormat in OpenGL backend." << std::endl;
            buffer->Unmap();
            return nullptr;
    }

    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, pixelData);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    buffer->Unmap();

    return (void*)(uintptr_t)textureId;
}
