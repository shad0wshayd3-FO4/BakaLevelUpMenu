#pragma once

#include "Menus/Utils/Utils.h"

namespace Menus
{
	class TakeAllCallback :
		public RE::IMessageBoxCallback
	{
	public:
		TakeAllCallback(RE::ContainerMenu* a_menu) :
			menu(a_menu)
		{}

		// override
		virtual void operator()(std::uint8_t a_buttonIdx)
		{
			if (a_buttonIdx == 0)
			{
				menu->TakeAllItems();
			}
			menu->SetMessageBoxMode(false);
		}

		// members
		RE::ContainerMenu* menu{ nullptr };
	};

	class ContainerMenu
	{
	public:
		static void Install()
		{
			REL::Relocation<std::uintptr_t> targetCTOR{ REL::ID(782822), 0x19 };
			REL::Relocation<std::uintptr_t> targetDTOR{ REL::ID(1473839), 0x65 };
			REL::Relocation<std::uintptr_t> targetVTBL_0{ RE::ContainerMenu::VTABLE[0] };
			REL::Relocation<std::uintptr_t> targetVTBL_1{ RE::ContainerMenu::VTABLE[1] };
			REL::Relocation<std::uintptr_t> targetPopulateObj{ REL::ID(969445), 0x53 };
			REL::Relocation<std::uintptr_t> targetPopulateItm{ REL::ID(969445), 0x5E };

			auto& trampoline = F4SE::GetTrampoline();
			_ContainerMenu__CTOR = trampoline.write_call<5>(targetCTOR.address(), ContainerMenu__CTOR);
			_ContainerMenu__DTOR = trampoline.write_branch<5>(targetDTOR.address(), ContainerMenu__DTOR);
			_ContainerMenu__Call = targetVTBL_0.write_vfunc(0x01, reinterpret_cast<std::uintptr_t>(ContainerMenu__Call));
			_ContainerMenu__MapCodeObjectFunctions = targetVTBL_0.write_vfunc(0x02, reinterpret_cast<std::uintptr_t>(ContainerMenu__MapCodeObjectFunctions));
			_ContainerMenu__OnButtonEventRelease = targetVTBL_0.write_vfunc(0x0F, reinterpret_cast<std::uintptr_t>(ContainerMenu__OnButtonEventRelease));
			targetVTBL_1.write_vfunc(0x08, reinterpret_cast<std::uintptr_t>(ContainerMenu__HandleEvent));
			trampoline.write_call<5>(targetPopulateObj.address(), InventoryUserUIUtils__PopulateMenuObj);
			trampoline.write_call<5>(targetPopulateItm.address(), InventoryUserUIUtils__PopulateItemCardInfo);
		}

		static inline RE::msvc::unique_ptr<RE::BSGFxShaderFXTarget> CategoryBar_mc;
		static inline RE::msvc::unique_ptr<RE::BSGFxShaderFXTarget> CategoryBarBackground_mc;

	private:
		static RE::ContainerMenuBase* ContainerMenu__CTOR(RE::ContainerMenuBase* a_this, const char* a_movieName)
		{
			_ContainerMenu__CTOR(a_this, a_movieName);

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

			auto Interface3D = RE::Interface3D::Renderer::GetByName("Container3D"sv);
			if (Interface3D)
			{
				Interface3D->postFX = RE::Interface3D::PostEffect::kNone;
			}

			auto UIMessageQueue = RE::UIMessageQueue::GetSingleton();
			if (UIMessageQueue)
			{
				UIMessageQueue->AddMessage("VignetteMenu"sv, RE::UI_MESSAGE_TYPE::kHide);
			}

			auto CanDisplayNextHUDMessage = RE::CanDisplayNextHUDMessage::GetEventSource();
			if (CanDisplayNextHUDMessage)
			{
				CanDisplayNextHUDMessage->Notify(false);
			}

			return a_this;
		}

		static void ContainerMenu__DTOR(RE::ContainerMenuBase* a_this)
		{
			_ContainerMenu__DTOR(a_this);

			CategoryBarBackground_mc.release();
			CategoryBar_mc.release();

			auto CanDisplayNextHUDMessage = RE::CanDisplayNextHUDMessage::GetEventSource();
			if (CanDisplayNextHUDMessage)
			{
				CanDisplayNextHUDMessage->Notify(true);
			}
		}

