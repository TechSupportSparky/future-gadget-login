#include "d3d_helpers.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "imgui_init_helpers.h"
#include "main_login.h"
#include "windows_helpers.h"
#include <d3d11.h>
#include <tchar.h>

// For audio
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

ID3D11ShaderResourceView* backgroundTexture = nullptr;
ImFont* phoneInputFont = nullptr;
ImFont* defaultImguiFont = nullptr;

void LoadStartupResources()
{
    backgroundTexture = LoadTextureFromPNG(L"assets\\images\\FutureGadetLabHomepage.png", g_pd3dDevice, g_pd3dDeviceContext);
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

int main(int, char**)
{
    WNDCLASSEXW wc; HWND hwnd;
    CreateD3DWindow(wc, hwnd);

    ImGuiIO io;
    SetupImGuiWindow(io);
    InitializeSystem(hwnd);

    LoadStartupResources();

    // Variables for controlling UI state
    bool proceedToCaptcha = false;
    bool showDmailTrigger = false;
    bool done = false;
    while (!done)
    {
        if (PeekWindow()) { done = true; break; }

        if (D3DHandleWindowEvents()) continue;

        // Startup the Dear ImGui
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        DrawBackground(backgroundTexture);

        ImGui::End();

        // Render the main UI
        ShowLoginScreen(proceedToCaptcha, showDmailTrigger);

        RenderAndPresent(io);
    }

    ShutDown(wc, hwnd);
    return 0;
}
