#pragma once
#include "Forms\Forms.h"
#include "Menus\Utils\Utils.h"
#include "PluginExplorer.h"

namespace Menus
{
	class PluginExplorerMenu :
		public RE::GameMenuBase
	{
	public:
		PluginExplorerMenu()
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
				ScaleformManager->LoadMovieEx(*this, "Interface/PluginExplorerMenu.swf", "root.Menu_mc");
			assert(LoadMovieSuccess);

			filterHolder = RE::msvc::make_unique<RE::BSGFxShaderFXTarget>(*uiMovie, "root.Menu_mc");
			if (filterHolder)
			{
				filterHolder->CreateAndSetFiltersToHUD(RE::HUDColorTypes::kGameplayHUDColor);
				shaderFXObjects.push_back(filterHolder.get());
			}

			LPaneBackground_mc = RE::msvc::make_unique<RE::BSGFxShaderFXTarget>(*filterHolder, "LeftPane_mc.Background_mc");
			if (LPaneBackground_mc)
			{
				LPaneBackground_mc->EnableShadedBackground(RE::HUDColorTypes::kMenuNoColorBackground);
				shaderFXObjects.push_back(LPaneBackground_mc.get());
			}

			RPaneBackground_mc = RE::msvc::make_unique<RE::BSGFxShaderFXTarget>(*filterHolder, "RightPane_mc.Background_mc");
			if (RPaneBackground_mc)
			{
				RPaneBackground_mc->EnableShadedBackground(RE::HUDColorTypes::kMenuNoColorBackground);
				shaderFXObjects.push_back(RPaneBackground_mc.get());
			}

			TPaneBackground_mc = RE::msvc::make_unique<RE::BSGFxShaderFXTarget>(*filterHolder, "CategoryBar_mc.BackerBar_mc");
			if (TPaneBackground_mc)
			{
				TPaneBackground_mc->EnableShadedBackground(RE::HUDColorTypes::kMenuNoColorBackground);
				shaderFXObjects.push_back(TPaneBackground_mc.get());
			}

			SetUpButtonBar(*filterHolder, "ButtonHintBar_mc", RE::HUDColorTypes::kGameplayHUDColor);
		}

		virtual ~PluginExplorerMenu()
		{
			LPaneBackground_mc.release();
			RPaneBackground_mc.release();
			TPaneBackground_mc.release();
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
				InitPluginList();
				break;

			case 4:
				if ((a_params.argCount == 1) && (a_params.args[0].IsBoolean()))
				{
					auto ControlMap = RE::ControlMap::GetSingleton();
					ControlMap->SetTextEntryMode(a_params.args[0].GetBoolean());
				}
				break;

			case 5:
				if ((a_params.argCount == 2) && a_params.args[0].IsUInt() && a_params.args[1].IsInt())
				{
					auto object = RE::TESForm::GetFormByID(a_params.args[0].GetUInt())->As<RE::TESBoundObject>();
					if (object)
					{
						auto PlayerCharacter = RE::PlayerCharacter::GetSingleton();
						PlayerCharacter->AddObjectToContainer(object, nullptr, a_params.args[1].GetInt(), nullptr, RE::ITEM_REMOVE_REASON::kNone);
					}
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
			MapCodeMethodToASFunction("InitPluginList", 3);
			MapCodeMethodToASFunction("SetTextEntry", 4);
			MapCodeMethodToASFunction("AddItem", 5);
		}

		virtual void HandleEvent(const RE::ButtonEvent* a_event) override
		{
			if (menuObj.IsObject() && menuObj.HasMember("ProcessUserEvent"))
			{
				if (!a_event->disabled && inputEventHandlingEnabled)
				{
					RE::Scaleform::GFx::Value args[2];
					args[0] = "DISABLED";
					args[1] = a_event->QJustPressed();

					switch (a_event->GetBSButtonCode())
					{
					case RE::BS_BUTTON_CODE::kTab:
					case RE::BS_BUTTON_CODE::kBButton:
						args[0] = a_event->QUserEvent() != "NextFocus" ? "Cancel" : "DISABLED"; // dumb workaround
						break;

					case RE::BS_BUTTON_CODE::kEnter:
					case RE::BS_BUTTON_CODE::kE:
					case RE::BS_BUTTON_CODE::kAButton:
						args[0] = "Accept";
						break;

					case RE::BS_BUTTON_CODE::kZ:
					case RE::BS_BUTTON_CODE::kLControl:
					case RE::BS_BUTTON_CODE::kLShoulder:
						args[0] = "Prev";
						break;

					case RE::BS_BUTTON_CODE::kC:
					case RE::BS_BUTTON_CODE::kLAlt:
					case RE::BS_BUTTON_CODE::kRShoulder:
						args[0] = "Next";
						break;

					case RE::BS_BUTTON_CODE::kQ:
					case RE::BS_BUTTON_CODE::kLStick:
						args[0] = "Search";
						break;
					}

					menuObj.Invoke("ProcessUserEvent", nullptr, args, 2);
				}
			}
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
					RE::SendHUDMessage::PushHUDMode("SpecialMode");

					return RE::UI_MESSAGE_RESULTS::kPassOn;
				}

			case RE::UI_MESSAGE_TYPE::kHide:
				{
					auto ControlMap = RE::ControlMap::GetSingleton();
					ControlMap->PopInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kBasicMenuNav);
					ControlMap->PopInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kThumbNav);
					ControlMap->PopInputContext(RE::UserEvents::INPUT_CONTEXT_ID::kVirtualController);
					RE::SendHUDMessage::PopHUDMode("SpecialMode");

