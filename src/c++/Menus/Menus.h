#pragma once

#include "Menus/ContainerMenu/BarterMenu.h"
#include "Menus/ContainerMenu/ContainerMenu.h"
#include "Menus/ContainerMenu/ContainerMenuBase.h"
#include "Menus/HUDMenuEx/HUDMenuEx.h"
#include "Menus/InventoryUserUIUtils/InventoryUserUIUtils.h"
#include "Menus/LevelUpMenu/LevelUpMenu.h"
#include "Menus/PipboyMenu/PipboyManager.h"
#include "Menus/PluginExplorerMenu/PluginExplorerMenu.h"
#include "Menus/Scaleform/Log.h"

namespace Menus
{
	void InstallHooks()
	{
		BarterMenu::Install();
		ContainerMenu::Install();
		ContainerMenuBase::Install();
		InventoryUserUIUtils::Install();
		LevelUpMenu::Install();
		PipboyManager::Install();
		Scaleform::Log::Install();
	}

	void Register()
	{
		if (const auto UI = RE::UI::GetSingleton(); UI)
		{
			// UI->RegisterMenu("HUDMenuEx", Menus::HUDMenuEx::Create);
			UI->RegisterMenu("PluginExplorerMenu", Menus::PluginExplorerMenu::Create);
		}

		Scaleform::Log::Register();
	}
}
