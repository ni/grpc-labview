#include <mutex>
#include <vector>
#include <string>
#include "proto_descriptor_strings.h"


namespace grpc_labview
{
    // Initialize the static members
    ProtoDescriptorStrings* ProtoDescriptorStrings::m_instance = nullptr;
    std::mutex ProtoDescriptorStrings::m_mutex;

    // Return the static class instance. Thread safe.
    ProtoDescriptorStrings* ProtoDescriptorStrings::getInstance() {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_instance == nullptr) {
            m_instance = new ProtoDescriptorStrings();
        }
        return m_instance;
    }

    // Get the descriptor string
    std::vector<std::string> ProtoDescriptorStrings::getAllDescriptors() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_descriptors;
    }

    // Set the descriptor string
    void ProtoDescriptorStrings::addDescriptor(std::string str) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_descriptors.push_back(str);
    }

    // Delete the instance based on the refcount
    void ProtoDescriptorStrings::deleteInstance() {
        std::unique_lock<std::mutex> lock(m_mutex);
        delete m_instance;
        m_instance = nullptr;
    }
}
