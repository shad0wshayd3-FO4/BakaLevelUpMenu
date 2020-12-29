#pragma once
#include "PerkManager.h"

namespace Menus
{
	class LevelUpMenu :
		public RE::GameMenuBase
	{
	public:
		static void Install()
		{
			REL::Relocation<std::uintptr_t> targetInit{ REL::ID(1564767) };
			REL::Relocation<std::uintptr_t> targetShow{ REL::ID(1436715) };
			REL::Relocation<std::uintptr_t> targetTrig{ REL::ID(997886) };

			auto& trampoline = F4SE::GetTrampoline();
			trampoline.write_branch<5>(targetInit.address(), LevelUpMenu__Init);
			trampoline.write_branch<5>(targetShow.address(), LevelUpMenu__ShowMenu);
			trampoline.write_branch<6>(targetTrig.address(), LevelUpMenu__Trigger);
		}

		static void ShowMenu(bool a_fromPipboy)
		{
			LevelUpMenu__ShowMenu(a_fromPipboy);
		}

		LevelUpMenu()
		{
			menuFlags.set(
				RE::UI_MENU_FLAGS::kPausesGame,
				RE::UI_MENU_FLAGS::kAlwaysOpen,
				RE::UI_MENU_FLAGS::kUsesCursor,
				RE::UI_MENU_FLAGS::kDisablePauseMenu,
				RE::UI_MENU_FLAGS::kUpdateUsesCursor,
				RE::UI_MENU_FLAGS::kCustomRendering,
				RE::UI_MENU_FLAGS::kUsesBlurredBackground,
				RE::UI_MENU_FLAGS::kUsesMovementToDirection);

			depthPriority = 9;

			const auto ScaleformManager = RE::BSScaleformManager::GetSingleton();
			const auto LoadMovieSuccess =
				ScaleformManager->LoadMovieEx(*this, "Interface/LevelUpMenu.swf", "root.Menu_mc");
			assert(LoadMovieSuccess);

			RE::BSGFxObject::make_unique_ptr(FilterHolder_mc, *uiMovie, "root.Menu_mc");
			if (FilterHolder_mc)
			{
				FilterHolder_mc->CreateAndSetFiltersToHUD(RE::HUDColorTypes::kGameplayHUDColor);
				shaderFXObjects.push_back(FilterHolder_mc.get());
			}

			RE::BSGFxObject::make_unique_ptr(Background_mc, *FilterHolder_mc, "Background_mc");
			if (Background_mc)
			{
				Background_mc->EnableShadedBackground(RE::HUDColorTypes::kMenuNoColorBackground);
				shaderFXObjects.push_back(Background_mc.get());
			}

			SetUpButtonBar(*FilterHolder_mc, "ButtonHintBar_mc", RE::HUDColorTypes::kGameplayHUDColor);
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
					PlayMenuSound(a_params.args[0].GetString());
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
				if ((a_params.argCount == 1) && (a_params.args[0].IsBoolean()))
				{
					auto ControlMap = RE::ControlMap::GetSingleton();
					ControlMap->SetTextEntryMode(a_params.args[0].GetBoolean());
				}
				break;

			case 7:
				if ((a_params.argCount == 1) && (a_params.args[0].IsUInt()))
				{
					auto PlayerCharacter = RE::PlayerCharacter::GetSingleton();
					PlayerCharacter->SelectPerk(a_params.args[0].GetUInt());
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
					auto ControlMap = RE::ControlMap::GetSingleton();
					ControlMap->PushInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kBasicMenuNav);
					ControlMap->PushInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kThumbNav);
					ControlMap->PushInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kVirtualController);
					ControlMap->PushInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kLevelUpMenu);
					ControlMap->PushInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kLevelUpMenuPrevNext);
					RE::SendHUDMessage::PushHUDMode("SpecialMode");

					if (LevelUpMenu::FromPipboy)
					{
						if (auto PipboyManager = RE::PipboyManager::GetSingleton(); PipboyManager)
						{
							PipboyManager->LowerPipboy(RE::PipboyManager::LOWER_REASON::kPerkGrid);
						}
					}

					if (LevelUpMenu::IsLoaded)
					{
						NotifyLoaded();
					}

					return RE::UI_MESSAGE_RESULTS::kPassOn;
				}

			case RE::UI_MESSAGE_TYPE::kHide:
				{
					auto ControlMap = RE::ControlMap::GetSingleton();
					ControlMap->PopInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kBasicMenuNav);
					ControlMap->PopInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kThumbNav);
					ControlMap->PopInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kVirtualController);
					ControlMap->PopInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kLevelUpMenu);
					ControlMap->PopInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kLevelUpMenuPrevNext);
					RE::SendHUDMessage::PopHUDMode("SpecialMode");

					if (LevelUpMenu::FromPipboy)
					{
						if (auto PipboyManager = RE::PipboyManager::GetSingleton(); PipboyManager)
						{
							PipboyManager->RaisePipboy();
						}
					}

					LevelUpMenu::FromPipboy = false;
					return RE::UI_MESSAGE_RESULTS::kPassOn;
				}

			default:
				return RE::IMenu::ProcessMessage(a_message);
			}
		}

	private:
		void CloseMenu()
		{
			auto UIMessageQueue = RE::UIMessageQueue::GetSingleton();
			if (UIMessageQueue)
			{
				UIMessageQueue->AddMessage(
					"LevelUpMenu", RE::UI_MESSAGE_TYPE::kHide);
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

			auto perkManager = PerkManager();
			for (auto& perkChain : perkManager.GetPerkChains())
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
			auto PlayerCharacter = RE::PlayerCharacter::GetSingleton();
			if (PlayerCharacter)
			{
				RE::Scaleform::GFx::Value PerkCount[1];
				PerkCount[0] = PlayerCharacter->perkCount;
				menuObj.Invoke("SetPerkCount", nullptr, PerkCount, 1);
			}
		}

		void GetHeaderText()
		{
			auto PlayerCharacter = RE::PlayerCharacter::GetSingleton();
			if (PlayerCharacter)
			{
				auto level = PlayerCharacter->GetLevel();
				HeaderText = IsNewLevel ? fmt::format(Forms::sBakaLevelUpText.GetString(), level) : Forms::sBakaPerkMenu.GetString();
				IsNewLevel = false;

				RE::Scaleform::GFx::Value Header[1];
				Header[0] = HeaderText.c_str();
				menuObj.Invoke("SetHeader", nullptr, Header, 1);
			}
		}

		RE::msvc::unique_ptr<RE::BSGFxShaderFXTarget> Background_mc{ nullptr };
		static inline std::string HeaderText;
		static inline bool FromPipboy{ false };
		static inline bool IsNewLevel{ false };
		static inline bool IsLoaded{ false };

	private:
		static void LevelUpMenu__Init()
		{
			const auto UI = RE::UI::GetSingleton();
			if (!UI)
			{
				stl::report_and_fail("LevelUpMenu override failed!");
			}

			UI->RegisterMenu("LevelUpMenu", LevelUpMenu::Create);
		}

		static void LevelUpMenu__ShowMenu(bool a_fromPipboy)
		{
			const auto UI = RE::UI::GetSingleton();
			if (a_fromPipboy || ((!UI->menuMode) && (!UI->GetMenu("WorkshopMenu")) && (RE::VATS::GetSingleton()->mode == RE::VATS::VATS_MODE_ENUM::kNone)))
			{
				auto UIMessageQueue = RE::UIMessageQueue::GetSingleton();
				if (UIMessageQueue)
				{
					LevelUpMenu::FromPipboy = a_fromPipboy;
					UIMessageQueue->AddMessage(
						"LevelUpMenu", RE::UI_MESSAGE_TYPE::kShow);
				}
			}
		}

		static void LevelUpMenu__Trigger(void*)
		{
			LevelUpMenu::IsNewLevel = true;
		}

		static RE::IMenu* Create(const RE::UIMessage&)
		{
			return new LevelUpMenu();
		}
	};
}