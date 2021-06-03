#pragma once

namespace Menus
{
	namespace Utils
	{
		namespace detail
		{
			enum FilterFlag
			{
				kAll = 0xFFFFFFFF,
				kFavorite = 1 << 0,
				kWeapon = 1 << 1,
				kApparel = 1 << 2,
				kAid = 1 << 3,
				kHolotapes = 1 << 4,
				kBooks = 1 << 5,
				kKeys = 1 << 6,
				kMisc = 1 << 9,
				kJunk = 1 << 10,
				kMods = 1 << 11,
				kAmmo = 1 << 12,
			};

			inline REL::Relocation<RE::BGSDefaultObject*> ObjectTypeSyringerAmmo_DO{ REL::ID(1430491) };

			std::uint32_t GetFilterFlag(const RE::BGSInventoryItem* a_inventoryItem, const RE::BGSInventoryItem::Stack* a_stack)
			{
				std::uint32_t result{ 0 };

				switch (a_inventoryItem->object->GetFormType())
				{
				case RE::ENUM_FORM_ID::kWEAP:  // Weapons
					result = FilterFlag::kWeapon;
					break;

				case RE::ENUM_FORM_ID::kARMO:  // Apparel
					result = FilterFlag::kApparel;
					break;

				case RE::ENUM_FORM_ID::kALCH:  // Aid
				case RE::ENUM_FORM_ID::kINGR:
					{
						auto alch = a_inventoryItem->object->As<RE::MagicItem>();
						if (alch)
						{
							auto objectTypeSyringerAmmo = ObjectTypeSyringerAmmo_DO->GetForm<RE::BGSKeyword>();
							if (objectTypeSyringerAmmo && alch->HasKeyword(objectTypeSyringerAmmo))
							{
								result = FilterFlag::kAmmo;
								break;
							}
						}
					}

					result = FilterFlag::kAid;
					break;

				case RE::ENUM_FORM_ID::kMISC:  // Misc, Junk, Mods
					{
						auto misc = a_inventoryItem->object->As<RE::TESObjectMISC>();
						if (misc)
						{
							if (misc->componentData && misc->componentData->size() > 0)
							{
								result = FilterFlag::kJunk;
								break;
							}

							if (misc->IsLooseMod())
							{
								result = FilterFlag::kMods;
								break;
							}
						}

						result = FilterFlag::kMisc;
						break;
					}

				case RE::ENUM_FORM_ID::kNOTE:  // Holo
					result = FilterFlag::kHolotapes;
					break;

				case RE::ENUM_FORM_ID::kBOOK:  // Note
					result = FilterFlag::kBooks;
					break;

				case RE::ENUM_FORM_ID::kKEYM:  // Keys
					result = FilterFlag::kKeys;
					break;

				case RE::ENUM_FORM_ID::kAMMO:  // Ammo
					result = FilterFlag::kAmmo;
					break;

				default:
					logger::error("Unhandled FilterFlag type: {:04X}", a_inventoryItem->object->GetFormType());
					break;
				}

				if (a_stack && a_stack->extra)
				{
					auto favorite = a_stack->extra->GetByType<RE::ExtraFavorite>();
					if (favorite)
					{
						// Assuming ExtraFavorite only exists on Favorited Items
						result |= FilterFlag::kFavorite;
					}
				}

				return result;
			}
		}

		struct InventoryItemDisplayDataEx :
			public RE::InventoryItemDisplayData
		{
		public:
			InventoryItemDisplayDataEx(const RE::ObjectRefHandle a_inventoryRef, const RE::InventoryUserUIInterfaceEntry& a_entry) :
				InventoryItemDisplayData(a_inventoryRef, a_entry)
			{
				auto BGSInventoryInterface = RE::BGSInventoryInterface::GetSingleton();
				if (BGSInventoryInterface)
				{
					auto item = BGSInventoryInterface->RequestInventoryItem(a_entry.invHandle.id);
					if (item)
					{
						if (a_entry.stackIndex.size() > 0)
						{
							auto stack = item->GetStackByID(a_entry.stackIndex[0]);
							if (stack)
							{
								filterFlag = detail::GetFilterFlag(item, stack);
							}
						}
						else
						{
							logger::error("[{:08X}] has size 0", item->object->formID);
							stl::report_and_fail("log");
						}
					}
				}
			}

			void PopulateFlashObject(RE::Scaleform::GFx::Value& a_flashObject)
			{
				InventoryItemDisplayData::PopulateFlashObject(a_flashObject);
				a_flashObject.SetMember("iconIndex", iconIndex);
			}

			// members
			std::uint32_t iconIndex{ 0 };
		};
	}
}