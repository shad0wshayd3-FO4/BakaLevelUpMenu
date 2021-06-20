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

			stl::asm_replace(targetInit.address(), 0x40, reinterpret_cast<std::uintptr_t>(LevelUpMenu__Init));
			stl::asm_replace(targetShow.address(), 0xA3, reinterpret_cast<std::uintptr_t>(LevelUpMenu__ShowMenu));
			stl::asm_replace(targetTrig.address(), 0x10, reinterpret_cast<std::uintptr_t>(LevelUpMenu__Trigger));
		}

		static void ShowMenu(bool a_fromPipboy)
		{
			LevelUpMenu__ShowMenu(a_fromPipboy);
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

			depthPriority = 9;

			const auto ScaleformManager = RE::BSScaleformManager::GetSingleton();
			const auto LoadMovieSuccess =
				ScaleformManager->LoadMovieEx(*this, "Interface/LevelUpMenu.swf", "root.Menu_mc");
			assert(LoadMovieSuccess);

			filterHolder = RE::msvc::make_unique<RE::BSGFxShaderFXTarget>(*uiMovie, "root.Menu_mc");
			if (filterHolder)
			{
				filterHolder->CreateAndSetFiltersToHUD(RE::HUDColorTypes::kGameplayHUDColor);
				shaderFXObjects.push_back(filterHolder.get());
			}

			Background_mc = RE::msvc::make_unique<RE::BSGFxShaderFXTarget>(*filterHolder, "Background_mc");
			if (Background_mc)
			{
				Background_mc->EnableShadedBackground(RE::HUDColorTypes::kMenuNoColorBackground);
				shaderFXObjects.push_back(Background_mc.get());
			}

			SetUpButtonBar(*filterHolder, "ButtonHintBar_mc", RE::HUDColorTypes::kGameplayHUDColor);
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
					if ((a_params.argCount == 1) && (a_params.args[0].IsBoolean()))
					{
						auto ControlMap = RE::ControlMap::GetSingleton();
						ControlMap->SetTextEntryMode(a_params.args[0].GetBoolean());
					}
					break;

				case 7:
					if ((a_params.argCount == 1) && (a_params.args[0].IsUInt()))
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
					"LevelUpMenu",
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
			if (auto PlayerCharacter = RE::PlayerCharacter::GetSingleton(); PlayerCharacter)
			{
				RE::Scaleform::GFx::Value PerkCount[1];
				PerkCount[0] = PlayerCharacter->perkCount;
				menuObj.Invoke("SetPerkCount", nullptr, PerkCount, 1);
			}
		}

		void GetHeaderText()
		{
			if (auto PlayerCharacter = RE::PlayerCharacter::GetSingleton(); PlayerCharacter)
			{
				auto level = PlayerCharacter->GetLevel();
				HeaderText = IsNewLevel ? fmt::format(Forms::sBakaLevelUpText.GetString(), level) : Forms::sBakaPerkMenu.GetString();
				IsNewLevel = false;

				RE::Scaleform::GFx::Value Header[1];
				Header[0] = HeaderText.c_str();
				menuObj.Invoke("SetHeader", nullptr, Header, 1);
			}
		}

		void SelectPerk(std::uint32_t a_perkID)
		{
			if (auto form = RE::TESForm::GetFormByID(a_perkID); form)
			{
				if (auto perk = form->As<RE::BGSPerk>(); perk)
				{
					auto PlayerCharacter = RE::PlayerCharacter::GetSingleton();
					PlayerCharacter->AddPerk(perk);
					PlayerCharacter->perkCount -= 1;

					// Notify
					auto evn = RE::PerkPointIncreaseEvent::GetEventSource();
					if (evn)
					{
						evn->Notify(PlayerCharacter->perkCount);
					}
				}
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
						"LevelUpMenu",
						RE::UI_MESSAGE_TYPE::kShow);
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
