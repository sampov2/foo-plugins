/*
    Wdgts for Foo-YC20 UI
    Copyright (C) 2009  Sampo Savolainen <v2@iki.fi>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef _FOOYC20_WDGTS_H
#define _FOOYC20_WDGTS_H

#include "wdgt.h"

#define YC20_PNG_DIR "/home/v2/dev/foo/foo-plugins/incubator/foo-yc20/paper/"

namespace Wdgt
{

bool
check_cairo_png(cairo_surface_t *s)
{
        cairo_status_t _stat = cairo_surface_status(s);
        return !(_stat == CAIRO_STATUS_NO_MEMORY ||
                 _stat == CAIRO_STATUS_FILE_NOT_FOUND ||
                 _stat == CAIRO_STATUS_READ_ERROR);
        
}

inline cairo_surface_t *
load_png(std::string file)
{       
        file = YC20_PNG_DIR + file;

        cairo_surface_t *ret = cairo_image_surface_create_from_png (file.c_str());
        if (!check_cairo_png(ret)) {
                std::cerr << "Foo-YC20: could not open " << file << std::endl;
        }
        return ret;
}

class Lever : public Wdgt::Object
{
	public:
		Lever()
		{
			setValue(0);
		}

		void setValueFromDrag(float prevValue, float startY, float y)
		{
			setValue(prevValue + (y-startY)/((y2-y1)/2.0));
		}

		void setValue(float v)
		{
			if (v < 0.0) {
				v = 0.0;
			} else if (v > 1.0) {
				v = 1.0;
			}

			imageNum = round(3.0*v);
			value = (float)imageNum / 3.0;
		}

		float getValue() const 
		{
			return value;
		}

		void setPosition(float posX, float posY)
		{
			x1 = posX;
			y1 = posY;
			x2 = x1 + 40;
			y2 = y1 + 85;
			setValue(0.0);
		}

		void drawEmphasis(bool hover, cairo_t *cr) const
		{
			if (hover) {
				cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.1);
				cairo_rectangle(cr, x1, y1, x2, y2);
				cairo_fill(cr);
			}

		}

	protected:
		float value;
		int imageNum;

};

class DrawbarWhite : public Lever
{
	public:
		DrawbarWhite(float posX, float posY) 
		{
			setPosition(posX, posY);
		}
		
		virtual void drawWidget(bool hover, cairo_t *cr) const
		{
			cairo_set_source_surface(cr, images[imageNum], x1, y1);
			cairo_paint(cr);

			drawEmphasis(hover, cr);
		}
	
	private:

		static cairo_surface_t *images[];

};
class DrawbarBlack : public Lever
{
	public:
		DrawbarBlack(float posX, float posY) 
		{
			setPosition(posX, posY);
		}
		
		virtual void drawWidget(bool hover, cairo_t *cr) const
		{
			cairo_set_source_surface(cr, images[imageNum], x1, y1);
			cairo_paint(cr);

			drawEmphasis(hover, cr);
		}
	
	private:

		static cairo_surface_t *images[];

};
class DrawbarGreen : public Lever
{
	public:
		DrawbarGreen(float posX, float posY) 
		{
			setPosition(posX, posY);
		}
		
		virtual void drawWidget(bool hover, cairo_t *cr) const
		{
			cairo_set_source_surface(cr, images[imageNum], x1, y1);
			cairo_paint(cr);

			drawEmphasis(hover, cr);
		}
	
	private:

		static cairo_surface_t *images[];

};



};


#endif /* _FOOYC20_WDGTS_H */

