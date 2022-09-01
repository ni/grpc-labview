<?xml version='1.0' encoding='UTF-8'?>
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
		<Item Name="Helpers" Type="Folder">
			<Item Name="GetClientServerTemplateBuildSpecPath.vi" Type="VI" URL="../GetClientServerTemplateBuildSpecPath.vi"/>
			<Item Name="GetInstalledVIPMPackages.vi" Type="VI" URL="../GetInstalledVIPMPackages.vi"/>
			<Item Name="GetLVServicerBuildSpecPath.vi" Type="VI" URL="../GetLVServicerBuildSpecPath.vi"/>
			<Item Name="GetLVSupportBuildSpecPath.vi" Type="VI" URL="../GetLVSupportBuildSpecPath.vi"/>
		</Item>
		<Item Name="BuildGRPCPackages.vi" Type="VI" URL="../BuildGRPCPackages.vi"/>
		<Item Name="BuildVIPBFile.vi" Type="VI" URL="../BuildVIPBFile.vi"/>
		<Item Name="InstallVIPMPackage.vi" Type="VI" URL="../InstallVIPMPackage.vi"/>
		<Item Name="UnInstallGRPCPackages.vi" Type="VI" URL="../UnInstallGRPCPackages.vi"/>
		<Item Name="Dependencies" Type="Dependencies">
			<Item Name="vi.lib" Type="Folder">
				<Item Name="1D Array to String__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/1D Array to String__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="8.6CompatibleGlobalVar.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/config.llb/8.6CompatibleGlobalVar.vi"/>
				<Item Name="Add State(s) to Queue__jki_lib_state_machine497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Add State(s) to Queue__jki_lib_state_machine497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Array of VData to VArray__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Array of VData to VArray__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Array of VData to VCluster__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Array of VData to VCluster__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Array Size(s)__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Array Size(s)__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Build Error Cluster__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Build Error Cluster__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Check if File or Folder Exists.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/libraryn.llb/Check if File or Folder Exists.vi"/>
				<Item Name="Clear Errors.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Clear Errors.vi"/>
				<Item Name="Cluster to Array of VData__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Cluster to Array of VData__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Encode Section and Key Names__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Encode Section and Key Names__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Error Cluster From Error Code.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Error Cluster From Error Code.vi"/>
				<Item Name="Generate Temporary File Path.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/libraryn.llb/Generate Temporary File Path.vi"/>
				<Item Name="Get Array Element TD__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Get Array Element TD__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Get Array Element TDEnum__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Get Array Element TDEnum__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Get Cluster Element Names__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Get Cluster Element Names__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Get Cluster Elements TDs__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Get Cluster Elements TDs__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Get Data Name from TD__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Get Data Name from TD__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Get Data Name__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Get Data Name__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Get Default Data from TD__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Get Default Data from TD__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Get Element TD from Array TD__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Get Element TD from Array TD__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Get File Extension.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/libraryn.llb/Get File Extension.vi"/>
				<Item Name="Get Header from TD__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Get Header from TD__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Get Last PString__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Get Last PString__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Get PString__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Get PString__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Get Strings from Enum TD__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Get Strings from Enum TD__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Get Strings from Enum__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Get Strings from Enum__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Get System Directory.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/sysdir.llb/Get System Directory.vi"/>
				<Item Name="Get TDEnum from Data__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Get TDEnum from Data__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Get Variant Attributes__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Get Variant Attributes__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Get Waveform Type Enum from TD__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Get Waveform Type Enum from TD__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="NI_FileType.lvlib" Type="Library" URL="/&lt;vilib&gt;/Utility/lvfile.llb/NI_FileType.lvlib"/>
				<Item Name="NI_LVConfig.lvlib" Type="Library" URL="/&lt;vilib&gt;/Utility/config.llb/NI_LVConfig.lvlib"/>
				<Item Name="NI_PackedLibraryUtility.lvlib" Type="Library" URL="/&lt;vilib&gt;/Utility/LVLibp/NI_PackedLibraryUtility.lvlib"/>
				<Item Name="Parse State Queue__jki_lib_state_machine497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Parse State Queue__jki_lib_state_machine497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Parse String with TDs__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Parse String with TDs__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Read INI Cluster__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Read INI Cluster__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Read Key (Variant)__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Read Key (Variant)__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Read Section Cluster__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Read Section Cluster__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Remove Duplicates from 1D Array (Path)__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Remove Duplicates from 1D Array (Path)__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Remove Duplicates from 1D Array (String)__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Remove Duplicates from 1D Array (String)__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Reshape 1D Array__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Reshape 1D Array__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Reshape Array to 1D VArray__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Reshape Array to 1D VArray__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Search and Replace Pattern.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Search and Replace Pattern.vi"/>
				<Item Name="Set Busy.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/cursorutil.llb/Set Busy.vi"/>
				<Item Name="Set Cursor (Cursor ID).vi" Type="VI" URL="/&lt;vilib&gt;/Utility/cursorutil.llb/Set Cursor (Cursor ID).vi"/>
				<Item Name="Set Cursor (Icon Pict).vi" Type="VI" URL="/&lt;vilib&gt;/Utility/cursorutil.llb/Set Cursor (Icon Pict).vi"/>
				<Item Name="Set Cursor.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/cursorutil.llb/Set Cursor.vi"/>
				<Item Name="Set Data Name__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Set Data Name__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Set Enum String Value__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Set Enum String Value__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Space Constant.vi" Type="VI" URL="/&lt;vilib&gt;/dlg_ctls.llb/Space Constant.vi"/>
				<Item Name="Split Cluster TD__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Split Cluster TD__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="Strip Units__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Strip Units__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="System Directory Type.ctl" Type="VI" URL="/&lt;vilib&gt;/Utility/sysdir.llb/System Directory Type.ctl"/>
				<Item Name="System Exec.vi" Type="VI" URL="/&lt;vilib&gt;/Platform/system.llb/System Exec.vi"/>
				<Item Name="Trim Whitespace.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Trim Whitespace.vi"/>
				<Item Name="Type Descriptor Enumeration__ogtk497EDDB02AA4B404E9F3AB1BAA994342.ctl" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Type Descriptor Enumeration__ogtk497EDDB02AA4B404E9F3AB1BAA994342.ctl"/>
				<Item Name="Type Descriptor Header__ogtk497EDDB02AA4B404E9F3AB1BAA994342.ctl" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Type Descriptor Header__ogtk497EDDB02AA4B404E9F3AB1BAA994342.ctl"/>
				<Item Name="Type Descriptor__ogtk497EDDB02AA4B404E9F3AB1BAA994342.ctl" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Type Descriptor__ogtk497EDDB02AA4B404E9F3AB1BAA994342.ctl"/>
				<Item Name="Unset Busy.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/cursorutil.llb/Unset Busy.vi"/>
				<Item Name="Variant to Header Info__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Variant to Header Info__ogtk497EDDB02AA4B404E9F3AB1BAA994342.vi"/>
				<Item Name="VIPM API_vipm_api.lvlib" Type="Library" URL="/&lt;vilib&gt;/JKI/VIPM API/VIPM API_vipm_api.lvlib"/>
				<Item Name="Waveform Subtype Enum__ogtk497EDDB02AA4B404E9F3AB1BAA994342.ctl" Type="VI" URL="/&lt;vilib&gt;/JKI/_VIPM API_internal_deps/Waveform Subtype Enum__ogtk497EDDB02AA4B404E9F3AB1BAA994342.ctl"/>
				<Item Name="whitespace.ctl" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/whitespace.ctl"/>
			</Item>
			<Item Name="System" Type="VI" URL="System">
				<Property Name="NI.PreserveRelativePath" Type="Bool">true</Property>
			</Item>
		</Item>
		<Item Name="Build Specifications" Type="Build"/>
	</Item>
</Project>
