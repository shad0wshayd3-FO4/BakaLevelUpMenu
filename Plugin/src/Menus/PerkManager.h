#pragma once

#include "Translations/Translations.h"

namespace Menus
{
	class PerkManager
	{
	public:
		using PerkMap = std::map<std::uint32_t, RE::BGSPerk*>;

		class PerkCondition
		{
		public:
			PerkCondition(RE::TESConditionItem* a_condition)
			{
				RE::stl::enumeration functionID{ RE::SCRIPT_OUTPUT::START_OF_FUNCTION_SECTION, a_condition->data.functionData.function.get() };
				switch (functionID.get())
				{
				case RE::SCRIPT_OUTPUT::FUNCTION_GET_ACTOR_VALUE:
				case RE::SCRIPT_OUTPUT::FUNCTION_GET_BASE_ACTOR_VALUE:
				case RE::SCRIPT_OUTPUT::FUNCTION_GET_PERMANENT_ACTOR_VALUE:
				{
					auto actorValue = static_cast<RE::ActorValueInfo*>(a_condition->data.functionData.param[0]);
					if (!actorValue)
					{
						_isValid = false;
						break;
					}

					auto compareValue = a_condition->GetComparisonValue();
					switch (a_condition->data.condition)
					{
					case RE::ENUM_COMPARISON_CONDITION::kEqual:
						_conditionText = fmt::format(
							fmt::runtime(Translations::Formatting::Equal.data()),
							actorValue->GetFullName(),
							compareValue);
						break;
					case RE::ENUM_COMPARISON_CONDITION::kNotEqual:
						_conditionText = fmt::format(
							fmt::runtime(Translations::Formatting::NotEqual.data()),
							actorValue->GetFullName(),
							compareValue);
						break;
					case RE::ENUM_COMPARISON_CONDITION::kGreaterThan:
						_conditionText = fmt::format(
							fmt::runtime(Translations::Formatting::Greater.data()),
							actorValue->GetFullName(),
							compareValue + 1.0F);
						break;
					case RE::ENUM_COMPARISON_CONDITION::kGreaterThanEqual:
						_conditionText = fmt::format(
							fmt::runtime(Translations::Formatting::GreaterEqual.data()),
							actorValue->GetFullName(),
							compareValue);
						break;
					case RE::ENUM_COMPARISON_CONDITION::kLessThan:
						_conditionText = fmt::format(
							fmt::runtime(Translations::Formatting::Less.data()),
							actorValue->GetFullName(),
							compareValue);
						break;
					case RE::ENUM_COMPARISON_CONDITION::kLessThanEqual:
						_conditionText = fmt::format(
							fmt::runtime(Translations::Formatting::LessEqual.data()),
							actorValue->GetFullName(),
							compareValue + 1.0F);
						break;
					default:
						_isValid = false;
						break;
					}

					_isTrue = a_condition->IsTrue(RE::PlayerCharacter::GetSingleton(), nullptr);
					break;
				}

				case RE::SCRIPT_OUTPUT::FUNCTION_GET_IS_SEX:
				case RE::SCRIPT_OUTPUT::FUNCTION_GET_GLOBAL_VALUE:
				{
					if (!a_condition->IsTrue(RE::PlayerCharacter::GetSingleton(), nullptr))
					{
						_isValid = false;
					}

					_isBlank = true;
					break;
				}

				case RE::SCRIPT_OUTPUT::FUNCTION_HAS_PERK:
				{
					auto perk = static_cast<RE::BGSPerk*>(a_condition->data.functionData.param[0]);
					if (!perk)
					{
						_isValid = false;
						break;
					}

					auto compareValue = a_condition->GetComparisonValue();
					if (compareValue != 1.0F)
					{
						_isValid = false;
						break;
					}

					switch (a_condition->data.condition)
					{
					case RE::ENUM_COMPARISON_CONDITION::kEqual:
						_conditionText = fmt::format(fmt::runtime(Translations::Formatting::HasPerk.data()), perk->GetFullName());
						break;
					case RE::ENUM_COMPARISON_CONDITION::kNotEqual:
						_conditionText = fmt::format(fmt::runtime(Translations::Formatting::NotPerk.data()), perk->GetFullName());
						break;
					default:
						_isValid = false;
						break;
					}

					_isTrue = a_condition->IsTrue(RE::PlayerCharacter::GetSingleton(), nullptr);
					break;
				}

				default:
					break;
				}

				_isOr = (a_condition->next && a_condition->data.compareOr);
			}

			constexpr std::string_view GetConditionText() const noexcept { return { _conditionText.data(), _conditionText.size() }; }

			constexpr bool IsOr() const noexcept { return _isOr; }

			constexpr bool IsTrue() const noexcept { return _isTrue; }

			constexpr bool IsBlank() const noexcept { return _isBlank; }

			constexpr bool IsValid() const noexcept { return _isValid; }

		private:
			std::string _conditionText;
			bool _isOr{ false };
			bool _isTrue{ true };
			bool _isValid{ true };
			bool _isBlank{ false };
		};

