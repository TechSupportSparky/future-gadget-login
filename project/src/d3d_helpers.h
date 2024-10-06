#pragma once

#include <d3d11.h>
#include <tchar.h>
#include "imgui.h"

extern ID3D11Device* g_pd3dDevice;
extern ID3D11DeviceContext* g_pd3dDeviceContext;
extern IDXGISwapChain* g_pSwapChain;
extern bool g_SwapChainOccluded;
extern UINT g_ResizeWidth, g_ResizeHeight;
extern ID3D11RenderTargetView* g_mainRenderTargetView;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();

// Handles resize and inactive events, returns if this
// imgui iteration should be skipped
bool D3DHandleWindowEvents();

// Create a new DirectX window and provide the class/handle as params
int CreateD3DWindow(WNDCLASSEXW& outWc, HWND& outHandle);

// Deals with clearing the screen and rendering the new frame
void RenderAndPresent(ImGuiIO io);
