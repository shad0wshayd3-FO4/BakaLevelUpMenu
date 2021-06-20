#pragma once

namespace Menus
{
	namespace Utils
	{
		namespace detail
		{
			namespace IC
			{
				namespace Settings
				{
					inline REL::Relocation<RE::SettingT<RE::INISettingCollection>*> uAmmoWeightPrecision{ REL::ID(880767) };
				}

				class ComponentData
				{
				public:
					ComponentData(RE::TESForm* a_component, std::uint32_t a_count) :
						_name(RE::TESFullName::GetFullName(*a_component)), _count(a_count)
					{
						if (auto FavoritesManager = RE::FavoritesManager::GetSingleton(); FavoritesManager)
						{
							_taggedForSearch = FavoritesManager->IsComponentFavorite(a_component->As<RE::TESBoundObject>());
						}
					}

					ComponentData(ComponentData& a_rhs, std::uint32_t a_count) :
						_name(a_rhs._name), _count(a_rhs._count + a_count), _taggedForSearch(a_rhs._taggedForSearch)
					{}

					constexpr bool operator<(ComponentData& a_rhs)
					{
						return _name < a_rhs._name;
					}

					constexpr auto GetName() const noexcept { return _name.c_str(); }
					constexpr auto GetCount() const noexcept { return _count; }
					constexpr auto GetTaggedForSearch() const noexcept { return _taggedForSearch; }

					void IncCount(std::uint32_t a_count)
					{
						_count += a_count;
					}

				private:
					std::string _name{ "" };
					std::uint32_t _count{ 0 };
					bool _taggedForSearch{ false };
				};
				using ComponentPair = std::pair<std::uint32_t, ComponentData>;
				using ComponentList = std::vector<ComponentPair>;

				using DamagePair = std::pair<std::uint32_t, float>;
				using DamageList = std::vector<DamagePair>;

				namespace Effects
				{
					class Base
					{
					public:
						Base(std::string a_name, float a_mag, float a_dur) :
							_name(a_name), _mag(a_mag), _dur(a_dur)
						{}

						virtual ~Base() = default;
						virtual void PopulateItemCard(RE::Scaleform::GFx::Value& a_itemCardInfoList, RE::Scaleform::GFx::Movie* a_movie) = 0;

					protected:
						std::string _name{ "" };
						float _mag{ 0.0f };
						float _dur{ 0.0f };
					};

					class Value :
						public Base
					{
					public:
						Value(std::string a_name, float a_mag, float a_dur) :
							Base(a_name, a_mag, a_dur)
						{}

						virtual void PopulateItemCard(RE::Scaleform::GFx::Value& a_itemCardInfoList, RE::Scaleform::GFx::Movie*) override
						{
							RE::Scaleform::GFx::Value gfx_Entry;
							RE::InventoryUserUIUtils::AddItemCardInfoEntry(a_itemCardInfoList, gfx_Entry, _name.c_str(), _mag);
							gfx_Entry.SetMember("duration", _dur);
						}
					};

					class Description :
						public Base
					{
					public:
						Description(std::string a_name) :
							Base(a_name, 0.0f, 0.0f)
						{}

						virtual void PopulateItemCard(RE::Scaleform::GFx::Value& a_itemCardInfoList, RE::Scaleform::GFx::Movie*) override
						{
							RE::Scaleform::GFx::Value gfx_Entry;
							RE::InventoryUserUIUtils::AddItemCardInfoEntry(a_itemCardInfoList, gfx_Entry, _name.c_str());
							gfx_Entry.SetMember("showAsDescription", true);
						}
					};

					class Stimpak :
						public Base
					{
					public:
						Stimpak(std::string a_name, float a_mag, float a_dur) :
							Base(a_name, a_mag, a_dur)
						{}

						virtual void PopulateItemCard(RE::Scaleform::GFx::Value& a_itemCardInfoList, RE::Scaleform::GFx::Movie*) override
						{
							RE::Scaleform::GFx::Value gfx_Entry;
							RE::InventoryUserUIUtils::AddItemCardInfoEntry(a_itemCardInfoList, gfx_Entry, _name.c_str(), _mag);
							gfx_Entry.SetMember("duration", _dur);
							gfx_Entry.SetMember("showAsPercent", true);
						}
					};
				}

