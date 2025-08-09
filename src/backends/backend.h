/**
 * @file backend.h
 * @author Raphael Jouretz (rjouretz.com)
 * @brief Headers containing the functions to interact with
 * the graphic backend (create window, get texture, ...).
 * The according implementation get bound during compilation.
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

#include <pxr/base/tf/token.h>
#include <pxr/base/vt/value.h>
#include <pxr/imaging/hgi/hgi.h>
#include <pxr/imaging/hgi/tokens.h>
#include <pxr/imaging/hd/renderBuffer.h>
#include <pxr/imaging/hdx/taskController.h>

/**
 * @brief A presentation target containing backend presentation state
 * 
 * @param api the backend api (OpenGL, Metal, ...)
 * @param handle the container for hydra render
 * @param buffer the buffer to display to ImGui
 */
struct PresentTarget{
    pxr::TfToken api;
    pxr::VtValue handle;
    void* buffer;
};

/**
 * @brief Initialize the backend and create a window
 *
 * @param title the title of the created window
 * @param width the width of the created window
 * @param height the height of the created window
 * 
 * @return 0 if successfully inititalized. Otherwise return non zero.
 */
int InitBackend(const char* title, int width, int height);

/**
 * @brief Run the main loop of the window
 *
 * @param callback the callback to a function called every frame
 */
void RunBackend(void (*callback)());

/**
 * @brief Shutdown the backend
 */
void ShutdownBackend();

/**
 * @brief Update the buffer size from the backend size
 * 
 * @param width the width of the new buffers
 * @param heigt the height of the new buffers
 * @param target the presentation target that will be updated
 */
void UpdateBufferSizeBackend(int width, int height, PresentTarget* target);

/**
 * @brief Present outputs (backend api and handle) to the given taskController
 * 
 * Currently, this only works for OpenGL. Indeed, HgiInterop 
 * only supports presentation to OpenGL. If trying to present to Metal
 * buffer: 
 * "hgiInterop.cpp -- Unsupported destination Hgi backend: Metal"
 * 
 * Without presentation (for Metal), we are loosing color correction.
 * 
 * @param target the presentation target
 * @param taskController the Task Controller that will be set with outputs
 */
void PresentBackend(const PresentTarget& target, pxr::HdxTaskController* taskController);

/**
 * @brief Retrieve a pointer to the buffer containing the texture
 * 
 * This function should only return a buffer created by the backend
 * and related to the one presented to the Task Controller. However,
 * Metal is not available yet as presentation output.
 * 
 * In order to be able to retrieve a pointer to the data for Metal,
 * Render Buffer and Hgi are provided as arguments. This allows to
 * get the render (without color correction) that we can convert for
 * Metal use.
 *
 * @param target the presentation target
 * @param buffer the hydra render buffer
 * @param hgi the hgi related to the buffer
 * 
 * @return a pointer to the texture data
 */
void* GetPointerToTextureBackend(const PresentTarget& target, pxr::HdRenderBuffer* buffer, pxr::Hgi* hgi);
