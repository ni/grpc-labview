#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>
#include <filesystem>

#include "feature_toggles.h"
#include "path_support.h"


namespace grpc_labview {

    // Function to reload all features from file.
    // If filePath is empty, the default feature configuration file will be used
    // If filePath contains a path, that specific configuration file will be used
    //   This filePath can contain either a filename (which is assumed to be in the
    //   same directory as this shared library) or an absolute path.
    //
    // Note that all features will be re-initialized using the new file.
    void FeatureConfig::ReloadFeaturesFromFile(const std::string& filePath) {
        // Clear all features from the map to re-initialize
        featureFlags.clear();

        // Apply the default feature configuration and then read from the passed in feature config file
        ApplyDefaultFeatures();
        ReadFeatureFile(filePath);

        //TODO: remove this post fixing LVMessageEfficient to let enable feature. See issue: #433
        if (featureFlags.find(kFeatureEfficientMessageCopy) != featureFlags.end())
            featureFlags[kFeatureEfficientMessageCopy] = false;

        efficientMessageCopy = IsFeatureEnabled(kFeatureEfficientMessageCopy);
        useOccurrence = IsFeatureEnabled(kFeatureUseOccurrence);
        utf8Strings = IsFeatureEnabled(kFeatureUtf8Strings);
    }

    // Function to check if a feature is enabled
    bool FeatureConfig::IsFeatureEnabled(const std::string& featureName) const {
        auto it = featureFlags.find(featureName);
        return (it != featureFlags.end()) ? it->second : false;
    }

    // Function to read feature configurations from an INI file
    void FeatureConfig::ReadFeatureFile(const std::string& filePath) {
        std::filesystem::path configFilePath = filePath;

        if (configFilePath.empty()) {
            // If no filepath passed into this function, use the default
            // configuration file path.
            configFilePath = GetDefaultConfigPath();
        }
        else if (configFilePath.is_relative()) {
            // If a relative path (ie, just a filename) was passed in,
            // assume that the is in the current directory
            configFilePath = GetFolderContainingDLL() / configFilePath;
        }

        // This flag is used to indicate that the default/given feature file was found/used.
        // Reset this flag to set to true after the file is successfully opened
        featureFlags[kFeatureFileFound] = false;

        std::ifstream configFile(configFilePath.string());
        if (!configFile.is_open()) {
            return;
        }

        // Set this feature to true once the file is opened.  Note that this feature flag
        // does not indicate if the file was successfully parsed; only that it was found
        // and opened.
        featureFlags[kFeatureFileFound] = true;

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
    }

    std::filesystem::path FeatureConfig::GetDefaultConfigPath() const {
        // Get the path of where this shared library is
        std::filesystem::path configPath = GetFolderContainingDLL();

        // Add the default config filename to path
        configPath /= defaultConfigFileName;
        return configPath;
    }
}