#include "Settings.h"

void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

void trim(std::string& s) {
    rtrim(s);
    ltrim(s);
}

void checkSettingsFile(std::string settingsPath) {
    std::ifstream SettingsFile(settingsPath);
    try {
        if (!SettingsFile.good()) {
            std::ofstream NewSettingsFile(settingsPath);

            // Speed settings
            NewSettingsFile << "# Sets the rotation speed of the fan" << std::endl;
            NewSettingsFile << "speed = 1" << std::endl;

            NewSettingsFile.close();
        }
    } catch (const std::exception&) {
    }
    SettingsFile.close();
}

std::vector<Setting> readSettings(std::string settingsPath) {
    checkSettingsFile(settingsPath);

    std::string fileInputLine;
    std::ifstream SettingsFile(settingsPath);

    std::vector<Setting> settings = {};

    // Ignores lines starting with '#' and lines without '=' character
    while (std::getline(SettingsFile, fileInputLine)) {
        if (fileInputLine[0] != '#' && fileInputLine.find('=') != std::string::npos) {
            std::size_t found = fileInputLine.find('=');

            if (found != std::string::npos) {
                // Split line along '=' character, trim the values and add the Setting to the settings vector
                std::string settingStr = fileInputLine.substr(0, found);
                std::string valueStr = fileInputLine.substr(found + 1, fileInputLine.length() - found);

                trim(settingStr);
                trim(valueStr);

                Setting setting = { settingStr, valueStr };
                settings.push_back(setting);
            }
        }
    }

    SettingsFile.close();

    return settings;
}
