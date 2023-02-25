#pragma once
#include "Font.hpp"
#include "Rect.hpp"
#include "Scene.hpp"
#include "Tab.hpp"
#include <vector>

class GuiVars
{
public:
	GuiVars();
	void Cleanup();
	Birb::Scene game_scene;

	std::vector<Tab*> tabs;
	static constexpr int tab_bar_height = 30;
	static constexpr int tab_button_width = 160;

private:
	Birb::Font tab_button_title_font;
	Birb::Font tab_view_title_font;
	Birb::Rect background;

	/* Tabs */
	Tab* tab_ongoing_flips;
	void PopulateOnGoingFlipsTab();

	Tab* tab_calc;
	void PopulateCalcTab();

	Tab* tab_new_flip;
	void PopulateNewFlipTab();

	Tab* tab_stats;
	void PopulateStatsTab();

	Tab* tab_recommendations;
	void PopulateRecommendationsTab();
};
