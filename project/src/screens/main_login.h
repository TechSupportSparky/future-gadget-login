#pragma once
#include <string>

// ****************************************************
// MAIN LABRATORY LOGIN
// AKA Future Labratory Gadget No. 16
// AKA SUPAH DELOREAN LOGIN
//
// Copyright (c) FG16, DaSH (￣ー￣)
// ****************************************************

using namespace std;
struct ID3D11ShaderResourceView;

// Main logic
void ShowLoginScreen();

void DrawBackground(ID3D11ShaderResourceView* bgView);
string ToLowerCase(const string& str);
