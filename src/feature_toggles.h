#include <string>
#include <unordered_map>
#include <filesystem>


namespace grpc_labview {
    class FeatureConfig {
    public:
        // Singleton instance
        static FeatureConfig& getInstance() {
            static FeatureConfig instance;
            return instance;
        }

        // Function to reload all features from file and re-initialize the feature map
        void ReloadFeaturesFromFile(const std::string& filePath);

        // Function to check if a feature is enabled
        bool IsFeatureEnabled(const std::string& featureName) const;

    private:
        std::unordered_map<std::string, bool> featureFlags;
        const std::string defaultConfigFileName = "feature_config.ini";

        // This stores the default feature configuration.
        // During ReloadFeaturesFromFile(), this configuration is always applied first prior to reading the
        // features configuration file
        void ApplyDefaultFeatures() {
            featureFlags["gRPC"] = true; // Enable gRPC by default as an example, this will never be overridden by config file
            featureFlags["featureFileFound"] = false;  // Used to indicate if the feature file was found/used during initialization
            featureFlags["data_EfficientMessageCopy"] = false;
            featureFlags["data_useOccurrence"] = true;
        }
        
        FeatureConfig() {
            // Read the default configuration file during initialization
            ReloadFeaturesFromFile("");
        }

        // Function to get the default configuration file path
        std::filesystem::path GetDefaultConfigPath() const;

        // Reads and parses a feature configuration file
        void ReadFeatureFile(const std::string& filePath);
    };
}