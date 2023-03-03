
#include "LogUtil.hpp"

void log(std::string msg)
{
    std::string logContent("::VDS-TEST::");
    logContent += msg;
    OutputDebugString(logContent.c_str());
}