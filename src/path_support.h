//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

#include <string>

namespace grpc_labview
{
	std::string GetFolderContainingDLL();
	std::string StripPath(const std::string& fullPath);
}