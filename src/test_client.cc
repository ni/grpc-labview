//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <data_marshal.grpc.pb.h>
#include <sstream>
#include <fstream>
#include <iostream>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using namespace queryserver;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class QueryClient
{
public:
    QueryClient(std::shared_ptr<Channel> channel);

public:
    void Invoke(const std::string& command, const std::string& parameters);
    std::string Query(const std::string &command);
    std::unique_ptr<grpc::ClientReader<ServerEvent>> Register(const std::string& eventName);

public:
    ClientContext m_context;
    std::unique_ptr<QueryServer::Stub> m_Stub;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
QueryClient::QueryClient(std::shared_ptr<Channel> channel)
    : m_Stub(QueryServer::NewStub(channel))
{        
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void QueryClient::Invoke(const std::string& command, const std::string& parameters)
{
    InvokeRequest request;
    request.set_command(command);
    request.set_parameter(parameters);

    ClientContext context;
    InvokeResponse reply;
    Status status = m_Stub->Invoke(&context, request, &reply);
    if (!status.ok())
    {
        std::cout << status.error_code() << ": " << status.error_message() << std::endl;
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
std::string QueryClient::Query(const std::string &command)
{
    QueryRequest request;
    request.set_query(command);
    QueryResponse reply;
    ClientContext context;

    Status status = m_Stub->Query(&context, request, &reply);

    if (status.ok())
    {
        return reply.message();
    }
    else
    {
        std::cout << status.error_code() << ": " << status.error_message() << std::endl;
        return "RPC failed";
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
std::unique_ptr<grpc::ClientReader<ServerEvent>> QueryClient::Register(const std::string& eventName)
{
    RegistrationRequest request;
    request.set_eventname(eventName);

    return m_Stub->Register(&m_context, request);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
std::string GetServerAddress(int argc, char** argv)
{
    std::string target_str;
    std::string arg_str("--target");
    if (argc > 1)
    {
        std::string arg_val = argv[1];
        size_t start_pos = arg_val.find(arg_str);
        if (start_pos != std::string::npos)
        {
            start_pos += arg_str.size();
            if (arg_val[start_pos] == '=')
            {
                target_str = arg_val.substr(start_pos + 1);
            }
            else
            {
                std::cout << "The only correct argument syntax is --target=" << std::endl;
                return 0;
            }
        }
        else
        {
            std::cout << "The only acceptable argument is --target=" << std::endl;
            return 0;
        }
    }
    else
    {
        target_str = "localhost:50051";
    }
    return target_str;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
std::string GetCertPath(int argc, char** argv)
{
    std::string cert_str;
    std::string arg_str("--cert");
    if (argc > 2)
    {
        std::string arg_val = argv[2];
        size_t start_pos = arg_val.find(arg_str);
        if (start_pos != std::string::npos)
        {
            start_pos += arg_str.size();
            if (arg_val[start_pos] == '=')
            {
                cert_str = arg_val.substr(start_pos + 1);
            }
            else
            {
                std::cout << "The only correct argument syntax is --cert=" << std::endl;
                return 0;
            }
        }
        else
        {
            std::cout << "The only acceptable argument is --cert=" << std::endl;
            return 0;
        }
    }
    return cert_str;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
std::string read_keycert( const std::string& filename)
{	
    std::string data;
    std::ifstream file(filename.c_str(), std::ios::in);
    if (file.is_open())
    {
        std::stringstream ss;
        ss << file.rdbuf();
        file.close();
        data = ss.str();
    }
    return data;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void DoDataTypeTest(QueryClient& client)
{    
    ClientContext ctx;
    TestDataTypesParameters request;
    request.set_root("Root String");
    request.mutable_repeatedstring()->Add("String 1");
    request.mutable_repeatedstring()->Add("String 2");
    request.mutable_repeatedstring()->Add("String 3");

    request.mutable_repeatedintvalue()->Add(1);
    request.mutable_repeatedintvalue()->Add(2);
    request.mutable_repeatedintvalue()->Add(3);
    request.mutable_repeatedintvalue()->Add(4);
    request.mutable_repeatedintvalue()->Add(5);
    request.mutable_repeatedintvalue()->Add(6);

    request.set_rootint32(42);
    request.set_rootdouble(42.42);
    request.set_rootbool(true);
    request.set_rootfloat(37.56f);

    request.mutable_repeateddoublevalue()->Add(1.1f);
    request.mutable_repeateddoublevalue()->Add(2.2f);
    request.mutable_repeateddoublevalue()->Add(3.3f);
    request.mutable_repeateddoublevalue()->Add(4.4f);
    request.mutable_repeateddoublevalue()->Add(5.5f);
    request.mutable_repeateddoublevalue()->Add(6.6f);

    request.mutable_repeatedboolvalue()->Add(true);
    request.mutable_repeatedboolvalue()->Add(true);
    request.mutable_repeatedboolvalue()->Add(false);
    request.mutable_repeatedboolvalue()->Add(true);
    request.mutable_repeatedboolvalue()->Add(true);
    request.mutable_repeatedboolvalue()->Add(false);

    request.mutable_repeatedfloatvalue()->Add(1.23f);
    request.mutable_repeatedfloatvalue()->Add(2.23f);
    request.mutable_repeatedfloatvalue()->Add(3.23f);
    request.mutable_repeatedfloatvalue()->Add(4.23f);
    request.mutable_repeatedfloatvalue()->Add(5.23f);
    request.mutable_repeatedfloatvalue()->Add(6.23f);

    request.mutable_nested()->set_boolvalaue(true);
    request.mutable_nested()->set_doublevalue(12.12);
    request.mutable_nested()->set_intvalue(4242);
    request.mutable_nested()->set_floatvalue(15.14f);

    QueryResponse anyResponse;
    anyResponse.set_message("This is a message");
    anyResponse.set_status(42);

    auto any = new google::protobuf::Any();
    any->PackFrom(anyResponse);
    request.set_allocated_generic_value(any);

    auto nested = request.mutable_repeatednested()->Add();
    nested->set_boolvalaue(true);
    nested->set_floatvalue(1.1f);
    nested = request.mutable_repeatednested()->Add();
    nested->set_boolvalaue(false);
    nested->set_floatvalue(2.2f);
    nested = request.mutable_repeatednested()->Add();
    nested->set_boolvalaue(true);
    nested->set_floatvalue(3.3f);

    TestDataTypesParameters response;
    auto result = client.m_Stub->TestDataTypes(&ctx, request, &response);

    std::cout << response.root() << std::endl;
    for (auto i: response.repeatedstring())
    {
        std::cout << i << std::endl;
    }
    for (int x = 0; x < response.mutable_repeatedintvalue()->size(); ++x)
    {
        std::cout << response.mutable_repeatedintvalue()->data()[x] << std::endl;
    }

    std::cout << response.rootint32() << std::endl;
    std::cout << response.rootdouble() << std::endl;
    std::cout << response.rootbool() << std::endl;
    std::cout << response.rootfloat() << std::endl;

    QueryResponse genericResponse;
    bool unpackedResult = response.generic_value().UnpackTo(&genericResponse);

    std::cout << "Unpacked Result: " << unpackedResult << "Unpacked Any: " << genericResponse.message() << std::endl;


    for (int x = 0; x < response.mutable_repeateddoublevalue()->size(); ++x)
    {
        std::cout << response.mutable_repeateddoublevalue()->data()[x] << std::endl;
    }

    for (int x = 0; x < response.mutable_repeatedfloatvalue()->size(); ++x)
    {
        std::cout << response.mutable_repeatedfloatvalue()->data()[x] << std::endl;
    }

    for (int x = 0; x < response.mutable_repeatedboolvalue()->size(); ++x)
    {
        std::cout << response.mutable_repeatedboolvalue()->data()[x] << std::endl;
    }

    std::cout << response.nested().boolvalaue() << std::endl;
    std::cout << response.nested().doublevalue() << std::endl;
    std::cout << response.nested().intvalue() << std::endl;
    std::cout << response.nested().floatvalue() << std::endl;

    for (int x = 0; x < response.repeatednested().size(); ++x)
    {
        std::cout << response.repeatednested()[x].boolvalaue() << ", " << response.repeatednested()[x].floatvalue() << std::endl;
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int main(int argc, char **argv)
{
    auto target_str = GetServerAddress(argc, argv);
    auto certificatePath = GetCertPath(argc, argv);

    std::shared_ptr<grpc::ChannelCredentials> creds;
    if (!certificatePath.empty())
    {
        std::string cacert = read_keycert(certificatePath);
        grpc::SslCredentialsOptions ssl_opts;
        ssl_opts.pem_root_certs=cacert;
        creds = grpc::SslCredentials(ssl_opts);
    }
    else
    {
        creds = grpc::InsecureChannelCredentials();
    }
    auto channel = grpc::CreateChannel(target_str, creds);
    QueryClient client(channel);

    DoDataTypeTest(client);

    auto result = client.Query("Uptime");
    std::cout << "Server uptime: " << result << std::endl;

    auto reader = client.Register("Heartbeat");
    int count = 0;
    ServerEvent event;
    while (reader->Read(&event))
    {
        std::cout << "Server Event: " << event.eventdata() << std::endl;
        count += 1;
        if (count == 10)
        {
            client.Invoke("Reset", "");
        }
    }
    Status status = reader->Finish();
    std::cout << "Server notifications complete" << std::endl;
}