		class PerkConditions
		{
		public:
			PerkConditions(RE::BGSPerk* a_perk)
			{
				if (auto condition = a_perk->perkConditions.head)
				{
					do
					{
						PerkCondition newCondition{ condition };
						if (!newCondition.IsValid())
						{
							_isValid = false;
							return;
						}

						if (!newCondition.IsTrue())
						{
							_isAvailable = false;
						}

						_conditions.emplace_back(newCondition);
						condition = condition->next;
					}
					while (condition);
				}

				std::stringstream ss;
				for (auto i = 0; i < _conditions.size();)
				{
					auto& condition = _conditions[i];
					if (condition.IsBlank())
					{
						i++;
						continue;
					}

					if (condition.IsTrue())
					{
						ss << condition.GetConditionText();
					}
					else
					{
						ss << ErrorTag(condition.GetConditionText());
					}

					_isEmpty = false;
					if (++i != _conditions.size() && !_conditions[i].IsBlank())
					{
						if (condition.IsOr())
						{
							ss << " or ";
						}
						else
						{
							ss << ", ";
						}
					}
				}

				_conditionText = ss.str();
			}

			constexpr std::string_view GetConditionText() const noexcept { return { _conditionText.data(), _conditionText.size() }; }

			constexpr bool IsEmpty() const noexcept { return _isEmpty; }

			constexpr bool IsValid() const noexcept { return _isValid; }

			constexpr bool IsAvailable() const noexcept { return _isAvailable; }

		private:
			std::vector<PerkCondition> _conditions;
			std::string _conditionText;
			bool _isEmpty{ true };
			bool _isValid{ true };
			bool _isAvailable{ true };
		};

		class PerkRank
		{
		public:
			PerkRank(RE::BGSPerk* a_perk)
			{
				_perk = a_perk;
				_perk->GetDescription(_description);
				_name = _perk->GetFullName();

				GetConditions();
			}

			constexpr RE::BGSPerk* operator->() const
			{
				return _perk;
			}

			constexpr std::string_view GetName() const noexcept { return { _name.data(), _name.size() }; }

			constexpr std::string_view GetConditionText() const noexcept { return { _conditionText.data(), _conditionText.size() }; }

			constexpr std::string_view GetDescription() const noexcept { return { _description.data(), _description.size() }; }

			constexpr std::string_view GetPerkIcon() const noexcept { return { _perkIcon.data(), _perkIcon.size() }; }

			constexpr RE::BGSPerk* GetPerk() const noexcept { return _perk; }

			constexpr bool IsValid() const noexcept { return _isValid; }

			constexpr bool IsAvailable() const noexcept { return _isAvailable; }

			constexpr std::int8_t GetPerkLevel() const noexcept { return _perkLevel; }

			void SetPerkIcon(std::string_view a_path) noexcept { _perkIcon = a_path; }

		private:
			void GetConditions()
			{
				PerkConditions perkConditions{ _perk };
				_isValid = perkConditions.IsValid();
				_isAvailable = perkConditions.IsAvailable();
				_perkLevel = std::max(_perk->data.level, static_cast<std::int8_t>(1));

				auto refrLevel = RE::PlayerCharacter::GetSingleton()->GetLevel();

				std::string levelText = fmt::format(fmt::runtime(Translations::Formatting::Level.data()), _perkLevel);
				std::string ranksText = fmt::format(fmt::runtime(Translations::Formatting::Ranks.data()), _perk->data.numRanks);
				if (refrLevel < _perkLevel)
				{
					levelText = ErrorTag(levelText);
					_isAvailable = false;
				}

				if (perkConditions.IsEmpty())
				{
					if (_perkLevel < 3)
					{
						levelText = "--";
					}

					std::string reqsText = fmt::format(fmt::runtime(Translations::Formatting::Reqs.data()), levelText);
					_conditionText = fmt::format(
						FMT_STRING("{:s}<br>{:s}<br><br>{:s}"sv),
						reqsText,
						ranksText,
						GetDescription());
				}
				else
				{
					std::string reqsText = fmt::format(fmt::runtime(Translations::Formatting::Reqs.data()), levelText);
					_conditionText = fmt::format(
						FMT_STRING("{:s}, {:s}<br>{:s}<br><br>{:s}"sv),
						reqsText,
						perkConditions.GetConditionText(),
						ranksText,
						GetDescription());
				}
			}

			std::string _name;
			std::string _conditionText;
			std::string _perkIcon;
			RE::BSStringT<char> _description;
			RE::BGSPerk* _perk{ nullptr };
			bool _isValid;
			bool _isAvailable;
			std::int8_t _perkLevel;
		};

