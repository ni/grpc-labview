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
			<Item Name="TestClientStreaming.vi" Type="VI" URL="../Client/TestClientStreaming.vi"/>
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
				<Item Name="1D String Array to Delimited String.vi" Type="VI" URL="/&lt;vilib&gt;/AdvancedString/1D String Array to Delimited String.vi"/>
				<Item Name="Application Directory.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/file.llb/Application Directory.vi"/>
				<Item Name="Error Cluster From Error Code.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Error Cluster From Error Code.vi"/>
				<Item Name="NI_Data Type.lvlib" Type="Library" URL="/&lt;vilib&gt;/Utility/Data Type/NI_Data Type.lvlib"/>
				<Item Name="NI_FileType.lvlib" Type="Library" URL="/&lt;vilib&gt;/Utility/lvfile.llb/NI_FileType.lvlib"/>
			</Item>
			<Item Name="grpc-lvsupport.lvlib" Type="Library" URL="../../labview source/gRPC lv Support/grpc-lvsupport.lvlib"/>
		</Item>
		<Item Name="Build Specifications" Type="Build"/>
	</Item>
</Project>
