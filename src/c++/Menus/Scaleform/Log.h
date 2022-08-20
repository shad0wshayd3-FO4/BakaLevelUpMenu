#pragma once

namespace Scaleform
{
	namespace Log
	{
		namespace
		{
			class BakaScaleformLog :
				public RE::Scaleform::Log
			{
			public:
				BakaScaleformLog()
				{
					auto path = logger::log_directory();
					if (!path)
					{
						stl::report_and_fail("Failed to find standard logging directory"sv);
					}

					*path /= "BakaScaleform.log"sv;
					auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

					log = std::make_shared<spdlog::logger>("scaleform log"s, std::move(sink));
					log->set_level(spdlog::level::trace);
					log->flush_on(spdlog::level::trace);
					log->set_pattern("[%^%l%$] %v"s);

					log->info("BakaScaleform log opened."sv);
					logger::info("BakaScaleform log opened."sv);
				}

				[[nodiscard]] static BakaScaleformLog* GetSingleton()
				{
					static BakaScaleformLog singleton;
					return std::addressof(singleton);
				}

				virtual void LogMessageVarg(RE::Scaleform::LogMessageId a_messageID, const char* a_fmt, std::va_list a_argList) override
				{
					std::array<char, 2048> buffer{ 0 };
					vsnprintf_s(buffer.data(), buffer.size(), 2048, a_fmt, a_argList);
					std::string formatted{ buffer.data(), buffer.size() };
					LogMessage(a_messageID, formatted.c_str());
				}

				void LogMessage(RE::Scaleform::LogMessageId a_messageID, const char* a_msg)
				{
					auto msg = std::string(a_msg);
					msg.erase(
						std::remove_if(msg.end() - 1, msg.end(), [](const char x)
					                   { return (x == '\n'); }),
						msg.end());

					switch (a_messageID.id)
					{
						case 0x24000:
							log->warn(msg);
							break;
						case 0x34000:
							log->error(msg);
							break;
						default:
							log->info(msg);
							break;
					}
				}

			private:
				std::shared_ptr<spdlog::logger> log;
			};

			void hkOutput(RE::Scaleform::GFx::AS3::FlashUI* a_this, RE::Scaleform::GFx::AS3::FlashUI::OutputMessageType a_type, const char* a_msg)
			{
				std::int32_t messageID{ 0 };
				switch (a_type)
				{
					case RE::Scaleform::GFx::AS3::FlashUI::OutputMessageType::kMessage:
						messageID = 0x1000;
						break;
					case RE::Scaleform::GFx::AS3::FlashUI::OutputMessageType::kError:
						messageID = 0x34000;
						break;
					case RE::Scaleform::GFx::AS3::FlashUI::OutputMessageType::kWarning:
						messageID = 0x24000;
						break;
					case RE::Scaleform::GFx::AS3::FlashUI::OutputMessageType::kAction:
						messageID = 0x6000;
						break;
				}

				std::filesystem::path fileName;
				if (auto p_this = stl::adjust_pointer<RE::Scaleform::GFx::AS3::MovieRoot>(a_this, -0x28))
				{
					if (auto p_VM = p_this->asVM.object)
					{
						if (auto p_CF = p_VM->GetCurrCallFrame())
						{
							if (p_CF->file && p_CF->file->file && !p_CF->file->file->source.empty())
							{
								fileName.assign(p_CF->file->file->source.c_str());
							}
						}
					}
				}

				auto ScaleformLog = BakaScaleformLog::GetSingleton();
				if (fileName.empty())
				{
					ScaleformLog->LogMessage(messageID, a_msg);
				}
				else
				{
					auto msg = fmt::format(
						FMT_STRING("[{:s}] {:s}"sv),
						(fileName.parent_path() == "Interface"sv)
							? fileName.stem().string()
							: fileName.string(),
						a_msg);
					ScaleformLog->LogMessage(messageID, msg.c_str());
				}
			}
		}

		void Register()
		{
			auto BSScaleformManager = RE::BSScaleformManager::GetSingleton();
			if (BSScaleformManager && BSScaleformManager->loader)
			{
				auto LogState = static_cast<RE::Scaleform::GFx::LogState*>(
					BSScaleformManager->loader->GetStateAddRef(RE::Scaleform::GFx::State::StateType::kLog));
				if (LogState)
				{
					LogState->log.reset(BakaScaleformLog::GetSingleton());
				}
			}
		}

		void Install()
		{
			REL::Relocation<std::uintptr_t> target{ REL::ID(610411) };
			stl::asm_replace(target.address(), 0x6C, reinterpret_cast<std::uintptr_t>(hkOutput));
		}
	}
}
