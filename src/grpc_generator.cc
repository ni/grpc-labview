//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <google/protobuf/compiler/importer.h>
#include <lv_interop.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using namespace google::protobuf::compiler;
using namespace google::protobuf;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#ifdef _PS_4
#pragma pack (push, 1)
#endif
struct LVMessageField
{    
    LStrHandle fieldName;
    LStrHandle embeddedMessage;
    int32_t protobufIndex;
    int32_t type;
    char isRepeated;
};
#ifdef _PS_4
#pragma pack (pop)
#endif

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class ErrorCollector : public MultiFileErrorCollector
{
public:
    void AddError(const std::string & filename, int line, int column, const std::string & message)
    {            
    }
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVProtoParser
{
public:
    LVProtoParser();
    void Import(const std::string& filePath, const std::string& searchPath);

public:
    DiskSourceTree m_SourceTree;
    ErrorCollector m_ErrorCollector;
    Importer m_Importer;

    const FileDescriptor* m_FileDescriptor;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVProtoParser::LVProtoParser()
    : m_Importer(&m_SourceTree, &m_ErrorCollector)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LVProtoParser::Import(const std::string& filePath, const std::string& search)
{
    std::string searchPath = search;
    std::replace(searchPath.begin(), searchPath.end(), '\\', '/');
    m_SourceTree.MapPath("", searchPath);
    
    std::string path = filePath;
    std::replace(path.begin(), path.end(), '\\', '/');
    
    m_FileDescriptor = m_Importer.Import(path);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVImportProto(const char* filePath, const char* searchPath, LVProtoParser** parser)
{
    InitCallbacks();

    *parser = new LVProtoParser();
    (*parser)->Import(filePath, searchPath);
    return 0;    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVGetServices(LVProtoParser* parser, LV1DArrayHandle* services)
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
    if (LVNumericArrayResize(0x08, 1, services, count * sizeof(ServiceDescriptor*)) != 0)
    {
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
void AddDependentMessages(const google::protobuf::FileDescriptor& descriptor, std::set<const google::protobuf::Descriptor*>& messages)
{
    auto count = descriptor.message_type_count();
    for (int x=0; x<count; ++x)
    {
        messages.emplace(descriptor.message_type(x));
    }
    auto imported = descriptor.dependency_count();
    for (int x=0; x<imported; ++x)
    {
        AddDependentMessages(*descriptor.dependency(x), messages);
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVGetMessages(LVProtoParser* parser, LV1DArrayHandle* messages)
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
    AddDependentMessages(*parser->m_FileDescriptor, allMessages);

    auto count = allMessages.size();
    if (LVNumericArrayResize(0x08, 1, messages, count * sizeof(Descriptor*)) != 0)
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
LIBRARY_EXPORT int LVGetServiceName(ServiceDescriptor* service, LStrHandle* name)
{
    if (service == nullptr)
    {
        return -1;
    }
    SetLVString(name, service->name());
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVGetServiceMethods(ServiceDescriptor* service, LV1DArrayHandle* methods)
{
    if (service == nullptr)
    {
        return -1;
    }
    auto count = service->method_count();
    auto size = sizeof(ServiceDescriptor*);
    if (LVNumericArrayResize(0x08, 1, methods, count * size) != 0)
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
LIBRARY_EXPORT int LVGetMethodName(MethodDescriptor* method, LStrHandle* name)
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
LIBRARY_EXPORT int LVGetMethodFullName(MethodDescriptor* method, LStrHandle* name)
{
    if (method == nullptr)
    {
        return -1;
    }
    SetLVString(name, method->full_name());
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
LIBRARY_EXPORT int LVMessageName(Descriptor* descriptor, LStrHandle* name)
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
LIBRARY_EXPORT int LVGetFields(Descriptor* descriptor, LV1DArrayHandle* fields)
{
    if (descriptor == nullptr)
    {
        return -1;
    }
    auto count = descriptor->field_count();
    if (LVNumericArrayResize(0x08, 1, fields, count * sizeof(FieldDescriptor*)) != 0)
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

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVFieldInfo(FieldDescriptor* field, LVMessageField* info)
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
            info->type = 99;
            break;
        case FieldDescriptor::TYPE_UINT64:
            info->type = 99;
            break;
        case FieldDescriptor::TYPE_INT32:
            info->type = 0;
            break;
        case FieldDescriptor::TYPE_FIXED64:
            info->type = 99;
            break;
        case FieldDescriptor::TYPE_FIXED32:
            info->type = 99;
            break;
        case FieldDescriptor::TYPE_BOOL:
            info->type = 3;
            break;
        case FieldDescriptor::TYPE_STRING:
            info->type = 4;
            break;
        case FieldDescriptor::TYPE_MESSAGE:
            info->type = 5;
            break;
        case FieldDescriptor::TYPE_BYTES:
            info->type = 99;
            break;
        case FieldDescriptor::TYPE_UINT32:
            info->type = 99;
            break;
        case FieldDescriptor::TYPE_ENUM:
            info->type = 99;
            break;
        case FieldDescriptor::TYPE_SFIXED32:
            info->type = 99;
            break;
        case FieldDescriptor::TYPE_SFIXED64:
            info->type = 99;
            break;
        case FieldDescriptor::TYPE_SINT32:
            info->type = 99;
            break;
        case FieldDescriptor::TYPE_SINT64:
            info->type = 99;
            break;
    }
    if (field->type() == FieldDescriptor::TYPE_MESSAGE)
    {
        SetLVString(&info->embeddedMessage, field->message_type()->full_name());
    }
    SetLVString(&info->fieldName, field->name());
    info->protobufIndex = field->number();
    info->isRepeated = field->is_repeated();
    if (info->type == 99)
    {
        error = -2;
    }
    return error;
}