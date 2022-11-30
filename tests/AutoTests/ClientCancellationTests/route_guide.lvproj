﻿<?xml version='1.0' encoding='UTF-8'?>
<Project Type="Project" LVVersion="19008000">
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
		<Item Name="route_guide_client" Type="Folder">
			<Item Name="route_guide_client.lvlib" Type="Library" URL="../route_guide_client/route_guide_client.lvlib"/>
		</Item>
		<Item Name="route_guide_server" Type="Folder">
			<Item Name="Is Call Cancelled Global.vi" Type="VI" URL="../route_guide_server/Is Call Cancelled Global.vi"/>
			<Item Name="route_guide_server.lvlib" Type="Library" URL="../route_guide_server/route_guide_server.lvlib"/>
			<Item Name="Server Error Global.vi" Type="VI" URL="../route_guide_server/Server Error Global.vi"/>
		</Item>
		<Item Name="BiDiStreamingCancellationTest.vi" Type="VI" URL="../BiDiStreamingCancellationTest.vi"/>
		<Item Name="ClientStreamingCancellationTest.vi" Type="VI" URL="../ClientStreamingCancellationTest.vi"/>
		<Item Name="ServerStreamingCancellationTest.vi" Type="VI" URL="../ServerStreamingCancellationTest.vi"/>
		<Item Name="UnaryClientCancellationTest.vi" Type="VI" URL="../UnaryClientCancellationTest.vi"/>
		<Item Name="Dependencies" Type="Dependencies">
			<Item Name="vi.lib" Type="Folder">
				<Item Name="1D String Array to Delimited String.vi" Type="VI" URL="/&lt;vilib&gt;/AdvancedString/1D String Array to Delimited String.vi"/>
				<Item Name="Application Directory.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/file.llb/Application Directory.vi"/>
				<Item Name="Error Cluster From Error Code.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Error Cluster From Error Code.vi"/>
				<Item Name="grpc-lvsupport-release.lvlib" Type="Library" URL="/&lt;vilib&gt;/gRPC/LabVIEW gRPC Library/grpc-lvsupport-release.lvlib"/>
				<Item Name="gRPC-servicer-release.lvlib" Type="Library" URL="/&lt;vilib&gt;/gRPC/LabVIEW gRPC Servicer/gRPC-servicer-release.lvlib"/>
				<Item Name="NI_FileType.lvlib" Type="Library" URL="/&lt;vilib&gt;/Utility/lvfile.llb/NI_FileType.lvlib"/>
				<Item Name="NI_PackedLibraryUtility.lvlib" Type="Library" URL="/&lt;vilib&gt;/Utility/LVLibp/NI_PackedLibraryUtility.lvlib"/>
			</Item>
			<Item Name="Add test to error descriptions.vi" Type="VI" URL="../../../gRPC_ATS/TestUtility/utility.llb/Add test to error descriptions.vi"/>
			<Item Name="array of error description.ctl" Type="VI" URL="../../../gRPC_ATS/TestUtility/utility.llb/array of error description.ctl"/>
			<Item Name="error description.ctl" Type="VI" URL="../../../gRPC_ATS/TestUtility/utility.llb/error description.ctl"/>
			<Item Name="error type.ctl" Type="VI" URL="../../../gRPC_ATS/TestUtility/utility.llb/error type.ctl"/>
			<Item Name="GetControlValue.vi" Type="VI" URL="../../Utilities/GetControlValue.vi"/>
		</Item>
		<Item Name="Build Specifications" Type="Build"/>
	</Item>
</Project>
