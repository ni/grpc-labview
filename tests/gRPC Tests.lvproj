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
		<Item Name="Data Marshal" Type="Folder">
			<Item Name="gRPC Interface" Type="Folder">
				<Item Name="gprc-lvsupport.lvlib" Type="Library" URL="../data_marshal/gRPC interface/gprc-lvsupport.lvlib"/>
			</Item>
			<Item Name="Protos" Type="Folder">
				<Item Name="data_marshal.proto" Type="Document" URL="../data_marshal/Protos/data_marshal.proto"/>
			</Item>
			<Item Name="SubVIs" Type="Folder">
				<Item Name="labview-grpc-query-server.lvlib" Type="Library" URL="../data_marshal/VIs/labview-grpc-query-server.lvlib"/>
			</Item>
			<Item Name="Data Marshal Main.vi" Type="VI" URL="../data_marshal/Data Marshal Main.vi"/>
		</Item>
		<Item Name="Dependencies" Type="Dependencies">
			<Item Name="vi.lib" Type="Folder">
				<Item Name="Error Cluster From Error Code.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Error Cluster From Error Code.vi"/>
			</Item>
			<Item Name="labview_grpc_server.dll" Type="Document" URL="../build/Debug/labview_grpc_server.dll"/>
			<Item Name="QueryServer_gRPCid.ctl" Type="VI" URL="../data_marshal/VIs/QueryServer_gRPCid.ctl"/>
		</Item>
		<Item Name="Build Specifications" Type="Build"/>
	</Item>
</Project>
