<?xml version='1.0' encoding='UTF-8'?>
<Project Type="Project" LVVersion="19008000">
	<Property Name="NI.LV.All.SourceOnly" Type="Bool">true</Property>
	<Item Name="My Computer" Type="My Computer">
		<Property Name="server.app.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="server.control.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="server.tcp.enabled" Type="Bool">false</Property>
		<Property Name="server.tcp.port" Type="Int">0</Property>
		<Property Name="server.tcp.serviceName" Type="Str">My Computer/VI Server</Property>
		<Property Name="server.tcp.serviceName.default" Type="Str">My Computer/VI Server</Property>
		<Property Name="server.vi.callsEnabled" Type="Bool">true</Property>
		<Property Name="server.vi.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="specify.custom.address" Type="Bool">false</Property>
		<Item Name="Client" Type="Folder">
			<Item Name="TestServerStreaming.vi" Type="VI" URL="../Client/TestServerStreaming.vi"/>
		</Item>
		<Item Name="Protobuf" Type="Folder">
			<Item Name="SimpleAnyBuilder.vi" Type="VI" URL="../Protobuf/SimpleAnyBuilder.vi"/>
			<Item Name="SimpleAnyPackUnpack.vi" Type="VI" URL="../Protobuf/SimpleAnyPackUnpack.vi"/>
			<Item Name="SimpleBuilder.vi" Type="VI" URL="../Protobuf/SimpleBuilder.vi"/>
			<Item Name="SimplePackUnpack.vi" Type="VI" URL="../Protobuf/SimplePackUnpack.vi"/>
			<Item Name="TestParseWithUnknownFields.vi" Type="VI" URL="../Protobuf/TestParseWithUnknownFields.vi"/>
			<Item Name="TestUnpackFields.vi" Type="VI" URL="../Protobuf/TestUnpackFields.vi"/>
		</Item>
		<Item Name="Protos" Type="Folder">
			<Item Name="data_marshal.proto" Type="Document" URL="../Protos/data_marshal.proto"/>
		</Item>
		<Item Name="TestDataMarshal" Type="Folder">
			<Item Name="TestDataMarshal.lvlib" Type="Library" URL="../Servers/TestDataMarshal/TestDataMarshal.lvlib"/>
		</Item>
		<Item Name="Dependencies" Type="Dependencies">
			<Item Name="vi.lib" Type="Folder">
				<Item Name="Any.ctl" Type="VI" URL="/&lt;vilib&gt;/gRPC/LabVIEW gRPC Library/Server API/typeDefs/Any.ctl"/>
				<Item Name="AnyBuilderAddValue.vim" Type="VI" URL="/&lt;vilib&gt;/gRPC/LabVIEW gRPC Library/Server API/Message Requests/AnyBuilderAddValue.vim"/>
				<Item Name="AnyBuilderBegin.vi" Type="VI" URL="/&lt;vilib&gt;/gRPC/LabVIEW gRPC Library/Server API/Message Requests/AnyBuilderBegin.vi"/>
				<Item Name="AnyBuilderBuild.vi" Type="VI" URL="/&lt;vilib&gt;/gRPC/LabVIEW gRPC Library/Server API/Message Requests/AnyBuilderBuild.vi"/>
				<Item Name="AnyBuilderBuildToBuffer.vi" Type="VI" URL="/&lt;vilib&gt;/gRPC/LabVIEW gRPC Library/Server API/Message Requests/AnyBuilderBuildToBuffer.vi"/>
				<Item Name="Application Directory.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/file.llb/Application Directory.vi"/>
				<Item Name="CompleteMetadataRegistration.vi" Type="VI" URL="/&lt;vilib&gt;/gRPC/LabVIEW gRPC Library/Server API/CompleteMetadataRegistration.vi"/>
				<Item Name="CreateSerializationSession.vi" Type="VI" URL="/&lt;vilib&gt;/gRPC/LabVIEW gRPC Library/Server API/CreateSerializationSession.vi"/>
				<Item Name="Error Cluster From Error Code.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Error Cluster From Error Code.vi"/>
				<Item Name="FreeSerializationSession.vi" Type="VI" URL="/&lt;vilib&gt;/gRPC/LabVIEW gRPC Library/Server API/FreeSerializationSession.vi"/>
				<Item Name="Get Server DLL Path.vi" Type="VI" URL="/&lt;vilib&gt;/gRPC/LabVIEW gRPC Library/Server API/Server/Get Server DLL Path.vi"/>
				<Item Name="grpcId.ctl" Type="VI" URL="/&lt;vilib&gt;/gRPC/LabVIEW gRPC Library/Server API/typeDefs/grpcId.ctl"/>
				<Item Name="Message Element Metadata.ctl" Type="VI" URL="/&lt;vilib&gt;/gRPC/LabVIEW gRPC Library/Server API/typeDefs/Message Element Metadata.ctl"/>
				<Item Name="Message Element Type.ctl" Type="VI" URL="/&lt;vilib&gt;/gRPC/LabVIEW gRPC Library/Server API/typeDefs/Message Element Type.ctl"/>
				<Item Name="Message Metadata.ctl" Type="VI" URL="/&lt;vilib&gt;/gRPC/LabVIEW gRPC Library/Server API/typeDefs/Message Metadata.ctl"/>
				<Item Name="NI_Data Type.lvlib" Type="Library" URL="/&lt;vilib&gt;/Utility/Data Type/NI_Data Type.lvlib"/>
				<Item Name="NI_FileType.lvlib" Type="Library" URL="/&lt;vilib&gt;/Utility/lvfile.llb/NI_FileType.lvlib"/>
				<Item Name="PackToAny.vim" Type="VI" URL="/&lt;vilib&gt;/gRPC/LabVIEW gRPC Library/Server API/Message Requests/PackToAny.vim"/>
				<Item Name="PackToBuffer.vim" Type="VI" URL="/&lt;vilib&gt;/gRPC/LabVIEW gRPC Library/Server API/PackToBuffer.vim"/>
				<Item Name="Register Message Metadata.vi" Type="VI" URL="/&lt;vilib&gt;/gRPC/LabVIEW gRPC Library/Server API/Server/Register Message Metadata.vi"/>
				<Item Name="UnpackFromAny.vim" Type="VI" URL="/&lt;vilib&gt;/gRPC/LabVIEW gRPC Library/Server API/Message Requests/UnpackFromAny.vim"/>
				<Item Name="UnpackFromBuffer.vim" Type="VI" URL="/&lt;vilib&gt;/gRPC/LabVIEW gRPC Library/Server API/UnpackFromBuffer.vim"/>
			</Item>
			<Item Name="gprc-lvsupport.lvlib" Type="Library" URL="../../labview source/gRPC lv Support/gprc-lvsupport.lvlib"/>
		</Item>
		<Item Name="Build Specifications" Type="Build"/>
	</Item>
</Project>
