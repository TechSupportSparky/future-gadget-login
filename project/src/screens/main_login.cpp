#include "main_login.h"

#include "d3d_helpers.h"
#include "imgui.h"
#include <string>
#include <random>
#include <vector>
using namespace std;

#define USABLE_CHARS 27
// DMails shouldn't look like gibberish, use natural phrases!
// 36 BYTES! "Captcha: " = 9 char + [27 characters]
vector<string> CaptchaPhrases = {
    "LaurieWired", "Nullpo_GAH", "TimeLeap",
    "D-Mail", "Reading_Steiner", "Attractor_Field",
    "Amadeus", "Shining_Finger", "Serial_Experiments",
    "Metal_Upa", "Stardust_Handshake", "Beta_Worldline",
    "John_Titor", "RaiNet", "Faris_NyanNyan", "Gero_Froggy",
    "Whose_Eyes_Are_Those_eyes", "Shibuya_Earthquake",
    "Neidhardt", "Gunvarrel", "Frau_Koujiro"
};

string GetRandomCaptchaPhrase()
{
    static mt19937 rng(random_device{}());
    uniform_int_distribution<size_t> dist(0, CaptchaPhrases.size() - 1);
    return CaptchaPhrases[dist(rng)];
}

void GenerateNoisyTexture(ID3D11ShaderResourceView*& noiseTextureView, ImVec2 imageSize)
{
    int width = static_cast<int>(imageSize.x); int height = static_cast<int>(imageSize.y);

    // Clear out any data currently in this pointer ref 
    if (noiseTextureView != nullptr)
    {
        noiseTextureView->Release(); 
        noiseTextureView = nullptr;
    }

    auto noiseData = GenerateNoiseData(width, height);
    ID3D11Texture2D* noiseTexture = CreateNoiseD11Texture(g_pd3dDevice, noiseData, width, height);
    if (noiseTexture)
    {
        noiseTextureView = CreateShaderResourceView(g_pd3dDevice, noiseTexture);
        noiseTexture->Release();  // Release the texture object after creating the view
    }
}

void DrawBackground(ID3D11ShaderResourceView* bgView)
{
    ImVec2 screenSize = ImVec2(1920, 1080); // This should be 1920x1080 for fullscreen

    // Draw the background image over the entire screen
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(screenSize, ImGuiCond_Always);

    // Set flags to prevent user interaction
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::Begin("Background", nullptr, windowFlags);
    ImGui::Image((void*)bgView, screenSize);
    ImGui::End();
}

static ID3D11ShaderResourceView* PhoneTextureView = nullptr;
void DrawPhone()
{
    if (!PhoneTextureView) {
        PhoneTextureView = LoadTextureFromPNG(L"C:\\Users\\TechS\\Downloads\\Phone.png", g_pd3dDevice, g_pd3dDeviceContext);
    }

    ImVec2 windowSize = ImVec2(300, 573);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowBorderSize = 0.0f; // Remove the window border
    style.FrameBorderSize = 0.0f;
    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize /* | ImGuiWindowFlags_NoMove */ |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::Begin("Phone Window", nullptr, windowFlags);
    ImGui::Image((void*)PhoneTextureView, windowSize);
}

void ShowLoginScreen(bool& showLoginScreen, bool& proceedToCaptcha, bool& triggerDmail)
{
    static ID3D11ShaderResourceView* noiseTextureView = nullptr;
    static string captcha = "";
    static bool needNewCaptcha = false;

    // Size and position setup of login window
    if (proceedToCaptcha)
    {
        ImVec2 expandedSize = ImVec2(600, 500); // For extra captcha space
        ImGui::SetNextWindowSize(expandedSize, ImGuiCond_Always);
    }
    else
    {
        ImVec2 initialSize = ImVec2(600, 300);
        ImGui::SetNextWindowSize(initialSize, ImGuiCond_Always);
    }
    ImVec2 initialPos = ImVec2(300, 100);
    ImGui::SetNextWindowPos(initialPos, ImGuiCond_FirstUseEver);
    
    ImGui::Begin("CONFIDENTIAL - FUTURE GADGET LABORATORY LOGIN");
    
    // Username and password fields (non-editable)
    ImGui::Text("Username:");
    ImGui::BeginDisabled(true);
    ImGui::InputText("##username", (char*)"KuriGohan", IM_ARRAYSIZE("KuriGohan"), ImGuiInputTextFlags_ReadOnly);
    
    ImGui::Text("Password:");
    ImGui::InputText("##password", (char*)"********", IM_ARRAYSIZE("********"), ImGuiInputTextFlags_ReadOnly);
    ImGui::EndDisabled();
    
    // Login Button, when clicked enter captcha
    if (ImGui::Button("Login"))
    {
        proceedToCaptcha = true;
        needNewCaptcha = true; // Generate a new captcha once
    }

    // Captcha flow
    static char userCaptchaInput[USABLE_CHARS] = "";
    if (proceedToCaptcha)
    {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Supicious login location detected, identify lab member status with this CAPTCHA:");
        ImVec2 captchaBoxSize = ImVec2(200.0f, 113.0f);
        if (needNewCaptcha)
        {
            captcha = GetRandomCaptchaPhrase();
            needNewCaptcha = false;

            GenerateNoisyTexture(noiseTextureView, captchaBoxSize);
        }
        ImGui::Image((void*)noiseTextureView, captchaBoxSize);

        // Incorrect text
        if (triggerDmail)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // Red text
            ImGui::Text("Incorrect, the answer was: %s", captcha.c_str());
            ImGui::PopStyleColor();
        }

        // Input box
        ImGui::InputText("##captcha_input", userCaptchaInput, IM_ARRAYSIZE(userCaptchaInput));

        // Logic for when we click verify
        if (ImGui::Button("Verify"))
        {
            if (captcha == string(userCaptchaInput))
            {
                // They guessed it?!
                proceedToCaptcha = false;
                showLoginScreen = false;

                // Clear up the captcha texture
                if (noiseTextureView != nullptr)
                {
                    noiseTextureView->Release();
                    noiseTextureView = nullptr;
                }
            }
            else
            {
                // Trigger phone logic!
                needNewCaptcha = true;
                triggerDmail = true;
            }
        }

        // Tooltip for hint
        ImGui::SameLine(); 
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::Text("Wrong answers encouraged!");
            ImGui::EndTooltip();
        }
    }

    if (triggerDmail)
    {
        static bool phoneActivated = false;
        if (ImGui::Button("Phone"))
        {
            phoneActivated = true;
        }

        if (phoneActivated)
        {
            DrawPhone();

            // Set position for input box to overlay on the phone screen section
            ImGui::SetCursorPos(ImVec2(60, 220)); // Adjust to match the black screen area
            static char inputBuffer[128] = "";
            ImGui::InputText("##phone_input", inputBuffer, IM_ARRAYSIZE(inputBuffer));


            ImGui::End();
        }
    }

    ImGui::End();
}
