#pragma once

#include "Menus/Utils/Utils.h"

namespace Menus
{
	namespace
	{
		class ConfirmTradeCallback :
			public RE::IMessageBoxCallback
		{
		public:
			ConfirmTradeCallback(RE::BarterMenu* a_menu) :
				menu(a_menu)
			{}

			// override
			virtual void operator()(std::uint8_t a_buttonIdx) override
			{
				if (a_buttonIdx == 0)
				{
					menu->CompleteTrade();
				}
				menu->SetMessageBoxMode(false);
				menu->confirmingTrade = false;
			}

			// members
			RE::BarterMenu* menu{ nullptr };
		};

		class CancelTradeCallback :
			public RE::IMessageBoxCallback
		{
		public:
			CancelTradeCallback(RE::BarterMenu* a_menu, bool a_closeMenu = false) :
				menu(a_menu), closeMenu(a_closeMenu)
			{}

			// override
			virtual void operator()(std::uint8_t a_buttonIdx) override
			{
				if (a_buttonIdx == 0)
				{
					if (closeMenu)
					{
						if (auto UIMessageQueue = RE::UIMessageQueue::GetSingleton(); UIMessageQueue)
						{
							UIMessageQueue->AddMessage(
								"BarterMenu",
								RE::UI_MESSAGE_TYPE::kHide);
						}
					}
					else
					{
						menu->ClearTradingData();
					}
				}
				menu->SetMessageBoxMode(false);
				menu->confirmingTrade = false;
			}

			// members
			RE::BarterMenu* menu{ nullptr };
			bool closeMenu{ false };
		};
	}

	class BarterMenu
	{
	public:
		static void Install()
		{
			REL::Relocation<std::uintptr_t> targetCTOR{ REL::ID(901312), 0x34 };
			REL::Relocation<std::uintptr_t> targetDTOR{ REL::ID(1405609), 0x10F };
			REL::Relocation<std::uintptr_t> targetVTBL_0{ RE::BarterMenu::VTABLE[0] };
			REL::Relocation<std::uintptr_t> targetVTBL_1{ RE::BarterMenu::VTABLE[1] };

			auto& trampoline = F4SE::GetTrampoline();
			_BarterMenu__CTOR = trampoline.write_call<5>(targetCTOR.address(), BarterMenu__CTOR);
			_BarterMenu__DTOR = trampoline.write_branch<5>(targetDTOR.address(), BarterMenu__DTOR);
			_BarterMenu__Call = targetVTBL_0.write_vfunc(0x01, reinterpret_cast<std::uintptr_t>(BarterMenu__Call));
			_BarterMenu__OnButtonEventRelease = targetVTBL_0.write_vfunc(0x0F, reinterpret_cast<std::uintptr_t>(BarterMenu__OnButtonEventRelease));
			targetVTBL_1.write_vfunc(0x08, reinterpret_cast<std::uintptr_t>(BarterMenu__HandleEvent));
		}

		inline static RE::msvc::unique_ptr<RE::BSGFxShaderFXTarget> CategoryBar_mc;
		inline static RE::msvc::unique_ptr<RE::BSGFxShaderFXTarget> CategoryBarBackground_mc;

	private:
		static RE::ContainerMenuBase* BarterMenu__CTOR(RE::ContainerMenuBase* a_this, const char* a_movieName)
		{
			_BarterMenu__CTOR(a_this, a_movieName);

			CategoryBar_mc = RE::msvc::make_unique<RE::BSGFxShaderFXTarget>(*a_this->filterHolder, "Menu_mc.CategoryBar_mc");
			if (CategoryBar_mc)
			{
				CategoryBar_mc->CreateAndSetFiltersToHUD(RE::HUDColorTypes::kGameplayHUDColor);
				a_this->shaderFXObjects.push_back(CategoryBar_mc.get());

				CategoryBarBackground_mc = RE::msvc::make_unique<RE::BSGFxShaderFXTarget>(*CategoryBar_mc, "BackerBar_mc");
				if (CategoryBarBackground_mc)
				{
					CategoryBarBackground_mc->EnableShadedBackground(RE::HUDColorTypes::kMenuNoColorBackground);
					a_this->shaderFXObjects.push_back(CategoryBarBackground_mc.get());
				}
			}

			if (auto Interface3D = RE::Interface3D::Renderer::GetByName("Container3D"sv); Interface3D)
			{
				Interface3D->postfx = RE::Interface3D::PostEffect::kNone;
			}

			if (auto UIMessageQueue = RE::UIMessageQueue::GetSingleton(); UIMessageQueue)
			{
				UIMessageQueue->AddMessage("VignetteMenu"sv, RE::UI_MESSAGE_TYPE::kHide);
			}

			if (auto CanDisplayNextHUDMessage = RE::CanDisplayNextHUDMessage::GetEventSource(); CanDisplayNextHUDMessage)
			{
				CanDisplayNextHUDMessage->Notify(false);
			}

			return a_this;
		}

