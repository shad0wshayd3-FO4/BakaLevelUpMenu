#pragma once

#include "Menus/BlurMenu/BlurMenu.h"
#include "Menus/HUDMenuEx/HUDMenuEx.h"
#include "Menus/LevelUpMenuEx/LevelUpMenuEx.h"

namespace Menus
{
	void InstallHooks()
	{
		LevelUpMenuEx::Install();
	}

	void Register()
	{
		if (const auto UI = RE::UI::GetSingleton(); UI)
		{
			UI->RegisterMenu("BlurMenu", Menus::BlurMenu::Create);
			// UI->RegisterMenu("HUDMenuEx", Menus::HUDMenuEx::Create);
		}
	}
}