				class EffectData
				{
				public:
					void ProcessEffects(RE::BSTArray<RE::EffectItem*> a_list, RE::MagicItem* a_magicItem)
					{
						auto PlayerCharacter = RE::PlayerCharacter::GetSingleton();

						for (auto item : a_list)
						{
							if (!item->conditions(PlayerCharacter, PlayerCharacter) ||
								!item->effectSetting ||
								!item->effectSetting->data.flags.none(RE::EffectSetting::EffectSettingData::Flag::kHideInUI))
							{
								continue;
							}

							auto itemMag = item->data.magnitude;
							auto itemDur = static_cast<float>(item->data.duration);
							RE::StatsMenuUtils::GetEffectDisplayInfo(a_magicItem, item, itemMag, itemDur);

							switch (item->effectSetting->data.archetype.get())
							{
								case RE::EffectArchetypes::ArchetypeID::kValueModifier:
								case RE::EffectArchetypes::ArchetypeID::kDualValueModifier:
								case RE::EffectArchetypes::ArchetypeID::kPeakValueModifier:
									{
										auto primaryAV = item->effectSetting->data.primaryAV;
										if (primaryAV)
										{
											std::string effectName{ !primaryAV->abbreviation.empty() ? primaryAV->abbreviation.c_str() : primaryAV->GetFullName() };
											if (!effectName.empty())
											{
												_effectList.push_back(
													new Effects::Value{ effectName.c_str(), itemMag, itemDur });
											}
										}
									}
									break;

								case RE::EffectArchetypes::ArchetypeID::kStimpak:
									{
										auto primaryAV = item->effectSetting->data.primaryAV;
										if (primaryAV)
										{
											std::string effectName{ !primaryAV->abbreviation.empty() ? primaryAV->abbreviation.c_str() : primaryAV->GetFullName() };
											if (!effectName.empty())
											{
												_effectList.push_back(
													new Effects::Stimpak{ effectName.c_str(), itemMag, itemDur });
											}
										}
									}
									break;

								case RE::EffectArchetypes::ArchetypeID::kScript:
								case RE::EffectArchetypes::ArchetypeID::kCalm:
								case RE::EffectArchetypes::ArchetypeID::kDemoralize:
								case RE::EffectArchetypes::ArchetypeID::kFrenzy:
								case RE::EffectArchetypes::ArchetypeID::kParalyze:
								case RE::EffectArchetypes::ArchetypeID::kCureAddiction:
								case RE::EffectArchetypes::ArchetypeID::kCloak:
								case RE::EffectArchetypes::ArchetypeID::kSlowTime:
									{
										RE::BSStringT<char> descriptionText;
										item->GetDescription(&descriptionText, "", "", itemMag, itemDur);
										if (!descriptionText.empty())
										{
											_effectList.push_back(
												new Effects::Description{ descriptionText.c_str() });
										}
									}
									break;

								case RE::EffectArchetypes::ArchetypeID::kChameleon:
								case RE::EffectArchetypes::ArchetypeID::kInvisibility:
									{
										// Chameleon for x Seconds?
									}
									break;

								case RE::EffectArchetypes::ArchetypeID::kStagger:
								case RE::EffectArchetypes::ArchetypeID::kUnused01:
								case RE::EffectArchetypes::ArchetypeID::kUnused02:
									{
										// Ignore
									}
									break;

								default:
									logger::warn(
										FMT_STRING("Unhandled ArchetypeID {:02d} on FormID {:08X}"),
										item->effectSetting->data.archetype.underlying(),
										a_magicItem->formID);
									break;
							}
						}
					}

