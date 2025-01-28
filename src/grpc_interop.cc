//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>
#include <cluster_copier.h>
#include <lv_interop.h>
#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <mutex>
#include <thread>
#include <assert.h>
#include <feature_toggles.h>

namespace grpc_labview
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void OccurServerEvent(LVUserEventRef event, gRPCid* data)
    {
        auto error = PostUserEvent(event, &data);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void OccurServerEvent(LVUserEventRef event, gRPCid* data, std::string eventMethodName)
    {
        GeneralMethodEventData eventData;
        eventData.methodData = data;
        eventData.methodName = nullptr;

        SetLVString(&eventData.methodName, eventMethodName);

        auto error = PostUserEvent(event, &eventData);

        DSDisposeHandle(eventData.methodName);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------

    std::vector<std::string> SplitString(std::string s, std::string delimiter)
    {
        size_t pos_start = 0, pos_end, delim_len = delimiter.length();
        std::string token;
        std::vector<std::string> res;

        while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos)
        {
            token = s.substr(pos_start, pos_end - pos_start);
            pos_start = pos_end + delim_len;
            res.push_back(token);
        }

        res.push_back(s.substr(pos_start));
        return res;
    }

    std::map<uint32_t, int32_t> CreateMapBetweenLVEnumAndProtoEnumvalues(std::string enumValues)
    {
        std::map<uint32_t, int32_t> lvEnumToProtoEnum;
        int seqLVEnumIndex = 0;
        for (std::string keyValuePair : SplitString(enumValues, ";"))
        {
            auto keyValue = SplitString(keyValuePair, "=");
            assert(keyValue.size() == 2);

            int protoEnumNumeric = std::stoi(keyValue[1]);
            lvEnumToProtoEnum.insert(std::pair<uint32_t, int32_t>(seqLVEnumIndex, protoEnumNumeric));
            seqLVEnumIndex += 1;
        }
        return lvEnumToProtoEnum;
    }

    void MapInsertOrAssign(std::map<int32_t, std::list<uint32_t>>*protoEnumToLVEnum, int protoEnumNumeric, std::list<uint32_t> lvEnumNumericValues)
    {
        auto existingElement = protoEnumToLVEnum->find(protoEnumNumeric);
        if (existingElement != protoEnumToLVEnum->end())
        {
            protoEnumToLVEnum->erase(protoEnumNumeric);
            protoEnumToLVEnum->insert(std::pair<int32_t, std::list<uint32_t>>(protoEnumNumeric, lvEnumNumericValues));
        }
        else
            protoEnumToLVEnum->insert(std::pair<int32_t, std::list<uint32_t>>(protoEnumNumeric, lvEnumNumericValues));
    }

    std::map<int32_t, std::list<uint32_t>> CreateMapBetweenProtoEnumAndLVEnumvalues(std::string enumValues)
    {
        std::map<int32_t, std::list<uint32_t>> protoEnumToLVEnum;
        int seqLVEnumIndex = 0;
        for (std::string keyValuePair : SplitString(enumValues, ";"))
        {
            auto keyValue = SplitString(keyValuePair, "=");
            int protoEnumNumeric = std::stoi(keyValue[1]);
            assert(keyValue.size() == 2);

            std::list<uint32_t> lvEnumNumericValues;
            auto existingElement = protoEnumToLVEnum.find(protoEnumNumeric);
            if (existingElement != protoEnumToLVEnum.end())
                lvEnumNumericValues = existingElement->second;

            lvEnumNumericValues.push_back(seqLVEnumIndex); // Add the new element

            MapInsertOrAssign(&protoEnumToLVEnum, protoEnumNumeric, lvEnumNumericValues);

            seqLVEnumIndex += 1;
        }
        return protoEnumToLVEnum;
    }

    std::shared_ptr<EnumMetadata> CreateEnumMetadata2(IMessageElementMetadataOwner* metadataOwner, LVEnumMetadata2* lvMetadata)
    {
        std::shared_ptr<EnumMetadata> enumMetadata(new EnumMetadata());

        enumMetadata->messageName = GetLVString(lvMetadata->messageName);
        enumMetadata->typeUrl = GetLVString(lvMetadata->typeUrl);
        enumMetadata->elements = GetLVString(lvMetadata->elements);
        enumMetadata->allowAlias = lvMetadata->allowAlias;

        // Create the map between LV enum and proto enum values
        enumMetadata->LVEnumToProtoEnum = CreateMapBetweenLVEnumAndProtoEnumvalues(enumMetadata->elements);
        enumMetadata->ProtoEnumToLVEnum = CreateMapBetweenProtoEnumAndLVEnumvalues(enumMetadata->elements);

        return enumMetadata;
    }
}

