#pragma once

namespace Menus
{
	namespace Utils
	{
		namespace detail
		{
			using DamageTypeInfo = RE::BSScrapArray<RE::BSTTuple<std::uint32_t, float>>;

			template <class T>
			void AddVW(RE::Scaleform::GFx::Value& a_itemCardInfoList, T* a_form, RE::TBO_InstanceData* a_data = nullptr)
			{
				auto val = RE::TESValueForm::GetFormValue(a_form, a_data);
				auto wgt = RE::TESWeightForm::GetFormWeight(a_form, a_data);
				auto vw = wgt != 0 ? (val / wgt) : -1.0F;

				RE::Scaleform::GFx::Value wtEntry;
				RE::InventoryUserUIUtils::AddItemCardInfoEntry(a_itemCardInfoList, "$wt",
					wtEntry, wgt);
				wtEntry.SetMember("precision", 2);

				RE::Scaleform::GFx::Value valEntry;
				RE::InventoryUserUIUtils::AddItemCardInfoEntry(a_itemCardInfoList, "$val",
					valEntry, val);
				wtEntry.SetMember("precision", 0);

				RE::Scaleform::GFx::Value valwtEntry;
				RE::InventoryUserUIUtils::AddItemCardInfoEntry(a_itemCardInfoList, "$val / $wt",
					valwtEntry, vw);
				wtEntry.SetMember("precision", 0);
			}

			void ProcessMagicItem(RE::Scaleform::GFx::Value& a_itemCardInfoList, RE::MagicItem* a_item)
			{
				auto PlayerCharacter = RE::PlayerCharacter::GetSingleton();

				for (auto effect : a_item->listOfEffects)
				{
					if (!effect)
					{
						continue;
					}

					if (!effect->conditions(PlayerCharacter, PlayerCharacter))
					{
						continue;
					}

					auto setting = effect->effectSetting;
					if (!setting)
					{
						continue;
					}

					if (!setting->conditions(PlayerCharacter, PlayerCharacter))
					{
						continue;
					}

					if ((setting->data.flags >> 15) & 1)
					{
						continue;
					}

					float mag{ 0.0F }, dur{ 0.0F };
					RE::StatsMenuUtils::GetEffectDisplayInfo(a_item, effect, mag, dur);
					RE::Scaleform::GFx::Value valueEntry;

					switch (setting->data.archetype.get())
					{
					case RE::EffectArchetypes::ArchetypeID::kValueModifier:
					case RE::EffectArchetypes::ArchetypeID::kDualValueModifier:
					case RE::EffectArchetypes::ArchetypeID::kPeakValueModifier:
						{
							auto PrimaryAV = effect->effectSetting->data.primaryAV;
							if (PrimaryAV)
							{
								std::string_view text{ PrimaryAV->abbreviation.data() };
								if (text.empty())
								{
									break;
								}

								RE::InventoryUserUIUtils::AddItemCardInfoEntry(a_itemCardInfoList,
									text.data(), valueEntry, mag);
							}
							break;
						}

					case RE::EffectArchetypes::ArchetypeID::kStimpak:
						{
							auto AVs = RE::ActorValue::GetSingleton();
							if (AVs && AVs->health)
							{
								RE::InventoryUserUIUtils::AddItemCardInfoEntry(a_itemCardInfoList,
									AVs->health->abbreviation.data(), valueEntry, mag);
								valueEntry.SetMember("showAsPercent", true);
							}
							break;
						}

					default:
						break;
					}
				}
			}

			void ProcessArmor(RE::Scaleform::GFx::Value& a_itemCardInfoList, RE::TESObjectARMO* a_armo, RE::TESObjectARMO::InstanceData* a_data, [[maybe_unused]] RE::ExtraDataList* a_extra)
			{
				RE::BGSInventoryItem item;
				item.object = a_armo;

				if (a_data->enchantments)
				{
					for (auto enchantment : *a_data->enchantments)
					{
						ProcessMagicItem(a_itemCardInfoList, enchantment);
					}
				}

				DamageTypeInfo damageTypes;
				RE::PipboyInventoryUtils::FillResistTypeInfo(item, nullptr, damageTypes, 1.0);
				for (auto& damageType : damageTypes)
				{
					RE::Scaleform::GFx::Value dmgEntry;
					RE::InventoryUserUIUtils::AddItemCardInfoEntry(a_itemCardInfoList, "$dr",
						dmgEntry, damageType.second);
					dmgEntry.SetMember("damageType", damageType.first);
					dmgEntry.SetMember("precision", 1);
				}

				AddVW(a_itemCardInfoList, a_armo, a_data);
			}

