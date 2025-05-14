#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>

namespace grpc_labview {
    class FeatureConfig {
    private:
        static FeatureConfig* _instance;

        std::unordered_map<std::string, bool> featureFlags;
        const std::string defaultConfigFileName = "feature_config.ini";

        // Constructor to initialize with default values
        FeatureConfig() {
            featureFlags["gRPC"] = true; // Enable gRPC by default as an example, this will never be overridden by config file
            featureFlags["data_EfficientMessageCopy"] = false;
            featureFlags["data_useOccurrence"] = true;

            // Read the default configuration file
            ReadConfigFromFile("");
        }

        // Function to get the default configuration file path
        std::string GetDefaultConfigPath();

    public:
        // Singleton instance
        static FeatureConfig& getInstance() {
            static FeatureConfig instance;
            return instance;
        }

        // Function to read feature configurations from an INI file
        void ReadConfigFromFile(const std::string& filePath);

        // Function to check if a feature is enabled
        bool IsFeatureEnabled(const std::string& featureName) const;
    };
}