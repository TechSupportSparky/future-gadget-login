#include "phone.h"

#include "d3d_helpers.h"
#include "d3d11.h"
#include "imgui.h"
#include "globals.h"

// ****************************************************
// The purpose of Gadget No. 17 is to have a permanent
// fixture that can aid in the main site login process.
//
// This phone is attached via zip-ties to the main lab
// computer to serve as a 2-factor form of authentication.
// (Thank you Mayuri for your phone's sacrafice)
// (っ´ω)ﾉ(╥ω╥)
// 
// Standard 2-factor authentication would not be enough
// however, thus we use D-Mail to further protect our
// site. Only those with the power to change time can
// access the administrator section of our page and
// view all of our secrets (/ω＼)
// 
//  ****************************************************

// Sets the style, size, position, and texture of our digital
// phone, note only the physical phone in the lab works correctly!
void CreatePhoneWindow()
{
    ImVec2 windowSize = ImVec2(400, 764); // Size of the phone
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 windowPos = ImVec2(
        viewport->Pos.x + viewport->Size.x - windowSize.x - 50.0f, // lil padding
        viewport->Pos.y + viewport->Size.y - windowSize.y
    );

    // Setup the phone's style. No border, scrollbar, titlebar etc.
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(windowSize);
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize  | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    static ID3D11ShaderResourceView* PhoneTextureView = nullptr;
    if (!PhoneTextureView) PhoneTextureView = LoadTextureFromPNG(L"assets\\images\\Phone.png", g_pd3dDevice);
    ImGui::Begin("Phone Window", nullptr, windowFlags);
    ImGui::Image((void*)PhoneTextureView, windowSize);
}

// (⌐■_■) Styleish
void SetCaptchaStyles()
{
    static float pulseTimer = 0.0f;

    pulseTimer += ImGui::GetIO().DeltaTime * 2.0f;
    float colorIntensity = (sinf(pulseTimer) + 1.0f) * 0.5f;
    ImVec4 borderColor = ImVec4(
        1.0f * colorIntensity, 1.0f * colorIntensity,
        1.0f * colorIntensity, 1.0f
    );

    if (phoneInputFont) ImGui::PushFont(phoneInputFont);

    // Input box style
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));           
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.95f, 0.95f, 0.95f, 1.0f)); 
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));     
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));              

    // Pulsing button color
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));            
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.1f));     
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 0.0f, 0.2f));      
    ImGui::PushStyleColor(ImGuiCol_Border, borderColor);                               

    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);                          
}

void CleanupCaptchaStyles()
{
    ImGui::PopStyleVar(1);
    ImGui::PopStyleColor(8);

    if (phoneInputFont)
    {
        ImGui::PopFont();
    }
}

// Draw the phone texture and calculate its position (relative to the bottom right corner)
static bool dmailSent = false;
static char phoneInputBuffer[128] = "";
bool DrawPhone(string captcha, bool& successfulShift)
{
    if (!dmailSent)
    {
        CreatePhoneWindow();

        SetCaptchaStyles();

        // Set position for input box to overlay on the phone screen section
        ImGui::SetCursorPos(ImVec2(65, 250));
        ImGui::InputText("##phone_input", phoneInputBuffer, IM_ARRAYSIZE(phoneInputBuffer));

        ImGui::PopStyleColor(1);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));

        // Send Dmail
        ImGui::SetCursorPos(ImVec2(65, 285));
        if (ImGui::Button("##blank_button", ImVec2(65, 25)))
        {
            dmailSent = true;

            // Okabe says only those with reading steiner can detect when
            // the worldline has shifted, but none of us really feel it (゜-゜)
            if (captcha == string(phoneInputBuffer))
            {
                successfulShift = true;
                PlaySound(L"assets\\audio\\ReadingSteinerEffect.wav", nullptr, SND_FILENAME | SND_ASYNC);
            }
        }

        CleanupCaptchaStyles();

        ImGui::End();        
    }

    return dmailSent;
}

// Powering off works too (－－) zzZ
void ResetPhone()
{
    dmailSent = false;
    memset(phoneInputBuffer, 0, sizeof(phoneInputBuffer));
}