		class PerkChain
		{
		public:
			PerkChain(RE::BGSPerk* a_perk)
			{
				if (!a_perk->nextPerk && a_perk->data.numRanks > 1)
				{
					for (auto i = 0; i < a_perk->data.numRanks; i++)
					{
						Add(a_perk);
					}
				}
				else
				{
					do
					{
						Add(a_perk);
						a_perk = a_perk->nextPerk;
					}
					while (a_perk && a_perk != _perkChain[0].GetPerk());
				}

				SetPerkIcons();
			}

			std::vector<PerkRank> Get()
			{
				return _perkChain;
			}

			std::pair<PerkRank, std::int8_t> GetFirstAvailableRank()
			{
				for (std::int8_t i = 0; i < _perkChain.size(); i++)
				{
					auto& curPerk = _perkChain[i];
					auto curRank = RE::PlayerCharacter::GetSingleton()
					                   ->GetPerkRank(curPerk.GetPerk());

					if (!curPerk.IsValid())
					{
						return { _perkChain[0], static_cast<std::int8_t>(-1) };
					}

					if (curRank == 0)
					{
						return { curPerk, i };
					}

					if (!curPerk->nextPerk && curPerk->data.numRanks > curRank)
					{
						return { curPerk, i };
					}
				}

				return { _perkChain[0], static_cast<std::int8_t>(-1) };
			}

		private:
			void Add(RE::BGSPerk* a_perk)
			{
				PerkRank rank{ a_perk };
				_perkChain.emplace_back(rank);
			}

			void SetPerkIcons()
			{
				auto IsValidPath = [](const std::string& a_path)
				{
					RE::BSTSmartPointer<RE::BSResource::Stream> stream{ nullptr };
					auto relativePath = fmt::format(FMT_STRING("Interface\\{:s}"sv), a_path);
					return (RE::BSResource::GetOrCreateStream(relativePath.c_str(), stream) == RE::BSResource::ErrorCode::kNone);
				};

				for (auto i = 0; i < _perkChain.size(); i++)
				{
					if (IsValidPath(_perkChain[i]->swfFile.c_str()))
					{
						_perkChain[i].SetPerkIcon(_perkChain[i]->swfFile);
						continue;
					}

					auto formattedPath = fmt::format(FMT_STRING("Components\\VaultBoys\\Perks\\PerkClip_{:x}.swf"sv), _perkChain[i]->formID);
					if (IsValidPath(formattedPath))
					{
						_perkChain[i].SetPerkIcon(formattedPath);
						continue;
					}

					if (i != 0)
					{
						_perkChain[i].SetPerkIcon(_perkChain[0].GetPerkIcon());
						continue;
					}

					_perkChain[i].SetPerkIcon("Components\\Quest Vault Boys\\Miscellaneous Quests\\DefaultBoy.swf"sv);
				}
			}

			std::vector<PerkRank> _perkChain;
		};

		class PerkChainList :
			public std::vector<PerkChain>
		{
		public:
			void AddChain(RE::BGSPerk* a_perk, PerkMap& a_perkMap)
			{
				PerkChain chain{ a_perk };
				for (auto& perk : chain.Get())
				{
					a_perkMap.insert_or_assign(perk->formID, nullptr);
				}

				emplace_back(chain);
			}
		};

		PerkManager()
		{
			auto TESDataHandler = RE::TESDataHandler::GetSingleton();
			if (!TESDataHandler)
			{
				return;
			}

			PerkMap perkMap;
			for (auto perk : TESDataHandler->GetFormArray<RE::BGSPerk>())
			{
				if (perk->data.hidden || !perk->data.playable)
				{
					continue;
				}

				std::string_view name{ perk->GetFullName() };
				if (name.empty())
				{
					continue;
				}

				perkMap.insert_or_assign(perk->formID, perk);
			}

			for (auto& [formID, perk] : perkMap)
			{
				auto firstPerk = GetFirstPerkInChain(perk);
				if (!firstPerk)
				{
					continue;
				}

				if (perk->data.trait)
				{
					m_TraitChains.AddChain(firstPerk, perkMap);
				}
				else
				{
					m_PerkChains.AddChain(firstPerk, perkMap);
				}
			}
		}

		PerkChainList GetPerkChains() const noexcept
		{
			return m_PerkChains;
		}

		PerkChainList GetTraitChains() const noexcept
		{
			return m_TraitChains;
		}

	private:
		static std::string ErrorTag(std::string_view a_string)
		{
			return fmt::format(FMT_STRING("<font color=\'#888888\'>{:s}</font>"sv), a_string);
		}

		RE::BGSPerk* GetFirstPerkInChain(RE::BGSPerk* a_perk)
		{
			if (!a_perk)
			{
				return nullptr;
			}

			auto basePerk = a_perk;
			auto nextPerk = a_perk->nextPerk;
			for (auto i = 0; nextPerk && i < a_perk->data.numRanks; i++)
			{
				if (basePerk->data.level > nextPerk->data.level)
				{
					basePerk = nextPerk;
				}

				nextPerk = nextPerk->nextPerk;
			}

			return basePerk;
		}

		PerkChainList m_PerkChains;
		PerkChainList m_TraitChains;
	};
}
