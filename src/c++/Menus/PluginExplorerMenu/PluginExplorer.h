#pragma once

namespace Menus
{
	class PluginExplorer
	{
	public:
		class PluginInfo
		{
		public:
			using FormMap = std::map<std::uint32_t, std::string_view>;

			PluginInfo(std::string_view a_name) :
				name(a_name)
			{}

			template<class FORM_TYPE>
			void AddForm(FORM_TYPE a_form)
			{
				auto formName = RE::TESFullName::GetFullName(*a_form);
				if (!a_form->GetPlayable(nullptr) || formName.empty())
				{
					return;
				}

				switch (a_form->GetFormType())
				{
					case RE::ENUM_FORM_ID::kALCH:
						{
							mapALCH.insert_or_assign(a_form->GetFormID(), formName);
							break;
						}
					case RE::ENUM_FORM_ID::kAMMO:
						{
							mapAMMO.insert_or_assign(a_form->GetFormID(), formName);
							break;
						}
					case RE::ENUM_FORM_ID::kARMO:
						{
							mapARMO.insert_or_assign(a_form->GetFormID(), formName);
							break;
						}
					case RE::ENUM_FORM_ID::kBOOK:
						{
							mapBOOK.insert_or_assign(a_form->GetFormID(), formName);
							break;
						}
					case RE::ENUM_FORM_ID::kKEYM:
						{
							mapKEYS.insert_or_assign(a_form->GetFormID(), formName);
							break;
						}
					case RE::ENUM_FORM_ID::kMISC:
						{
							mapMISC.insert_or_assign(a_form->GetFormID(), formName);
							break;
						}
					case RE::ENUM_FORM_ID::kNOTE:
						{
							mapHOLO.insert_or_assign(a_form->GetFormID(), formName);
							break;
						}
					case RE::ENUM_FORM_ID::kWEAP:
						{
							mapWEAP.insert_or_assign(a_form->GetFormID(), formName);
							break;
						}
					default:
						{
							logger::warn(FMT_STRING("ModInfo::Add - Unhandled FormType: {:d}"sv),
								stl::to_underlying(a_form->GetFormType()));
							break;
						}
				}
			}

			[[nodiscard]] std::string_view GetName() const noexcept { return name; }
			[[nodiscard]] FormMap GetMapALCH() const noexcept { return mapALCH; }
			[[nodiscard]] FormMap GetMapAMMO() const noexcept { return mapAMMO; }
			[[nodiscard]] FormMap GetMapARMO() const noexcept { return mapARMO; }
			[[nodiscard]] FormMap GetMapBOOK() const noexcept { return mapBOOK; }
			[[nodiscard]] FormMap GetMapHOLO() const noexcept { return mapHOLO; }
			[[nodiscard]] FormMap GetMapKEYS() const noexcept { return mapKEYS; }
			[[nodiscard]] FormMap GetMapMISC() const noexcept { return mapMISC; }
			[[nodiscard]] FormMap GetMapWEAP() const noexcept { return mapWEAP; }

			[[nodiscard]] std::uint64_t GetCount() const noexcept
			{
				std::uint64_t result{ 0 };
				result += mapALCH.size();
				result += mapAMMO.size();
				result += mapARMO.size();
				result += mapBOOK.size();
				result += mapHOLO.size();
				result += mapKEYS.size();
				result += mapMISC.size();
				result += mapWEAP.size();
				return result;
			}

		private:
			std::string_view name{ "" };
			FormMap mapALCH;
			FormMap mapAMMO;
			FormMap mapARMO;
			FormMap mapBOOK;
			FormMap mapHOLO;
			FormMap mapKEYS;
			FormMap mapMISC;
			FormMap mapWEAP;
		};

		using PluginList = std::vector<std::pair<std::uint32_t, PluginInfo>>;

		static void Initialize()
		{
			auto TESDataHandler = RE::TESDataHandler::GetSingleton();
			if (!TESDataHandler)
			{
				logger::error("Missing TESDataHandler!"sv);
				return;
			}

			auto pluginCount = 0;
			for (auto file : TESDataHandler->files)
			{
				if (file->IsActive())
				{
					auto combinedIndex = GetCombinedIndex(file);
					modMap.insert_or_assign(combinedIndex, file->GetFilename());
					idxMap.insert_or_assign(pluginCount++, combinedIndex);
				}
			}

			AddForms<RE::AlchemyItem>();
			AddForms<RE::BGSNote>();
			AddForms<RE::TESAmmo>();
			AddForms<RE::TESKey>();
			AddForms<RE::TESObjectARMO>();
			AddForms<RE::TESObjectBOOK>();
			AddForms<RE::TESObjectMISC>();
			AddForms<RE::TESObjectWEAP>();

			for (auto& index : idxMap)
			{
				auto info = modMap.find(index.second);
				if (info != modMap.end())
				{
					pluginList.emplace_back(info->first, info->second);
				}
			}
		}

		static void Reset()
		{
			pluginList.clear();
		}

		static PluginList GetPluginList() noexcept { return pluginList; }

	private:
		static std::uint32_t GetCombinedIndex(const RE::TESFile* a_file)
		{
			return static_cast<std::uint32_t>(a_file->GetCompileIndex() + a_file->GetSmallFileCompileIndex());
		}

		template<class FORM_TYPE>
		static void AddForms()
		{
			auto TESDataHandler = RE::TESDataHandler::GetSingleton();
			for (auto form : TESDataHandler->GetFormArray<FORM_TYPE>())
			{
				auto file = form->GetFile(0);
				if (file && file->compileIndex < 0xFF)
				{
					auto info = modMap.find(GetCombinedIndex(file));
					if (info != modMap.end())
					{
						info->second.AddForm(form);
					}
				}
			}
		}

		static inline std::map<std::uint32_t, PluginInfo> modMap;
		static inline std::map<std::uint32_t, std::uint32_t> idxMap;
		static inline PluginList pluginList;
	};
}
