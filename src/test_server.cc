//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <thread>
#include <sstream>
#include <fstream>
#include <iostream>
#include <src/core/lib/iomgr/executor.h>
#include <src/core/lib/iomgr/timer_manager.h>
#include <thread>
#include <grpcpp/grpcpp.h>
#include <data_marshal.grpc.pb.h>
#include <data_marshal.pb.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerWriter;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class DataMarshalTestServer final : public queryserver::QueryServer::Service
{
public:
    grpc::Status Invoke(grpc::ServerContext* context, const queryserver::InvokeRequest* request, queryserver::InvokeResponse* response) override;
    grpc::Status Query(grpc::ServerContext* context, const queryserver::QueryRequest* request, queryserver::QueryResponse* response) override;
    grpc::Status Register(grpc::ServerContext* context, const queryserver::RegistrationRequest* request, grpc::ServerWriter<queryserver::ServerEvent>* writer) override;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
grpc::Status DataMarshalTestServer::Invoke(grpc::ServerContext* context, const queryserver::InvokeRequest* request, queryserver::InvokeResponse* response)
{
    return Status::OK;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
grpc::Status DataMarshalTestServer::Query(grpc::ServerContext* context, const queryserver::QueryRequest* request, queryserver::QueryResponse* response)
{
    return Status::OK;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
grpc::Status DataMarshalTestServer::Register(grpc::ServerContext* context, const queryserver::RegistrationRequest* request, grpc::ServerWriter<queryserver::ServerEvent>* writer)
{
    queryserver::ServerEvent event;
    for (int x=0; x<10; ++x)
    {
        event.set_eventdata("Hello");
        event.set_status(x);
        writer->Write(event);
    }
    event.set_eventdata("Done");
    event.set_status(0);
    writer->WriteLast(event, ::grpc::WriteOptions());
    return Status::OK;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
std::string GetServerAddress(int argc, char** argv)
{
    std::string target_str = "0.0.0.0:50051";
    std::string arg_str("--address");
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
                std::cout << "The only correct argument syntax is --address=" << std::endl;
            }
        }
        else
        {
            std::cout << "The only acceptable argument is --address=" << std::endl;
        }
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
std::shared_ptr<grpc::ServerCredentials> CreateCredentials(int argc, char **argv)
{
	auto certPath = GetCertPath(argc, argv);

	std::shared_ptr<grpc::ServerCredentials> creds;
	if (!certPath.empty())
	{
		std::string servercert = read_keycert(certPath + ".crt");
		std::string serverkey = read_keycert(certPath + ".key");

		grpc::SslServerCredentialsOptions::PemKeyCertPair pkcp;
		pkcp.private_key = serverkey;
		pkcp.cert_chain = servercert;

		grpc::SslServerCredentialsOptions ssl_opts;
		ssl_opts.pem_root_certs="";
		ssl_opts.pem_key_cert_pairs.push_back(pkcp);

		creds = grpc::SslServerCredentials(ssl_opts);
	}
	else
	{
		creds = grpc::InsecureServerCredentials();
	}
	return creds;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void RunServer(int argc, char **argv, const char* server_address)
{
	auto creds = CreateCredentials(argc, argv);

	DataMarshalTestServer service;
	grpc::ServerBuilder builder;
	builder.AddListeningPort(server_address, creds);
	builder.RegisterService(&service);
	auto server = builder.BuildAndStart();
	std::cout << "Server listening on " << server_address << std::endl;
	server->Wait();
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int main(int argc, char **argv)
{
    RunServer(argc, argv, "0.0.0.0:50051");
	return 0;
}
