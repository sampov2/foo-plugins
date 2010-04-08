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

class Draggable : public Wdgt::Object
{
	public:
		virtual bool setValue(float v) = 0;
		virtual float getValue() const = 0;
		virtual bool setValueFromDrag(float prevValue, float startY, float y) = 0;
};

class Lever : public Draggable
{
	public:
		Lever(bool notches)
		{
			notched = notches;
			setValue(0);
		}

		bool setValueFromDrag(float prevValue, float startY, float y)
		{
			return setValue(prevValue + (y-startY)/((y2-y1)/2.0));
		}

		virtual bool setValue(float v)
		{
			if (v < 0.0) {
				v = 0.0;
			} else if (v > 1.0) {
				v = 1.0;
			}

			imageNum = round(3.0*v);

			float newvalue;

			if (notched) {
				newvalue = (float)imageNum / 3.0;
			} else {
				newvalue = v;
			}
			
			if (value == newvalue) {
				return false;
			}

			value = newvalue;
			return true;
			//std::cerr << "value = " << value << ", image = " << imageNum << std::endl;
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
		bool notched;
		float value;
		int imageNum;

};

class DrawbarWhite : public Lever
{
	public:
		DrawbarWhite(float posX, float posY) 
			: Lever(true)
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
		DrawbarBlack(float posX, float posY, bool notches) 
			: Lever(notches)
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
			: Lever(true)
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


class SwitchBlack : public DrawbarBlack
{
	public:
		SwitchBlack(float posX, float posY)
			: DrawbarBlack(posX, posY, false)
		{
		}

		virtual bool setValue(float v)
		{
			if (v < 0.5) {
				v = 0.0;
			} else {
				v = 1.0;
			}
			return DrawbarBlack::setValue(v);
		}
};


class Potentiometer : public Draggable
{
	public:
		Potentiometer(float posX, float posY, float min, float max)
		{
			minValue = min;
			maxValue = max;

			value = maxValue / (maxValue - minValue);

			setValue( (min+max)/2.0 );

			setPosition(posX, posY);
		}
		
		virtual bool setValue(float v)
		{
			if (v > maxValue) {
				v = maxValue;
			} else if (v < minValue) {
				v = minValue;
			}

			if (v == value) {
				return false;
			}

			value = v;

			return true;
		}


		float getValue() const 
		{
			return value;
		}

		void setPosition(float posX, float posY)
		{
			x1 = posX;
			y1 = posY;
			x2 = x1 + 72;
			y2 = y1 + 83;

			origoX = x1 + 72.0/2.0;
			origoY = y1 + 37.5;
		}

		bool setValueFromDrag(float prevValue, float startY, float y)
		{
			float v = prevValue + (startY - y)/100.0;

			return setValue(v);

		}

		virtual void drawWidget(bool hover, cairo_t *cr) const
		{
			cairo_set_source_surface(cr, image, x1, y1);
			cairo_paint(cr);

			cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
			cairo_set_line_width(cr, 2.5);

			float relativeValue = -(maxValue - value) / (maxValue - minValue);
			relativeValue *= (5.0/6.0);
			relativeValue += 1.0/6.0;

			float x2offt = 32.0 * cos(M_PI * 2.0 * relativeValue);
			float y2offt = 32.0 * sin(M_PI * 2.0 * relativeValue);

			float x1offt = 8.0  * cos(M_PI * 2.0 * relativeValue);
			float y1offt = 8.0  * sin(M_PI * 2.0 * relativeValue);

			cairo_move_to(cr, origoX + x1offt, origoY + y1offt);

			cairo_line_to(cr, origoX + x2offt, origoY + y2offt);

			cairo_stroke(cr);

			drawEmphasis(hover, cr);
		}

		void drawEmphasis(bool hover, cairo_t *cr) const
		{
			if (hover) {
				cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.1);
				cairo_rectangle(cr, x1, y1, x2, y2);
				cairo_fill(cr);
			}
		}

	private:
		float value;
		float minValue;
		float maxValue;

		float origoX;
		float origoY;

		static cairo_surface_t *image;
		
};

};


#endif /* _FOOYC20_WDGTS_H */

