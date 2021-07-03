#pragma once

namespace Menus
{
	class ContainerMenuBase
	{
	public:
		static void Install()
		{
			REL::Relocation<std::uintptr_t> targetMapCodeObj{ REL::ID(512372) };
			stl::asm_replace(targetMapCodeObj.address(), 0x16D, reinterpret_cast<std::uintptr_t>(ContainerMenuBase__MapCodeObjectFunctions));
		}

	private:
		static void ContainerMenuBase__MapCodeObjectFunctions(RE::ContainerMenuBase* a_this)
		{
			a_this->MapCodeMethodToASFunction("PlaySound", 0);
			a_this->MapCodeMethodToASFunction("transferItem", 1);
			a_this->MapCodeMethodToASFunction("onIntroAnimComplete", 2);
			a_this->MapCodeMethodToASFunction("show3D", 3);
			a_this->MapCodeMethodToASFunction("exitMenu", 4);
			a_this->MapCodeMethodToASFunction("takeAllItems", 5);
			a_this->MapCodeMethodToASFunction("getItemValue", 6);
			a_this->MapCodeMethodToASFunction("updateItemPickpocketInfo", 7);
			a_this->MapCodeMethodToASFunction("getSelectedItemEquippable", 8);
			a_this->MapCodeMethodToASFunction("getSelectedItemEquipped", 9);
			a_this->MapCodeMethodToASFunction("toggleSelectedItemEquipped", 10);
			a_this->MapCodeMethodToASFunction("confirmInvest", 11);
			a_this->MapCodeMethodToASFunction("sendXButton", 12);
			a_this->MapCodeMethodToASFunction("sendYButton", 13);
			a_this->MapCodeMethodToASFunction("updateSortButtonLabel", 15);
			a_this->MapCodeMethodToASFunction("sortItems", 16);
			a_this->MapCodeMethodToASFunction("inspectItem", 17);
			a_this->MapCodeMethodToASFunction("TradeAccept", 18);
			a_this->MapCodeMethodToASFunction("TradeReset", 19);
			a_this->MapCodeMethodToASFunction("PauseForDebugging", 99);
		}
	};
}
