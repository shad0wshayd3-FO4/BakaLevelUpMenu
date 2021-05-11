#pragma once

namespace Menus
{
	class BarterMenu
	{
	public:
		static void Install()
		{
			REL::Relocation<std::uintptr_t> targetCTOR{ REL::ID(901312), 0x34 };
			REL::Relocation<std::uintptr_t> targetDTOR{ REL::ID(1405609), 0x10F };

			auto& trampoline = F4SE::GetTrampoline();
			_BarterMenu__CTOR = trampoline.write_call<5>(targetCTOR.address(), BarterMenu__CTOR);
			_BarterMenu__DTOR = trampoline.write_branch<5>(targetDTOR.address(), BarterMenu__DTOR);
		}

	private:
		static RE::ContainerMenuBase* BarterMenu__CTOR(RE::ContainerMenuBase* a_this, const char* a_movieName)
		{
			_BarterMenu__CTOR(a_this, a_movieName);
			return a_this;
		}

		static void BarterMenu__DTOR(RE::ContainerMenuBase* a_this)
		{
			_BarterMenu__DTOR(a_this);
		}

		static inline REL::Relocation<decltype(BarterMenu__CTOR)> _BarterMenu__CTOR;
		static inline REL::Relocation<decltype(BarterMenu__DTOR)> _BarterMenu__DTOR;
	};
}