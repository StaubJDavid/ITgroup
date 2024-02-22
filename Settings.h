#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

struct Setting {
	std::string name;
	std::string value;
};

void ltrim(std::string& s); // Trims left side of string from whitespaces
void rtrim(std::string& s); // Trims right side of string from whitespaces
void trim(std::string& s);// Calls ltrim and rtrim

void checkSettingsFile(std::string settingsPath); // Check if there's a settings.txt, if not, creates one
std::vector<Setting> readSettings(std::string settingsPath); // Reads settings from settings.txt