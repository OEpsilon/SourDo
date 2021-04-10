#pragma once

namespace sourdo
{
    #if defined(__APPLE__) || defined(__LINUX__)
    static const char* const COLOR_RED = "\x1b[31m";
    static const char* const COLOR_DEFAULT = "\x1b[0m";
    #else
        //#error Platform is not supported
    #endif
} // namespace sourdo
