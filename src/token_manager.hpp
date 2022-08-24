#include "token_manager.h"
#include "grpc_client.h"

namespace grpc_labview
{
    template <typename T>
    TokenManager<T>::TokenManager() :
        _pointerForToken(),
        _mutex()
    {
    }

    template <typename T>
    template <typename TDerivedType>
    std::shared_ptr<TDerivedType> TokenManager<T>::GetPtrForToken(TokenManager::token_type token, int32_t* status)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto ptr = _pointerForToken.find(token);
        if (ptr == _pointerForToken.end())
        {
            std::cerr << "ERROR: NO POINTER EXISTS FOR THIS TOKEN" << std::endl;
            if (status != nullptr)
            {
                *status = -1;
            }
            return std::shared_ptr<TDerivedType>(nullptr);
        }

        std::shared_ptr<TDerivedType> derivedPtr = std::dynamic_pointer_cast<TDerivedType>(ptr->second);
        if (!derivedPtr)
        {
            std::cerr << "ERROR: CASTING POINTER TO MORE SPECIFIC TYPE FAILED" << std::endl;
            if (status != nullptr)
            {
                *status = -1;
            }
            return std::shared_ptr<TDerivedType>(nullptr);
        }

        return derivedPtr;
    }

    template <typename T>
    typename TokenManager<T>::token_type TokenManager<T>::CreateTokenForPtr(T* ptr)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        // NOTE: With this implementation of token generation (just reinterpreting the pointer to an int),
        // we could just use a std::set instead of a std::map since the key & value are basically identical.
        // But I like this approach so that we have the option to obfuscate the token we return to the user.
        auto tokenForPtr = reinterpret_cast<typename TokenManager<T>::token_type>(ptr);
        if (_pointerForToken.find(tokenForPtr) == _pointerForToken.end())
        {
            _pointerForToken.insert({ tokenForPtr, std::shared_ptr<T>(ptr) });
        }
        return tokenForPtr;
    }

    template <typename T>
    bool TokenManager<T>::DestroyTokenForPtr(TokenManager::token_type token)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _pointerForToken.erase(token) > 0;
    }
}
