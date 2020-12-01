#pragma once

namespace Menus
{
	class BlurMenu :
		public RE::GameMenuBase
	{
	public:
		BlurMenu()
		{
			menuFlags.set(RE::UI_MENU_FLAGS::kUsesBlurredBackground);
			depthPriority = 8;
		}

		~BlurMenu()
		{
		}

		static void ShowMenu()
		{
			if (auto singleton = RE::UIMessageQueue::GetSingleton(); singleton)
			{
				singleton->AddMessage("BlurMenu", RE::UI_MESSAGE_TYPE::kShow);
			}
		}

		static void HideMenu()
		{
			if (auto singleton = RE::UIMessageQueue::GetSingleton(); singleton)
			{
				singleton->AddMessage("BlurMenu", RE::UI_MESSAGE_TYPE::kHide);
			}
		}

		static RE::IMenu* Create(const RE::UIMessage&)
		{
			return new BlurMenu();
		}
	};
}