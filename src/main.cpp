#include "Menus/Menus.h"

void F4SEMessageHandler(F4SE::MessagingInterface::Message* a_msg)
{
	if (!a_msg)
	{
		return;
	}

	switch (a_msg->type)
	{
	case F4SE::MessagingInterface::kGameDataReady:
		{
			if (static_cast<bool>(a_msg->data))
			{
				logger::debug("GameDataReady - Loaded"sv);

				// Register Menus
				Menus::Register();
			}
			else
			{
				logger::debug("GameDataReady - Unloaded"sv);
			}

			break;
		}

	default:
		break;
	}
}

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Query(const F4SE::QueryInterface* a_F4SE, F4SE::PluginInfo* a_info)
{
	// Create log
	auto logPath = logger::log_directory();
	if (!logPath)
	{
		return false;
	}

	*logPath /= "BakaInterface.log"sv;
	auto logSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath->string(), true);
	auto log = std::make_shared<spdlog::logger>("plugin_log"s, std::move(logSink));

	// Set default log traits
	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%m/%d/%Y - %T] [%^%l%$] %v"s);
	spdlog::flush_on(spdlog::level::debug);

	// Set log level from ini
	Settings::Load();
	if (*Settings::EnableDebugLogging)
	{
		spdlog::set_level(spdlog::level::debug);
	}
	else
	{
		spdlog::set_level(spdlog::level::info);
	}

	// Initial messages
	logger::info("{} log opened."sv, "BakaInterface"sv);
	logger::debug("Debug logging enabled."sv);

	// Initialize PluginInfo
	a_info->infoVersion = F4SE::PluginInfo::kVersion;
	a_info->name = "BakaInterface";
	a_info->version = Version::MAJOR;

	// Check if we're being loaded in the CK.
	if (a_F4SE->IsEditor())
	{
		logger::critical("Loaded in editor, marking as incompatible."sv);
		return false;
	}

	// Check if we're being loaded by a supported version of the game.
	const auto ver = a_F4SE->RuntimeVersion();
	if (ver < F4SE::RUNTIME_1_10_163)
	{
		logger::critical("Unsupported runtime v{}, marking as incompatible."sv, ver.string());
		return false;
	}

	return true;
}

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Load(const F4SE::LoadInterface* a_F4SE)
{
	// Initialize F4SE
	F4SE::Init(a_F4SE);
	F4SE::AllocTrampoline(1 << 6);

	// Register Messaging Interface Listeners
	const auto messaging = F4SE::GetMessagingInterface();
	if (!messaging || !messaging->RegisterListener(F4SEMessageHandler))
	{
		logger::critical("Failed to get F4SE messaging interface, marking as incompatible."sv);
		return false;
	}

	// Hook Forms
	Forms::InstallHooks();

	// Hook Menus
	Menus::InstallHooks();

	// Finish load
	logger::info("Plugin loaded successfully."sv);
	return true;
}