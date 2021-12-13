#pragma once

namespace Menus
{
	class InventoryUserUIUtils
	{
	public:
		static void Install()
		{
			REL::Relocation<std::uintptr_t> targetPopulateObj{ REL::ID(969445), 0x53 };
			REL::Relocation<std::uintptr_t> targetPopulateItm{ REL::ID(969445), 0x5E };

			auto& trampoline = F4SE::GetTrampoline();
			trampoline.write_call<5>(targetPopulateObj.address(), InventoryUserUIUtils__PopulateMenuObj);
			trampoline.write_call<5>(targetPopulateItm.address(), InventoryUserUIUtils__PopulateItemCardInfo);
		}

	private:
		static void InventoryUserUIUtils__PopulateMenuObj(
			RE::ObjectRefHandle a_inventoryRef,
			const RE::InventoryUserUIInterfaceEntry& a_entry,
			RE::Scaleform::GFx::Value& a_menuObj)
		{
			if (auto BGSInventoryInterface = RE::BGSInventoryInterface::GetSingleton(); BGSInventoryInterface)
			{
				auto item = BGSInventoryInterface->RequestInventoryItem(a_entry.invHandle.id);
				if (item)
				{
					auto iidd = Utils::InventoryItemDisplayDataEx(a_inventoryRef, a_entry);
					iidd.PopulateFlashObject(a_menuObj);
				}
			}
		}

		static void InventoryUserUIUtils__PopulateItemCardInfo(
			const RE::InventoryUserUIInterfaceEntry& a_entry,
			RE::Scaleform::GFx::Value& a_menuObj)
		{
			if (auto BGSInventoryInterface = RE::BGSInventoryInterface::GetSingleton(); BGSInventoryInterface)
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
	};
}
