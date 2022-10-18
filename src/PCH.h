#pragma once

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <mutex>
#include <random>
#include <string>


#pragma warning(push)
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <REL/Relocation.h>

#include <AutoTOML.hpp>
#include <fmt/format.h>
#pragma warning(pop)

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

#include <ShlObj_core.h>
#include <Windows.h>
#include <Psapi.h>
#undef cdecl // Workaround for Clang 14 CMake configure error.

// Compatible declarations with other sample projects.
#define DLLEXPORT __declspec(dllexport)

using namespace std::literals;
using namespace REL::literals;

namespace logger = SKSE::log;

namespace util {
    using SKSE::stl::report_and_fail;
}

#include "Settings.h"