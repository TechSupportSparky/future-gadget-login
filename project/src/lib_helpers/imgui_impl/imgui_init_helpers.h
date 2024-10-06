#pragma once

#include "imgui.h"
#include "Windows.h"

void SetupImGuiWindow(ImGuiIO& outIO);

void InitializeSystem(HWND hwnd);

void ShutDown(WNDCLASSEXW wc, HWND hwnd);
