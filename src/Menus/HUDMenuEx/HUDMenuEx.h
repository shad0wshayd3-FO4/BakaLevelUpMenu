#pragma once

namespace Menus
{
	class HUDMenuEx :
		public RE::GameMenuBase
	{
	public:
		HUDMenuEx()
		{
			menuFlags.set(RE::UI_MENU_FLAGS::kCustomRendering);
			inputEventHandlingEnabled = false;
			depthPriority = 6;

			const auto ScaleformManager = RE::BSScaleformManager::GetSingleton();
			const auto success =
				ScaleformManager->LoadMovieEx(*this, "Interface/HUDMenuEx.swf", "root", RE::Scaleform::GFx::Movie::ScaleModeType::kNoScale);

			assert(success);

			RE::BSGFxObject::make_unique_ptr(FusionCoreMeter_mc, *uiMovie, "root.FusionCoreMeter_mc");
			if (FusionCoreMeter_mc)
			{
				FusionCoreMeter_mc->CreateAndSetFiltersToHUD(RE::HUDColorTypes::kGameplayHUDColor, 1.0);
				shaderFXObjects.push_back(FusionCoreMeter_mc.get());
			}
		}

		~HUDMenuEx()
		{
			FusionCoreMeter_mc.release();
		}

		virtual void AdvanceMovie(float a_timeDelta, [[maybe_unused]] std::uint64_t a_time) override  // 04
		{
			auto PlayerRef = RE::PlayerCharacter::GetSingleton();
			if (PlayerRef)
			{
				const auto showPowerArmorHUD = RE::PowerArmor::QActorInPowerArmor(*PlayerRef);
				SetFusionCoreMeterVisible(showPowerArmorHUD);

				if (showPowerArmorHUD)
				{
					auto batteryValue = PlayerRef->GetActorValue(*RE::ActorValue::GetSingleton()->powerArmorBattery);
					SetFusionCoreMeterValue(batteryValue);

					auto batteryCount = PlayerRef->GetInventoryObjectCount(RE::PowerArmor::GetDefaultBatteryObject());
					SetFusionCoreMeterCount(batteryCount);
				}
			}

			__super::AdvanceMovie(a_timeDelta, a_time);
		}

		static RE::IMenu* Create(const RE::UIMessage&)
		{
			return new HUDMenuEx();
		}

	private:
		void SetFusionCoreMeterVisible(bool a_visible)
		{
			if (FusionCoreMeter_mc)
			{
				RE::Scaleform::GFx::Value args[1];
				args[0] = a_visible;
				FusionCoreMeter_mc->Invoke("SetVisible", nullptr, args, 1);
			}
		}

		void SetFusionCoreMeterValue(float a_value)
		{
			if (FusionCoreMeter_mc)
			{
				RE::Scaleform::GFx::Value args[1];
				args[0] = a_value;
				FusionCoreMeter_mc->Invoke("SetPercent", nullptr, args, 1);
				FusionCoreMeter_mc->SetHostile(a_value <= 20.0);
			}
		}

		void SetFusionCoreMeterCount(std::uint32_t a_count)
		{
			if (FusionCoreMeter_mc)
			{
				RE::Scaleform::GFx::Value args[1];
				args[0] = a_count;
				FusionCoreMeter_mc->Invoke("SetCount", nullptr, args, 1);
			}
		}

		RE::msvc::unique_ptr<RE::BSGFxShaderFXTarget> FusionCoreMeter_mc;
	};
}