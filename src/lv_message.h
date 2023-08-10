//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <message_value.h>
#include <message_metadata.h>
#include <google/protobuf/message.h>
#include <vector>
#include <list>
#include <memory>
#include <stdexcept>
#include <mutex>


namespace grpc_labview 
{

    // ---
    // Custom Allocator
    // ---
    // template <class T>
    // class MyAllocator: public std::allocator<T> {
    // public:

    //     std::vector<T> _storageVector;
    //     size_t _currentSize = 0;
    //     size_t _occupiedCount = 0;
    //     // std::mutex _allocatorMutex;
        
    //     template <class U>
    //     constexpr MyAllocator (const MyAllocator <U>&) noexcept {}
    //     MyAllocator() { }

    //     T* allocate(std::size_t n) {
    //         // std::lock_guard<std::mutex> guard(_allocatorMutex);
    //         if (_currentSize + n >= _storageVector.size()) {
    //             _storageVector.resize(100 * (_storageVector.size() + n));
    //         }

    //         T* retPtr = &_storageVector[_currentSize];
    //         _currentSize += n;
    //         _occupiedCount += n;
    //         return retPtr;
    //     }

    //     void deallocate(T* p, std::size_t n) {
    //         // std::lock_guard<std::mutex> guard(_allocatorMutex);
    //         _occupiedCount -= n;
    //         if (_occupiedCount < 0) {
    //             throw std::runtime_error("Invalid number of deallocations");
    //         }
    //         if (_occupiedCount == 0) {
    //             _currentSize = 0;
    //         }
    //     }
    // };

    template <typename T>
    class VectorList {
    public:
        void resize(size_t n) {
            if (_capacity >= n) {
                return;
            }
            _vectorList.push_back(std::move(std::vector<T>(n - _capacity)));
            _capacity = n;
        }
        T operator[](size_t index) {
            if (index > _capacity) {
                throw std::out_of_range();
            }
            auto iter = _vectorList.begin();
            size_t sumSize = iter->size();
            while(iter != _vectorList.end()) {
                if (sumSize < index) {
                    iter++;
                    sumSize += iter->size();
                } else {
                    break;
                }
            }
            return (*iter)[index - sumSize];
        }
    private:
        std::list<std::vector<T>> _vectorList;
        size_t _capacity = 0;
    };

    template <class T>
    class MyAllocator: public std::allocator<T> {
    public:
        using value_type = T;
        VectorList<T> _storageVector;
        size_t _currentSize = 0;
        size_t _occupiedCount = 0;
        // std::mutex _allocatorMutex;
        
        template <class U>
        constexpr MyAllocator (const MyAllocator <U>&) noexcept {}
        MyAllocator() { }

        T* allocate(std::size_t n) {
            // std::lock_guard<std::mutex> guard(_allocatorMutex);
            if (_currentSize + n >= _storageVector.size()) {
                _storageVector.resize(100 * (_storageVector.size() + n));
            }

            T* retPtr = &_storageVector[_currentSize];
            _currentSize += n;
            _occupiedCount += n;
            return retPtr;
        }

        void deallocate(T* p, std::size_t n) {
            // std::lock_guard<std::mutex> guard(_allocatorMutex);
            _occupiedCount -= n;
            if (_occupiedCount < 0) {
                throw std::runtime_error("Invalid number of deallocations");
            }
            if (_occupiedCount == 0) {
                _currentSize = 0;
            }
        }
    };

    // template<class T, class U>
    // bool operator==(const MyAllocator <T>&, const MyAllocator <U>&) { return true; }
    // template<class T, class U>
    // bool operator!=(const MyAllocator <T>&, const MyAllocator <U>&) { return false; }

    class VectorValuesMap;

    class VectorValuesMapIterator {
    public:
        const VectorValuesMap* _valuesMap;
        size_t _index;
        VectorValuesMapIterator(size_t index, const VectorValuesMap* valuesMap);
        std::shared_ptr<LVMessageValue> operator*(void);
        std::shared_ptr<LVMessageValue> operator->(void);
        VectorValuesMapIterator operator++();
        bool operator==(VectorValuesMapIterator rhs);
        bool operator!=(VectorValuesMapIterator rhs);
    };

    class VectorValuesMap {
    public:
        // Known issues:
        // 1. If the vector is modified (resized) when using iterators might lead to invalid state.
        // Work around this by always calling begin/end rather using stale variables.
        std::vector<std::shared_ptr<LVMessageValue>> _values;
        VectorValuesMapIterator begin() const;
        VectorValuesMapIterator end() const;
        void emplace(size_t index, std::shared_ptr<grpc_labview::LVMessageValue> value);
        VectorValuesMapIterator find(size_t index);
        size_t size() { return _values.size(); }
        void clear();
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVMessage : public google::protobuf::Message, public gRPCid
    {
    public:
        LVMessage(std::shared_ptr<MessageMetadata> metadata);
        ~LVMessage();

        google::protobuf::UnknownFieldSet& UnknownFields();

        Message* New(google::protobuf::Arena* arena) const override;
        void SharedCtor();
        void SharedDtor();
        void ArenaDtor(void* object);
        void RegisterArenaDtor(google::protobuf::Arena*);

        void Clear()  final;
        bool IsInitialized() const final;

        const char* _InternalParse(const char *ptr, google::protobuf::internal::ParseContext *ctx)  override final;
        google::protobuf::uint8* _InternalSerialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override final;
        void SetCachedSize(int size) const final;
        int GetCachedSize(void) const final;
        size_t ByteSizeLong() const final;
        
        void MergeFrom(const google::protobuf::Message &from) final;
        void MergeFrom(const LVMessage &from);
        void CopyFrom(const google::protobuf::Message &from) final;
        void CopyFrom(const LVMessage &from);
        void InternalSwap(LVMessage *other);
        google::protobuf::Metadata GetMetadata() const final;

        bool ParseFromByteBuffer(const grpc::ByteBuffer& buffer);
        std::unique_ptr<grpc::ByteBuffer> SerializeToByteBuffer();

    public:
        // std::map<int, std::shared_ptr<LVMessageValue>> _values;
        VectorValuesMap _values;
        std::shared_ptr<MessageMetadata> _metadata;

    private:
        mutable google::protobuf::internal::CachedSize _cached_size_;
        google::protobuf::UnknownFieldSet _unknownFields;

        const char *ParseBoolean(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseInt32(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseUInt32(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseEnum(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseInt64(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseUInt64(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseFloat(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseDouble(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char* ParseSInt32(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx);
        const char* ParseSInt64(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx);
        const char* ParseFixed32(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx);
        const char* ParseFixed64(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx);
        const char* ParseSFixed32(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx);
        const char* ParseSFixed64(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx);
        const char *ParseString(unsigned int tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseBytes(unsigned int tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseNestedMessage(google::protobuf::uint32 tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        bool ExpectTag(google::protobuf::uint32 tag, const char* ptr);
    };
}