int32_t ServerCleanupProc(grpc_labview::gRPCid* serverId);

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT void readIniFile(const char* filePath)
{
    grpc_labview::FeatureConfig::getInstance().readConfigFromFile(filePath);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t LVCreateServer(grpc_labview::gRPCid** id)
{
    grpc_labview::InitCallbacks();
    auto server = new grpc_labview::LabVIEWgRPCServer();
    grpc_labview::gPointerManager.RegisterPointer(server);
    *id = server;
    grpc_labview::RegisterCleanupProc(ServerCleanupProc, server);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t LVStartServer(char* address, char* serverCertificatePath, char* serverKeyPath, grpc_labview::gRPCid** id)
{
    auto server = (*id)->CastTo<grpc_labview::LabVIEWgRPCServer>();
    if (server == nullptr)
    {
        return -1;
    }
    return server->Run(address, serverCertificatePath, serverKeyPath);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t LVGetServerListeningPort(grpc_labview::gRPCid** id, int* listeningPort)
{
    auto server = (*id)->CastTo<grpc_labview::LabVIEWgRPCServer>();
    if (server == nullptr)
    {
        return -1;
    }
    *listeningPort = server->ListeningPort();
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t LVStopServer(grpc_labview::gRPCid** id)
{
    auto server = (*id)->CastTo<grpc_labview::LabVIEWgRPCServer>();
    if (server == nullptr)
    {
        return -1;
    }
    server->StopServer();

    grpc_labview::DeregisterCleanupProc(ServerCleanupProc, *id);
    grpc_labview::gPointerManager.UnregisterPointer(server.get());
    return 0;
}

int32_t ServerCleanupProc(grpc_labview::gRPCid* serverId)
{
    return LVStopServer(&serverId);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t RegisterMessageMetadata(grpc_labview::gRPCid** id, grpc_labview::LVMessageMetadata* lvMetadata)
{
    auto server = (*id)->CastTo<grpc_labview::MessageElementMetadataOwner>();
    if (server == nullptr)
    {
        return -1;
    }
    auto metadata = std::make_shared<grpc_labview::MessageMetadata>(server.get(), lvMetadata);
    server->RegisterMetadata(metadata);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t RegisterMessageMetadata2(grpc_labview::gRPCid** id, grpc_labview::LVMessageMetadata2* lvMetadata)
{
    auto server = (*id)->CastTo<grpc_labview::MessageElementMetadataOwner>();
    if (server == nullptr)
    {
        return -1;
    }
    auto metadata = std::make_shared<grpc_labview::MessageMetadata>(server.get(), lvMetadata);
    server->RegisterMetadata(metadata);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t RegisterEnumMetadata2(grpc_labview::gRPCid** id, grpc_labview::LVEnumMetadata2* lvMetadata)
{
    auto server = (*id)->CastTo<grpc_labview::MessageElementMetadataOwner>();
    if (server == nullptr)
    {
        return -1;
    }
    auto metadata = CreateEnumMetadata2(server.get(), lvMetadata);
    server->RegisterMetadata(metadata);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT uint32_t GetLVEnumValueFromProtoValue(grpc_labview::gRPCid** id, const char* enumName, int protoValue, uint32_t* lvEnumValue)
{
    auto server = (*id)->CastTo<grpc_labview::MessageElementMetadataOwner>();
    if (server == nullptr)
    {
        return -1;
    }
    auto metadata = (server.get())->FindEnumMetadata(std::string(enumName));
    *(uint32_t*)lvEnumValue = metadata.get()->GetLVEnumValueFromProtoValue(protoValue);

    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t GetProtoValueFromLVEnumValue(grpc_labview::gRPCid** id, const char* enumName, int lvEnumValue, int32_t* protoValue)
{
    auto server = (*id)->CastTo<grpc_labview::MessageElementMetadataOwner>();
    if (server == nullptr)
    {
        return -1;
    }
    auto metadata = (server.get())->FindEnumMetadata(std::string(enumName));
    *(int32_t*)protoValue = metadata.get()->GetProtoValueFromLVEnumValue(lvEnumValue);

    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CompleteMetadataRegistration(grpc_labview::gRPCid** id)
{
    auto server = (*id)->CastTo<grpc_labview::MessageElementMetadataOwner>();
    if (server == nullptr)
    {
        return -1;
    }
    server->FinalizeMetadata();
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t RegisterServerEvent(grpc_labview::gRPCid** id, const char* name, grpc_labview::LVUserEventRef* item, const char* requestMessageName, const char* responseMessageName)
{
    auto server = (*id)->CastTo<grpc_labview::LabVIEWgRPCServer>();
    if (server == nullptr)
    {
        return -1;
    }

    server->RegisterEvent(name, *item, requestMessageName, responseMessageName);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t RegisterGenericMethodServerEvent(grpc_labview::gRPCid** id, grpc_labview::LVUserEventRef* item)
{
    auto server = (*id)->CastTo<grpc_labview::LabVIEWgRPCServer>();
    if (server == nullptr)
    {
        return -1;
    }

    server->RegisterGenericMethodEvent(*item);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t GetRequestData(grpc_labview::gRPCid** id, int8_t* lvRequest)
{
    auto data = (*id)->CastTo<grpc_labview::GenericMethodData>();
    if (data == nullptr)
    {
        return -1;
    }
    if (data->_call->IsCancelled())
    {
        return -(1000 + grpc::StatusCode::CANCELLED);
    }
    if (data->_call->IsActive() && data->_call->ReadNext())
    {
        try
        {
            grpc_labview::ClusterDataCopier::CopyToCluster(*data->_request, lvRequest);
        }
        catch (grpc_labview::InvalidEnumValueException& e)
        {
            // Before returning, set the call to complete, otherwise the server hangs waiting for the call.
            data->_call->ReadComplete();
            return e.code;
        }
        data->_call->ReadComplete();
        return 0;
    }
    return -2;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t SetResponseData(grpc_labview::gRPCid** id, int8_t* lvRequest)
{
    auto data = (*id)->CastTo<grpc_labview::GenericMethodData>();
    if (data == nullptr)
    {
        return -1;
    }
    try
    {
        grpc_labview::ClusterDataCopier::CopyFromCluster(*data->_response, lvRequest);
    }
    catch (grpc_labview::InvalidEnumValueException& e)
    {
        return e.code;
    }
    if (data->_call->IsCancelled())
    {
        return -(1000 + grpc::StatusCode::CANCELLED);
    }
    if (!data->_call->IsActive() || !data->_call->Write())
    {
        return -2;
    }
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CloseServerEvent(grpc_labview::gRPCid** id)
{
    auto data = (*id)->CastTo<grpc_labview::GenericMethodData>();
    if (data == nullptr)
    {
        return -1;
    }
    data->NotifyComplete();
    data->_call->Finish();
    grpc_labview::gPointerManager.UnregisterPointer(*id);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t SetCallStatus(grpc_labview::gRPCid** id, int grpcErrorCode, const char* errorMessage)
{
    auto data = (*id)->CastTo<grpc_labview::GenericMethodData>();
    if (data == nullptr)
    {
        return -1;
    }
    data->_call->SetCallStatusError((grpc::StatusCode)grpcErrorCode, errorMessage);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t IsCancelled(grpc_labview::gRPCid** id)
{
    auto data = (*id)->CastTo<grpc_labview::GenericMethodData>();
    if (data == nullptr)
    {
        return -1;
    }
    return data->_call->IsCancelled();
}

//---------------------------------------------------------------------
// Allows for definition of the LVRT DLL path to be used for callback functions
// This function should be called prior to any other gRPC functions in this library
   //---------------------------------------------------------------------
LIBRARY_EXPORT int32_t SetLVRTModulePath(const char* modulePath)
{
    if (modulePath == nullptr)
    {
        return -1;
    }

    grpc_labview::SetLVRTModulePath(modulePath);

    return 0;
}