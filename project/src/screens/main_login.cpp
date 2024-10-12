#include "main_login.h"

#include "d3d_helpers.h"
#include "globals.h"
#include "imgui.h"
#include "phone.h"
#include <string>
#include <random>
#include <vector>
using namespace std;

#define USABLE_CHARS 36
// DMails shouldn't look like gibberish, use natural phrases!
// 36 Char MAX!
vector<string> CaptchaPhrases = {
    "LaurieWired", "Nullpo_GAH", "TimeLeap",
    "D-Mail", "Reading_Steiner", "Attractor_Field",
    "Amadeus", "Shining_Finger", "Serial_Experiments",
    "Metal_Upa", "Stardust_Handshake", "Beta_Worldline",
    "John_Titor", "Faris_NyanNyan", "Gero_Froggy",
    "Whose_Eyes_Are_Those_Eyes", "Shibuya_Earthquake",
    "Neidhardt", "Gunvarrel", "Frau_Koujiro"
};

string GetRandomCaptchaPhrase()
{
    static mt19937 rng(random_device{}());
    uniform_int_distribution<size_t> dist(0, CaptchaPhrases.size() - 1);
    return CaptchaPhrases[dist(rng)];
}

// Makes a noisy texture to mimic a captcha window to waste dumb bots time!
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

// Draws the main viewport background (docking a texture)
void DrawBackground(ID3D11ShaderResourceView* bgView)
{
    ImGuiWindowFlags mainWindowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    mainWindowFlags |= ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration;
    ImGui::Begin("MainBackgroundWindow", nullptr, mainWindowFlags);
    ImGui::PopStyleVar(3);

    if (bgView)
    {
        ImVec2 screenSize = viewport->Size;
        ImGui::Image((void*)bgView, screenSize);
    }
    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
}

static bool isFadingIn = false;
static bool signedIn = false; // Let us setup the background during transition
bool ShowSuccessScreen()
{
    static float fadeAlpha = 0.0f;
    static float holdTime = 3.0f;

    if (isFadingIn)
    {
        fadeAlpha += ImGui::GetIO().DeltaTime * 0.25f;

        // Clamp zoomScale and fadeAlpha to their limits
        if (fadeAlpha > 1.0f) fadeAlpha = 1.0f;

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 viewportSize = viewport->Size;

        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewportSize);
        ImGui::SetNextWindowBgAlpha(0.0f);

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoInputs;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Fade In Effect", nullptr, windowFlags);
        ImGui::SetWindowFocus(); // Need to force this on top

        static ID3D11ShaderResourceView* timeTravelTexture = nullptr;
        if (!timeTravelTexture) timeTravelTexture = LoadTextureFromPNG(L"assets\\images\\DivergenceMeter.png", g_pd3dDevice);
        ImGui::Image((void*)timeTravelTexture, viewportSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.0f, 1.0f, 1.0f, fadeAlpha));

        ImGui::End();
        ImGui::PopStyleVar(3);

        // Once we've finished scaling, hold for a second then transition
        static float waitTime = 0.0f;
        if (fadeAlpha >= 1.0f)
        {
            waitTime += ImGui::GetIO().DeltaTime;

            if (waitTime > holdTime)
            {
                signedIn = true;
                // Reset vars in case we want to fade in again
                isFadingIn = false;
                fadeAlpha = 0.0f;  
                waitTime = 0.0f;   
                return true;       
            }
        }
    }
    return false;
}

// This call updates the main login window to show a successful sign in
void DrawSuccessfulLogin()
{
    static ID3D11ShaderResourceView* completeBackground = nullptr;
    if (backgroundTexture != completeBackground)
    {
        if (backgroundTexture != nullptr)
        {
            backgroundTexture->Release(); // Cleanup your memory!
            backgroundTexture = nullptr;
        }
        backgroundTexture = LoadTextureFromPNG(L"assets\\images\\GadetLab.png", g_pd3dDevice);
        completeBackground = backgroundTexture; // Cache it
    }

    // Successfully logged in!
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255)); // Set text color to green
    ImGui::Text("You successfully logged in!");
    ImGui::PopStyleColor(); // Revert text color to the default

    static ID3D11ShaderResourceView* kurisuTexture = nullptr;
    if (!kurisuTexture)
    {
        kurisuTexture = LoadTextureFromPNG(L"assets\\images\\KurisuThumbsUp.png", g_pd3dDevice);
    }

    // Image 
    ImVec2 imageSize = ImVec2(250.0f, 141.0f); // Adjust as necessary for your image size

    // Render the image directly below the text
    ImGui::Image((void*)kurisuTexture, imageSize);
}

