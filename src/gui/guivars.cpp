#include "GuiVars.hpp"
#include "Color.hpp"
#include <iostream>
#include <vector>

using namespace Birb;

GuiVars::GuiVars()
{
	/* Load resources */
	tab_button_title_font.LoadFont("birb2d_res/fonts/freefont/FreeMonoBold.ttf", 16);
	tab_view_title_font.LoadFont("birb2d_res/fonts/freefont/FreeMonoBold.ttf", 24);

	/* Construct default values for everything */
	background = Rect(0, 0, 1280, 720, Colors::Nord::PolarNight::nord0);
	game_scene.AddObject(background);

	/* Tabs */
	tab_ongoing_flips	= new Tab("Flips", &tab_button_title_font, "Current flips", &tab_view_title_font, game_scene, 0);
	tab_calc 			= new Tab("Flip test", &tab_button_title_font, "Flip calculator", &tab_view_title_font, game_scene, 1);
	tab_new_flip 		= new Tab("New flip", &tab_button_title_font, "Start a new flip", &tab_view_title_font, game_scene, 2);
	tab_recommendations = new Tab("Recommendations", &tab_button_title_font, "Recommended flips", &tab_view_title_font, game_scene, 3);
	tab_stats 			= new Tab("Stats", &tab_button_title_font, "Statistics", &tab_view_title_font, game_scene, 4);

	/* Add all of the tabs into the tab vector */
	tabs = std::vector<Tab*> {
		tab_ongoing_flips, tab_calc, tab_new_flip, tab_recommendations, tab_stats
	};

	/* Activate the current flips tab by default */
	tab_ongoing_flips->Activate();

	/** Populate the tabs **/
	PopulateOnGoingFlipsTab();
	PopulateCalcTab();
	PopulateNewFlipTab();
	PopulateStatsTab();
	PopulateRecommendationsTab();
}

void GuiVars::Cleanup()
{
	for (size_t i = 0; i < tabs.size(); ++i)
		delete tabs[i];
}

void GuiVars::PopulateOnGoingFlipsTab()
{

}

void GuiVars::PopulateCalcTab()
{

}

void GuiVars::PopulateNewFlipTab()
{

}

void GuiVars::PopulateStatsTab()
{

}

void GuiVars::PopulateRecommendationsTab()
{

}
