#pragma once

namespace Menus::Utils
{
	using SORT_ON_FIELD = RE::ContainerMenuBase::ItemSorter::SORT_ON_FIELD;
	namespace detail
	{
		SORT_ON_FIELD GetNextSort(SORT_ON_FIELD a_current, std::vector<SORT_ON_FIELD> a_valid)
		{
			auto iter = std::find(a_valid.begin(), a_valid.end(), a_current);
			return (std::next(iter) == a_valid.end()) ? a_valid.front() : *std::next(iter);
		}
	}

	void ContainerMenuBase__IncrementSort(RE::ContainerMenuBase::ItemSorter* a_this)
	{
		auto currentTab = a_this->currentTab;
		switch (currentTab)
		{
			case 1:	 // Weapons
				a_this->currentSort[currentTab] =
					detail::GetNextSort(
						a_this->currentSort[currentTab].get(),
						{ SORT_ON_FIELD::kAlphabetical,
						  SORT_ON_FIELD::kDamage,
						  SORT_ON_FIELD::kRateOfFire,
						  SORT_ON_FIELD::kRange,
						  SORT_ON_FIELD::kAccuracy,
						  SORT_ON_FIELD::kValue,
						  SORT_ON_FIELD::kWeight });
				break;

			case 2:	 // Apparel
				a_this->currentSort[currentTab] =
					detail::GetNextSort(
						a_this->currentSort[currentTab].get(),
						{ SORT_ON_FIELD::kAlphabetical,
						  SORT_ON_FIELD::kDamage,
						  SORT_ON_FIELD::kValue,
						  SORT_ON_FIELD::kWeight });
				break;

			default:  // Other
				a_this->currentSort[currentTab] =
					detail::GetNextSort(
						a_this->currentSort[currentTab].get(),
						{ SORT_ON_FIELD::kAlphabetical,
						  SORT_ON_FIELD::kValue,
						  SORT_ON_FIELD::kWeight });
				break;
		}
	}
}
