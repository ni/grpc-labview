#include <feature_toggles.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>
#include <path_support.h>


namespace grpc_labview {
    // Function to read feature configurations from an INI file
    void FeatureConfig::ReadConfigFromFile(const std::string& filePath) {
        std::string configFilePath = filePath;
        
        if (configFilePath.empty()) {
            // If no filepath passed into this function, use the default
            // configuration file path.
            configFilePath = GetDefaultConfigPath();
        }

        std::ifstream configFile(configFilePath);
        if (!configFile.is_open()) {
            return;
        }

        std::string line;
        std::string currentSection; // For handling INI sections

        while (std::getline(configFile, line)) {
            // Trim leading and trailing whitespaces
            line.erase(line.find_last_not_of(" \t") + 1);
            line.erase(0, line.find_first_not_of(" \t"));

            // Skip comments and empty lines
            if (line.empty() || line[0] == ';') {
                continue;
            }

            // Check for section header
            if (line[0] == '[' && line[line.length() - 1] == ']') {
                currentSection = line.substr(1, line.length() - 2);
            }
            else {
                // Parse key-value pairs
                std::istringstream iss(line);
                std::string key, value;
                if (std::getline(iss, key, '=') && std::getline(iss, value)) {
                    // Append section name to key for uniqueness
                    key.erase(std::remove_if(key.begin(), key.end(), ::isspace),key.end());
                    value.erase(std::remove_if(value.begin(), value.end(), ::isspace),value.end());
                    std::string fullKey = currentSection.empty() ? key : currentSection + "_" + key;
                    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
                    featureFlags[fullKey] = (value == "true");
                }
            }
        }

        configFile.close();
        //TODO: remove this post fixing LVMessageEfficient to let enable feature. See issue: #433
        if (featureFlags.find("data_EfficientMessageCopy") != featureFlags.end())
            featureFlags["data_EfficientMessageCopy"] = false;
    }

    // Function to check if a feature is enabled
    bool FeatureConfig::IsFeatureEnabled(const std::string& featureName) const {
        auto it = featureFlags.find(featureName);
        return (it != featureFlags.end()) ? it->second : false;
    }

    std::string FeatureConfig::GetDefaultConfigPath() {
#ifdef _WIN32
        return GetFolderContainingDLL() + "\\" + defaultConfigFileName;
#else
        return GetFolderContainingDLL() + "/" + defaultConfigFileName;
#endif
    }
}