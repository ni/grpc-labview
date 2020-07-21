//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <query_server.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using namespace std;
using namespace queryserver;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
EventData::EventData(ServerContext* _context)
{
    context = _context;    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void EventData::WaitForComplete()
{    
    std::unique_lock<std::mutex> lck(lockMutex);
    lock.wait(lck);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void EventData::NotifyComplete()
{    
	std::unique_lock<std::mutex> lck(lockMutex);
	lock.notify_all();
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
InvokeData::InvokeData(ServerContext* _context, const InvokeRequest* _request, InvokeResponse* _response)
    : EventData(_context)
{
    request = _request;
    response = _response;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
QueryData::QueryData(ServerContext* _context, const QueryRequest* _request, QueryResponse* _response)
    : EventData(_context)
{    
    request = _request;
    response = _response;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
RegistrationRequestData::RegistrationRequestData(ServerContext* _context, const RegistrationRequest* _request, ::grpc::ServerWriter<ServerEvent>* _writer)
    : EventData(_context)
{
    request = _request;
    eventWriter = _writer;
}

