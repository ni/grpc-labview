#include <string>
#include <unordered_map>
#include <filesystem>


namespace grpc_labview {
    static constexpr const char* kFeatureFileFound = "featureFileFound";
    static constexpr const char* kFeatureEfficientMessageCopy = "data_EfficientMessageCopy";
    static constexpr const char* kFeatureUseOccurrence = "data_useOccurrence";
    static constexpr const char* kFeatureUtf8Strings = "data_utf8Strings";
    static constexpr const char* kFeatureVerifyStringEncoding = "data_verifyStringEncoding";

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

        // Functions to check specific feature toggles
        bool IsEfficientMessageCopyEnabled() const { return efficientMessageCopy; }
        bool IsUseOccurrenceEnabled() const { return useOccurrence; }
        bool AreUtf8StringsEnabled() const { return utf8Strings; }
        bool IsVerifyStringEncodingEnabled() const { return verifyStringEncoding; }

    private:
        std::unordered_map<std::string, bool> featureFlags;
        const std::string defaultConfigFileName = "feature_config.ini";

        bool efficientMessageCopy;
        bool useOccurrence;
        bool utf8Strings;
        bool verifyStringEncoding;

        // This stores the default feature configuration.
        // During ReloadFeaturesFromFile(), this configuration is always applied first prior to reading the
        // features configuration file
        void ApplyDefaultFeatures() {
            featureFlags[kFeatureFileFound] = false;  // Used to indicate if the feature file was found/used during initialization
            featureFlags[kFeatureEfficientMessageCopy] = false;
            featureFlags[kFeatureUseOccurrence] = true;
            featureFlags[kFeatureUtf8Strings] = true;
            featureFlags[kFeatureVerifyStringEncoding] = true;
        }

        FeatureConfig() :
            efficientMessageCopy(false),
            useOccurrence(false),
            utf8Strings(false),
            verifyStringEncoding(false)
        {
            // Read the default configuration file during initialization
            ReloadFeaturesFromFile("");
        }

        // Function to get the default configuration file path
        std::filesystem::path GetDefaultConfigPath() const;

        // Reads and parses a feature configuration file
        void ReadFeatureFile(const std::string& filePath);
    };
}