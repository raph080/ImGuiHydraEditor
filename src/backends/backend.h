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

/**
 * 
 */
#include "pxr/imaging/hd/renderBuffer.h"
#include "pxr/imaging/hgi/hgi.h"

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
 * @brief Retrieve a pointer to texture data of the given buffer
 *
 * @param buffer the hydra render buffer the texture data
 * @param hgi the hgi related to the buffer
 * 
 * @return a pointer to the texture data
 */
void *GetPointerToTextureBackend(pxr::HdRenderBuffer* buffer, pxr::Hgi* hgi);

/**
 * @brief Request the backend to delete the texture and free the resources.
 * @param texturePtr the pointer to the texture data
 * @param hgi the hgi related to the buffer
 */
void DeleteTextureBackend(void* texturePtr, pxr::Hgi* hgi);