					void ProcessDescription(std::string a_description)
					{
						_effectList.push_back(
							new Effects::Description{ a_description });
					}

					void PopulateItemCard(RE::Scaleform::GFx::Value& a_itemCardInfoList, RE::Scaleform::GFx::Movie* a_movie)
					{
						for (auto& iter : _effectList)
						{
							iter->PopulateItemCard(a_itemCardInfoList, a_movie);
						}
					}

				private:
					std::vector<Effects::Base*> _effectList;
				};
			}

			class ItemCardInfo
			{
			public:
				ItemCardInfo(const RE::BGSInventoryItem* a_item, std::uint32_t a_stackID) :
					_item(a_item), _stackID(a_stackID), _object(a_item->object)
				{
					_stack = a_item->GetStackByID(_stackID);
					if (_stack && _stack->extra)
					{
						auto extraID = _stack->extra->GetByType<RE::ExtraInstanceData>();
						if (extraID)
						{
							_data = extraID->data.get();
						}
					}

					InitComponents();
					InitDescription();
					InitDamage();
					InitHealth();
					InitWeaponData();
					InitEffects();
					InitValue();
					InitWeight();
				}

				template<class T>
				T* GetObjectAs() const noexcept
				{
					return _object ? _object->As<T>() : nullptr;
				}

			private:
				void InitComponents()
				{
					switch (_object->formType.get())
					{
						case RE::ENUM_FORM_ID::kMISC:
							{
								auto misc = _object->As<RE::TESObjectMISC>();
								if (misc && misc->componentData)
								{
									for (auto iter : *misc->componentData)
									{
										auto formID = iter.first->formID;
										if (auto idx = std::find_if(_componentList.begin(), _componentList.end(), [&](IC::ComponentPair& a_idx)
																	{ return a_idx.first == formID; });
											idx != _componentList.end())
										{
											idx->second.IncCount(iter.second.i);
										}
										else
										{
											_componentList.emplace_back(formID, IC::ComponentData{ iter.first, iter.second.i });
										}
									}
								}
							}
							break;
					}

					std::sort(
						_componentList.begin(),
						_componentList.end(),
						[](IC::ComponentPair& t1, IC::ComponentPair& t2)
						{ return t1.second < t2.second; });
				}

				void InitDescription()
				{
					switch (_object->formType.get())
					{
						case RE::ENUM_FORM_ID::kARMO:
						case RE::ENUM_FORM_ID::kWEAP:
							{
								if (_stack && _stack->extra)
								{
									auto omod = _stack->extra->GetLegendaryMod();
									if (omod)
									{
										RE::BSStringT<char> descriptionText;
										omod->GetDescription(descriptionText);
										if (!descriptionText.empty())
										{
											_effectData.ProcessDescription(descriptionText.c_str());
										}
									}
								}
							}
							break;

						case RE::ENUM_FORM_ID::kMISC:
							{
								auto misc = _object->As<RE::TESObjectMISC>();
								if (misc)
								{
									RE::BSScrapArray<RE::BGSMod::Attachment::Mod*> omods;
									RE::BGSMod::Attachment::Mod::FindModsForLooseMod(misc, omods);
									if (omods.size())
									{
										RE::BSStringT<char> descriptionText;
										omods[0]->GetDescription(descriptionText);
										if (!descriptionText.empty())
										{
											_effectData.ProcessDescription(descriptionText.c_str());
										}
									}
								}
							}
							break;
					}
				}

				void InitDamage()
				{
					RE::BSScrapArray<RE::BSTTuple<std::uint32_t, float>> TypeInfo;
					switch (_object->formType.get())
					{
						case RE::ENUM_FORM_ID::kARMO:
							RE::PipboyInventoryUtils::FillResistTypeInfo(*_item, _stack, TypeInfo, 1.0f);
							break;

						case RE::ENUM_FORM_ID::kWEAP:
							RE::PipboyInventoryUtils::FillDamageTypeInfo(*_item, _stack, TypeInfo);
							break;
					}

					for (auto iter : TypeInfo)
					{
						_damageList.emplace_back(iter.first, iter.second);
					}
				}