		static void ContainerMenu__Call(RE::ContainerMenu* a_this, const RE::Scaleform::GFx::FunctionHandler::Params& a_params)
		{
			switch (reinterpret_cast<std::uint64_t>(a_params.userData))
			{
				case 3:	 // Show3D
					if (a_params.argCount == 2 && a_params.args[0].IsInt() && a_params.args[1].IsBoolean())
					{
						if (a_params.args[0].GetInt() == -1)
						{
							a_this->inv3DModelManager.ClearModel();
							break;
						}

						_ContainerMenu__Call(a_this, a_params);
					}
					break;

				case 4:	 // ExitMenu
					{
						auto UIMessageQueue = RE::UIMessageQueue::GetSingleton();
						if (UIMessageQueue)
						{
							UIMessageQueue->AddMessage(
								"ContainerMenu",
								RE::UI_MESSAGE_TYPE::kHide);
						}
					}
					break;

				case 5:	 // TakeAllItems
					{
						if (a_this->containerInv.stackedEntries.size() < uConfirmContainerTakeAllMinimumItems->GetUInt())
						{
							a_this->TakeAllItems();
						}
						else
						{
							auto MessageMenuManager = RE::MessageMenuManager::GetSingleton();
							if (MessageMenuManager)
							{
								auto mbCallback = new TakeAllCallback(a_this);
								MessageMenuManager->Create(
									"",
									sConfirmContainerTakeAll->GetString().data(),
									mbCallback,
									RE::WARNING_TYPES::kInGameMessage,
									"$OK",
									"$Cancel");
								a_this->SetMessageBoxMode(true);
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
							case RE::ContainerMenuBase::ItemSorter::SORT_ON_FIELD::kAlphabetical:
								a_this->menuObj.SetMember("sortButtonLabel"sv, "$SORT");
								break;
							case RE::ContainerMenuBase::ItemSorter::SORT_ON_FIELD::kDamage:
								a_this->menuObj.SetMember("sortButtonLabel"sv, "$SORT_DMG");
								break;
							case RE::ContainerMenuBase::ItemSorter::SORT_ON_FIELD::kRateOfFire:
								a_this->menuObj.SetMember("sortButtonLabel"sv, "$SORT_ROF");
								break;
							case RE::ContainerMenuBase::ItemSorter::SORT_ON_FIELD::kRange:
								a_this->menuObj.SetMember("sortButtonLabel"sv, "$SORT_RNG");
								break;
							case RE::ContainerMenuBase::ItemSorter::SORT_ON_FIELD::kAccuracy:
								a_this->menuObj.SetMember("sortButtonLabel"sv, "$SORT_ACC");
								break;
							case RE::ContainerMenuBase::ItemSorter::SORT_ON_FIELD::kValue:
								a_this->menuObj.SetMember("sortButtonLabel"sv, "$SORT_VAL");
								break;
							case RE::ContainerMenuBase::ItemSorter::SORT_ON_FIELD::kWeight:
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
							ContainerMenuBase__ItemSorter__IncrementSort(&a_this->containerItemSorter);
							ContainerMenuBase__ItemSorter__IncrementSort(&a_this->playerItemSorter);
						}

						a_this->UpdateList(true);
						a_this->UpdateList(false);
						a_this->menuObj.Invoke("InvalidateLists");
					}
					break;

				default:
					_ContainerMenu__Call(a_this, a_params);
					break;
			}
		}

		static void ContainerMenu__MapCodeObjectFunctions(RE::ContainerMenuBase* a_this)
		{
			_ContainerMenu__MapCodeObjectFunctions(a_this);
			// a_this->MapCodeMethodToASFunction("", 18);
		}

		static void ContainerMenu__HandleEvent(RE::BSInputEventUser* a_this, const RE::ButtonEvent* a_event)
		{
			auto menu = RE::fallout_cast<RE::ContainerMenu*>(a_this);
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
							args[0] = "TakeAll";
							break;

						case RE::BS_BUTTON_CODE::kT:
						case RE::BS_BUTTON_CODE::kYButton:
							args[0] = "Equip";
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

		static bool ContainerMenu__OnButtonEventRelease([[maybe_unused]] RE::ContainerMenu* a_this, [[maybe_unused]] const RE::BSFixedString& a_eventName)
		{
			return true;
		}

		static void ContainerMenuBase__ItemSorter__IncrementSort(RE::ContainerMenuBase::ItemSorter* a_this)
		{
			using SORT_ON_FIELD = RE::ContainerMenuBase::ItemSorter::SORT_ON_FIELD;
			auto GetNextSort = [](SORT_ON_FIELD a_current, std::vector<SORT_ON_FIELD> a_valid)
			{
				auto iter = std::find(a_valid.begin(), a_valid.end(), a_current);
				return (std::next(iter) == a_valid.end()) ? a_valid.front() : *std::next(iter);
			};

			auto currentTab = a_this->currentTab;
			switch (currentTab)
			{
				case 1:	 // Weapons
					a_this->currentSort[currentTab] =
						GetNextSort(
							a_this->currentSort[currentTab].get(),
							{ SORT_ON_FIELD::kAlphabetical,
							  SORT_ON_FIELD::kDamage,
							  SORT_ON_FIELD::kRateOfFire,
							  SORT_ON_FIELD::kRange,
							  SORT_ON_FIELD::kAccuracy,
							  SORT_ON_FIELD::kValue,
							  SORT_ON_FIELD::kWeight });
					break;

				case 2:	 // Apparel
					a_this->currentSort[currentTab] =
						GetNextSort(
							a_this->currentSort[currentTab].get(),
							{ SORT_ON_FIELD::kAlphabetical,
							  SORT_ON_FIELD::kDamage,
							  SORT_ON_FIELD::kValue,
							  SORT_ON_FIELD::kWeight });
					break;

				default:  // Other
					a_this->currentSort[currentTab] =
						GetNextSort(
							a_this->currentSort[currentTab].get(),
							{ SORT_ON_FIELD::kAlphabetical,
							  SORT_ON_FIELD::kValue,
							  SORT_ON_FIELD::kWeight });
					break;
			}
		}

		static void InventoryUserUIUtils__PopulateMenuObj(RE::ObjectRefHandle a_inventoryRef, const RE::InventoryUserUIInterfaceEntry& a_entry, RE::Scaleform::GFx::Value& a_menuObj)
		{
			auto BGSInventoryInterface = RE::BGSInventoryInterface::GetSingleton();
			if (BGSInventoryInterface)
			{
				auto item = BGSInventoryInterface->RequestInventoryItem(a_entry.invHandle.id);
				if (item)
				{
					auto iidd = Utils::InventoryItemDisplayDataEx(a_inventoryRef, a_entry);
					iidd.PopulateFlashObject(a_menuObj);
				}
			}
		}

		static void InventoryUserUIUtils__PopulateItemCardInfo(const RE::InventoryUserUIInterfaceEntry& a_entry, RE::Scaleform::GFx::Value& a_menuObj)
		{
			auto BGSInventoryInterface = RE::BGSInventoryInterface::GetSingleton();
			if (BGSInventoryInterface)
			{
				auto item = BGSInventoryInterface->RequestInventoryItem(a_entry.invHandle.id);
				if (item && item->object)
				{
					RE::UIUtils::ComparisonItems comparisonItems;
					RE::UIUtils::GetComparisonItems(item->object, comparisonItems);
					Utils::PopulateItemCardInfo(a_menuObj, *item, a_entry.stackIndex[0], comparisonItems, false);
				}
			}
		}

		static inline REL::Relocation<decltype(ContainerMenu__CTOR)> _ContainerMenu__CTOR;
		static inline REL::Relocation<decltype(ContainerMenu__DTOR)> _ContainerMenu__DTOR;
		static inline REL::Relocation<decltype(ContainerMenu__Call)> _ContainerMenu__Call;
		static inline REL::Relocation<decltype(ContainerMenu__MapCodeObjectFunctions)> _ContainerMenu__MapCodeObjectFunctions;
		static inline REL::Relocation<decltype(ContainerMenu__OnButtonEventRelease)> _ContainerMenu__OnButtonEventRelease;

		static inline REL::Relocation<RE::SettingT<RE::GameSettingCollection>*> sConfirmContainerTakeAll{ REL::ID(1418009) };
		static inline REL::Relocation<RE::SettingT<RE::INISettingCollection>*> uConfirmContainerTakeAllMinimumItems{ REL::ID(122882) };
	};
}
