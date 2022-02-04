#pragma once

#pragma warning(push, 0)
#include "F4SE/F4SE.h"
#include "RE/Fallout.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <AutoTOML.hpp>

#ifndef NDEBUG
#	include <spdlog/sinks/msvc_sink.h>
#endif
#pragma warning(pop)

using namespace std::literals;

namespace logger = F4SE::log;

namespace stl
{
	using F4SE::stl::adjust_pointer;
	using F4SE::stl::emplace_vtable;
	using F4SE::stl::report_and_fail;
	using F4SE::stl::to_underlying;
	using F4SE::stl::unrestricted_cast;

#ifdef F4SE_SUPPORT_XBYAK
	void asm_replace(std::uintptr_t a_from, std::size_t a_size, std::uintptr_t a_to);
#endif
}

#include "Plugin.h"
#include "Settings.h"

#define DLLEXPORT __declspec(dllexport)
