<?xml version='1.0' encoding='UTF-8'?>
<Project Type="Project" LVVersion="19008000">
	<Item Name="My Computer" Type="My Computer">
		<Property Name="NI.SortType" Type="Int">3</Property>
		<Property Name="server.app.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="server.control.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="server.tcp.enabled" Type="Bool">false</Property>
		<Property Name="server.tcp.port" Type="Int">0</Property>
		<Property Name="server.tcp.serviceName" Type="Str">My Computer/VI Server</Property>
		<Property Name="server.tcp.serviceName.default" Type="Str">My Computer/VI Server</Property>
		<Property Name="server.vi.callsEnabled" Type="Bool">true</Property>
		<Property Name="server.vi.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="specify.custom.address" Type="Bool">false</Property>
		<Item Name="labview-grpc-query-server.lvlib" Type="Library" URL="../labview-grpc-query-server.lvlib"/>
		<Item Name="labview_query_server.dll" Type="Document" URL="../labview_query_server.dll"/>
		<Item Name="Dependencies" Type="Dependencies">
			<Item Name="labview_query_server.dll" Type="Document" URL="labview_query_server.dll">
				<Property Name="NI.PreserveRelativePath" Type="Bool">true</Property>
			</Item>
			<Item Name="labview_query_server.dll" Type="Document" URL="../../Debug/labview_query_server.dll"/>
			<Item Name="labview_query_server.dll" Type="Document" URL="../../../../dev/labview-grpc-query-server/Debug/labview_query_server.dll"/>
		</Item>
		<Item Name="Build Specifications" Type="Build"/>
	</Item>
</Project>
