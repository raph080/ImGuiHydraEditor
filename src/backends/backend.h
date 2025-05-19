// backend.h
#pragma once

#include "pxr/imaging/hgi/texture.h"

void InitBackend();
void ShutdownBackend();
bool ShouldCloseApp();
void PollEvents();
bool BeginFrame();
void EndFrame();

void *GetPointerToHgiTextureBackend(pxr::HgiTextureHandle texHandle, float width, float height);