				void InitHealth()
				{
					switch (_object->formType.get())
					{
						case RE::ENUM_FORM_ID::kARMO:
							{
								_maxHealth = static_cast<float>(RE::TESHealthForm::GetFormHealth(_object, _data));
								_curHealth = 0.0f;

								if (_stack && _stack->extra)
								{
									_curHealth = _maxHealth * 1.0f;
									auto extraHealth = _stack->extra->GetByType<RE::ExtraHealth>();
									if (extraHealth)
									{
										_curHealth = _maxHealth * extraHealth->health;
									}
								}
							}
							break;

						case RE::ENUM_FORM_ID::kWEAP:
							{
							}
							break;

						case RE::ENUM_FORM_ID::kAMMO:
							{
								auto ammo = _object->As<RE::TESAmmo>();
								if (ammo && ammo->HasKeyword(RE::PowerArmor::GetBatteryKeyword()))
								{
									_maxHealth = RE::PowerArmor::fNewBatteryCapacity->GetFloat();
									_curHealth = 0.0f;

									if (_stack && _stack->extra)
									{
										_curHealth = _maxHealth * 1.0f;
										auto extraHealth = _stack->extra->GetByType<RE::ExtraHealth>();
										if (extraHealth)
										{
											_curHealth = _maxHealth * extraHealth->health;
										}
									}
								}
							}
							break;

						case RE::ENUM_FORM_ID::kALCH:
						case RE::ENUM_FORM_ID::kINGR:
							{
							}
							break;
					}
				}

				void InitWeaponData()
				{
					auto weap = _object->As<RE::TESObjectWEAP>();
					if (weap)
					{
						auto data = &weap->weaponData;
						if (_data)
						{
							data = reinterpret_cast<RE::TESObjectWEAP::Data*>(_data);
						}

						switch (data->type.get())
						{
							case RE::WEAPON_TYPE::kGun:
								{
									_weaponType = RE::WEAPON_TYPE::kGun;
									if (data->ammo)
									{
										_ammoName = data->ammo->shortDesc.c_str();
										if (RE::TESAmmo::GetReloadsWithAmmoRef(data->ammo))
										{
											_ammoText = "-"sv;
										}
										else
										{
											_ammoText = fmt::format(
												FMT_STRING("{:d}"),
												RE::PlayerCharacter::GetSingleton()->GetInventoryObjectCount(data->ammo));
										}
									}

									auto objectInstance = RE::BGSObjectInstanceT<RE::TESObjectWEAP>(weap, data);
									_weaponROF = RE::CombatFormulas::GetWeaponDisplayRateOfFire(*weap, data);
									_weaponRNG = RE::CombatFormulas::GetWeaponDisplayRange(objectInstance);
									_weaponACC = RE::CombatFormulas::GetWeaponDisplayAccuracy(objectInstance, nullptr);
								}
								break;

							case RE::WEAPON_TYPE::kGrenade:
							case RE::WEAPON_TYPE::kMine:
								{
									_weaponType = RE::WEAPON_TYPE::kGrenade;

									auto objectInstance = RE::BGSObjectInstanceT<RE::TESObjectWEAP>(weap, data);
									_weaponRNG = RE::CombatFormulas::GetWeaponDisplayRange(objectInstance);
								}
								break;

							default:
								{
									_weaponType = RE::WEAPON_TYPE::kOneHandSword;
									_meleeSpeed = RE::TESObjectWEAP::GetMeleeAttackSpeedLabel(weap->GetMeleeAttackSpeed());
								}
								break;
						}
					}
				}

