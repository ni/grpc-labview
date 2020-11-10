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
struct LVMessageField
{    
    int type;
    LStrHandle label;
    int tagNumber;
    bool isRepeated;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class ErrorCollector : public MultiFileErrorCollector
{
public:
    void MultiFileErrorCollector::AddError(const std::string & filename, int line, int column, const std::string & message) final
    {            
    }
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVProtoParser
{
public:
    LVProtoParser();
    void Import(const std::string& filePath);

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
void LVProtoParser::Import(const std::string& filePath)
{
    m_SourceTree.MapPath("", "C:\\dev");
    std::string path = filePath;
    std::replace(path.begin(), path.end(), '\\', '/');
    m_FileDescriptor = m_Importer.Import(path);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int LVImportProto(const char* filePath, LVProtoParser** parser)
{
    InitCallbacks();

    *parser = new LVProtoParser();
    (*parser)->Import(filePath);
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
    if (LVNumericArrayResize(0x01, 1, services, count * sizeof(ServiceDescriptor*)) != 0)
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
    if (LVNumericArrayResize(0x01, 1, methods, count * sizeof(ServiceDescriptor*)) != 0)
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
LIBRARY_EXPORT int LVGetFields(Descriptor* descriptor, LV1DArrayHandle* fields)
{
    if (descriptor == nullptr)
    {
        return -1;
    }
    auto count = descriptor->field_count();
    if (LVNumericArrayResize(0x01, 1, fields, count * sizeof(FieldDescriptor*)) != 0)
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
    info->type = field->type();
    SetLVString(&info->label, field->name());
    info->tagNumber = field->number();
    info->isRepeated = field->is_repeated();
    return 0;
}