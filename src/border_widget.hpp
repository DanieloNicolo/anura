/*
	Copyright (C) 2003-2013 by David White <davewx7@gmail.com>
	
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef BORDER_WIDGET_HPP_INCLUDED
#define BORDER_WIDGET_HPP_INCLUDED

#include "graphics.hpp"
#include "color_utils.hpp"
#include "widget.hpp"

namespace gui {

//a widget which draws a border around another widget it holds as its child.
class border_widget : public widget
{
public:
	border_widget(widget_ptr child, graphics::color col, int border_size=2);
	border_widget(widget_ptr child, const SDL_Color& color, int border_size=2);
	border_widget(const variant& v, game_logic::formula_callable* e);
	void set_color(const graphics::color& col);
	virtual widget_ptr get_widget_by_id(const std::string& id);
	const_widget_ptr get_widget_by_id(const std::string& id) const;
protected:
	virtual void handle_draw() const;
	virtual void handle_process();
private:
	bool handle_event(const SDL_Event& event, bool claimed);

	widget_ptr child_;
	graphics::color color_;
	int border_size_;
};

typedef boost::intrusive_ptr<border_widget> border_widget_ptr;

}

#endif
