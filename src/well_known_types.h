//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

namespace grpc_labview
{
    namespace wellknown
    {
        //---------------------------------------------------------------------
        // Well known message types that are supported natively without having to
        // explicitly generate code from a proto file.
        //---------------------------------------------------------------------
        enum class Types
        {
            None,
            Double2DArray,
            String2DArray
        };
    }
}