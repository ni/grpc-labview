#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

class FeatureConfig {
private:
    std::map<std::string, bool> featureFlags;

    // Constructor to initialize with default values
    FeatureConfig() {
        featureFlags["gRPC"] = true; // Enable gRPC by default as an example, this will never be overridden by config file
    }

public:
    // Singleton instance
    static FeatureConfig& getInstance() {
        static FeatureConfig instance;
        return instance;
    }

    // Function to read feature configurations from an INI file
    void readConfigFromFile(const std::string& filePath) {
        std::ifstream configFile(filePath);
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
            } else {
                // Parse key-value pairs
                std::istringstream iss(line);
                std::string key, value;
                if (std::getline(iss, key, '=') && std::getline(iss, value)) {
                    // Append section name to key for uniqueness
                    std::string fullKey = currentSection.empty() ? key : currentSection + "_" + key;
                    featureFlags[fullKey] = (value == "true");
                }
            }
        }

        configFile.close();
    }

    // Function to check if a feature is enabled
    bool isFeatureEnabled(const std::string& featureName) const {
        auto it = featureFlags.find(featureName);
        return (it != featureFlags.end()) ? it->second : false;
    }
};