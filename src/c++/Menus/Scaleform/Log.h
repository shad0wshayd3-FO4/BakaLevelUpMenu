#pragma once

class Scaleform
{
public:
	static void Register()
	{
		auto BSScaleformManager = RE::BSScaleformManager::GetSingleton();
		if (BSScaleformManager && BSScaleformManager->loader)
		{
			auto LogState = static_cast<RE::Scaleform::GFx::LogState*>(
				BSScaleformManager->loader->GetStateAddRef(RE::Scaleform::GFx::State::StateType::kLog));
			LogState->log.reset(&bakaScaleformLog);
		}
	}

private:
	class BakaScaleformLog :
		public RE::Scaleform::Log
	{
	public:
		BakaScaleformLog()
		{
#ifndef NDEBUG
			auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
			auto path = logger::log_directory();
			if (!path)
			{
				stl::report_and_fail("Failed to find standard logging directory"sv);
			}

			*path /= "BakaScaleform.log"sv;
			auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

			log = std::make_shared<spdlog::logger>("scaleform log"s, std::move(sink));
			log->set_level(spdlog::level::trace);
			log->flush_on(spdlog::level::trace);

			spdlog::set_default_logger(std::move(log));
			spdlog::set_pattern("[%m/%d/%Y - %T] [%^%l%$] %v"s);

			// Initial message
			log->info("BakaScaleform log opened."sv);
			logger::info("BakaScaleform log opened."sv);
		}

		virtual void LogMessageVarg(RE::Scaleform::LogMessageId a_messageID, const char* a_fmt, std::va_list a_argList) override
		{
			std::array<char, 2048> buffer;
			vsnprintf(buffer.data(), buffer.size(), a_fmt, a_argList);

			std::string formatted{ buffer.data() };
			formatted.erase(std::remove_if(formatted.end() - 1, formatted.end(), [](const char x)
										   { return (x == '\n'); }),
							formatted.end());

			log->info(formatted.data());
		}

	private:
		std::shared_ptr<spdlog::logger> log;
	};

	static inline BakaScaleformLog bakaScaleformLog;
};
