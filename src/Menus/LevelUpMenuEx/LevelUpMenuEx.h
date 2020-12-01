#pragma once

#include "Forms/PerkManager.h"

void PlayMenuSound(const char* a_soundName)
{
	using func_t = decltype(&PlayMenuSound);
	REL::Relocation<func_t> func{ REL::ID(1227993) };
	return func(a_soundName);
}

namespace Menus
{
	class LevelUpMenuEx :
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

		LevelUpMenuEx()
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

		~LevelUpMenuEx()
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
				InitPerkList();
				break;

			case 3:
				GetPerkCount();
				break;

			case 4:
				GetHeaderText();
				break;

			case 5:
				if ((a_params.argCount == 1) && (a_params.args[0].IsBoolean()))
				{
					auto ControlMap = RE::ControlMap::GetSingleton();
					ControlMap->SetTextEntryMode(a_params.args[0].GetBoolean());
				}
				break;

			case 6:
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
			MapCodeMethodToASFunction("InitPerkList", 2);
			MapCodeMethodToASFunction("GetPerkCount", 3);
			MapCodeMethodToASFunction("UpdateHeader", 4);
			MapCodeMethodToASFunction("SetTextEntry", 5);
			MapCodeMethodToASFunction("AddPerk", 6);
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
					ControlMap->PushInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kLevelupMenu);
					ControlMap->PushInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kLevelupMenuPrevNext);
					RE::SendHUDMessage::PushHUDMode("SpecialMode");

					return RE::UI_MESSAGE_RESULTS::kPassOn;
				}

			case RE::UI_MESSAGE_TYPE::kHide:
				{
					auto ControlMap = RE::ControlMap::GetSingleton();
					ControlMap->PopInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kBasicMenuNav);
					ControlMap->PopInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kThumbNav);
					ControlMap->PopInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kLevelupMenu);
					ControlMap->PopInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kLevelupMenuPrevNext);
					RE::SendHUDMessage::PopHUDMode("SpecialMode");

					auto PipboyManager = RE::PipboyManager::GetSingleton();
					if (PipboyManager)
					{
						PipboyManager->RaisePipboy();
					}

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

		void InitPerkList()
		{
			if (!uiMovie)
			{
				logger::error("uiMovie null.");
				return;
			}

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

	private:
		RE::msvc::unique_ptr<RE::BSGFxShaderFXTarget> Background_mc;
		static inline std::string HeaderText;
		static inline bool IsNewLevel{ false };

	private:
		static void LevelUpMenu__Init()
		{
			const auto UI = RE::UI::GetSingleton();
			if (!UI)
			{
				stl::report_and_fail("LevelUpMenu override failed!");
			}

			UI->RegisterMenu("LevelUpMenu", LevelUpMenuEx::Create);
		}

		static void LevelUpMenu__ShowMenu(bool a_fromPipboy)
		{
			const auto UI = RE::UI::GetSingleton();
			if (a_fromPipboy ||
				((!UI->menuMode) && (!UI->GetMenu("WorkshopMenu")) && (RE::VATS::GetSingleton()->mode == RE::VATS::VATS_MODE_ENUM::kNone)))
			{
				RE::BSUIMessageData::SendUIBoolMessage("LevelUpMenu", RE::UI_MESSAGE_TYPE::kShow, a_fromPipboy);
			}
		}

		static void LevelUpMenu__Trigger(void*)
		{
			LevelUpMenuEx::IsNewLevel = true;
		}

		static RE::IMenu* Create(const RE::UIMessage&)
		{
			return new LevelUpMenuEx();
		}
	};
}