				void InitEffects()
				{
					switch (_object->formType.get())
					{
						case RE::ENUM_FORM_ID::kARMO:
							{
								auto armo = _object->As<RE::TESObjectARMO>();
								if (armo)
								{
									RE::TESObjectARMO::InstanceData* data{ &armo->armorData };
									if (_data)
									{
										data = reinterpret_cast<RE::TESObjectARMO::InstanceData*>(_data);
									}

									if (armo->formEnchanting)
									{
										_effectData.ProcessEffects(armo->formEnchanting->listOfEffects, armo->formEnchanting);
									}

									if (data && data->enchantments)
									{
										for (auto iter : *data->enchantments)
										{
											_effectData.ProcessEffects(iter->listOfEffects, iter);
										}
									}
								}
							}
							break;

							/*
					case RE::ENUM_FORM_ID::kWEAP:
						{
							auto weap = _object->As<RE::TESObjectWEAP>();
							if (weap)
							{
								RE::TESObjectWEAP::Data* data{ &weap->weaponData };
								if (_data)
								{
									data = reinterpret_cast<RE::TESObjectWEAP::Data*>(_data);
								}

								if (weap->formEnchanting)
								{
									_effectData.ProcessEffects(weap->formEnchanting->listOfEffects, weap->formEnchanting);
								}

								if (data && data->enchantments)
								{
									for (auto iter : *data->enchantments)
									{
										_effectData.ProcessEffects(iter->listOfEffects, iter);
									}
								}
							}
						}
						break;
					*/

						case RE::ENUM_FORM_ID::kALCH:
						case RE::ENUM_FORM_ID::kINGR:
							{
								auto alch = _object->As<RE::MagicItem>();
								if (alch)
								{
									_effectData.ProcessEffects(alch->listOfEffects, alch);
								}
							}
							break;
					}
				}

				void InitValue()
				{
					_itemValue = _item->GetInventoryValue(_stackID, false);
					_fullValue = _item->GetInventoryValue(_stackID, true);
				}

				void InitWeight()
				{
					_itemWeight = std::max(0.0f, RE::TESWeightForm::GetFormWeight(_object, _data));
					if (_stack)
					{
						bool modifyStack{ false };
						_itemWeight = RE::PlayerCharacter::GetSingleton()->AdjustItemWeight(*_object, *_stack, _itemWeight, &modifyStack);
						_fullWeight = _itemWeight * _stack->count;
					}
				}

			protected:
				const RE::BGSInventoryItem* _item{ nullptr };
				RE::TESBoundObject* _object{ nullptr };
				RE::BGSInventoryItem::Stack* _stack{ nullptr };
				RE::TBO_InstanceData* _data{ nullptr };
				std::uint32_t _stackID{ 0 };

			public:
				IC::ComponentList _componentList;
				IC::DamageList _damageList;
				IC::EffectData _effectData;
				std::string _ammoName{ "" };
				std::string _ammoText{ "" };
				std::string _meleeSpeed{ "" };
				float _weaponROF{ 0.0f };
				float _weaponRNG{ 0.0f };
				float _weaponACC{ 0.0f };
				RE::WEAPON_TYPE _weaponType{ RE::WEAPON_TYPE::kNone };
				float _maxHealth{ 0.0f };
				float _curHealth{ 0.0f };
				float _itemWeight{ 0.0f };
				float _fullWeight{ 0.0f };
				std::int32_t _itemValue{ 0 };
				std::int32_t _fullValue{ 0 };
			};

