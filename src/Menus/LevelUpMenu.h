#pragma once

#include "PerkManager.h"
#include "Translations/Translations.h"

namespace Menus
{
	class LevelUpMenu :
		public RE::GameMenuBase
	{
	public:
		static void Install()
		{
			hkRegisterMenu<740126, 0x43>::InstallC();
			hkRegisterMenu<1564767, 0x3B>::InstallJ();
			hkSendUIBoolMessage<1436715, 0x70>::Install();
		}

		LevelUpMenu()
		{
			menuFlags.set(
				RE::UI_MENU_FLAGS::kPausesGame,
				RE::UI_MENU_FLAGS::kUsesCursor,
				RE::UI_MENU_FLAGS::kDisablePauseMenu,
				RE::UI_MENU_FLAGS::kUpdateUsesCursor,
				RE::UI_MENU_FLAGS::kCustomRendering,
				RE::UI_MENU_FLAGS::kUsesBlurredBackground,
				RE::UI_MENU_FLAGS::kUsesMovementToDirection);
			menuHUDMode = "SpecialMode";
			depthPriority = RE::UI_DEPTH_PRIORITY::kTerminal;

			const auto ScaleformManager = RE::BSScaleformManager::GetSingleton();
			[[maybe_unused]] const auto LoadMovieSuccess =
				ScaleformManager->LoadMovieEx(*this, "Interface/LevelUpMenu.swf"sv, "root.Menu_mc");
			assert(LoadMovieSuccess);

			filterHolder = RE::msvc::make_unique<RE::BSGFxShaderFXTarget>(*uiMovie, "root.Menu_mc");
			if (filterHolder)
			{
				filterHolder->CreateAndSetFiltersToHUD(RE::HUDColorTypes::kPlayerSetColor);
				shaderFXObjects.push_back(filterHolder.get());
			}

			Background_mc = RE::msvc::make_unique<RE::BSGFxShaderFXTarget>(*filterHolder, "Background_mc");
			if (Background_mc)
			{
				Background_mc->EnableShadedBackground(RE::HUDColorTypes::kMenuNoColorBackground);
				shaderFXObjects.push_back(Background_mc.get());
			}

			SetUpButtonBar(*filterHolder, "ButtonHintBar_mc", RE::HUDColorTypes::kPlayerSetColor);
		}

		~LevelUpMenu()
		{
			Background_mc.release();
		}

		virtual void Call(const Params& a_params) override
		{
			switch ((*((std::uint32_t*)&(a_params.userData))))
			{
			case 0:
				CloseMenu();
				break;

			case 1:
				if (a_params.argCount == 1 && a_params.args[0].IsString())
				{
					RE::UIUtils::PlayMenuSound(a_params.args[0].GetString());
				}
				break;

			case 2:
				NotifyLoaded();
				break;

			case 3:
				InitPerkList();
				break;

			case 4:
				GetPerkCount();
				break;

			case 5:
				GetHeaderText();
				break;

			case 6:
				if (a_params.argCount == 1 && a_params.args[0].IsBoolean())
				{
					auto ControlMap = RE::ControlMap::GetSingleton();
					ControlMap->SetTextEntryMode(a_params.args[0].GetBoolean());
				}
				break;

			case 7:
				if (a_params.argCount == 1 && a_params.args[0].IsUInt())
				{
					SelectPerk(a_params.args[0].GetUInt());
				}
				break;

			default:
				break;
			}
		}

		virtual void MapCodeObjectFunctions() override
		{
			MapCodeMethodToASFunction("CloseMenu", 0);
			MapCodeMethodToASFunction("PlaySound", 1);
			MapCodeMethodToASFunction("NotifyLoaded", 2);
			MapCodeMethodToASFunction("InitPerkList", 3);
			MapCodeMethodToASFunction("GetPerkCount", 4);
			MapCodeMethodToASFunction("UpdateHeader", 5);
			MapCodeMethodToASFunction("SetTextEntry", 6);
			MapCodeMethodToASFunction("AddPerk", 7);
		}

		virtual RE::UI_MESSAGE_RESULTS ProcessMessage(RE::UIMessage& a_message) override
		{
			switch (*a_message.type)
			{
			case RE::UI_MESSAGE_TYPE::kShow:
			{
				if (auto ControlMap = RE::ControlMap::GetSingleton())
				{
					ControlMap->PushInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kBasicMenuNav);
					ControlMap->PushInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kThumbNav);
					ControlMap->PushInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kVirtualController);
					ControlMap->PushInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kLevelUpMenu);
					ControlMap->PushInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kLevelUpMenuPrevNext);
				}

				if (LevelUpMenu::FromPipboy)
				{
					if (auto PipboyManager = RE::PipboyManager::GetSingleton())
					{
						PipboyManager->LowerPipboy(RE::PipboyManager::LOWER_REASON::kPerkGrid);
					}
				}

				if (LevelUpMenu::IsLoaded)
				{
					NotifyLoaded();
				}

				return RE::UI_MESSAGE_RESULTS::kHandled;
			}

			case RE::UI_MESSAGE_TYPE::kHide:
			{
				if (auto ControlMap = RE::ControlMap::GetSingleton())
				{
					ControlMap->PopInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kBasicMenuNav);
					ControlMap->PopInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kThumbNav);
					ControlMap->PopInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kVirtualController);
					ControlMap->PopInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kLevelUpMenu);
					ControlMap->PopInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kLevelUpMenuPrevNext);
				}

				if (LevelUpMenu::FromPipboy)
				{
					if (auto PipboyManager = RE::PipboyManager::GetSingleton())
					{
						PipboyManager->RaisePipboy();
					}
				}

				LevelUpMenu::FromPipboy = false;
				return RE::UI_MESSAGE_RESULTS::kHandled;
			}

			default:
				return RE::IMenu::ProcessMessage(a_message);
			}
		}

	private:
		static RE::IMenu* Create(const RE::UIMessage&)
		{
			return new LevelUpMenu();
		}

		void CloseMenu()
		{
			if (auto UIMessageQueue = RE::UIMessageQueue::GetSingleton())
			{
				UIMessageQueue->AddMessage(
					"LevelUpMenu"sv,
					RE::UI_MESSAGE_TYPE::kHide);
			}
		}

		void NotifyLoaded()
		{
			LevelUpMenu::IsLoaded = true;
			menuObj.Invoke("RefreshDisplay");
		}

		void InitPerkList()
		{
			RE::Scaleform::GFx::Value PerkList[1];
			uiMovie->CreateArray(&PerkList[0]);

			for (auto& perkChain : PerkManager().GetPerkChains())
			{
				auto perkIndex = perkChain.GetFirstAvailableRank();
				if (perkIndex.second == -1)
				{
					continue;
				}

				RE::Scaleform::GFx::Value descs, paths;
				uiMovie->CreateArray(&descs);
				uiMovie->CreateArray(&paths);

				for (auto& _perk : perkChain.Get())
				{
					descs.PushBack(_perk.GetConditionText().data());
					paths.PushBack(_perk.GetPerkIcon().data());
				}

				RE::Scaleform::GFx::Value listEntry;
				uiMovie->CreateObject(&listEntry);
				listEntry.SetMember("text", perkIndex.first.GetName().data());
				listEntry.SetMember("RankDescs", descs);
				listEntry.SetMember("IconPaths", paths);
				listEntry.SetMember("PerkLevel", perkIndex.first.GetPerkLevel());
				listEntry.SetMember("RankCount", perkIndex.first->data.numRanks);
				listEntry.SetMember("RankIndex", perkIndex.second);
				listEntry.SetMember("IsAvailable", perkIndex.first.IsAvailable());
				listEntry.SetMember("IsSelected", false);
				listEntry.SetMember("FormID", perkIndex.first->formID);
				PerkList[0].PushBack(listEntry);
			}

			menuObj.Invoke("SetPerkList", nullptr, PerkList, 1);
		}

		void GetPerkCount()
		{
			if (auto PlayerCharacter = RE::PlayerCharacter::GetSingleton())
			{
				RE::Scaleform::GFx::Value PerkCount[1];
				PerkCount[0] = PlayerCharacter->perkCount;
				menuObj.Invoke("SetPerkCount", nullptr, PerkCount, 1);
			}
		}

		void GetHeaderText()
		{
			if (auto PlayerCharacter = RE::PlayerCharacter::GetSingleton())
			{
				auto level = PlayerCharacter->GetLevel();
				HeaderText = IsNewLevel ? std::vformat(Translations::Formatting::LevelUpText, std::make_format_args(level))
				                        : Translations::Formatting::PerkMenu;
				IsNewLevel = false;

				RE::Scaleform::GFx::Value Header[1];
				Header[0] = HeaderText.c_str();
				menuObj.Invoke("SetHeader", nullptr, Header, 1);
			}
		}

		void SelectPerk(std::uint32_t a_perkID)
		{
			if (auto form = RE::TESForm::GetFormByID(a_perkID))
			{
				if (auto perk = form->As<RE::BGSPerk>())
				{
					auto PlayerCharacter = RE::PlayerCharacter::GetSingleton();
					PlayerCharacter->AddPerk(perk);
					PlayerCharacter->perkCount -= 1;

					if (auto EventSource = RE::PerkPointIncreaseEvent::GetEventSource())
					{
						EventSource->Notify(PlayerCharacter->perkCount);
					}
				}
			}
		}

		RE::msvc::unique_ptr<RE::BSGFxShaderFXTarget> Background_mc{ nullptr };
		inline static std::string HeaderText;
		inline static bool FromPipboy{ false };
		inline static bool IsNewLevel{ false };
		inline static bool IsLoaded{ false };

	protected:
		template <std::uint64_t ID, std::ptrdiff_t OFF>
		class hkRegisterMenu
		{
		public:
			static void InstallC()
			{
				static REL::Relocation<std::uintptr_t> target{ REL::ID(ID), OFF };
				auto& trampoline = F4SE::GetTrampoline();
				_RegisterMenu = trampoline.write_call<5>(target.address(), RegisterMenu);
			}

			static void InstallJ()
			{
				static REL::Relocation<std::uintptr_t> target{ REL::ID(ID), OFF };
				auto& trampoline = F4SE::GetTrampoline();
				_RegisterMenu = trampoline.write_branch<5>(target.address(), RegisterMenu);
			}

		private:
			using Create_t = RE::IMenu* (*)(const RE::UIMessage&);
			using Update_t = void (*)();

			static void RegisterMenu(
				[[maybe_unused]] RE::UI* a_this,
				[[maybe_unused]] const char* a_menu,
				[[maybe_unused]] Create_t a_create,
				[[maybe_unused]] Update_t a_update)
			{
				return _RegisterMenu(a_this, a_menu, LevelUpMenu::Create, nullptr);
			}

			inline static REL::Relocation<decltype(&RegisterMenu)> _RegisterMenu;
		};

		template <std::uint64_t ID, std::ptrdiff_t OFF>
		class hkSendUIBoolMessage
		{
		public:
			static void Install()
			{
				static REL::Relocation<std::uintptr_t> target{ REL::ID(ID), OFF };
				auto& trampoline = F4SE::GetTrampoline();
				trampoline.write_call<5>(target.address(), SendUIBoolMessage);
			}

		private:
			static void SendUIBoolMessage(
				[[maybe_unused]] const RE::BSFixedString& a_menu,
				[[maybe_unused]] RE::UI_MESSAGE_TYPE a_type,
				[[maybe_unused]] bool a_fromPipboy)
			{
				if (auto UIMessageQueue = RE::UIMessageQueue::GetSingleton())
				{
					LevelUpMenu::FromPipboy = a_fromPipboy;
					UIMessageQueue->AddMessage(
						"LevelUpMenu"sv,
						RE::UI_MESSAGE_TYPE::kShow);
				}
			}
		};
	};
}
