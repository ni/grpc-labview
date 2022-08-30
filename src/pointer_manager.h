#pragma once

#include <memory>
#include <mutex>
#include <map>

namespace grpc_labview
{
    //---------------------------------------------------------------------
    // Definition
    //---------------------------------------------------------------------

    /// A pointer manager to track & manage the lifetime of pointers we expose to LabVIEW
    /// This class serves two purposes:
    /// 1. Ensure the validity of pointers -- This class knows which pointers are valid, so if
    ///    LabVIEW passes us an invalid/stale pointer, we know it's invalid without ever trying to cast it.
    /// 2. Gracefully clean up pointers only when they're no longer being used by utilizing RAII (std::shared_ptr).
    ///    Client code never has to explicitly call `delete` on pointers. std::shared_ptr automatically calls
    ///    delete when there are no remaining instances of std::shared_ptr for a given pointer. Usually, the
    ///    call to UnregisterPointer() will remove the final instance of std::shared_ptr from the set, and that will
    ///    cause `delete` to be called on that pointer. But if some other method in some other thread still has the std::shared_ptr
    ///    from TryCastTo(), then `delete` won't be called until that other std::shared_ptr goes out of scope.
    template <typename T>
    class PointerManager
    {
    public:
        PointerManager<T>();

        /// Register a pointer. This creates a std::shared_ptr for that pointer and stuffs it into a std::set.
        /// The pointer will be alive until that shared_ptr and all its copies (created via GetPointer) are out of scope.
        /// Call UnregisterPointer to allow the pointer to be destroyed.
        T* RegisterPointer(T* ptr);

        /// Get the pointer for a given pointer.
        /// If the pointer has been registered, returns a shared_ptr containing the requested pointer.
        /// If the pointer is null or unregistered, returns a default std::shared_ptr and optionally sets a status code.
        template<typename TDerivedType>
        std::shared_ptr<TDerivedType> TryCastTo(T* ptr, int32_t* status = nullptr);

        /// Unregister a given pointer. This will remove the std::shared_ptr from the set, which will usually cause `delete` to be called
        /// (unless some other callstack still has a copy of the std::shared_ptr returned by the GetPointer method).
        /// Returns true if the pointer was found (and removed), false otherwise
        bool UnregisterPointer(T* ptr);

    private:
        std::map<T*, std::shared_ptr<T>> _registeredPointers;
        std::mutex _mutex;
    };

    //---------------------------------------------------------------------
    // Implementation
    //---------------------------------------------------------------------

    template <typename T>
    PointerManager<T>::PointerManager() :
        _registeredPointers(),
        _mutex()
    {
    }

    template <typename T>
    template <typename TDerivedType>
    std::shared_ptr<TDerivedType> PointerManager<T>::TryCastTo(T* ptr, int32_t* status)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto storedPtr = _registeredPointers.find(ptr);
        if (storedPtr == _registeredPointers.end())
        {
            std::cerr << "ERROR: THIS POINTER IS NOT REGISTERED" << std::endl;
            if (status != nullptr)
            {
                *status = -1;
            }
            return std::shared_ptr<TDerivedType>(nullptr);
        }

        std::shared_ptr<TDerivedType> derivedPtr = std::dynamic_pointer_cast<TDerivedType>(storedPtr->second);
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
    typename T* PointerManager<T>::RegisterPointer(T* ptr)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!ptr)
        {
            std::cerr << "ERROR: CANNOT REGISTER A NULL POINTER" << std::endl;
            return nullptr;
        }

        if (_registeredPointers.find(ptr) == _registeredPointers.end())
        {
            _registeredPointers.insert({ ptr, std::shared_ptr<T>(ptr) });
        }
        return ptr;
    }

    template <typename T>
    bool PointerManager<T>::UnregisterPointer(T* ptr)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _registeredPointers.erase(ptr) > 0;
    }
}
