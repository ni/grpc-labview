//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <google/protobuf/compiler/importer.h>
#include <lv_interop.h>
#include <sstream>
#include <list>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using namespace google::protobuf::compiler;
using namespace google::protobuf;

namespace grpc_labview
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    #ifdef _PS_4
    #pragma pack (push, 1)
    #endif
    struct MessageFieldCluster
    {    
        LStrHandle fieldName;
        LStrHandle embeddedMessage;
        int32_t protobufIndex;
        int32_t type;
        char isRepeated;
        char isInOneof;
        LStrHandle oneofContainerName;
    };

    struct EnumFieldCluster
    {
        int32_t version;
        LStrHandle name;
        LStrHandle typeUrl;
        LStrHandle enumValues;
        char isAliasAllowed;
    };
    #ifdef _PS_4
    #pragma pack (pop)
    #endif

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class ErrorCollector : public MultiFileErrorCollector
    {
    public:
        void AddError(const std::string & filename, int line, int column, const std::string & message) override;
        std::string GetLVErrorMessage();

    private:
        std::list<std::string> _errors;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ErrorCollector::AddError(const std::string & filename, int line, int column, const std::string & message)
    {
        std::string errorMessage = filename + ": " + std::to_string(line) + " - " + message; 
        _errors.emplace_back(errorMessage);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    std::string ErrorCollector::GetLVErrorMessage()
    {
        std::stringstream result;
        for (auto& s: _errors)
        {
            result << s << std::endl;
        }
        return result.str();
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVProtoParser
    {
    public:
        LVProtoParser();
        void AddSearchPath(const std::string& searchPath);
        void Import(const std::string& filePath, const std::string& searchPath);

    public:
        DiskSourceTree m_SourceTree;
        ErrorCollector m_ErrorCollector;
        Importer m_Importer;
        std::list<std::string> m_searchPaths;
        const FileDescriptor* m_FileDescriptor;
        static LVProtoParser* s_Parser;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LVProtoParser* LVProtoParser::s_Parser = nullptr;

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LVProtoParser::LVProtoParser()
        : m_Importer(&m_SourceTree, &m_ErrorCollector)
    {    
        s_Parser = this;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVProtoParser::AddSearchPath(const std::string& searchPath)
    {
        std::string path = searchPath;
        std::replace(path.begin(), path.end(), '\\', '/');
        m_searchPaths.push_back(path);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVProtoParser::Import(const std::string& filePath, const std::string& search)
    {
        std::string searchPath = search;
        std::replace(searchPath.begin(), searchPath.end(), '\\', '/');
        m_SourceTree.MapPath("", searchPath);
        for (auto path: m_searchPaths)
        {
            m_SourceTree.MapPath("", path);
        }
        
        std::string path = filePath;
        std::replace(path.begin(), path.end(), '\\', '/');
        
        m_FileDescriptor = m_Importer.Import(path);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    std::string TransformMessageName(const std::string& messageName)
    {
        std::string result = messageName;
        std::replace(result.begin(), result.end(), '.', '_');
        return result;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void AddFieldError(FieldDescriptor* field, std::string message)
    {
        grpc_labview::LVProtoParser::s_Parser->m_ErrorCollector.AddError("", 0, 0, message);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void AddNestedMessages(const google::protobuf::Descriptor& descriptor, std::set<const google::protobuf::Descriptor*>& messages)
    {
        auto count = descriptor.nested_type_count();
        for (int x=0; x<count; ++x)
        {
            auto current = descriptor.nested_type(x);        
            AddNestedMessages(*current, messages);
            messages.emplace(current);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void AddDependentMessages(const google::protobuf::FileDescriptor& descriptor, std::set<const google::protobuf::Descriptor*>& messages)
    {
        auto imported = descriptor.dependency_count();
        for (int x=0; x<imported; ++x)
        {
            AddDependentMessages(*descriptor.dependency(x), messages);
        }

        auto count = descriptor.message_type_count();
        for (int x=0; x<count; ++x)
        {
            auto current = descriptor.message_type(x);        
            AddNestedMessages(*current, messages);
            messages.emplace(current);
        }
    }
}


//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVGetgRPCAPIVersion(int* version)
{
    grpc_labview::InitCallbacks();

    *version = 2;
    return 0;    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVCreateParser(grpc_labview::LVProtoParser** parser)
{
    grpc_labview::InitCallbacks();

    *parser = new grpc_labview::LVProtoParser();
    return 0;
}


//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVAddParserSearchPath(grpc_labview::LVProtoParser* parser, const char* searchPath)
{
    if (parser == nullptr)
    {
        return -1;
    }
    parser->AddSearchPath(searchPath);
    return 0;    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVImportProto2(grpc_labview::LVProtoParser* parser, const char* filePath, const char* searchPath)
{
    if (parser == nullptr)
    {
        return -1;
    }
    parser->Import(filePath, searchPath);
    return 0;    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVImportProto(const char* filePath, const char* searchPath, grpc_labview::LVProtoParser** parser)
{
    grpc_labview::InitCallbacks();

    *parser = new grpc_labview::LVProtoParser();
    (*parser)->Import(filePath, searchPath);
    return 0;    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVGetErrorString(grpc_labview::LVProtoParser* parser, grpc_labview::LStrHandle* error)
{
    if (parser == nullptr)
    {
        return -1;
    }
    auto errorMessage = parser->m_ErrorCollector.GetLVErrorMessage();
    grpc_labview::SetLVString(error, errorMessage);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVGetServices(grpc_labview::LVProtoParser* parser, grpc_labview::LV1DArrayHandle* services)
{
    if (parser == nullptr)
    {
        return -1;
    }
    if (parser->m_FileDescriptor == nullptr)
    {
        return -2;
    }

    auto count = parser->m_FileDescriptor->service_count();
    if (NumericArrayResize(0x08, 1, services, count * sizeof(ServiceDescriptor*)) != 0)
    {
        parser->m_ErrorCollector.AddError("", 0, 0, "Failed to resize array");
        return -3;
    }
    (**services)->cnt = count;
    const ServiceDescriptor** serviceElements = (**services)->bytes<const ServiceDescriptor*>();
    for (int x=0; x<count; ++x)
    {
        serviceElements[x] = parser->m_FileDescriptor->service(x);
    }
    return 0;    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVGetMessages(grpc_labview::LVProtoParser* parser, grpc_labview::LV1DArrayHandle* messages)
{
    if (parser == nullptr)
    {
        return -1;
    }
    if (parser->m_FileDescriptor == nullptr)
    {
        return -2;
    }

    std::set<const google::protobuf::Descriptor*> allMessages;
    grpc_labview::AddDependentMessages(*parser->m_FileDescriptor, allMessages);

    auto count = allMessages.size();
    if (grpc_labview::NumericArrayResize(0x08, 1, messages, count * sizeof(Descriptor*)) != 0)
    {
        return -3;
    }
    (**messages)->cnt = count;
    const Descriptor** messageElements = (**messages)->bytes<const Descriptor*>();
    int x=0;
    for (auto& it: allMessages)
    {
        messageElements[x] = it;
        x += 1;
    }
    return 0;    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void AddTopLevelEnums(const google::protobuf::FileDescriptor& descriptor, std::set<const google::protobuf::EnumDescriptor*>& allEnums)
{
    // Get global enums defined in the proto file and other imported proto files.
    // FileDescriptor is being used to fetch these enum descriptors.
    auto noOfImportedFiles = descriptor.dependency_count();
    for (int x = 0; x < noOfImportedFiles; ++x)
    {
        AddTopLevelEnums(*descriptor.dependency(x), allEnums);
    }

    int topLevelEnumCount = descriptor.enum_type_count();
    for (int x = 0; x < topLevelEnumCount; ++x)
    {
        auto current = descriptor.enum_type(x);
        allEnums.emplace(current);
    }
}

void AddNestedEnums(grpc_labview::LV1DArrayHandle* messages, std::set<const google::protobuf::EnumDescriptor*>& allEnums)
{
    // Get enums nested within messages.
    // Descriptor is being used to fetch these enum descriptors.

    // Read messages sent from LV layer. These are the messages already collected
    // by the LV layer during the parsing of the proto file.
    std::set<const google::protobuf::Descriptor*> allMessages;
    const Descriptor** messageElements = (**messages)->bytes<const Descriptor*>();
    for (int i = 0; i < (**messages)->cnt; i++)
    {
        allMessages.emplace(messageElements[i]);
    }

    // Get enums nested within each message.
    for (auto& message : allMessages)
    {
        int enumCount = message->enum_type_count();
        for (int i = 0; i < enumCount; i++)
        {
            auto current = message->enum_type(i);
            allEnums.emplace(current);
        }
    }
}

LIBRARY_EXPORT int LVGetEnums(grpc_labview::LVProtoParser* parser, grpc_labview::LV1DArrayHandle* messages, grpc_labview::LV1DArrayHandle* enums)
{
    if (parser == nullptr)
    {
        return -1;
    }
    if (parser->m_FileDescriptor == nullptr)
    {
        return -2;
    }

    std::set<const google::protobuf::EnumDescriptor*> allEnums;
    AddTopLevelEnums(*(parser->m_FileDescriptor), allEnums);
    AddNestedEnums(messages, allEnums);    

    auto count = allEnums.size();
    if (grpc_labview::NumericArrayResize(0x08, 1, enums, count * sizeof(EnumDescriptor*)) != 0)
    {
        return -3;
    }
    (**enums)->cnt = count;
    const EnumDescriptor** enumElements = (**enums)->bytes<const EnumDescriptor*>();
    int x = 0;
    for (auto& it : allEnums)
    {
        enumElements[x] = it;
        x += 1;
    }
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVGetServiceName(ServiceDescriptor* service, grpc_labview::LStrHandle* name)
{
    if (service == nullptr)
    {
        return -1;
    }
    grpc_labview::SetLVString(name, service->name());
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVGetServiceMethods(ServiceDescriptor* service, grpc_labview::LV1DArrayHandle* methods)
{
    if (service == nullptr)
    {
        return -1;
    }
    auto count = service->method_count();
    auto size = sizeof(ServiceDescriptor*);
    if (grpc_labview::NumericArrayResize(0x08, 1, methods, count * size) != 0)
    {
        return -3;
    }
    (**methods)->cnt = count;
    auto methodElements = (**methods)->bytes<const MethodDescriptor*>();
    for (int x=0; x<count; ++x)
    {
        methodElements[x] = service->method(x);
    }
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVGetMethodName(MethodDescriptor* method, grpc_labview::LStrHandle* name)
{
    if (method == nullptr)
    {
        return -1;
    }
    SetLVString(name, method->name());
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVGetMethodFullName(MethodDescriptor* method, grpc_labview::LStrHandle* name)
{
    if (method == nullptr)
    {
        return -1;
    }
    grpc_labview::SetLVString(name, method->full_name());
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVIsMethodClientStreaming(MethodDescriptor* method, int* clientStreaming)
{
    if (method == nullptr)
    {
        return -1;
    }
    *clientStreaming = method->client_streaming();
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVIsMethodServerStreaming(MethodDescriptor* method, int* serverStreaming)
{
    if (method == nullptr)
    {
        return -1;
    }
    *serverStreaming = method->server_streaming();
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVGetMethodInput(MethodDescriptor* method, const Descriptor** inputType)
{
    if (method == nullptr)
    {
        return -1;
    }
    *inputType = method->input_type();
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVGetMethodOutput(MethodDescriptor* method, const Descriptor** outputType)
{
    if (method == nullptr)
    {
        return -1;
    }
    *outputType = method->output_type();
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVMessageName(Descriptor* descriptor, grpc_labview::LStrHandle* name)
{
    if (descriptor == nullptr)
    {
        return -1;
    }
    SetLVString(name, grpc_labview::TransformMessageName(descriptor->full_name()));
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVMessageTypeUrl(Descriptor* descriptor, grpc_labview::LStrHandle* name)
{
    if (descriptor == nullptr)
    {
        return -1;
    }
    SetLVString(name, descriptor->full_name());
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVMessageHasOneof(Descriptor* descriptor, int* hasOneof)
{
    if (descriptor == nullptr)
    {
        return -1;
    }
    *hasOneof = descriptor->real_oneof_decl_count();
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVEnumName(EnumDescriptor* descriptor, grpc_labview::LStrHandle* name)
{
    if (descriptor == nullptr)
    {
        return -1;
    }
    SetLVString(name, grpc_labview::TransformMessageName(descriptor->full_name()));
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVEnumTypeUrl(EnumDescriptor* descriptor, grpc_labview::LStrHandle* name)
{
    if (descriptor == nullptr)
    {
        return -1;
    }
    SetLVString(name, descriptor->full_name());
    return 0;
}

LIBRARY_EXPORT void SerializeReflectionInfo(grpc_labview::LVProtoParser *parser, grpc_labview::LStrHandle *outbuffer)
{
    FileDescriptorProto fProto;
    parser->m_FileDescriptor->CopyTo(&fProto);
    std::string output;
    fProto.SerializeToString(&output);
    grpc_labview::SetLVString(outbuffer, output);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVGetFields(Descriptor* descriptor, grpc_labview::LV1DArrayHandle* fields)
{
    if (descriptor == nullptr)
    {
        return -1;
    }
    auto count = descriptor->field_count();
    if (NumericArrayResize(0x08, 1, fields, count * sizeof(FieldDescriptor*)) != 0)
    {
        return -3;
    }
    (**fields)->cnt = count;
    auto fieldElements = (**fields)->bytes<const FieldDescriptor*>();
    for (int x=0; x<count; ++x)
    {
        fieldElements[x] = descriptor->field(x);
    }
    return 0;
}

std::string GetEnumNames(google::protobuf::EnumDescriptor* field);

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVFieldInfo(FieldDescriptor* field, grpc_labview::MessageFieldCluster* info)
{
    if (field == nullptr)
    {
        return -1;
    }
    int error = 0;
    switch (field->type())
    {
        case FieldDescriptor::TYPE_DOUBLE:
            info->type = 2;
            break;
        case FieldDescriptor::TYPE_FLOAT:
            info->type = 1;
            break;
        case FieldDescriptor::TYPE_INT64:
            info->type = 6;
            break;
        case FieldDescriptor::TYPE_UINT64:
            info->type = 8;
            break;
        case FieldDescriptor::TYPE_INT32:
            info->type = 0;
            break;
        case FieldDescriptor::TYPE_UINT32:
            info->type = 7;
            break;
        case FieldDescriptor::TYPE_ENUM:
            info->type = 9;
            break;
        case FieldDescriptor::TYPE_BOOL:
            info->type = 3;
            break;
        case FieldDescriptor::TYPE_STRING:
            info->type = 4;
            break;
        case FieldDescriptor::TYPE_BYTES:
            info->type = 10;
            break;
        case FieldDescriptor::TYPE_MESSAGE:
            info->type = 5;
            break;
        case FieldDescriptor::TYPE_FIXED64:
            info->type = 11;
            break;
        case FieldDescriptor::TYPE_FIXED32:
            info->type = 12;
            break;
        case FieldDescriptor::TYPE_SFIXED32:
            info->type = 14;
            break;
        case FieldDescriptor::TYPE_SFIXED64:
            info->type = 13;
            break;
        case FieldDescriptor::TYPE_SINT32:
            info->type = 16;
            break;
        case FieldDescriptor::TYPE_SINT64:
            info->type = 15;
            break;
    }
    if (field->type() == FieldDescriptor::TYPE_MESSAGE)
    {
        SetLVString(&info->embeddedMessage, grpc_labview::TransformMessageName(field->message_type()->full_name()));
    }
    if (field->type() == FieldDescriptor::TYPE_ENUM)
    {
        SetLVString(&info->embeddedMessage, grpc_labview::TransformMessageName(field->enum_type()->full_name()));
    }
    SetLVString(&info->fieldName, field->name());
    info->protobufIndex = field->number();
    info->isRepeated = field->is_repeated();
    info->isInOneof = (field->real_containing_oneof() != nullptr);
    if (info->isInOneof)
    {
        SetLVString(&info->oneofContainerName, grpc_labview::TransformMessageName(field->real_containing_oneof()->full_name()));
    }

    if (info->type == 99)
    {
        error = -2;
    }
    return error;
}

LIBRARY_EXPORT int GetEnumInfo(EnumDescriptor* enumDescriptor, grpc_labview::EnumFieldCluster* info)
{
    if (enumDescriptor == nullptr)
    {
        return -1;
    }

    SetLVString(&info->name, grpc_labview::TransformMessageName(enumDescriptor->full_name()));
    SetLVString(&info->typeUrl, enumDescriptor->full_name());
    SetLVString(&info->enumValues, GetEnumNames(enumDescriptor));
    info->isAliasAllowed = (enumDescriptor->options().has_allow_alias() && enumDescriptor->options().allow_alias());
    
    return 0;
}

std::string GetEnumNames(google::protobuf::EnumDescriptor* enumDescriptor)
{
    int enumValueCount = enumDescriptor->value_count();
    std::string enumNames = "";

    for (int i = 0; i < enumValueCount; i++)
    {
        std::string enumVal = enumDescriptor->value(i)->name() + "=" + std::to_string(enumDescriptor->value(i)->number());
        enumNames += enumVal + ((i < enumValueCount - 1) ? ";" : "");
    }
    
    return enumNames;
}