			void ProcessWeapon(RE::Scaleform::GFx::Value& a_itemCardInfoList, RE::TESObjectWEAP* a_weap, RE::TESObjectWEAP::InstanceData* a_data, [[maybe_unused]] RE::ExtraDataList* a_extra)
			{
				RE::BGSInventoryItem item;
				item.object = a_weap;

				if (a_data->enchantments)
				{
					for (auto enchantment : *a_data->enchantments)
					{
						ProcessMagicItem(a_itemCardInfoList, enchantment);
					}
				}

				DamageTypeInfo damageTypes;
				RE::PipboyInventoryUtils::FillDamageTypeInfo(item, nullptr, damageTypes);
				for (auto& damageType : damageTypes)
				{
					RE::Scaleform::GFx::Value dmgEntry;
					RE::InventoryUserUIUtils::AddItemCardInfoEntry(a_itemCardInfoList, "$dmg",
						dmgEntry, damageType.second);
					dmgEntry.SetMember("damageType", damageType.first);
					dmgEntry.SetMember("precision", 1);
				}

				if (a_data->ammo)
				{
					auto PlayerCharacter = RE::PlayerCharacter::GetSingleton();

					RE::Scaleform::GFx::Value ammoEntry;
					RE::InventoryUserUIUtils::AddItemCardInfoEntry(a_itemCardInfoList, a_data->ammo->shortDesc.data(),
						ammoEntry, PlayerCharacter->GetInventoryObjectCount(a_data->ammo));
					ammoEntry.SetMember("damageType", 10);
				}

				if (a_data->type >= 7)
				{
					RE::BGSObjectInstanceT<RE::TESObjectWEAP> weapInst{ a_weap, a_data };

					RE::Scaleform::GFx::Value rofEntry;
					RE::InventoryUserUIUtils::AddItemCardInfoEntry(a_itemCardInfoList, "$ROF",
						rofEntry, RE::CombatFormulas::GetWeaponDisplayRateOfFire(*a_weap, a_data));
					rofEntry.SetMember("precision", 0);

					RE::Scaleform::GFx::Value rngEntry;
					RE::InventoryUserUIUtils::AddItemCardInfoEntry(a_itemCardInfoList, "$rng",
						rngEntry, RE::CombatFormulas::GetWeaponDisplayRange(weapInst));
					rngEntry.SetMember("precision", 1);

					RE::Scaleform::GFx::Value accEntry;
					RE::InventoryUserUIUtils::AddItemCardInfoEntry(a_itemCardInfoList, "$acc",
						accEntry, RE::CombatFormulas::GetWeaponDisplayAccuracy(weapInst, nullptr));
					accEntry.SetMember("precision", 1);
				}
				else
				{
					RE::Scaleform::GFx::Value speedEntry;
					RE::InventoryUserUIUtils::AddItemCardInfoEntry(a_itemCardInfoList, "$speed",
						speedEntry, RE::TESObjectWEAP::GetMeleeAttackSpeedLabel(a_weap->GetMeleeAttackSpeed()));
				}

				AddVW(a_itemCardInfoList, a_weap, a_data);
			};
		}