		static void BarterMenu__DTOR(RE::ContainerMenuBase* a_this)
		{
			_BarterMenu__DTOR(a_this);

			CategoryBarBackground_mc.release();
			CategoryBar_mc.release();

			if (auto CanDisplayNextHUDMessage = RE::CanDisplayNextHUDMessage::GetEventSource(); CanDisplayNextHUDMessage)
			{
				CanDisplayNextHUDMessage->Notify(true);
			}
		}

		static void BarterMenu__Call(RE::BarterMenu* a_this, const RE::Scaleform::GFx::FunctionHandler::Params& a_params)
		{
			switch (reinterpret_cast<std::uint64_t>(a_params.userData))
			{
				case 3:  // Show3D
					if (a_params.argCount == 2 && a_params.args[0].IsInt() && a_params.args[1].IsBoolean())
					{
						if (a_params.args[0].GetInt() == -1)
						{
							a_this->inv3DModelManager.ClearModel();
							break;
						}

						_BarterMenu__Call(a_this, a_params);
					}
					break;

				case 4:  // ExitMenu
					{
						if (a_this->barteredItems.size() > 0)
						{
							if (auto MessageMenuManager = RE::MessageMenuManager::GetSingleton(); MessageMenuManager)
							{
								auto mbCallback = new CancelTradeCallback(a_this, true);
								MessageMenuManager->Create(
									"",
									"$CancelTradeInProgress",
									mbCallback,
									RE::WARNING_TYPES::kInGameMessage,
									"$OK",
									"$Cancel");
								a_this->SetMessageBoxMode(true);
							}
						}
						else
						{
							if (auto UIMessageQueue = RE::UIMessageQueue::GetSingleton(); UIMessageQueue)
							{
								UIMessageQueue->AddMessage(
									"BarterMenu",
									RE::UI_MESSAGE_TYPE::kHide);
							}
						}
					}
					break;

				case 15:  // UpdateSortButtonLabel
					if (a_params.argCount == 1 && a_params.args[0].IsUInt())
					{
						auto currentTab = a_params.args[0].GetUInt();
						a_this->containerItemSorter.SetTab(currentTab);
						a_this->playerItemSorter.SetTab(currentTab);

						switch (a_this->playerItemSorter.currentSort[currentTab].get())
						{
							case Utils::SORT_ON_FIELD::kAlphabetical:
								a_this->menuObj.SetMember("sortButtonLabel"sv, "$SORT");
								break;
							case Utils::SORT_ON_FIELD::kDamage:
								a_this->menuObj.SetMember("sortButtonLabel"sv, "$SORT_DMG");
								break;
							case Utils::SORT_ON_FIELD::kRateOfFire:
								a_this->menuObj.SetMember("sortButtonLabel"sv, "$SORT_ROF");
								break;
							case Utils::SORT_ON_FIELD::kRange:
								a_this->menuObj.SetMember("sortButtonLabel"sv, "$SORT_RNG");
								break;
							case Utils::SORT_ON_FIELD::kAccuracy:
								a_this->menuObj.SetMember("sortButtonLabel"sv, "$SORT_ACC");
								break;
							case Utils::SORT_ON_FIELD::kValue:
								a_this->menuObj.SetMember("sortButtonLabel"sv, "$SORT_VAL");
								break;
							case Utils::SORT_ON_FIELD::kWeight:
								a_this->menuObj.SetMember("sortButtonLabel"sv, "$SORT_WT");
								break;
							default:
								logger::error("Unhandled sort type"sv);
								break;
						}
					}
					break;

				case 16:  // SortItems
					if (a_params.argCount == 2 && a_params.args[0].IsUInt() && a_params.args[1].IsBoolean())
					{
						auto currentTab = a_params.args[0].GetUInt();
						a_this->containerItemSorter.SetTab(currentTab);
						a_this->playerItemSorter.SetTab(currentTab);

						if (a_params.args[1].GetBoolean())
						{
							Utils::ContainerMenuBase__IncrementSort(&a_this->containerItemSorter);
							Utils::ContainerMenuBase__IncrementSort(&a_this->playerItemSorter);
						}

						a_this->UpdateList(true);
						a_this->UpdateList(false);
						a_this->menuObj.Invoke("InvalidateLists");
					}
					break;

				case 18:  // TradeAccept
					{
						RE::Scaleform::GFx::Value isValidTrade;
						a_this->menuObj.GetMember("isValidTrade", &isValidTrade);

						if (isValidTrade.GetBoolean())
						{
							a_this->confirmingTrade = true;

							auto PlayerCaps = a_this->GetCapsOwedByPlayer();
							auto BarterCaps =
								a_this->containerRef.get()
									? a_this->containerRef.get()->GetGoldAmount()
									: static_cast<std::int64_t>(0);

							auto mbMessage =
								(PlayerCaps >= 0 || BarterCaps >= -PlayerCaps)
									? "$ConfirmTrade"
									: "$VendorCapsTooLow";

							if (auto MessageMenuManager = RE::MessageMenuManager::GetSingleton(); MessageMenuManager)
							{
								auto mbCallback = new ConfirmTradeCallback(a_this);
								MessageMenuManager->Create(
									"",
									mbMessage,
									mbCallback,
									RE::WARNING_TYPES::kInGameMessage,
									"$OK",
									"$Cancel");
								a_this->SetMessageBoxMode(true);
							}
						}
					}
					break;

				case 19:  // TradeReset
					{
						if (a_this->barteredItems.size() > 0)
						{
							if (auto MessageMenuManager = RE::MessageMenuManager::GetSingleton(); MessageMenuManager)
							{
								auto mbCallback = new CancelTradeCallback(a_this);
								MessageMenuManager->Create(
									"",
									"$CancelTradeInProgress",
									mbCallback,
									RE::WARNING_TYPES::kInGameMessage,
									"$OK",
									"$Cancel");
								a_this->SetMessageBoxMode(true);
							}
						}
					}
					break;

#ifdef _DEBUG
				case 99:  // PauseForDebugging
					logger::debug("Breakpoint!"sv);
					break;
#endif

				default:
					_BarterMenu__Call(a_this, a_params);
					break;
			}
		}