			class ItemCardInfoEntry :
				public ItemCardInfo
			{
			public:
				ItemCardInfoEntry(
					RE::Scaleform::GFx::Movie* a_movie,
					RE::Scaleform::GFx::Value* a_itemCardInfoList,
					const RE::BGSInventoryItem* a_item,
					std::uint32_t a_stackID,
					RE::UIUtils::ComparisonItems& a_comparisonItems,
					bool a_forceArmorComparison = false) :
					ItemCardInfo(a_item, a_stackID),
					_movie(a_movie), _itemCardInfoList(a_itemCardInfoList), _forceArmorComparison(a_forceArmorComparison)
				{
					for (auto iter : a_comparisonItems)
					{
						_comparisonItems.emplace_back(iter.first, iter.second);
					}

					PopulateItemCardInfoList();
				}

			private:
				void PopulateItemCardInfoList()
				{
					switch (_object->formType.get())
					{
						case RE::ENUM_FORM_ID::kARMO:
							PopulateDamage();
							PopulateHealth();
							PopulateEffects();
							PopulateWeightValue();
							break;

						case RE::ENUM_FORM_ID::kWEAP:
							PopulateDamage();
							PopulateWeaponAmmo();
							// PopulateHealth();
							PopulateEffects();
							PopulateWeaponData();
							PopulateWeightValue();
							break;

						case RE::ENUM_FORM_ID::kALCH:
						case RE::ENUM_FORM_ID::kINGR:
							PopulateEffects();
							// PopulateHealth();
							PopulateWeightValue();
							break;

						case RE::ENUM_FORM_ID::kMISC:
							PopulateComponents();
							PopulateEffects();
							PopulateWeightValue();
							break;

						case RE::ENUM_FORM_ID::kAMMO:
							PopulateHealth();
							PopulateWeightValue();
							break;

						default:
							PopulateWeightValue();
					}
				}

				void PopulateComponents()
				{
					if (_componentList.size())
					{
						RE::Scaleform::GFx::Value gfx_ComponentA;
						_movie->CreateArray(&gfx_ComponentA);

						for (auto iter : _componentList)
						{
							RE::Scaleform::GFx::Value gfx_ComponentEntry;
							_movie->CreateObject(&gfx_ComponentEntry);
							gfx_ComponentEntry.SetMember("text", iter.second.GetName());
							gfx_ComponentEntry.SetMember("count", iter.second.GetCount());
							gfx_ComponentEntry.SetMember("taggedForSearch", iter.second.GetTaggedForSearch());
							gfx_ComponentA.PushBack(gfx_ComponentEntry);
						}

						RE::Scaleform::GFx::Value gfx_Components;
						RE::InventoryUserUIUtils::AddItemCardInfoEntry(*_itemCardInfoList, gfx_Components);
						gfx_Components.SetMember("components", gfx_ComponentA);
					}
				}

				void PopulateDamage()
				{
					IC::DamageList cDamageList(6);
					bool HasComparisonItem{ false };

					switch (_object->formType.get())
					{
						case RE::ENUM_FORM_ID::kARMO:
							{
								auto tARMO = _object->As<RE::TESObjectARMO>();
								for (auto iter : _comparisonItems)
								{
									auto cARMO = iter.GetObjectAs<RE::TESObjectARMO>();
									if (cARMO && RE::PipboyInventoryUtils::DoSlotsOverlap(tARMO, cARMO))
									{
										if (RE::PowerArmor::PlayerInPowerArmor())
										{
											auto pKYWD = RE::PowerArmor::GetArmorKeyword();
											if (!tARMO->HasKeyword(pKYWD) || !cARMO->HasKeyword(pKYWD))
											{
												continue;
											}
										}

										HasComparisonItem = true;
										for (auto i = 0; i < cDamageList.size(); i++)
										{
											cDamageList[i].second += iter._damageList[i].second;
										}
									}
								}
							}
							break;

						case RE::ENUM_FORM_ID::kWEAP:
							{
								for (auto iter : _comparisonItems)
								{
									if (_weaponType == iter._weaponType)
									{
										HasComparisonItem = true;
										for (auto i = 0; i < cDamageList.size(); i++)
										{
											cDamageList[i].second += iter._damageList[i].second;
										}
									}
								}
							}
							break;
					}

					if (!HasComparisonItem)
					{
						cDamageList = _damageList;
					}

					auto ItemCardStr = (_object->formType == RE::ENUM_FORM_ID::kARMO) ? "$dr" : "$dmg";
					for (auto i = 0; i < _damageList.size(); i++)
					{
						RE::Scaleform::GFx::Value gfx_Damage;
						auto tValue = _damageList[i].second;
						auto cValue = cDamageList[i].second;
						RE::InventoryUserUIUtils::AddItemCardInfoEntry(*_itemCardInfoList, gfx_Damage, ItemCardStr, tValue, tValue - cValue, tValue, cValue);
						gfx_Damage.SetMember("damageType", _damageList[i].first);
					}
				}

				void PopulateHealth()
				{
					if (_maxHealth == 0.0f)
					{
						return;
					}

					auto healthStr = fmt::format(FMT_STRING("{:.0f}/{:.0f}"), _curHealth, _maxHealth);
					switch (_object->formType.get())
					{
						case RE::ENUM_FORM_ID::kARMO:
							RE::InventoryUserUIUtils::AddItemCardInfoEntry(*_itemCardInfoList, "$health", healthStr.c_str());
							break;

						case RE::ENUM_FORM_ID::kWEAP:
							break;

						case RE::ENUM_FORM_ID::kAMMO:
							RE::InventoryUserUIUtils::AddItemCardInfoEntry(*_itemCardInfoList, "$charge", healthStr.c_str());
							break;

						case RE::ENUM_FORM_ID::kALCH:
						case RE::ENUM_FORM_ID::kINGR:
							break;
					}
				}

				void PopulateWeaponAmmo()
				{
					switch (_weaponType)
					{
						case RE::WEAPON_TYPE::kGun:
							{
								RE::Scaleform::GFx::Value gfx_Ammo;
								RE::InventoryUserUIUtils::AddItemCardInfoEntry(*_itemCardInfoList, gfx_Ammo, _ammoName.c_str(), _ammoText.c_str());
								gfx_Ammo.SetMember("damageType", 10);
							}
							break;
					}
				}

				void PopulateWeaponData()
				{
					auto cWeaponROF = _weaponROF;
					auto cWeaponRNG = _weaponRNG;
					auto cWeaponACC = _weaponACC;
					for (auto iter : _comparisonItems)
					{
						if (_weaponType == iter._weaponType)
						{
							cWeaponROF = iter._weaponROF;
							cWeaponRNG = iter._weaponRNG;
							cWeaponACC = iter._weaponACC;
							break;
						}
					}

					switch (_weaponType)
					{
						case RE::WEAPON_TYPE::kGun:
							{
								RE::Scaleform::GFx::Value gfx_ROF;
								RE::InventoryUserUIUtils::AddItemCardInfoEntry(*_itemCardInfoList, gfx_ROF, "$ROF", _weaponROF, _weaponROF - cWeaponROF);

								RE::Scaleform::GFx::Value gfx_RNG;
								RE::InventoryUserUIUtils::AddItemCardInfoEntry(*_itemCardInfoList, gfx_RNG, "$rng", _weaponRNG, _weaponRNG - cWeaponRNG);

								RE::Scaleform::GFx::Value gfx_ACC;
								RE::InventoryUserUIUtils::AddItemCardInfoEntry(*_itemCardInfoList, gfx_ACC, "$acc", _weaponACC, _weaponACC - cWeaponACC);
							}
							break;

						case RE::WEAPON_TYPE::kGrenade:
							{
								RE::Scaleform::GFx::Value gfx_RNG;
								RE::InventoryUserUIUtils::AddItemCardInfoEntry(*_itemCardInfoList, gfx_RNG, "$rng", _weaponRNG, _weaponRNG - cWeaponRNG);
							}
							break;

						case RE::WEAPON_TYPE::kOneHandSword:
							{
								RE::InventoryUserUIUtils::AddItemCardInfoEntry(*_itemCardInfoList, "$speed", _meleeSpeed.c_str());
							}
							break;
					}
				}

				void PopulateEffects()
				{
					_effectData.PopulateItemCard(*_itemCardInfoList, _movie);
				}

				void PopulateWeightValue()
				{
					// Weight
					{
						auto cItemWeight{ _itemWeight };
						auto WeightPrecision = (_object->formType == RE::ENUM_FORM_ID::kAMMO) ? IC::Settings::uAmmoWeightPrecision->GetUInt() : 1;

						if (_comparisonItems.size())
						{
							switch (_object->formType.get())
							{
								case RE::ENUM_FORM_ID::kARMO:
									{
										if (_forceArmorComparison)
										{
											auto tARMO = _object->As<RE::TESObjectARMO>();
											cItemWeight = 0.0f;

											for (auto iter : _comparisonItems)
											{
												auto cARMO = iter.GetObjectAs<RE::TESObjectARMO>();
												if (cARMO && RE::PipboyInventoryUtils::DoSlotsOverlap(tARMO, cARMO))
												{
													cItemWeight += iter._itemWeight;
												}
											}
										}
									}
									break;

								case RE::ENUM_FORM_ID::kWEAP:
									{
										for (auto iter : _comparisonItems)
										{
											if (_weaponType == iter._weaponType)
											{
												cItemWeight = iter._itemWeight;
												break;
											}
										}
									}
									break;
							}
						}

						RE::Scaleform::GFx::Value gfx_Weight;
						RE::InventoryUserUIUtils::AddItemCardInfoEntry(*_itemCardInfoList, gfx_Weight, "$wt", _itemWeight, _itemWeight - cItemWeight, _itemWeight, cItemWeight);
						gfx_Weight.SetMember("precision"sv, WeightPrecision);

						if (_itemWeight != _fullWeight)
						{
							RE::Scaleform::GFx::Value gfx_StackWeight;
							RE::InventoryUserUIUtils::AddItemCardInfoEntry(*_itemCardInfoList, gfx_StackWeight, "$stackwt", _fullWeight);
							gfx_StackWeight.SetMember("precision"sv, WeightPrecision);
						}
					}

					// Value
					{
						RE::Scaleform::GFx::Value gfx_Value;
						RE::InventoryUserUIUtils::AddItemCardInfoEntry(*_itemCardInfoList, gfx_Value, "$val", _itemValue);

						if (_itemValue != _fullValue)
						{
							RE::Scaleform::GFx::Value gfx_StackValue;
							RE::InventoryUserUIUtils::AddItemCardInfoEntry(*_itemCardInfoList, gfx_StackValue, "$stackval", _fullValue);
						}
					}

					// Value/Weight
					{
						if (_itemWeight > 0.0F)
						{
							RE::Scaleform::GFx::Value gfx_ValueWeight;
							auto ValueWeight = _itemValue / _itemWeight;
							RE::InventoryUserUIUtils::AddItemCardInfoEntry(*_itemCardInfoList, gfx_ValueWeight, "$valwt", ValueWeight);
						}
					}
				}

				std::vector<ItemCardInfo> _comparisonItems;
				RE::Scaleform::GFx::Movie* _movie{ nullptr };
				RE::Scaleform::GFx::Value* _itemCardInfoList{ nullptr };
				bool _forceArmorComparison{ false };
			};
		}

		void PopulateItemCardInfo(RE::Scaleform::GFx::Value& a_menuObj, const RE::BGSInventoryItem& a_item, std::uint32_t a_stackID, RE::UIUtils::ComparisonItems& a_comparisonItems, bool a_forceArmorComparison)
		{
			if (auto movie = a_menuObj.GetMovie(); movie)
			{
				RE::Scaleform::GFx::Value ItemCardInfoList;
				movie->CreateArray(&ItemCardInfoList);

				detail::ItemCardInfoEntry{ movie, &ItemCardInfoList, &a_item, a_stackID, a_comparisonItems, a_forceArmorComparison };
				a_menuObj.SetMember("ItemCardInfoList", ItemCardInfoList);
			}
			else
			{
				logger::error("Menus::Utils::PopulateItemCardInfo: menuObj has no parent movie."sv);
			}
		}
	}
}
