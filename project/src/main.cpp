#include "d3d_helpers.h"
#include "globals.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "imgui_init_helpers.h"
#include "main_login.h"
#include "windows_helpers.h"
#include <d3d11.h>
#include <tchar.h>
#pragma comment(lib, "winmm.lib")

// ****************************************************
// FUTURE GADGET LOGIN 
//
// This app is to showcase the capabilities of the
// future gadget labratory.  The aim of which is to
// prevent CERN and other nefarious entities from gaining
// access to CRITICAL labratory activities and projects
//
// If the Future Gadget Labratory homepage were cracked,
// the very WORLD may be in danger.  Thus, we've implemented
// a highly secure and impenetrable security system that
// relies on existing gadget inventions.
//
// ****************************************************

ImFont* phoneInputFont = nullptr;
ImFont* defaultImguiFont = nullptr;

ID3D11ShaderResourceView* backgroundTexture = nullptr;

void LoadStartupResources()
{
    // (´｡• ᵕ •｡)♡ Up to 300 visits and counting
    backgroundTexture = LoadTextureFromPNG(L"assets\\images\\FutureGadetLabHomepage.png", g_pd3dDevice);
    if (!backgroundTexture)
    {
        fprintf(stderr, "Failed to load background texture\n");
        exit(EXIT_FAILURE);
    }

    ImGuiIO& io = ImGui::GetIO();
    defaultImguiFont = io.Fonts->AddFontDefault();
    phoneInputFont = io.Fonts->AddFontFromFileTTF("assets\\font\\Garamond Light Condensed SSi Light Condensed.ttf", 22.0f);
    if (!phoneInputFont)
    {
        fprintf(stderr, "Failed to load font resource\n");
        exit(EXIT_FAILURE);
    }
}

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
int main(int, char**)
{
    // Main viewport ⊙-⊙
    WNDCLASSEXW wc; HWND hwnd;
    CreateD3DWindow(wc, hwnd);

    // Imgui and backend initialization
    ImGuiIO io;
    SetupImGuiWindow(io);
    InitializeSystem(hwnd);

    // Images and fonts
    LoadStartupResources();

    bool done = false;
    while (!done)
    {
        // Check and handle window events
        if (PeekWindow()) { done = true; break; }
        if (D3DHandleWindowEvents()) continue;

        // Startup the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        DrawBackground(backgroundTexture);

        // Our main login page
        ShowLoginScreen();

        RenderAndPresent(io);
    }

    ShutDown(wc, hwnd);
    return 0;
}
