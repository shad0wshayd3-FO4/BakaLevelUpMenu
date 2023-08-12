#pragma once

namespace Translations
{
	class Formatting
	{
	public:
		inline static std::string Equal;
		inline static std::string Greater;
		inline static std::string GreaterEqual;
		inline static std::string HasPerk;
		inline static std::string Less;
		inline static std::string LessEqual;
		inline static std::string Level;
		inline static std::string LevelUpText;
		inline static std::string NotEqual;
		inline static std::string NotPerk;
		inline static std::string PerkMenu;
		inline static std::string Ranks;
		inline static std::string Reqs;

		inline static bool m_RunOnce{ true };
	};

	static void GetTranslationStrings()
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
