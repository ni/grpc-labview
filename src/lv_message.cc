//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>
#include <lv_message.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using namespace google::protobuf::internal;

namespace grpc_labview
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LVMessage::LVMessage(std::shared_ptr<MessageMetadata> metadata) : 
        _metadata(metadata)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LVMessage::~LVMessage()
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    google::protobuf::UnknownFieldSet& LVMessage::UnknownFields()
    {
        return _unknownFields;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessage::ParseFromByteBuffer(const grpc::ByteBuffer& buffer)
    {
        Clear();

        std::vector<grpc::Slice> slices;
        buffer.Dump(&slices);
        std::string buf;
        buf.reserve(buffer.Length());
        for (auto s = slices.begin(); s != slices.end(); s++)
        {
            buf.append(reinterpret_cast<const char *>(s->begin()), s->size());
        }
        return ParseFromString(buf);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    std::unique_ptr<grpc::ByteBuffer> LVMessage::SerializeToByteBuffer()
    {
        std::string buf;
        SerializeToString(&buf);
        grpc::Slice slice(buf);
        return std::unique_ptr<grpc::ByteBuffer>(new grpc::ByteBuffer(&slice, 1));
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    google::protobuf::Message* LVMessage::New(google::protobuf::Arena* arena) const
    {
        assert(false); // not expected to be called
        return nullptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVMessage::SetCachedSize(int size) const
    {
        _cached_size_.Set(size);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    int LVMessage::GetCachedSize(void) const
    {
        return _cached_size_.Get();
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVMessage::Clear()
    {
        _values.clear();
    }

    extern bool grpc_labview::useHardCodedParse = TRUE;
    extern bool grpc_labview::ackPtrChange = FALSE;
    extern bool grpc_labview::callCtxDone = TRUE;

    void DebugOutput(const char* format, ...) {
        char debugStringBuf[1024]; // Adjust the buffer size as needed
        va_list args;
        va_start(args, format);
        vsnprintf_s(debugStringBuf, sizeof(debugStringBuf), _TRUNCATE, format, args);
        va_end(args);
        // Append a newline character by default
        strcat_s(debugStringBuf, sizeof(debugStringBuf), "\n");
        // add prefix to debugStringBuf
        char prefixedBuf[1024];
        sprintf_s(prefixedBuf, sizeof(prefixedBuf), "grpc_labview: %s", debugStringBuf);
        OutputDebugStringA(prefixedBuf);
    }

    void readBinaryData(const char* dataLocation, unsigned char*& byteArray, size_t& byteArraySize) {
        // Initialize variables to store the binary data
        size_t dataSize = 0;
        const char* currentLocation = dataLocation;

        // Find the size of the binary data (until null termination)
        while (*currentLocation != '\0') {
            ++dataSize;
            ++currentLocation;
        }

        // Allocate memory for the byte array
        byteArray = new unsigned char[dataSize];
        byteArraySize = dataSize;

        // Copy the binary data into the byte array
        std::memcpy(byteArray, dataLocation, dataSize);
    }

    void DebugOutput2(int iter, const char* ptr) {
        char debugStringBuf[4096]; // Adjust the buffer size as needed

        unsigned char* byteArray;
        size_t byteArraySize;
        readBinaryData(ptr, byteArray, byteArraySize);

        // Create a string to store the formatted hexadecimal data
        // Each byte is represented by 2 hex digits and a space
        char* formattedHexData = new char[byteArraySize * 3 + 1];

        // Format the byte array in hexadecimal format
        int offset = 0;
        for (size_t i = 0; i < byteArraySize; ++i) {
            offset += std::sprintf(formattedHexData + offset, "%02X ", static_cast<unsigned int>(byteArray[i]));
        }


        sprintf_s(debugStringBuf, sizeof(debugStringBuf), "iter = %d ptr = %p data = ", iter, ptr);
        OutputDebugStringA(debugStringBuf);
        OutputDebugStringA(formattedHexData);
        OutputDebugStringA("\n");
        delete[] formattedHexData;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::_InternalParse(const char *ptr, ParseContext *ctx)
    {
        assert(ptr != nullptr);
        if (!useHardCodedParse) {
            auto backupPtr = ptr;
            while (!ctx->Done(&ptr))
            {
                /*if (ackPtrChange)
                    if (backupPtr == ptr) {
                        DebugOutput("ptr changed in iteration %d", _lvResponseCluster.clientCallIter);
                    }
                else
                    ackPtrChange = TRUE;*/

                DebugOutput2(_lvResponseCluster.clientCallIter, ptr);
                google::protobuf::uint32 tag;
                ptr = ReadTag(ptr, &tag);
                auto index = (tag >> 3);
                if (_metadata == nullptr)
                {
                    ptr = UnknownFieldParse(tag, &_unknownFields, ptr, ctx);
                    assert(ptr != nullptr);
                }
                else
                {
                    auto fieldIt = _metadata->_mappedElements.find(index);
                    if (fieldIt != _metadata->_mappedElements.end())
                    {
                        auto fieldInfo = (*fieldIt).second;
                        LVMessageMetadataType dataType = fieldInfo->type;
                        switch (dataType)
                        {
                        case LVMessageMetadataType::Int32Value:
                            ptr = ParseInt32(*fieldInfo, index, ptr, ctx);
                            break;
                        case LVMessageMetadataType::FloatValue:
                            ptr = ParseFloat(*fieldInfo, index, ptr, ctx);
                            break;
                        case LVMessageMetadataType::DoubleValue:
                            ptr = ParseDouble(*fieldInfo, index, ptr, ctx);
                            break;
                        case LVMessageMetadataType::BoolValue:
                            ptr = ParseBoolean(*fieldInfo, index, ptr, ctx);
                            break;
                        case LVMessageMetadataType::StringValue:
                            ptr = ParseString(tag, *fieldInfo, index, ptr, ctx);
                            break;
                        case LVMessageMetadataType::BytesValue:
                            ptr = ParseBytes(tag, *fieldInfo, index, ptr, ctx);
                            break;
                        case LVMessageMetadataType::MessageValue:
                            ptr = ParseNestedMessage(tag, *fieldInfo, index, ptr, ctx);
                            break;
                        case LVMessageMetadataType::Int64Value:
                            ptr = ParseInt64(*fieldInfo, index, ptr, ctx);
                            break;
                        case LVMessageMetadataType::UInt32Value:
                            ptr = ParseUInt32(*fieldInfo, index, ptr, ctx);
                            break;
                        case LVMessageMetadataType::UInt64Value:
                            ptr = ParseUInt64(*fieldInfo, index, ptr, ctx);
                            break;
                        case LVMessageMetadataType::EnumValue:
                            ptr = ParseEnum(*fieldInfo, index, ptr, ctx);
                            break;
                        case LVMessageMetadataType::SInt32Value:
                            ptr = ParseSInt32(*fieldInfo, index, ptr, ctx);
                            break;
                        case LVMessageMetadataType::SInt64Value:
                            ptr = ParseSInt64(*fieldInfo, index, ptr, ctx);
                            break;
                        case LVMessageMetadataType::Fixed32Value:
                            ptr = ParseFixed32(*fieldInfo, index, ptr, ctx);
                            break;
                        case LVMessageMetadataType::Fixed64Value:
                            ptr = ParseFixed64(*fieldInfo, index, ptr, ctx);
                            break;
                        case LVMessageMetadataType::SFixed32Value:
                            ptr = ParseSFixed32(*fieldInfo, index, ptr, ctx);
                            break;
                        case LVMessageMetadataType::SFixed64Value:
                            ptr = ParseSFixed64(*fieldInfo, index, ptr, ctx);
                            break;
                        }
                        assert(ptr != nullptr);
                    }
                    else
                    {
                        if (tag == 0 || WireFormatLite::GetTagWireType(tag) == WireFormatLite::WIRETYPE_END_GROUP) {
                            ctx->SetLastTag(tag);
                            return ptr;
                        }
                        ptr = UnknownFieldParse(tag, &_unknownFields, ptr, ctx);
                        assert(ptr != nullptr);
                    }
                }
            }
        }
        else {
            // get the response cluster pointer
            auto start = _lvResponseCluster._lvResponseClusterPtrStack.back();
            int state = 0;
            uint32_t headerSize;
            uint64_t header_id64;
            uint32_t header_id32;
            google::protobuf::uint32 tag;
            uint32_t bodySize;
            int num_array_elements = 5;
            int x = 0;
            const char* protobufHeaderStart = nullptr;
            const char* protobufHeaderEnd = nullptr;
            const char* protobufMessageStart = nullptr;
            const char* protobufMessageEnd = nullptr;
            uint32_t index = 0;
            LV1DArrayHandle array;

            try {
                // DebugOutput("Starting parse at: %p ", ptr );
                while (!ctx->Done(&ptr))
                {
                    DebugOutput2(_lvResponseCluster.clientCallIter, ptr);
                    // hard coding for now to parse 4 repeated messages of 4 uint64 values
                    // this is for testing the performance of the grpc server
                    switch (state) {
                    case 0:
                        protobufHeaderStart = ptr;
                        ptr = ReadTag(ptr, &tag);
                        // DebugOutput("Reading Tag: %x", tag);
                        if (callCtxDone && ctx->Done(&ptr));
                        headerSize = ReadSize(&ptr);
                        // DebugOutput("Header Size: %x", headerSize);
                        protobufHeaderEnd = protobufHeaderStart + headerSize + 2;
                        // DebugOutput("Header End: %p", protobufHeaderEnd);

                        // DebugOutput("Reading Header");
                        while (ptr < protobufHeaderEnd) {
                            if (callCtxDone && ctx->Done(&ptr));
                            DebugOutput2(_lvResponseCluster.clientCallIter, ptr);
                            ptr = ReadTag(ptr, &tag);
                            if (callCtxDone && ctx->Done(&ptr));
                            // DebugOutput("Reading Tag: %x", tag);
                            index = (tag >> 3);
                            // DebugOutput("Index: %x", index);
                            switch (index) {
                                // t_s0
                                // ptr = ReadTag(ptr, &tag);
                                case 1:
                                    ptr = ReadUINT64(ptr, &header_id64);
                                    *(uint64_t*)start = header_id64;
                                    // DebugOutput("Header t_s0: %x", header_id64);
                                    break;

                                    // t_s1
                                    // ptr = ReadTag(ptr, &tag);
                                case 2: ptr = ReadUINT64(ptr, &header_id64); break;

                                    // t_s2
                                    // ptr = ReadTag(ptr, &tag);
                                case 3: ptr = ReadUINT64(ptr, &header_id64); break;

                                    // t_s3
                                    // ptr = ReadTag(ptr, &tag);
                                case 4: ptr = ReadUINT32(ptr, &header_id32); break;

                                    // t_s4
                                    // ptr = ReadTag(ptr, &tag);
                                case 5: ptr = ReadUINT32(ptr, &header_id32); break;

                                    // t_s5
                                    // ptr = ReadTag(ptr, &tag);
                                case 6: ptr = ReadUINT32(ptr, &header_id32); break;

                                    // t_r0
                                    // ptr = ReadTag(ptr, &tag);
                                case 7:
                                    ptr = ReadUINT64(ptr, &header_id64);
                                    start += 40;
                                    *(uint64_t*)start = header_id64;
                                    // DebugOutput("Header t_r0: %x", header_id64);
                                    break;

                                    // t_r1
                                    // ptr = ReadTag(ptr, &tag);
                                case 8:
                                    ptr = ReadUINT64(ptr, &header_id64);
                                    start += 8;
                                    *(uint64_t*)start = header_id64;
                                    // DebugOutput("Header t_r0: %x", header_id64);
                                    break;

                                    // t_r2
                                    // ptr = ReadTag(ptr, &tag);
                                case 9: ptr = ReadUINT64(ptr, &header_id64); break;

                                    // t_r3
                                    // ptr = ReadTag(ptr, &tag);
                                case 10: ptr = ReadUINT32(ptr, &header_id32); break;
                                default: break;
                            }
                        }
                        state = 1;
                        break;

                    case 1:
                        // DebugOutput("Moving the cluster pointer");
                        start += 24;
                        state = 2;
                        break;

                    case 2:
                        // DebugOutput("Reading the message");
                        DebugOutput2(_lvResponseCluster.clientCallIter, ptr);
                        protobufMessageStart = ptr;
                        ptr = ReadTag(ptr, &tag);
                        // DebugOutput("Reading Tag: %x", tag);
                        if (callCtxDone && ctx->Done(&ptr));
                        bodySize = ReadSize(&ptr);
                        num_array_elements = 5;
                        protobufMessageEnd = protobufMessageStart + bodySize;
                        // DebugOutput("Message End: %p", protobufMessageEnd);

                        NumericArrayResize(0x08, 1, start, num_array_elements * 32);
                        array = *(LV1DArrayHandle*)start;
                        (*array)->cnt = num_array_elements;

                        // for each message in the repeated message
                        //    for each uint64 in the message read the value
                        x = 0;
                        while (!ctx->Done(&ptr)) {
                            //google::protobuf::uint32 tag;
                            DebugOutput2(_lvResponseCluster.clientCallIter, ptr);
                            ptr = ReadTag(ptr, &tag);
                            index = (tag >> 3);
                            // DebugOutput("Reading Tag: %x", tag);
                            // DebugOutput("Index: %x", index);

                            // for (auto str : repeatedNested->_value)
                            // {
                            //     auto lvCluster = (LVCluster**)(*array)->bytes(x * clusterSize, nestedMetadata->alignmentRequirement);
                            //     *lvCluster = nullptr;
                            //     CopyToCluster(*str, (int8_t*)lvCluster);
                            //     x += 1;
                            // }

                            // read length of the message
                            if (callCtxDone && ctx->Done(&ptr));
                            auto len = ReadSize(&ptr);
                            for (int i = 0; i < 4; i++)
                            {
                                // DebugOutput("Reading elements of the array: %x", i);
                                auto lvCluster = (LVCluster**)(*array)->bytes(x * 32, 8);
                                
                                if (callCtxDone && ctx->Done(&ptr));
                                DebugOutput2(_lvResponseCluster.clientCallIter, ptr);
                                // read the tag
                                ptr = ReadTag(ptr, &tag);
                                index = (tag >> 3);
                                // DebugOutput("Reading Tag: %x", tag);
                                // DebugOutput("Index: %x", index);
                                // assert the index
                                // assert(index == (i+1));

                                // read the value
                                uint64_t result;
                                if (callCtxDone && ctx->Done(&ptr));
                                ptr = ReadUINT64(ptr, &result);
                                // assert((result & 0x0f) == (x & 0x0f));
                                // DebugOutput("Reading Value: %x", result);
                                // DebugOutput("Reading Value from addr: %p", ptr);
                                // write the value to the cluster
                                auto offset = ((int8_t*)lvCluster) + ((int)i * 8);
                                *(uint64_t*)offset = result;
                            }
                            x += 1;
                            // DebugOutput("x: %x", x);
                        }
                        state = 3;
                        break;
                    default:
                        
                        break;
                    }
                }
            }
            catch (google::protobuf::FatalException fe)
            {
                DebugOutput2(_lvResponseCluster.clientCallIter, ptr);
                DebugOutput("HardCoded copy exception: %s\n", fe.message());

                std::cout << "HardCoded copy exception: " << fe.message() << std::endl;
            }
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseBoolean(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedBooleanMessageValue>(index);
            ptr = PackedBoolParser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            bool result;
            ptr = ReadBOOL(ptr, &result);
            auto v = std::make_shared<LVBooleanMessageValue>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseInt32(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {    
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedInt32MessageValue>(index);
            ptr = PackedInt32Parser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            int32_t result;
            ptr = ReadINT32(ptr, &result);
            auto v = std::make_shared<LVInt32MessageValue>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseUInt32(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {    
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedUInt32MessageValue>(index);
            ptr = PackedUInt32Parser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            uint32_t result;
            ptr = ReadUINT32(ptr, &result);
            auto v = std::make_shared<LVUInt32MessageValue>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseEnum(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {    
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedEnumMessageValue>(index);
            ptr = PackedEnumParser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            int32_t result;
            ptr = ReadENUM(ptr, &result);
            auto v = std::make_shared<LVEnumMessageValue>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseInt64(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {    
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedInt64MessageValue>(index);
            ptr = PackedInt64Parser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            int64_t result;
            ptr = ReadINT64(ptr, &result);
            auto v = std::make_shared<LVInt64MessageValue>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseUInt64(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {    
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedUInt64MessageValue>(index);
            ptr = PackedUInt64Parser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            uint64_t result;
            ptr = ReadUINT64(ptr, &result);
            auto v = std::make_shared<LVUInt64MessageValue>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseFloat(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {    
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedFloatMessageValue>(index);
            ptr = PackedFloatParser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            float result;
            ptr = ReadFLOAT(ptr, &result);
            auto v = std::make_shared<LVFloatMessageValue>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseDouble(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {    
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedDoubleMessageValue>(index);
            ptr = PackedDoubleParser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            double result;
            ptr = ReadDOUBLE(ptr, &result);
            auto v = std::make_shared<LVDoubleMessageValue>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseString(google::protobuf::uint32 tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {    
        if (fieldInfo.isRepeated)
        {
            std::shared_ptr<LVRepeatedStringMessageValue> v;
            auto it = _values.find(index);
            if (it == _values.end())
            {
                v = std::make_shared<LVRepeatedStringMessageValue>(index);
                _values.emplace(index, v);
            }
            else
            {
                v = std::static_pointer_cast<LVRepeatedStringMessageValue>((*it).second);
            }
            ptr -= 1;
            do {
                ptr += 1;
                auto str = v->_value.Add();
                ptr = InlineGreedyStringParser(str, ptr, ctx);
                if (!ctx->DataAvailable(ptr))
                {
                    break;
                }
            } while (ExpectTag(tag, ptr));
        }
        else
        {
            auto str = std::string();
            ptr = InlineGreedyStringParser(&str, ptr, ctx);
            auto v = std::make_shared<LVStringMessageValue>(index, str);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseBytes(google::protobuf::uint32 tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {
        return ParseString(tag, fieldInfo, index, ptr, ctx);    
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessage::ExpectTag(google::protobuf::uint32 tag, const char* ptr)
    {
        if (tag < 128)
        {
            return *ptr == tag;
        } 
        else
        {
            char buf[2] = {static_cast<char>(tag | 0x80), static_cast<char>(tag >> 7)};
            return std::memcmp(ptr, buf, 2) == 0;
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char* LVMessage::ParseSInt32(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, ParseContext* ctx)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedSInt32MessageValue>(index);
            ptr = PackedSInt32Parser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            int32_t result;
            ptr = ReadSINT32(ptr, &result);
            auto v = std::make_shared<LVSInt32MessageValue>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char* LVMessage::ParseSInt64(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, ParseContext* ctx)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedSInt64MessageValue>(index);
            ptr = PackedSInt64Parser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            int64_t result;
            ptr = ReadSINT64(ptr, &result);
            auto v = std::make_shared<LVSInt64MessageValue>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char* LVMessage::ParseFixed32(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, ParseContext* ctx)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedFixed32MessageValue>(index);
            ptr = PackedFixed32Parser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            uint32_t result;
            ptr = ReadFIXED32(ptr, &result);
            auto v = std::make_shared<LVFixed32MessageValue>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }


    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char* LVMessage::ParseFixed64(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, ParseContext* ctx)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedFixed64MessageValue>(index);
            ptr = PackedFixed64Parser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            uint64_t result;
            ptr = ReadFIXED64(ptr, &result);
            auto v = std::make_shared<LVFixed64MessageValue>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char* LVMessage::ParseSFixed32(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, ParseContext* ctx)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedFixed32MessageValue>(index);
            ptr = PackedSFixed32Parser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            int32_t result;
            ptr = ReadSFIXED32(ptr, &result);
            auto v = std::make_shared<LVSFixed32MessageValue>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char* LVMessage::ParseSFixed64(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, ParseContext* ctx)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedSFixed64MessageValue>(index);
            ptr = PackedSFixed64Parser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            int64_t result;
            ptr = ReadSFIXED64(ptr, &result);
            auto v = std::make_shared<LVSFixed64MessageValue>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }

    LVMessage::LabVIEWResponseCluster LVMessage::_lvResponseCluster;

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseNestedMessage(google::protobuf::uint32 tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {    
        auto metadata = fieldInfo._owner->FindMetadata(fieldInfo.embeddedMessageName);
        if (fieldInfo.isRepeated)
        {
            // auto ptrBackup = ptr;
            // auto len = ReadSize(&ptr);
            // ptr = ptrBackup;

            ptr -= 1;
            do {
                std::shared_ptr<LVRepeatedNestedMessageMessageValue> v;
                auto it = _values.find(index);
                if (it == _values.end())
                {
                    v = std::make_shared<LVRepeatedNestedMessageMessageValue>(index);
                    _values.emplace(index, v);
                }
                else
                {
                    v = std::static_pointer_cast<LVRepeatedNestedMessageMessageValue>((*it).second);
                }
                ptr += 1;
                auto nestedMessage = std::make_shared<LVMessage>(metadata);
                ptr = ctx->ParseMessage(nestedMessage.get(), ptr);
                v->_value.push_back(nestedMessage);
                if (!ctx->DataAvailable(ptr))
                {
                    break;
                }
            } while (ExpectTag(tag, ptr));
        }
        else
        {
            auto nestedMessage = std::make_shared<LVMessage>(metadata);
            ptr = ctx->ParseMessage(nestedMessage.get(), ptr);
            auto v = std::make_shared<LVNestedMessageMessageValue>(index, nestedMessage);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    google::protobuf::uint8 *LVMessage::_InternalSerialize(google::protobuf::uint8 *target, google::protobuf::io::EpsCopyOutputStream *stream) const
    {
        for (auto e : _values)
        {
            target = e.second->Serialize(target, stream);
        }
        return target;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    size_t LVMessage::ByteSizeLong() const
    {
        size_t totalSize = 0;

        for (auto e : _values)
        {
            totalSize += e.second->ByteSizeLong();
        }
        int cachedSize = ToCachedSize(totalSize);
        SetCachedSize(cachedSize);
        return totalSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessage::IsInitialized() const
    {
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVMessage::SharedCtor()
    {
        assert(false); // not expected to be called
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVMessage::SharedDtor()
    {
        assert(false); // not expected to be called
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVMessage::ArenaDtor(void *object)
    {
        assert(false); // not expected to be called
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVMessage::RegisterArenaDtor(google::protobuf::Arena *)
    {
        assert(false); // not expected to be called
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVMessage::MergeFrom(const google::protobuf::Message &from)
    {
        assert(false); // not expected to be called
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVMessage::MergeFrom(const LVMessage &from)
    {
        assert(false); // not expected to be called
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVMessage::CopyFrom(const google::protobuf::Message &from)
    {
        assert(false); // not expected to be called
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVMessage::CopyFrom(const LVMessage &from)
    {
        assert(false); // not expected to be called
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVMessage::InternalSwap(LVMessage *other)
    {
        assert(false); // not expected to be called
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    google::protobuf::Metadata LVMessage::GetMetadata() const
    {
        assert(false); // not expected to be called
        return google::protobuf::Metadata();
    }
}

