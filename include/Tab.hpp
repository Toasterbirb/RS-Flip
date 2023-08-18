#pragma once
#include "Color.hpp"
#include "Entities/Text.hpp"
#include "Font.hpp"
#include "Scene.hpp"

class Tab
{
public:
	Tab(std::string name, Birb::Font* button_font, std::string title, Birb::Font* title_font, Birb::Scene& game_scene, const int& tab_index);
	std::string name;
	Birb::Scene tab_view;
	Birb::Scene tab_button;

	static inline Birb::Color active_color 		= Birb::Colors::Nord::Frost::nord10;
	static inline Birb::Color inactive_color 	= Birb::Colors::Nord::PolarNight::nord3;
	static inline Birb::Color text_color 		= Birb::Colors::Nord::SnowStorm::nord6;
	static inline Birb::Rect button_dimensions 	= Birb::Rect(0, 0, 160, 30);

	void Activate();
	void Deactivate();

private:
	Birb::Rect tab_text_background;
	Birb::Entity::Text tab_button_text_entity;
	Birb::Entity::Text tab_view_title_entity;
	bool active;
};
