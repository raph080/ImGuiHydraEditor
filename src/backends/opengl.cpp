#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "backend.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

#include <pxr/imaging/hgiGL/texture.h>
#include <pxr/imaging/hgi/texture.h>

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

void UpdateBufferSizeBackend(int width, int height, PresentTarget* target)
{
    GLuint handle = static_cast<GLuint>(target->handle.UncheckedGet<uint32_t>());
    GLuint buffer = static_cast<GLuint>(reinterpret_cast<uintptr_t>(target->buffer));

    if (buffer) glDeleteTextures(1, &buffer);
    if (handle)   glDeleteFramebuffers(1, &handle);  
    
    // here is the texture provided to Hydra to render into
    glGenTextures(1, &buffer);
    glBindTexture(GL_TEXTURE_2D, buffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // here is the buffer provided to Imgui to display from
    glGenFramebuffers(1, &handle);
    glBindFramebuffer(GL_FRAMEBUFFER, handle);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, buffer, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    *target = PresentTarget{
        pxr::HgiTokens->OpenGL,
        pxr::VtValue(uint32_t(handle)),
        (void*)(uintptr_t)buffer
    };
}

void PresentBackend(const PresentTarget& target, pxr::HdxTaskController* taskController)
{
    taskController->SetPresentationOutput(target.api, target.handle);
    taskController->SetEnablePresentation(true);
}

void* GetPointerToTextureBackend(const PresentTarget& target, pxr::HdRenderBuffer* buffer, pxr::Hgi* hgi)
{
    return target.buffer;
}