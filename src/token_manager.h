#pragma once

#include <memory>
#include <mutex>
#include <map>

namespace grpc_labview
{
    //---------------------------------------------------------------------
    // Definition
    //---------------------------------------------------------------------

    /// A token manager to track & manage the lifetime of pointers
    /// This class serves two purposes:
    /// 1. Ensure the validity of pointers -- This class knows which tokens are valid, so if
    ///    LabVIEW passes us an invalid/stale token, we know it's invalid without ever trying to cast it.
    /// 2. Gracefully clean up pointers only when they're no longer being used by utilizing RAII (std::shared_ptr).
    ///    Client code never has to explicitly call `delete` on pointers. std::shared_ptr automatically calls
    ///    delete when there are no remaining instances of std::shared_ptr for a given pointer. Usually, the
    ///    call to DestroyTokenForPtr() will remove the final instance of std::shared_ptr from the map, and that will
    ///    cause `delete` to be called on that pointer. But if some other method in some other thread still has the std::shared_ptr
    ///    from GetPtrFromToken(), then `delete` won't be called until that other std::shared_ptr goes out of scope.
    template <typename T>
    class TokenManager
    {
    public:
        typedef uintptr_t token_type;
        TokenManager<T>();

        /// Create a token for a given pointer. This creates a std::shared_ptr for that pointer and stuffs it into a std::map.
        /// The token will be alive until that shared_ptr and all its copies (created via GetPtrForToken) are out of scope.
        /// Call DestroyTokenForPtr to allow the pointer to be destroyed.
        token_type CreateTokenForPtr(T* ptr);

        /// Get the pointer for a given token.
        /// If the token is valid, returns a shared_ptr containing the requested pointer.
        /// If the token is invalid, returns a default std::shared_ptr and optionally sets a status code.
        template<typename TDerivedType>
        std::shared_ptr<TDerivedType> GetPtrForToken(token_type token, int32_t* status = nullptr);

        /// Destroy a given token. This will remove the std::shared_ptr from the map, which will usually cause `delete` to be called
        /// (unless some other callstack still has a copy of the std::shared_ptr returned by the GetPtrForToken method).
        /// Returns true if the token was found (and removed), false otherwise
        bool DestroyTokenForPtr(token_type token);

    private:
        std::map<token_type, std::shared_ptr<T>> _pointerForToken;
        std::mutex _mutex;
    };

    //---------------------------------------------------------------------
    // Implementation
    //---------------------------------------------------------------------

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