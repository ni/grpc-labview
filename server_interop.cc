//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <query-server.h>
#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <mutex>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using namespace std;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
typedef int (*NumericArrayResize_T)(__int32, __int32, void* handle, size_t size);
typedef int (*PostLVUserEvent_T)(LVUserEventRef ref, void *data);

//---------------------------------------------------------------------
//---------------------------------------------------------------------
static NumericArrayResize_T NumericArrayResize;
static PostLVUserEvent_T PostLVUserEvent;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
std::map<string, LVUserEventRef> _registeredServerMethods;

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
InvokeData::InvokeData(ServerContext* _context, const queryserver::InvokeRequest* _request, queryserver::InvokeResponse* _response)
    : EventData(_context)
{
    request = _request;
    response = _response;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
QueryData::QueryData(ServerContext* _context, const ::queryserver::QueryRequest* _request, ::queryserver::QueryResponse* _response)
    : EventData(_context)
{    
    request = _request;
    response = _response;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
RegistrationRequestData::RegistrationRequestData(ServerContext* _context, const ::queryserver::RegistrationRequest* _request, ::grpc::ServerWriter<queryserver::ServerEvent>* _writer)
    : EventData(_context)
{
    request = _request;
    eventWriter = _writer;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
static void InitCallbacks()
{
	if (NumericArrayResize != NULL)
	{
		return;
	}

	auto lvModule = GetModuleHandle("LabVIEW.exe");
	if (lvModule == NULL)
	{
		lvModule = GetModuleHandle("lvffrt.dll");
	}
	if (lvModule == NULL)
	{
		lvModule = GetModuleHandle("lvrt.dll");
	}
	NumericArrayResize = (NumericArrayResize_T)GetProcAddress(lvModule, "NumericArrayResize");
	PostLVUserEvent = (PostLVUserEvent_T)GetProcAddress(lvModule, "PostLVUserEvent");
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void OccurServerEvent(const char* name, EventData* data)
{
	auto occurrence = _registeredServerMethods.find(name);
	if (occurrence != _registeredServerMethods.end())
	{
		auto error = PostLVUserEvent(occurrence->second, &data);
	}
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LStrHandle SetLVString(LStrHandle lvString, std::string str)
{
    auto length = str.length();    
	auto error = NumericArrayResize(0x01, 1, &lvString, length);
	memcpy((*lvString)->str, str.c_str(), length);
	(*lvString)->cnt = (int)length;
    return lvString;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
std::string GetLVString(LStrHandle lvString)
{    
    if (lvString == NULL || *lvString == NULL)
    {
        return std::string();
    }

	auto count = (*lvString)->cnt;
	auto chars = (*lvString)->str;

	std::string result(chars, count);
    return result;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT __int32 LVStartServer(char* address)
{   
	InitCallbacks();
	auto thread = new std::thread(RunServer, address);
	return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT __int32 LVStopServer()
{
	StopServer();
	return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT __int32 RegisterServerEvent(const char* name, LVUserEventRef* item)
{
	_registeredServerMethods.insert(pair<string,LVUserEventRef>(string(name), *item));
	return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT __int32 InvokeGetRequest(LVgRPCid id, LVInvokeRequest* request)
{
	InvokeData* data = *(InvokeData**)id;
    request->command = SetLVString(request->command, data->request->command());
    request->parameter = SetLVString(request->parameter, data->request->parameter());
	return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT __int32 InvokeSetResponse(LVgRPCid id, LVInvokeResponse* response)
{
	InvokeData* data = *(InvokeData**)id;
    data->response->set_status(response->status);
    data->NotifyComplete();
	return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT __int32 QueryGetRequest(LVgRPCid id, LVQueryRequest* request)
{
	QueryData* data = *(QueryData**)id;
    request->query = SetLVString(request->query, data->request->query());
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT __int32 QuerySetResponse(LVgRPCid id, LVQueryResponse* response)
{
	QueryData* data = *(QueryData**)id;
    data->response->set_message(GetLVString(response->message));
    data->response->set_status(response->status);
    data->NotifyComplete();
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT __int32 RegisterGetRequest(LVgRPCid id, LVRegistrationRequest* request)
{
	RegistrationRequestData* data = *(RegistrationRequestData**)id;
    request->eventName = SetLVString(request->eventName, data->request->eventname());
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT __int32 NotifyServerEvent(LVgRPCid id, LVServerEvent* event)
{
	RegistrationRequestData* data = *(RegistrationRequestData**)id;
    queryserver::ServerEvent e;
    e.set_eventdata(GetLVString(event->eventData));
    e.set_serverid(event->serverId);
    e.set_status(event->status);
    data->eventWriter->Write(e);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT __int32 CloseServerEvent(LVgRPCid id)
{
	RegistrationRequestData* data = *(RegistrationRequestData**)id;
    data->NotifyComplete();
    return 0;
}