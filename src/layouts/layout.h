/**
 * @file layout.h
 * @author Raphael Jouretz (rjouretz.com)
 * @brief Header containing function to manage ImGui layout
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

/**
 * @brief Load the custum imgui layout if exists (imgui.ini in current
 * directory) or load a default layout if any.
 *
 */
void LoadDefaultOrCustomLayout();