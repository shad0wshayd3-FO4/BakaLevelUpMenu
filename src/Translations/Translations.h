#pragma once

namespace Translations
{
	class Formatting
	{
	public:
		inline static std::string Equal{ "{:s} is exactly {:0.0F}" };
		inline static std::string Greater{ "{:s} {:0.0F}" };
		inline static std::string GreaterEqual{ "{:s} {:0.0F}" };
		inline static std::string HasPerk{ "has {:s} perk" };
		inline static std::string Less{ "{:s} &lt; {:0.0F}" };
		inline static std::string LessEqual{ "{:s} &lt; {:0.0F}" };
		inline static std::string Level{ "Level {:d}" };
		inline static std::string LevelUpText{ "Welcome to Level {:d}" };
		inline static std::string NotEqual{ "{:s} is not {:0.0F}" };
		inline static std::string NotPerk{ "does not have {:s} perk" };
		inline static std::string PerkMenu{ "Perk Menu" };
		inline static std::string Ranks{ "Ranks: {:d}" };
		inline static std::string Reqs{ "Reqs: {:s}" };
	};

	inline static void GetTranslationStrings()
	{
		if (auto BSScaleformManager = RE::BSScaleformManager::GetSingleton(); BSScaleformManager && BSScaleformManager->loader)
		{
			if (auto BSScaleformTranslator =
			        static_cast<RE::BSScaleformTranslator*>(
						BSScaleformManager->loader->GetStateAddRef(
							RE::Scaleform::GFx::State::StateType::kTranslator)))
			{
				auto FetchTranslation = [](RE::BSScaleformTranslator* a_trns, const wchar_t* a_key, std::string& a_output)
				{
					auto it = a_trns->translator.translationMap.find(a_key);
					if (it != a_trns->translator.translationMap.end())
					{
						a_output.resize(512);
						sprintf_s(a_output.data(), 512, "%ws", it->second.data());
					}
				};

				FetchTranslation(BSScaleformTranslator, L"$BakaLUM_Equal", Formatting::Equal);
				FetchTranslation(BSScaleformTranslator, L"$BakaLUM_Greater", Formatting::Greater);
				FetchTranslation(BSScaleformTranslator, L"$BakaLUM_GreaterEqual", Formatting::GreaterEqual);
				FetchTranslation(BSScaleformTranslator, L"$BakaLUM_HasPerk", Formatting::HasPerk);
				FetchTranslation(BSScaleformTranslator, L"$BakaLUM_Less", Formatting::Less);
				FetchTranslation(BSScaleformTranslator, L"$BakaLUM_LessEqual", Formatting::LessEqual);
				FetchTranslation(BSScaleformTranslator, L"$BakaLUM_Level", Formatting::Level);
				FetchTranslation(BSScaleformTranslator, L"$BakaLUM_LevelUpText", Formatting::LevelUpText);
				FetchTranslation(BSScaleformTranslator, L"$BakaLUM_NotEqual", Formatting::NotEqual);
				FetchTranslation(BSScaleformTranslator, L"$BakaLUM_NotPerk", Formatting::NotPerk);
				FetchTranslation(BSScaleformTranslator, L"$BakaLUM_PerkMenu", Formatting::PerkMenu);
				FetchTranslation(BSScaleformTranslator, L"$BakaLUM_Ranks", Formatting::Ranks);
				FetchTranslation(BSScaleformTranslator, L"$BakaLUM_Reqs", Formatting::Reqs);
			}
		}
	}
}
