#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>

namespace grpc_labview {
    class FeatureConfig {
    private:
        std::unordered_map<std::string, bool> featureFlags;

        // Constructor to initialize with default values
        FeatureConfig() {
            featureFlags["gRPC"] = true; // Enable gRPC by default as an example, this will never be overridden by config file
            featureFlags["data_EfficientMessageCopy"] = true;
            featureFlags["data_useOccurrence"] = true;
        }

    public:
        // Singleton instance
        static FeatureConfig& getInstance() {
            static FeatureConfig instance;
            return instance;
        }

        // Function to read feature configurations from an INI file
        void readConfigFromFile(const std::string& filePath);

        // Function to check if a feature is enabled
        bool isFeatureEnabled(const std::string& featureName) const;
    };

}