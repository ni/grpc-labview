<?xml version='1.0' encoding='UTF-8'?>
<Project Type="Project" LVVersion="19008000">
	<Property Name="NI.LV.All.SaveVersion" Type="Str">Editor version</Property>
	<Property Name="NI.LV.All.SourceOnly" Type="Bool">true</Property>
	<Property Name="NI.Project.Description" Type="Str"></Property>
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
		<Item Name="Dependencies" Type="Dependencies"/>
		<Item Name="Build Specifications" Type="Build"/>
	</Item>
</Project>
