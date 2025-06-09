//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <vector>
#include <string>
#include <mutex>


namespace grpc_labview
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class ProtoDescriptorStrings {
        // Static members
        static ProtoDescriptorStrings* m_instance;
        static std::mutex m_mutex;

        // Non static members
        std::vector<std::string> m_descriptors;
        int m_refcount = 0; // Not a normal refcount. Its counts the number of time we set the descriptor string.

        // Default private constructor to prevent instantiation
        ProtoDescriptorStrings() = default;

        // Delete copy constructor and assignment operator
        ProtoDescriptorStrings(const ProtoDescriptorStrings&) = delete;
        ProtoDescriptorStrings& operator=(const ProtoDescriptorStrings&) = delete;
    public:
        // Return the static class instance
        static ProtoDescriptorStrings* getInstance();

        // Set the descriptor string
        void addDescriptor(std::string);

        // Get the descriptor string
        std::vector<std::string> getAllDescriptors();

        // Delete the instance based on the refcount
        void deleteInstance();
    };
}
