#include "d3d_helpers.h"

#include <d3d11.h>
#include <wincodec.h>
#include <wrl/client.h>
#include <random>
#include "imgui_impl_dx11.h"
#include "windows_helpers.h"

ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
bool g_SwapChainOccluded = false;
UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

bool D3DHandleWindowEvents()
{
    // Handle window being minimized or screen locked
    if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
    {
        ::Sleep(10);
        return true;
    }
    g_SwapChainOccluded = false;

    // Handle window resize (we don't resize directly in the WM_SIZE handler)
    if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
    {
        CleanupRenderTarget();
        g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
        g_ResizeWidth = g_ResizeHeight = 0;
        CreateRenderTarget();
    }
    return false;
}

// Initializes and creates the viewport, returns non-zero if we failed to create the window
int CreateD3DWindow(WNDCLASSEXW& outWc, HWND& outHandle)
{
    // Window parameters
    outWc = { sizeof(outWc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&outWc);
    //outHandle = ::CreateWindowW(outWc.lpszClassName, L"Dear ImGui DirectX11 Example", WS_OVERLAPPEDWINDOW, 100, 100, width, height, nullptr, nullptr, outWc.hInstance, nullptr);
    outHandle = ::CreateWindowW(
        outWc.lpszClassName,
        L"Future Gadget Lab Fullscreen Window",
        WS_POPUP | WS_VISIBLE,                   // Use WS_POPUP and WS_VISIBLE to cover the entire screen without borders
        0, 0, 1920, 1080,                        // Fullscreen size (1920x1080)
        nullptr, nullptr, outWc.hInstance, nullptr
    );

    // Initialize Direct3D
    if (!CreateDeviceD3D(outHandle))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(outWc.lpszClassName, outWc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(outHandle, SW_SHOWDEFAULT);
    //::ShowWindow(outHandle, SW_HIDE);
    ::UpdateWindow(outHandle);

    return 0;
}

void RenderAndPresent(ImGuiIO io)
{
    // Rendering
    ImGui::Render();
    const float clear_color[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color);

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    g_pSwapChain->Present(1, 0); // Present with vsync
    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    // Present
    HRESULT hr = g_pSwapChain->Present(1, 0);   // Present with vsync
    g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
}

vector<unsigned char> GenerateNoiseData(int width, int height)
{
    std::vector<unsigned char> noiseData(width * height * 4);
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<> dist(0, 255);

    for (int i = 0; i < width * height * 4; i++)
    {
        noiseData[i] = static_cast<unsigned char>(dist(rng));
    }

    return noiseData;
}

ID3D11ShaderResourceView* CreateShaderResourceView(ID3D11Device* device, ID3D11Texture2D* texture)
{
    ID3D11ShaderResourceView* textureView = nullptr;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    device->CreateShaderResourceView(texture, &srvDesc, &textureView);
    return textureView;
}

ID3D11Texture2D* CreateNoiseD11Texture(ID3D11Device* device, const std::vector<unsigned char>& noiseData, int width, int height)
{
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = noiseData.data();
    initData.SysMemPitch = width * 4;

    ID3D11Texture2D* texture = nullptr;
    device->CreateTexture2D(&desc, &initData, &texture);
    return texture;
}

ID3D11ShaderResourceView* LoadTextureFromPNG(const wchar_t* filePath, ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
    using Microsoft::WRL::ComPtr;

    // Initialize the WIC factory
    ComPtr<IWICImagingFactory> wicFactory;
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&wicFactory)
    );

    // Failed to create WIC Imaging Factory
    if (FAILED(hr))
    {
        return nullptr; 
    }

    ComPtr<IWICBitmapDecoder> decoder;
    hr = wicFactory->CreateDecoderFromFilename(
        filePath,
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnDemand,
        &decoder
    );

    // Couldn't decode image
    if (FAILED(hr))
    {
        return nullptr; 
    }

    // Get the first frame of the image
    ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr))
    {
        return nullptr;
    }

    // 32bpp RGBA for DirectX
    ComPtr<IWICFormatConverter> formatConverter;
    hr = wicFactory->CreateFormatConverter(&formatConverter);
    if (FAILED(hr))
    {
        return nullptr;
    }

    hr = formatConverter->Initialize(
        frame.Get(),
        GUID_WICPixelFormat32bppRGBA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeCustom
    );

    if (FAILED(hr))
    {
        return nullptr; 
    }

    // Get the width and height of the image
    UINT width, height;
    hr = formatConverter->GetSize(&width, &height);
    if (FAILED(hr))
    {
        return nullptr; 
    }

    // Copy the image data into a buffer
    std::vector<BYTE> imageData(width * height * 4); // 4 bytes per pixel (RGBA)
    hr = formatConverter->CopyPixels(
        nullptr,
        width * 4, // Number of bytes per row
        static_cast<UINT>(imageData.size()),
        imageData.data()
    );

    if (FAILED(hr))
    {
        return nullptr;
    }

    // Create a texture description
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 32-bit RGBA
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    // Describe the subresource data
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = imageData.data();
    initData.SysMemPitch = width * 4; 

    // Create the texture
    ComPtr<ID3D11Texture2D> texture;
    hr = device->CreateTexture2D(&textureDesc, &initData, &texture);
    if (FAILED(hr))
    {
        return nullptr; 
    }

    // Create a shader resource view for the texture
    ID3D11ShaderResourceView* textureView = nullptr;
    hr = device->CreateShaderResourceView(texture.Get(), nullptr, &textureView);
    if (FAILED(hr))
    {
        return nullptr; 
    }

    return textureView;
}