		void PopulateItemCard(RE::Scaleform::Ptr<RE::Scaleform::GFx::Movie>& a_movie, RE::Scaleform::GFx::Value& a_itemCardInfoList, RE::TESForm* a_form, RE::TBO_InstanceData* a_data = nullptr)
		{
			if (!a_itemCardInfoList.IsArray())
			{
				a_movie->CreateArray(&a_itemCardInfoList);
			}

			switch (a_form->GetFormType())
			{
			case RE::ENUM_FORM_ID::kALCH:
				{
					if (auto alch = a_form->As<RE::AlchemyItem>(); alch)
					{
						detail::ProcessMagicItem(a_itemCardInfoList, alch);
						detail::AddVW(a_itemCardInfoList, alch);
					}
					break;
				}

			case RE::ENUM_FORM_ID::kAMMO:
				{
					if (auto ammo = a_form->As<RE::TESAmmo>(); ammo)
					{
						if (ammo->HasKeyword(RE::PowerArmor::GetBatteryKeyword(), a_data))
						{
							RE::Scaleform::GFx::Value healthEntry;
							RE::InventoryUserUIUtils::AddItemCardInfoEntry(a_itemCardInfoList, "$health",
								healthEntry, 0.8);
							// healthEntry;
						}

						detail::AddVW(a_itemCardInfoList, ammo);
					}
					break;
				}

			case RE::ENUM_FORM_ID::kARMO:
				{
					if (auto armo = a_form->As<RE::TESObjectARMO>(); armo)
					{
						RE::ExtraDataList extra;
						if (!a_data)
						{
							RE::BGSMod::Template::Items::CreateInstanceDataForObjectAndExtra(*armo, extra, nullptr, true);
							if (auto extraInstance = extra.GetByType<RE::ExtraInstanceData>(); extraInstance)
							{
								a_data = extraInstance->data.get();
							}
						}

						auto data = static_cast<RE::TESObjectARMO::InstanceData*>(a_data);
						if (data)
						{
							detail::ProcessArmor(a_itemCardInfoList, armo, data, &extra);
						}
						else
						{
							RE::ExtraDataList tempExtra;
							detail::ProcessArmor(a_itemCardInfoList, armo, &armo->data, &tempExtra);
						}
					}
					break;
				}

			case RE::ENUM_FORM_ID::kBOOK:
				{
					if (auto book = a_form->As<RE::TESObjectBOOK>(); book)
					{
						detail::AddVW(a_itemCardInfoList, book);
					}
					break;
				}

			case RE::ENUM_FORM_ID::kKEYM:
				{
					if (auto keym = a_form->As<RE::TESKey>(); keym)
					{
						detail::AddVW(a_itemCardInfoList, keym);
					}
					break;
				}

			case RE::ENUM_FORM_ID::kMISC:
				{
					if (auto misc = a_form->As<RE::TESObjectMISC>(); misc)
					{
						if (misc->componentData && misc->componentData->size() > 0)
						{
							RE::Scaleform::GFx::Value cmpoEntry;
							RE::InventoryUserUIUtils::AddItemCardInfoEntry(a_itemCardInfoList, "cmpo", cmpoEntry, "");

							RE::Scaleform::GFx::Value components;
							a_movie->CreateArray(&components);

							auto FavoritesManager = RE::FavoritesManager::GetSingleton();
							for (auto data : *misc->componentData)
							{
								if (!data.first)
								{
									continue;
								}

								if (auto cmpo = data.first->As<RE::TESBoundObject>(); cmpo)
								{
									auto name = RE::TESFullName::GetFullName(*cmpo);
									auto taggedForSearch = FavoritesManager->IsComponentFavorite(cmpo);

									RE::Scaleform::GFx::Value entry;
									a_movie->CreateObject(&entry);
									entry.SetMember("text", name.data());
									entry.SetMember("count", data.second.i);
									entry.SetMember("taggedForSearch", taggedForSearch);
									components.PushBack(entry);
								}
							}

							cmpoEntry.SetMember("components", components);
						}

						detail::AddVW(a_itemCardInfoList, misc);
					}
					break;
				}

			case RE::ENUM_FORM_ID::kNOTE:
				{
					if (auto note = a_form->As<RE::BGSNote>(); note)
					{
						detail::AddVW(a_itemCardInfoList, note);
					}
					break;
				}

			case RE::ENUM_FORM_ID::kWEAP:
				{
					if (auto weap = a_form->As<RE::TESObjectWEAP>(); weap)
					{
						RE::ExtraDataList extra;
						if (!a_data)
						{
							RE::BGSMod::Template::Items::CreateInstanceDataForObjectAndExtra(*weap, extra, nullptr, true);
							if (auto extraInstance = extra.GetByType<RE::ExtraInstanceData>(); extraInstance)
							{
								a_data = extraInstance->data.get();
							}
						}

						if (auto data = static_cast<RE::TESObjectWEAP::InstanceData*>(a_data); data)
						{
							detail::ProcessWeapon(a_itemCardInfoList, weap, data, nullptr);
						}
						else
						{
							RE::ExtraDataList tempExtra; // These shouldn't be nullptr, but crash otherwise...?
							detail::ProcessWeapon(a_itemCardInfoList, weap, &weap->weaponData, nullptr);
						}
					}
					break;
				}

			default:
				break;
			}
		}
	}
}