		static void BarterMenu__HandleEvent(RE::BSInputEventUser* a_this, const RE::ButtonEvent* a_event)
		{
			auto menu = RE::fallout_cast<RE::BarterMenu*>(a_this);
			if (menu && menu->menuObj.IsObject() && menu->menuObj.HasMember("ProcessUserEvent"))
			{
				if (!a_event->disabled && menu->inputEventHandlingEnabled)
				{
					RE::Scaleform::GFx::Value args[2];
					args[0] = "DISABLED";
					args[1] = a_event->QJustPressed();

					switch (a_event->GetBSButtonCode())
					{
						case RE::BS_BUTTON_CODE::kTab:
						case RE::BS_BUTTON_CODE::kBButton:
							args[0] = "Cancel";
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

						case RE::BS_BUTTON_CODE::kLTrigger:
							args[0] = "LTrigger";
							break;

						case RE::BS_BUTTON_CODE::kRTrigger:
							args[0] = "RTrigger";
							break;

						case RE::BS_BUTTON_CODE::kQ:
						case RE::BS_BUTTON_CODE::kLStick:
							args[0] = "Sort";
							break;

						case RE::BS_BUTTON_CODE::kR:
						case RE::BS_BUTTON_CODE::kXButton:
							args[0] = "TradeAccept";
							break;

						case RE::BS_BUTTON_CODE::kT:
						case RE::BS_BUTTON_CODE::kYButton:
							args[0] = "TradeReset";
							break;

						case RE::BS_BUTTON_CODE::kV:
						case RE::BS_BUTTON_CODE::kSelect:
							args[0] = "Invest";
							break;

						case RE::BS_BUTTON_CODE::kX:
						case RE::BS_BUTTON_CODE::kRStick:
							args[0] = "Inspect";
							break;
					}

					menu->menuObj.Invoke("ProcessUserEvent", nullptr, args, 2);
				}
			}
		}

		static bool BarterMenu__OnButtonEventRelease([[maybe_unused]] RE::BarterMenu* a_this, [[maybe_unused]] const RE::BSFixedString& a_eventName)
		{
			return true;
		}

		inline static REL::Relocation<decltype(BarterMenu__CTOR)> _BarterMenu__CTOR;
		inline static REL::Relocation<decltype(BarterMenu__DTOR)> _BarterMenu__DTOR;
		inline static REL::Relocation<decltype(BarterMenu__Call)> _BarterMenu__Call;
		inline static REL::Relocation<decltype(BarterMenu__OnButtonEventRelease)> _BarterMenu__OnButtonEventRelease;
	};
}