void SetupWindowCredentials()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 initialPos = ImVec2(viewport->Pos.x + 25.0f, viewport->Pos.y + 25.0f);
    ImGui::SetNextWindowPos(initialPos, ImGuiCond_Once);

    ImVec2 minSize = ImVec2(400, 150); // Need a minimum size to include the entire titlebar
    ImGui::SetNextWindowSizeConstraints(minSize, ImVec2(FLT_MAX, FLT_MAX));

    // Titlebar and background colors
    ImGui::PushStyleColor(ImGuiCol_TitleBg, IM_COL32(255, 143, 43, 245));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, IM_COL32(255, 143, 43, 245));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(10, 0, 0, 255));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));

    ImGui::Begin("CONFIDENTIAL - FUTURE GADGET LABORATORY LOGIN", nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse );
    ImGui::PopStyleColor(4);

    // Static username and passwords
    ImGui::Text("Username:");
    ImGui::BeginDisabled(true);
    ImGui::InputText("##username", (char*)"KuriGohan", IM_ARRAYSIZE("KuriGohan"), ImGuiInputTextFlags_ReadOnly);

    ImGui::Text("Password:");
    ImGui::InputText("##password", (char*)"********", IM_ARRAYSIZE("********"), ImGuiInputTextFlags_ReadOnly);
    ImGui::EndDisabled();
}

void ShowLoginScreen(bool& proceedToCaptcha, bool& triggerDmail)
{
    static ID3D11ShaderResourceView* noiseTextureView = nullptr;
    static string captcha = "";
    static bool needNewCaptcha = false;

    static float colorTimer = 0.0f;
    static bool successfullyLoggedIn = false;
    static bool successfulShift = false;

    SetupWindowCredentials();

    if (!signedIn)
    {
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
            ImGui::Text("Supicious login location detected, identify lab member status with this CAPTCHA");
            ImVec2 captchaBoxSize = ImVec2(300.0f, 113.0f);
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
                memset(userCaptchaInput, 0, sizeof(userCaptchaInput));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // Red text
                ImGui::Text("Incorrect, the previous answer was: %s", captcha.c_str());
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

            // Pulse the phone activate button until they click it
            if (!phoneActivated) {
                colorTimer += ImGui::GetIO().DeltaTime * 1.0f;
                float colorIntensity = (sin(colorTimer) + 1.0f) * 0.3f;  // Oscillates 0-1

                // Should probably store these off, but we're out of here once they click
                ImVec4 blueColor = ImVec4(42.0f / 255.0f, 74.0f / 255.0f, 114.0f / 255.0f, 1.0f);
                ImVec4 goldColor = ImVec4(255.0f / 255.0f, 215.0f / 255.0f, 0.0f / 255.0f, 1.0f);

                ImVec4 buttonColor = ImVec4(
                    blueColor.x + (goldColor.x - blueColor.x) * colorIntensity,
                    blueColor.y + (goldColor.y - blueColor.y) * colorIntensity,
                    blueColor.z + (goldColor.z - blueColor.z) * colorIntensity,
                    1.0f
                );

                ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonColor.x + 0.1f, buttonColor.y + 0.1f, buttonColor.z + 0.1f, buttonColor.w));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonColor.x - 0.1f, buttonColor.y - 0.1f, buttonColor.z - 0.1f, buttonColor.w));

                if (ImGui::Button("Phone"))
                {
                    phoneActivated = true;
                }

                ImGui::PopStyleColor(3);
            }

            // Callup the phone and wait on a response
            if (phoneActivated)
            {
                // Wait for phone flags
                bool sentDmail = DrawPhone(captcha, successfulShift);
                if (sentDmail)
                {
                    // They successfully entered in the right captcha
                    if (successfulShift) {
                        isFadingIn = true;
                        if (ShowSuccessScreen()) // Wait until the animation finishes
                        {
                            // Reset phone and captcha windows
                            ResetPhone();
                            phoneActivated = false;
                            proceedToCaptcha = false;
                            triggerDmail = false;
                        }
                    }
                    else // Entered a captcha, but it did not trigger a world line shift
                    {
                        ResetPhone();
                        phoneActivated = false;
                        proceedToCaptcha = false;
                        triggerDmail = false;
                    }
                }
            }
        }
    }
    else
    {
        DrawSuccessfulLogin();
    }

    ImGui::End();
}
