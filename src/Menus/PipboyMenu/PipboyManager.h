#pragma once

namespace Menus
{
	class PipboyManager
	{
	public:
		static void Install()
		{
			REL::Relocation<std::uintptr_t> targetLower{ REL::ID(302903) };
			REL::Relocation<std::uintptr_t> targetRaise{ REL::ID(726763) };

			stl::asm_replace(targetLower.address(), 0x066, reinterpret_cast<std::uintptr_t>(PipboyManager__ProcessLoweringReason));
			stl::asm_replace(targetRaise.address(), 0x100, reinterpret_cast<std::uintptr_t>(PipboyManager__RaisePipboy));
		}

	private:
		static void PipboyManager__ProcessLoweringReason(RE::PipboyManager* a_this)
		{
			if (auto renderer = RE::Interface3D::Renderer::GetByName("PipboyMenu"); renderer)
			{
				renderer->Disable();
			}

			switch (a_this->loweringReason.get())
			{
				case RE::PipboyManager::LOWER_REASON::kBook:
					{
						auto UIMessageQueue = RE::UIMessageQueue::GetSingleton();
						UIMessageQueue->AddMessage("BookMenu", RE::UI_MESSAGE_TYPE::kShow);
						break;
					}

				case RE::PipboyManager::LOWER_REASON::kPerkGrid:
					{
						auto PlayerControls = RE::PlayerControls::GetSingleton();
						PlayerControls->DoAction(
							RE::DEFAULT_OBJECT::kActionPipboyInspect,
							RE::ActionInput::ACTIONPRIORITY::kTry);
						LevelUpMenu::ShowMenu(true);
						break;
					}
			}
		}

		static void PipboyManager__RaisePipboy(RE::PipboyManager* a_this)
		{
			if (a_this->loweringReason)
			{
				a_this->pipboyRaising = true;
				if (auto renderer = RE::Interface3D::Renderer::GetByName("PipboyMenu"); renderer)
				{
					renderer->Enable(false);
				}

				auto MenuCursor = RE::MenuCursor::GetSingleton();
				auto BSInputDeviceManager = RE::BSInputDeviceManager::GetSingleton();
				auto PlayerInPowerArmor = RE::PowerArmor::PlayerInPowerArmor();

				if (BSInputDeviceManager->IsGamepadConnected())
				{
					if (PlayerInPowerArmor)
					{
						MenuCursor->SetCursorConstraintsRaw(
							uPipboyConstraintX_PA->GetUInt(),
							uPipboyConstraintY_PA->GetUInt(),
							uPipboyConstraintW_PA->GetUInt(),
							uPipboyConstraintH_PA->GetUInt());
					}
					else
					{
						MenuCursor->SetCursorConstraintsRaw(
							uPipboyConstraintX->GetUInt(),
							uPipboyConstraintY->GetUInt(),
							uPipboyConstraintW->GetUInt(),
							uPipboyConstraintH->GetUInt());
					}
				}
				else
				{
					MenuCursor->ClearConstraints();
				}

				if (PlayerInPowerArmor)
				{
					a_this->loweringReason = RE::PipboyManager::LOWER_REASON::kNone;
					a_this->pipboyRaising = false;
				}
				else
				{
					auto PlayerControls = RE::PlayerControls::GetSingleton();
					PlayerControls->DoAction(RE::DEFAULT_OBJECT::kActionPipboy, RE::ActionInput::ACTIONPRIORITY::kTry);
				}
			}
		}

		inline static REL::Relocation<RE::SettingT<RE::INISettingCollection>*> uPipboyConstraintX{ REL::ID(60674) };
		inline static REL::Relocation<RE::SettingT<RE::INISettingCollection>*> uPipboyConstraintY{ REL::ID(719279) };
		inline static REL::Relocation<RE::SettingT<RE::INISettingCollection>*> uPipboyConstraintW{ REL::ID(1376729) };
		inline static REL::Relocation<RE::SettingT<RE::INISettingCollection>*> uPipboyConstraintH{ REL::ID(452591) };
		inline static REL::Relocation<RE::SettingT<RE::INISettingCollection>*> uPipboyConstraintX_PA{ REL::ID(1110986) };
		inline static REL::Relocation<RE::SettingT<RE::INISettingCollection>*> uPipboyConstraintY_PA{ REL::ID(187113) };
		inline static REL::Relocation<RE::SettingT<RE::INISettingCollection>*> uPipboyConstraintW_PA{ REL::ID(844985) };
		inline static REL::Relocation<RE::SettingT<RE::INISettingCollection>*> uPipboyConstraintH_PA{ REL::ID(1503497) };
	};
}
