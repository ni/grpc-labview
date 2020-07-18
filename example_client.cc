//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <query-server.grpc.pb.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class QueryClient
{
public:
    QueryClient(std::shared_ptr<Channel> channel)
        : stub_(queryserver::QueryServer::NewStub(channel))
    {        
    }

    void Invoke(const std::string& command, const std::string& parameters)
    {
        queryserver::InvokeRequest request;
        request.set_command(command);
        request.set_parameter(parameters);

        ClientContext context;
        queryserver::InvokeResponse reply;
        Status status = stub_->Invoke(&context, request, &reply);
    }

    std::string Query(const std::string &command)
    {
        // Data we are sending to the server.
        queryserver::QueryRequest request;
        request.set_query(command);

        // Container for the data we expect from the server.
        queryserver::QueryResponse reply;

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        ClientContext context;

        // The actual RPC.
        Status status = stub_->Query(&context, request, &reply);

        // Act upon its status.
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

    std::unique_ptr<grpc::ClientReader<queryserver::ServerEvent>> Register(const std::string& eventName)
    {
        queryserver::RegistrationRequest request;
        request.set_eventname(eventName);

        return stub_->Register(&_context, request);
    }

private:
    ClientContext _context;
    std::unique_ptr<queryserver::QueryServer::Stub> stub_;
};

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
int main(int argc, char **argv)
{
    auto target_str = GetServerAddress(argc, argv);
    QueryClient client(grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));

    auto result = client.Query("Uptime");
    std::cout << "Server uptime: " << result << "\n";

    auto reader = client.Register("Heartbeat");
    int count = 0;
    queryserver::ServerEvent event;
    while (reader->Read(&event))
    {
        std::cout << "Server Event: " << event.eventdata() << "\n";
        count += 1;
        if (count == 10)
        {
            client.Invoke("Reset", "");
        }
    }
    Status status = reader->Finish();
    std::cout << "Server notifications complete\n";
}
