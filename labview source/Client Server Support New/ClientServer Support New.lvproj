<?xml version='1.0' encoding='UTF-8'?>
<Project Type="Project" LVVersion="19008000">
	<Property Name="NI.LV.All.SaveVersion" Type="Str">19.0</Property>
	<Property Name="NI.LV.All.SourceOnly" Type="Bool">true</Property>
	<Property Name="NI.Project.Description" Type="Str"></Property>
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
		<Item Name="gRPC Scripting Tools" Type="Folder">
			<Item Name="gRPC Template Creation Utility.lvlib" Type="Library" URL="../gRPC Scripting Tools/gRPC Template Creation Utility.lvlib"/>
			<Item Name="Open gRPC Server-Client [2] - Code Generator.vi" Type="VI" URL="../gRPC Scripting Tools/Open gRPC Server-Client [2] - Code Generator.vi"/>
		</Item>
		<Item Name="Server Template" Type="Folder">
			<Item Name="Service Class" Type="Folder">
				<Item Name="ServiceName Template.lvclass" Type="LVClass" URL="../Server Template/RPC Service/ServiceName/ServiceName Template.lvclass"/>
			</Item>
			<Item Name="Service Template.lvlib" Type="Library" URL="../Server Template/Service Template.lvlib"/>
		</Item>
		<Item Name="Client Template" Type="Folder">
			<Item Name="Client Template.lvlib" Type="Library" URL="../Client Template/Client Template.lvlib"/>
		</Item>
		<Item Name="Oneof Template" Type="Folder">
			<Item Name="OneOfTemplate.lvclass" Type="LVClass" URL="../Oneof Template/OneOfTemplate.lvclass"/>
		</Item>
		<Item Name="Dependencies" Type="Dependencies">
			<Item Name="vi.lib" Type="Folder">
				<Item Name="Application Directory.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/file.llb/Application Directory.vi"/>
				<Item Name="Error Cluster From Error Code.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Error Cluster From Error Code.vi"/>
				<Item Name="gRPC-servicer-release.lvlib" Type="Library" URL="/&lt;vilib&gt;/gRPC/LabVIEW gRPC Servicer/gRPC-servicer-release.lvlib"/>
				<Item Name="NI_FileType.lvlib" Type="Library" URL="/&lt;vilib&gt;/Utility/lvfile.llb/NI_FileType.lvlib"/>
				<Item Name="VI Scripting - Traverse.lvlib" Type="Library" URL="/&lt;vilib&gt;/Utility/traverseref.llb/VI Scripting - Traverse.lvlib"/>
				<Item Name="TRef TravTarget.ctl" Type="VI" URL="/&lt;vilib&gt;/Utility/traverseref.llb/TRef TravTarget.ctl"/>
				<Item Name="Check if File or Folder Exists.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/libraryn.llb/Check if File or Folder Exists.vi"/>
				<Item Name="NI_PackedLibraryUtility.lvlib" Type="Library" URL="/&lt;vilib&gt;/Utility/LVLibp/NI_PackedLibraryUtility.lvlib"/>
				<Item Name="CreateOrAddLibraryToProject.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/Variable/CreateOrAddLibraryToProject.vi"/>
				<Item Name="CreateOrAddLibraryToParent.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/Variable/CreateOrAddLibraryToParent.vi"/>
				<Item Name="LVPointTypeDef.ctl" Type="VI" URL="/&lt;vilib&gt;/Utility/miscctls.llb/LVPointTypeDef.ctl"/>
				<Item Name="LVNumericRepresentation.ctl" Type="VI" URL="/&lt;vilib&gt;/numeric/LVNumericRepresentation.ctl"/>
				<Item Name="LVPositionTypeDef.ctl" Type="VI" URL="/&lt;vilib&gt;/Utility/miscctls.llb/LVPositionTypeDef.ctl"/>
				<Item Name="Clear Errors.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Clear Errors.vi"/>
				<Item Name="Space Constant.vi" Type="VI" URL="/&lt;vilib&gt;/dlg_ctls.llb/Space Constant.vi"/>
				<Item Name="Trim Whitespace.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Trim Whitespace.vi"/>
				<Item Name="whitespace.ctl" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/whitespace.ctl"/>
				<Item Name="Delimited String to 1D String Array.vi" Type="VI" URL="/&lt;vilib&gt;/AdvancedString/Delimited String to 1D String Array.vi"/>
				<Item Name="1D String Array to Delimited String.vi" Type="VI" URL="/&lt;vilib&gt;/AdvancedString/1D String Array to Delimited String.vi"/>
				<Item Name="lveventtype.ctl" Type="VI" URL="/&lt;vilib&gt;/event_ctls.llb/lveventtype.ctl"/>
				<Item Name="LVBoundsTypeDef.ctl" Type="VI" URL="/&lt;vilib&gt;/Utility/miscctls.llb/LVBoundsTypeDef.ctl"/>
				<Item Name="Search Unsorted 1D Array.vim" Type="VI" URL="/&lt;vilib&gt;/Array/Search Unsorted 1D Array.vim"/>
				<Item Name="Equal Functor.lvclass" Type="LVClass" URL="/&lt;vilib&gt;/Comparison/Equal/Equal Functor/Equal Functor.lvclass"/>
				<Item Name="Equal Comparable.lvclass" Type="LVClass" URL="/&lt;vilib&gt;/Comparison/Equal/Equal Comparable/Equal Comparable.lvclass"/>
				<Item Name="Search Unsorted 1D Array Core.vim" Type="VI" URL="/&lt;vilib&gt;/Array/Helpers/Search Unsorted 1D Array Core.vim"/>
				<Item Name="Equals.vim" Type="VI" URL="/&lt;vilib&gt;/Comparison/Equals.vim"/>
				<Item Name="Get File Extension.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/libraryn.llb/Get File Extension.vi"/>
				<Item Name="Simple Error Handler.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Simple Error Handler.vi"/>
				<Item Name="DialogType.ctl" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/DialogType.ctl"/>
				<Item Name="General Error Handler.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/General Error Handler.vi"/>
				<Item Name="DialogTypeEnum.ctl" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/DialogTypeEnum.ctl"/>
				<Item Name="General Error Handler Core CORE.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/General Error Handler Core CORE.vi"/>
				<Item Name="Check Special Tags.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Check Special Tags.vi"/>
				<Item Name="TagReturnType.ctl" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/TagReturnType.ctl"/>
				<Item Name="Set String Value.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Set String Value.vi"/>
				<Item Name="GetRTHostConnectedProp.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/GetRTHostConnectedProp.vi"/>
				<Item Name="Error Code Database.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Error Code Database.vi"/>
				<Item Name="Format Message String.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Format Message String.vi"/>
				<Item Name="Find Tag.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Find Tag.vi"/>
				<Item Name="Search and Replace Pattern.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Search and Replace Pattern.vi"/>
				<Item Name="Set Bold Text.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Set Bold Text.vi"/>
				<Item Name="Details Display Dialog.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Details Display Dialog.vi"/>
				<Item Name="ErrWarn.ctl" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/ErrWarn.ctl"/>
				<Item Name="eventvkey.ctl" Type="VI" URL="/&lt;vilib&gt;/event_ctls.llb/eventvkey.ctl"/>
				<Item Name="Not Found Dialog.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Not Found Dialog.vi"/>
				<Item Name="Three Button Dialog.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Three Button Dialog.vi"/>
				<Item Name="Three Button Dialog CORE.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Three Button Dialog CORE.vi"/>
				<Item Name="LVRectTypeDef.ctl" Type="VI" URL="/&lt;vilib&gt;/Utility/miscctls.llb/LVRectTypeDef.ctl"/>
				<Item Name="Longest Line Length in Pixels.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Longest Line Length in Pixels.vi"/>
				<Item Name="Convert property node font to graphics font.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Convert property node font to graphics font.vi"/>
				<Item Name="Get Text Rect.vi" Type="VI" URL="/&lt;vilib&gt;/picture/picture.llb/Get Text Rect.vi"/>
				<Item Name="Get String Text Bounds.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Get String Text Bounds.vi"/>
				<Item Name="BuildHelpPath.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/BuildHelpPath.vi"/>
				<Item Name="GetHelpDir.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/GetHelpDir.vi"/>
				<Item Name="TRef Traverse.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/traverseref.llb/TRef Traverse.vi"/>
				<Item Name="grpc-lvsupport-release.lvlib" Type="Library" URL="/&lt;vilib&gt;/gRPC/LabVIEW gRPC Library/grpc-lvsupport-release.lvlib"/>
				<Item Name="lv_icon.lvlib" Type="Library" URL="/&lt;vilib&gt;/LabVIEW Icon API/lv_icon/lv_icon.lvlib"/>
				<Item Name="Alignment.ctl" Type="VI" URL="/&lt;vilib&gt;/LabVIEW Icon API/lv_icon/Controls/Alignment.ctl"/>
				<Item Name="Font.ctl" Type="VI" URL="/&lt;vilib&gt;/LabVIEW Icon API/lv_icon/Controls/Font.ctl"/>
				<Item Name="BodyText.ctl" Type="VI" URL="/&lt;vilib&gt;/LabVIEW Icon API/lv_icon/Controls/BodyText.ctl"/>
				<Item Name="LayerType.ctl" Type="VI" URL="/&lt;vilib&gt;/LabVIEW Icon API/lv_icon/Controls/LayerType.ctl"/>
				<Item Name="Layer.ctl" Type="VI" URL="/&lt;vilib&gt;/LabVIEW Icon API/lv_icon/Controls/Layer.ctl"/>
				<Item Name="LVPoint32TypeDef.ctl" Type="VI" URL="/&lt;vilib&gt;/Utility/miscctls.llb/LVPoint32TypeDef.ctl"/>
				<Item Name="Layer.lvclass" Type="LVClass" URL="/&lt;vilib&gt;/LabVIEW Icon API/lv_icon/Classes/Layer/Layer.lvclass"/>
				<Item Name="LabVIEW Icon Stored Information.ctl" Type="VI" URL="/&lt;vilib&gt;/LabVIEW Icon API/lv_icon/Controls/LabVIEW Icon Stored Information.ctl"/>
				<Item Name="Load &amp; Unload.lvclass" Type="LVClass" URL="/&lt;vilib&gt;/LabVIEW Icon API/lv_icon/Classes/Load_Unload/Load &amp; Unload.lvclass"/>
				<Item Name="FixBadRect.vi" Type="VI" URL="/&lt;vilib&gt;/picture/pictutil.llb/FixBadRect.vi"/>
				<Item Name="imagedata.ctl" Type="VI" URL="/&lt;vilib&gt;/picture/picture.llb/imagedata.ctl"/>
				<Item Name="Draw Flattened Pixmap.vi" Type="VI" URL="/&lt;vilib&gt;/picture/picture.llb/Draw Flattened Pixmap.vi"/>
				<Item Name="Bit-array To Byte-array.vi" Type="VI" URL="/&lt;vilib&gt;/picture/pictutil.llb/Bit-array To Byte-array.vi"/>
				<Item Name="Unflatten Pixmap.vi" Type="VI" URL="/&lt;vilib&gt;/picture/pixmap.llb/Unflatten Pixmap.vi"/>
				<Item Name="Create Mask.vi" Type="VI" URL="/&lt;vilib&gt;/picture/pictutil.llb/Create Mask.vi"/>
				<Item Name="Picture to Pixmap.vi" Type="VI" URL="/&lt;vilib&gt;/picture/pictutil.llb/Picture to Pixmap.vi"/>
				<Item Name="Icon Framework.lvclass" Type="LVClass" URL="/&lt;vilib&gt;/LabVIEW Icon API/lv_icon/Classes/Icon Framework/Icon Framework.lvclass"/>
				<Item Name="Icon.lvclass" Type="LVClass" URL="/&lt;vilib&gt;/LabVIEW Icon API/lv_icon/Classes/Icon/Icon.lvclass"/>
				<Item Name="PCT Pad String.vi" Type="VI" URL="/&lt;vilib&gt;/picture/picture.llb/PCT Pad String.vi"/>
				<Item Name="Draw Text in Rect.vi" Type="VI" URL="/&lt;vilib&gt;/picture/picture.llb/Draw Text in Rect.vi"/>
				<Item Name="Draw Text at Point.vi" Type="VI" URL="/&lt;vilib&gt;/picture/picture.llb/Draw Text at Point.vi"/>
				<Item Name="Empty Picture" Type="VI" URL="/&lt;vilib&gt;/picture/picture.llb/Empty Picture"/>
				<Item Name="Color to RGB.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/colorconv.llb/Color to RGB.vi"/>
				<Item Name="Flatten Pixmap.vi" Type="VI" URL="/&lt;vilib&gt;/picture/pixmap.llb/Flatten Pixmap.vi"/>
				<Item Name="Draw True-Color Pixmap.vi" Type="VI" URL="/&lt;vilib&gt;/picture/picture.llb/Draw True-Color Pixmap.vi"/>
				<Item Name="Draw 1-Bit Pixmap.vi" Type="VI" URL="/&lt;vilib&gt;/picture/picture.llb/Draw 1-Bit Pixmap.vi"/>
				<Item Name="Draw 8-Bit Pixmap.vi" Type="VI" URL="/&lt;vilib&gt;/picture/picture.llb/Draw 8-Bit Pixmap.vi"/>
				<Item Name="Draw 4-Bit Pixmap.vi" Type="VI" URL="/&lt;vilib&gt;/picture/picture.llb/Draw 4-Bit Pixmap.vi"/>
				<Item Name="Draw Unflattened Pixmap.vi" Type="VI" URL="/&lt;vilib&gt;/picture/picture.llb/Draw Unflattened Pixmap.vi"/>
				<Item Name="RGB to Color.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/colorconv.llb/RGB to Color.vi"/>
				<Item Name="Graphic.ctl" Type="VI" URL="/&lt;vilib&gt;/LabVIEW Icon API/lv_icon/Controls/Graphic.ctl"/>
				<Item Name="IEColor.ctl" Type="VI" URL="/&lt;vilib&gt;/LabVIEW Icon API/lv_icon/Controls/IEColor.ctl"/>
				<Item Name="BodyTextPosition.ctl" Type="VI" URL="/&lt;vilib&gt;/LabVIEW Icon API/lv_icon/Controls/BodyTextPosition.ctl"/>
				<Item Name="Set Pen State.vi" Type="VI" URL="/&lt;vilib&gt;/picture/picture.llb/Set Pen State.vi"/>
				<Item Name="Draw Rect.vi" Type="VI" URL="/&lt;vilib&gt;/picture/picture.llb/Draw Rect.vi"/>
				<Item Name="Coerce Bad Rect.vi" Type="VI" URL="/&lt;vilib&gt;/picture/pictutil.llb/Coerce Bad Rect.vi"/>
				<Item Name="Get Image Subset.vi" Type="VI" URL="/&lt;vilib&gt;/picture/pictutil.llb/Get Image Subset.vi"/>
				<Item Name="LabVIEW Icon API.lvlib" Type="Library" URL="/&lt;vilib&gt;/LabVIEW Icon API/LabVIEW Icon API.lvlib"/>
				<Item Name="Compare Two Paths.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/libraryn.llb/Compare Two Paths.vi"/>
				<Item Name="Dflt Data Dir.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/file.llb/Dflt Data Dir.vi"/>
				<Item Name="Pathes.ctl" Type="VI" URL="/&lt;vilib&gt;/LabVIEW Icon API/lv_icon/Controls/Pathes.ctl"/>
				<Item Name="Get GObject Label.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/traverseref.llb/Get GObject Label.vi"/>
				<Item Name="Trim Whitespace One-Sided.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Trim Whitespace One-Sided.vi"/>
				<Item Name="niceiplib.lvlib" Type="Library" URL="/&lt;vilib&gt;/UDC/niceiplib.lvlib"/>
				<Item Name="NI_LVConfig.lvlib" Type="Library" URL="/&lt;vilib&gt;/Utility/config.llb/NI_LVConfig.lvlib"/>
				<Item Name="8.6CompatibleGlobalVar.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/config.llb/8.6CompatibleGlobalVar.vi"/>
				<Item Name="Is Path and Not Empty.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/file.llb/Is Path and Not Empty.vi"/>
			</Item>
			<Item Name="Arrange VIWin - Window Margins.ctl" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - Window Margins.ctl"/>
			<Item Name="Arrange VIWin - Min Window Dimensions.ctl" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - Min Window Dimensions.ctl"/>
			<Item Name="Arrange VIWin - Window Gaps.ctl" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - Window Gaps.ctl"/>
			<Item Name="Arrange VIWin - Rectangle Dimensions.ctl" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - Rectangle Dimensions.ctl"/>
			<Item Name="Arrange VIWin - Compute Window Bounds.vi" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - Compute Window Bounds.vi"/>
			<Item Name="Arrange VIWin - Get Window Margins.vi" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - Get Window Margins.vi"/>
			<Item Name="Arrange VIWin - Establish Window Bounds.vi" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - Establish Window Bounds.vi"/>
			<Item Name="Arrange VIWin - FP Objects Arrangement Info.ctl" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - FP Objects Arrangement Info.ctl"/>
			<Item Name="Arrange VIWin - Resize FP.vi" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - Resize FP.vi"/>
			<Item Name="Arrange VIWin - Arrange Other FP Objects.vi" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - Arrange Other FP Objects.vi"/>
			<Item Name="Arrange VIWin - Get FP Controls Max Bottom.vi" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - Get FP Controls Max Bottom.vi"/>
			<Item Name="Arrange VIWin - Align 1 Row of FP Controls.vi" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - Align 1 Row of FP Controls.vi"/>
			<Item Name="Arrange VIWin - Compute Bound on Grid.vi" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - Compute Bound on Grid.vi"/>
			<Item Name="Arrange VIWin - FP Control Info.ctl" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - FP Control Info.ctl"/>
			<Item Name="Arrange VIWin - Get FP Controls Rows.vi" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - Get FP Controls Rows.vi"/>
			<Item Name="Arrange VIWin - Arrange FP Controls Rows.vi" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - Arrange FP Controls Rows.vi"/>
			<Item Name="Arrange VIWin - Space FP Controls.vi" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - Space FP Controls.vi"/>
			<Item Name="Arrange VIWin - Compute Top Row Vertical Delta.vi" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - Compute Top Row Vertical Delta.vi"/>
			<Item Name="Arrange VIWin - Arrange FP Controls Columns.vi" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - Arrange FP Controls Columns.vi"/>
			<Item Name="Arrange VIWin - Order FP Conn Pane Controls.vi" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - Order FP Conn Pane Controls.vi"/>
			<Item Name="Arrange VIWin - Filter FP Conn Pane Controls.vi" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - Filter FP Conn Pane Controls.vi"/>
			<Item Name="Arrange VIWin - Arrange FP Controls.vi" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - Arrange FP Controls.vi"/>
			<Item Name="LV Config Read Numeric (I32).vi" Type="VI" URL="/&lt;resource&gt;/dialog/lvconfig.llb/LV Config Read Numeric (I32).vi"/>
			<Item Name="LV Config Read Color.vi" Type="VI" URL="/&lt;resource&gt;/dialog/lvconfig.llb/LV Config Read Color.vi"/>
			<Item Name="LV Config Read Pathlist.vi" Type="VI" URL="/&lt;resource&gt;/dialog/lvconfig.llb/LV Config Read Pathlist.vi"/>
			<Item Name="LV Config Read String.vi" Type="VI" URL="/&lt;resource&gt;/dialog/lvconfig.llb/LV Config Read String.vi"/>
			<Item Name="LV Config Read Boolean.vi" Type="VI" URL="/&lt;resource&gt;/dialog/lvconfig.llb/LV Config Read Boolean.vi"/>
			<Item Name="LV Config Read.vi" Type="VI" URL="/&lt;resource&gt;/dialog/lvconfig.llb/LV Config Read.vi"/>
			<Item Name="Arrange VIWin - Get FP Objects Arrangement Info.vi" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - Get FP Objects Arrangement Info.vi"/>
			<Item Name="Arrange VIWin - Window Type.ctl" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - Window Type.ctl"/>
			<Item Name="Arrange VIWin - Get Window Gaps.vi" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - Get Window Gaps.vi"/>
			<Item Name="Arrange VIWin - Get Min Window Dimensions.vi" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - Get Min Window Dimensions.vi"/>
			<Item Name="Arrange VIWin - Get Display Workspace Bounds.vi" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - Get Display Workspace Bounds.vi"/>
			<Item Name="Arrange VIWin - Get Window INI tokens.vi" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - Get Window INI tokens.vi"/>
			<Item Name="Arrange VIWin - Arrange FP.vi" Type="VI" URL="/&lt;resource&gt;/dialog/QuickDrop/plugins/_Arrange VIWin SubVIs/Arrange VIWin - Arrange FP.vi"/>
		</Item>
		<Item Name="Build Specifications" Type="Build"/>
	</Item>
</Project>
