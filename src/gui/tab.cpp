#include "Tab.hpp"
#include "Vector/Vector2Int.hpp"

using namespace Birb;

Tab::Tab(std::string name, Birb::Font* button_font, std::string title, Birb::Font* title_font, Birb::Scene& game_scene, const int& tab_index)
{
	tab_text_background = button_dimensions;

	tab_button.renderingPriority 	= 2;
	tab_view.renderingPriority 		= 2;

	tab_text_background.color = inactive_color;
	tab_button.AddObject(tab_text_background);

	/* Create the tab button text */
	tab_button_text_entity.Construct(name, Vector2Int(0, 0), button_font, text_color);
	tab_button_text_entity.renderingPriority = 1;

	/* Center the tab text entity relative to its background */
	tab_button_text_entity.rect.x = (tab_text_background.w - tab_button_text_entity.rect.w) / 2.0f;
	tab_button_text_entity.rect.y = (tab_text_background.h - tab_button_text_entity.rect.h) / 2.0f;

	tab_button.AddObject(tab_button_text_entity);

	/* De-activate the tab by default */
	active = true;
	Deactivate();

	/* Add the tab scenes into the main game scene */
	game_scene.AddObject(tab_button);
	game_scene.AddObject(tab_view);

	/* Construct the tab view title text */
	tab_view_title_entity.Construct(title, Vector2Int(16, 16), title_font, text_color);
	tab_view.AddObject(tab_view_title_entity);

	/* Move the tab view scene into the correct position below the tab buttons */
	tab_view.SetPosition(Vector2(0, button_dimensions.h));

	/* Move the tab button into its correct position */
	tab_button.SetPosition(Vector2(tab_text_background.w * tab_index, tab_text_background.y));
}

void Tab::Activate()
{
	if (!active)
	{
		tab_view.Activate();
		tab_text_background.color = active_color;
		active = true;
	}
}

void Tab::Deactivate()
{
	if (active)
	{
		tab_view.Deactivate();
		tab_text_background.color = inactive_color;
		active = false;
	}
}
