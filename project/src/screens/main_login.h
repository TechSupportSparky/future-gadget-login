#pragma once

// ****************************************************
// FUTURE GADGET LABRATORY LOGIN
// AKA Future Labratory Gadget No. 16
// AKA SUPAH DELOREAN LOGIN
//
// Copyright (c) FG16, DaSH
// ****************************************************

struct ID3D11ShaderResourceView;

// Main logic
void ShowLoginScreen(bool& proceedToCaptcha, bool& triggerDmail);

void DrawBackground(ID3D11ShaderResourceView* bgView);