					return RE::UI_MESSAGE_RESULTS::kPassOn;
				}

			default:
				return RE::IMenu::ProcessMessage(a_message);
			}
		}

		static RE::IMenu* Create(const RE::UIMessage&)
		{
			return new PluginExplorerMenu();
		}

	private:
		void CloseMenu()
		{
			auto UIMessageQueue = RE::UIMessageQueue::GetSingleton();
			if (UIMessageQueue)
			{
				UIMessageQueue->AddMessage(
					"PluginExplorerMenu", RE::UI_MESSAGE_TYPE::kHide);
			}
		}

		void NotifyLoaded()
		{
			IsLoaded = true;
			menuObj.Invoke("RefreshDisplay");
		}

		void InitPluginList()
		{
			RE::Scaleform::GFx::Value PluginList[1];
			uiMovie->CreateArray(&PluginList[0]);

			for (auto& iter : PluginExplorer::GetPluginList())
			{
				if (iter.second.GetCount() == 0)
				{
					continue;
				}

				auto ProcessForms = [&](RE::Scaleform::GFx::Value& a_value, const PluginExplorer::PluginInfo::FormMap a_map)
				{
					uiMovie->CreateArray(&a_value);
					for (auto& entry : a_map)
					{
						auto textFormID = fmt::format("[{:08X}]", entry.first);

						RE::Scaleform::GFx::Value listEntry;
						uiMovie->CreateObject(&listEntry);
						listEntry.SetMember("text", entry.second.data());
						listEntry.SetMember("textFormID", textFormID.data());
						listEntry.SetMember("FormID", entry.first);
						a_value.PushBack(listEntry);
					}
				};

				auto ProcessMISCs = [&](RE::Scaleform::GFx::Value& a_misc, RE::Scaleform::GFx::Value& a_junk, RE::Scaleform::GFx::Value& a_mods, const PluginExplorer::PluginInfo::FormMap a_map)
				{
					uiMovie->CreateArray(&a_misc);
					uiMovie->CreateArray(&a_junk);
					uiMovie->CreateArray(&a_mods);
					for (auto& entry : a_map)
					{
						auto textFormID = fmt::format("[{:08X}]", entry.first);
						if (auto form = RE::TESForm::GetFormByID(entry.first); form)
						{
							if (auto misc = form->As<RE::TESObjectMISC>(); misc)
							{
								RE::Scaleform::GFx::Value listEntry;
								uiMovie->CreateObject(&listEntry);
								listEntry.SetMember("text", entry.second.data());
								listEntry.SetMember("textFormID", textFormID.data());
								listEntry.SetMember("FormID", entry.first);

								if (misc->componentData && misc->componentData->size() > 0)
								{
									a_junk.PushBack(listEntry);
								}
								else if (misc->IsLooseMod())
								{
									a_mods.PushBack(listEntry);
								}
								else
								{
									a_misc.PushBack(listEntry);
								}
							}
						}
					}
				};

				RE::Scaleform::GFx::Value WEAP, ARMO, ALCH, MISC, JUNK, MODS, HOLO, BOOK, KEYS, AMMO;
				ProcessForms(WEAP, iter.second.GetMapWEAP());
				ProcessForms(ARMO, iter.second.GetMapARMO());
				ProcessForms(ALCH, iter.second.GetMapALCH());
				ProcessForms(HOLO, iter.second.GetMapHOLO());
				ProcessForms(BOOK, iter.second.GetMapBOOK());
				ProcessForms(KEYS, iter.second.GetMapKEYS());
				ProcessForms(AMMO, iter.second.GetMapAMMO());
				ProcessMISCs(MISC, JUNK, MODS, iter.second.GetMapMISC());

				std::string pluginIndex =
					(iter.first >= 0xFE) ?
						fmt::format("[FE][{:03X}]", iter.first - 0xFE) :
						fmt::format("[{:02X}]", iter.first);

				RE::Scaleform::GFx::Value listEntry;
				uiMovie->CreateObject(&listEntry);
				listEntry.SetMember("text", iter.second.GetName().data());
				listEntry.SetMember("textFormID", pluginIndex.data());
				listEntry.SetMember("WEAP", WEAP);
				listEntry.SetMember("ARMO", ARMO);
				listEntry.SetMember("ALCH", ALCH);
				listEntry.SetMember("MISC", MISC);
				listEntry.SetMember("JUNK", JUNK);
				listEntry.SetMember("MODS", MODS);
				listEntry.SetMember("HOLO", HOLO);
				listEntry.SetMember("BOOK", BOOK);
				listEntry.SetMember("KEYS", KEYS);
				listEntry.SetMember("AMMO", AMMO);
				PluginList[0].PushBack(listEntry);
			}

			menuObj.Invoke("SetPluginList", nullptr, PluginList, 1);
		}

		RE::msvc::unique_ptr<RE::BSGFxShaderFXTarget> LPaneBackground_mc{ nullptr };
		RE::msvc::unique_ptr<RE::BSGFxShaderFXTarget> RPaneBackground_mc{ nullptr };
		RE::msvc::unique_ptr<RE::BSGFxShaderFXTarget> TPaneBackground_mc{ nullptr };
		static inline bool IsLoaded{ false };
	};
}