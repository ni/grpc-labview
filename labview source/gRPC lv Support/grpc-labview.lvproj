<?xml version='1.0' encoding='UTF-8'?>
<Project Type="Project" LVVersion="19008000">
	<Property Name="NI.LV.All.SaveVersion" Type="Str">19.0</Property>
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
		<Item Name="Libraries" Type="Folder">
			<Item Name="Linux" Type="Folder">
				<Item Name="liblabview_grpc_server.so" Type="Document" URL="../Libraries/Linux/liblabview_grpc_server.so"/>
			</Item>
			<Item Name="LinuxRT" Type="Folder">
				<Item Name="liblabview_grpc_server.so" Type="Document" URL="../Libraries/LinuxRT/liblabview_grpc_server.so"/>
			</Item>
			<Item Name="Win32" Type="Folder">
				<Item Name="labview_grpc_server.dll" Type="Document" URL="../Libraries/Win32/labview_grpc_server.dll"/>
			</Item>
			<Item Name="Win64" Type="Folder">
				<Item Name="labview_grpc_server.dll" Type="Document" URL="../Libraries/Win64/labview_grpc_server.dll"/>
			</Item>
		</Item>
		<Item Name="grpc-lvsupport.lvlib" Type="Library" URL="../grpc-lvsupport.lvlib"/>
		<Item Name="Dependencies" Type="Dependencies"/>
		<Item Name="Build Specifications" Type="Build"/>
	</Item>
</Project>
