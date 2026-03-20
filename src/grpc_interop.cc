//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_interop.h>
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

        SetLVAsciiString(&eventData.methodName, eventMethodName);

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

        enumMetadata->messageName = GetLVAsciiString(lvMetadata->messageName);
        enumMetadata->typeUrl = GetLVAsciiString(lvMetadata->typeUrl);
        enumMetadata->elements = GetLVAsciiString(lvMetadata->elements);
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
LIBRARY_EXPORT int32_t IsFeatureEnabled(const char* featureName, uint8_t* featureEnabled)
{
    try {
        *featureEnabled = grpc_labview::FeatureConfig::getInstance().IsFeatureEnabled(featureName);
        return 0;
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t LVCreateServer(grpc_labview::gRPCid** id)
{
    try {
        grpc_labview::InitCallbacks();
        auto server = std::make_shared<grpc_labview::LabVIEWgRPCServer>();
        grpc_labview::gPointerManager.RegisterPointer(server);
        *id = server.get();
        grpc_labview::RegisterCleanupProc(ServerCleanupProc, *id);
        return 0;
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t LVStartServer(char* address, char* serverCertificatePath, char* serverKeyPath, grpc_labview::gRPCid** id)
{
    try {
        auto server = (*id)->CastTo<grpc_labview::LabVIEWgRPCServer>();
        if (server == nullptr)
        {
            return -1;
        }
        return server->Run(address, serverCertificatePath, serverKeyPath);
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t LVGetServerListeningPort(grpc_labview::gRPCid** id, int* listeningPort)
{
    try {
        auto server = (*id)->CastTo<grpc_labview::LabVIEWgRPCServer>();
        if (server == nullptr)
        {
            return -1;
        }
        *listeningPort = server->ListeningPort();
        return 0;
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t LVStopServer(grpc_labview::gRPCid** id)
{
    try {
        auto server = (*id)->CastTo<grpc_labview::LabVIEWgRPCServer>();
        if (server == nullptr)
        {
            return -1;
        }
        server->StopServer();

        grpc_labview::DeregisterCleanupProc(ServerCleanupProc, *id);
        grpc_labview::gPointerManager.UnregisterPointer(server.get());
        return 0;
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

int32_t ServerCleanupProc(grpc_labview::gRPCid* serverId)
{
    return LVStopServer(&serverId);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t RegisterMessageMetadata(grpc_labview::gRPCid** id, grpc_labview::LVMessageMetadata* lvMetadata)
{
    try {
        auto server = (*id)->CastTo<grpc_labview::MessageElementMetadataOwner>();
        if (server == nullptr)
        {
            return -1;
        }
        auto metadata = std::make_shared<grpc_labview::MessageMetadata>(server.get(), lvMetadata);
        server->RegisterMetadata(metadata);
        return 0;
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t RegisterMessageMetadata2(grpc_labview::gRPCid** id, grpc_labview::LVMessageMetadata2* lvMetadata)
{
    try {
        auto server = (*id)->CastTo<grpc_labview::MessageElementMetadataOwner>();
        if (server == nullptr)
        {
            return -1;
        }
        auto metadata = std::make_shared<grpc_labview::MessageMetadata>(server.get(), lvMetadata);
        server->RegisterMetadata(metadata);
        return 0;
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t RegisterEnumMetadata2(grpc_labview::gRPCid** id, grpc_labview::LVEnumMetadata2* lvMetadata)
{
    try {
        auto server = (*id)->CastTo<grpc_labview::MessageElementMetadataOwner>();
        if (server == nullptr)
        {
            return -1;
        }
        auto metadata = CreateEnumMetadata2(server.get(), lvMetadata);
        server->RegisterMetadata(metadata);
        return 0;
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT uint32_t GetLVEnumValueFromProtoValue(grpc_labview::gRPCid** id, const char* enumName, int protoValue, uint32_t* lvEnumValue)
{
    try {
        auto server = (*id)->CastTo<grpc_labview::MessageElementMetadataOwner>();
        if (server == nullptr)
        {
            return -1;
        }
        auto metadata = (server.get())->FindEnumMetadata(std::string(enumName));
        *(uint32_t*)lvEnumValue = metadata.get()->GetLVEnumValueFromProtoValue(protoValue);

        return 0;
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t GetProtoValueFromLVEnumValue(grpc_labview::gRPCid** id, const char* enumName, int lvEnumValue, int32_t* protoValue)
{
    try {
        auto server = (*id)->CastTo<grpc_labview::MessageElementMetadataOwner>();
        if (server == nullptr)
        {
            return -1;
        }
        auto metadata = (server.get())->FindEnumMetadata(std::string(enumName));
        *(int32_t*)protoValue = metadata.get()->GetProtoValueFromLVEnumValue(lvEnumValue);

        return 0;
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CompleteMetadataRegistration(grpc_labview::gRPCid** id)
{
    try {
        auto server = (*id)->CastTo<grpc_labview::MessageElementMetadataOwner>();
        if (server == nullptr)
        {
            return -1;
        }
        server->FinalizeMetadata();
        return 0;
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t RegisterServerEvent(grpc_labview::gRPCid** id, const char* name, grpc_labview::LVUserEventRef* item, const char* requestMessageName, const char* responseMessageName)
{
    try {
        auto server = (*id)->CastTo<grpc_labview::LabVIEWgRPCServer>();
        if (server == nullptr)
        {
            return -1;
        }

        server->RegisterEvent(name, *item, requestMessageName, responseMessageName);
        return 0;
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t RegisterGenericMethodServerEvent(grpc_labview::gRPCid** id, grpc_labview::LVUserEventRef* item)
{
    try {
        auto server = (*id)->CastTo<grpc_labview::LabVIEWgRPCServer>();
        if (server == nullptr)
        {
            return -1;
        }

        server->RegisterGenericMethodEvent(*item);
        return 0;
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
static int32_t GetErrorCodeForFailedReadWrite(const std::shared_ptr<grpc_labview::CallData>& callData)
{
    auto statusCode = callData->GetCallStatusCode();
    if (statusCode != grpc::StatusCode::OK)
    {
        return -(1000 + statusCode);
    }
    if (callData->IsCancelled())
    {
        return -(1000 + grpc::StatusCode::CANCELLED);
    }
    return -2;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t GetRequestData(grpc_labview::gRPCid** id, int8_t* cluster)
{
    try {
        auto callData = (*id)->CastTo<grpc_labview::CallData>();
        if (callData == nullptr)
        {
            return -1;
        }

        if (callData->ReadNext(cluster))
        {
            return 0;
        }
        return GetErrorCodeForFailedReadWrite(callData);
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t SetResponseData(grpc_labview::gRPCid** id, int8_t* cluster)
{
    try {
        auto callData = (*id)->CastTo<grpc_labview::CallData>();
        if (callData == nullptr)
        {
            return -1;
        }

        if (callData->Write(cluster))
        {
            return 0;
        }
        return GetErrorCodeForFailedReadWrite(callData);
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CloseServerEvent(grpc_labview::gRPCid** id)
{
    try {
        auto callData = (*id)->CastTo<grpc_labview::CallData>();
        if (callData == nullptr)
        {
            return -1;
        }

        callData->FinishFromLabVIEW();
        grpc_labview::DeregisterCleanupProc(grpc_labview::CloseServerEventCleanupProc, *id);
        grpc_labview::gPointerManager.UnregisterPointer(*id);
        return 0;
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

namespace grpc_labview
{
    int32_t CloseServerEventCleanupProc(grpc_labview::gRPCid* id)
    {
        return ::CloseServerEvent(&id);
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t SetCallStatus(grpc_labview::gRPCid** id, int grpcErrorCode, const char* errorMessage)
{
    try {
        auto callData = (*id)->CastTo<grpc_labview::CallData>();
        if (callData == nullptr)
        {
            return -1;
        }
        callData->SetCallStatusError((grpc::StatusCode)grpcErrorCode, errorMessage);
        return 0;
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t IsCancelled(grpc_labview::gRPCid** id)
{
    try {
        auto callData = (*id)->CastTo<grpc_labview::CallData>();
        if (callData == nullptr)
        {
            return -1;
        }
        return callData->IsCancelled();
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
// Allows for definition of the LVRT DLL path to be used for callback functions
// This function should be called prior to any other gRPC functions in this library
   //---------------------------------------------------------------------
LIBRARY_EXPORT int32_t SetLVRTModulePath(const char* modulePath)
{
    try {
        if (modulePath == nullptr)
        {
            return -1;
        }

        grpc_labview::SetLVRTModulePath(modulePath);

        return 0;
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}
