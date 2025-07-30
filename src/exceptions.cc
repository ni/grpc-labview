#include "exceptions.h"

#include <iostream>

namespace grpc_labview
{
    int32_t TranslateException() {
        try {
            throw;
        }
        catch (const GrpcException& e) {
#ifndef NDEBUG
            std::cerr << "gRPC ERROR: " << e.GetStatusCode() << " - " << e.what() << std::endl;
#endif
            return -(1000 + e.GetStatusCode());
        }
        catch (const LabVIEWException& e) {
#ifndef NDEBUG
            std::cerr << "LabVIEW ERROR: " << e.GetCode() << " - " << e.what() << std::endl;
#endif
            return e.GetCode();
        }
        catch (const std::exception& e) {
#ifndef NDEBUG
            std::cerr << "ERROR: " << e.what() << std::endl;
#endif
            return -1;
        }
    }

    void SetErrorMessage(LStrHandle* errorMessageOut, const char* message) {
        if (!errorMessageOut) return;
        if (!message) message = "";

        try {
            SetLVString(errorMessageOut, message);
        } catch (const std::exception&) {
            // Ignore errors
        }
    }
}
