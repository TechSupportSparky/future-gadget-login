#pragma once

#include <imgui_impl_win32.h>
#include <Windows.h>

// Handles windows messaging (resize, quit, win specific functions)
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Dispatch windows messaging, returns whether we should quit
bool PeekWindow();
