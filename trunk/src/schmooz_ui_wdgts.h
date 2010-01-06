/*
    Wdgts for Schmooz UI
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

#ifndef _SCHMOOZ_UI_WDGTS_H
#define _SCHMOOZ_UI_WDGTS_H

#include "wdgt.h"

namespace Wdgt
{

inline cairo_surface_t *
load_png(std::string file)
{	
	file = SCHMOOZ_PNG_DIR + file;

	cairo_surface_t *ret = cairo_image_surface_create_from_png (file.c_str());
	if (!check_cairo_png(ret)) {
		std::cerr << "SchmoozUI: could not open " << file << std::endl;
	}
	return ret;
}


class Button : public Wdgt::Object
{
public:
	Button(std::string pngBase)
	{
		std::string png_on           = pngBase + "_on.png";
		std::string png_on_prelight  = pngBase + "_on_prelight.png";
		std::string png_off          = pngBase + "_off.png";
		std::string png_off_prelight = pngBase + "_off_prelight.png";

		image_on           = load_png(png_on);
		image_on_prelight  = load_png(png_on_prelight);
		image_off          = load_png(png_off);
		image_off_prelight = load_png(png_off_prelight);

		status = false;
	}

	~Button()
	{
		cairo_surface_destroy(image_on);
		cairo_surface_destroy(image_on_prelight);
		cairo_surface_destroy(image_off);
		cairo_surface_destroy(image_off_prelight);
	}

	virtual void drawWidget(bool hover, cairo_t *cr) const
	{
		cairo_surface_t *tmp = NULL;
		switch( (hover ? 1 : 0) | (status ? 2: 0)) {
			case 0: tmp = image_off;
				break;
			case 1: tmp = image_off_prelight;
				break;
			case 2: tmp = image_on;
				break;
			case 3: tmp = image_on_prelight;
				break;
		}

		cairo_set_source_surface(cr, tmp, x1, y1);
		cairo_paint(cr);
	}

	bool getStatus() const   { return status; }
	void setStatus(bool tmp) { status = tmp; }
	bool toggleStatus()      { status = !status; return status; }

private:
	bool status;

	cairo_surface_t *image_on;
	cairo_surface_t *image_on_prelight;
	cairo_surface_t *image_off;
	cairo_surface_t *image_off_prelight;
};


// A "sliding control" which contains a single value for the widget
// value setters make sure the member float relative_value is between 0 and 1
// represeting the current value on a linear scale from the set minimum and
// maximum value. offset_x and offset_y are also computed. use as you wish
class SlidingControl : public Wdgt::Object
{
public:
	SlidingControl(float _min_value, float _max_value)
	{
		min_value = _min_value;
		max_value = _max_value;

		control_offset_x1 = 0.0;
		control_offset_x2 = 0.0;
		control_offset_y1 = 0.0;
		control_offset_y2 = 0.0;


		setValue(min_value);
	}

	float getValue() const { return value; }
	float getRelativeValue() const { return relative_value; }

	void setValue(float newvalue) 
	{ 
		value = newvalue;

		relative_value = (value - min_value) / (max_value - min_value);
		offset_x = control_offset_x1 + ( (x2+control_offset_x2) - (x1+control_offset_x1) ) * relative_value;
		offset_y = control_offset_y1 + ( (y2+control_offset_y2) - (y1+control_offset_y1) ) * relative_value;
		//std::cerr << "new value = " << value << ", offsets: " << offset_x << " or " << offset_y << std::endl;
	}


	void setValueFromVerticalDrag(float valueAtStart, int dragStart, int y)
	{
		float tmp = (max_value - min_value) * 
				(float)(y - dragStart) / 
				( (y2+control_offset_y2) - (y1+control_offset_y1) ) + valueAtStart;
		//std::cerr << " [" << min_value << " .. " << max_value << "], drag = " << dragStart << " -> " << y << " => value = " << tmp << std::endl;


		tmp = fmax(min_value, fmin(tmp, max_value) );
	
		setValue(tmp);
	}

	void setValueFromHorizontalDrag(float valueAtStart, int dragStart, int x)
	{
		float tmp = (max_value - min_value) * 
				(float)(x - dragStart) / 
				( (x2+control_offset_x2) - (x1+control_offset_x1) ) + valueAtStart;
	
		//std::cerr << " [" << min_value << " .. " << max_value << "], drag = " << dragStart << " -> " << x << " => value = " << tmp << std::endl;

		tmp = fmax(min_value, fmin(tmp, max_value) );

		setValue(tmp);
	}

	void setControlOffsetX(float offt1, float offt2)
	{
		control_offset_x1 = offt1;
		control_offset_x2 = offt2;

		setValue(value);
	}

	void setControlOffsetY(float offt1, float offt2)
	{
		control_offset_y1 = offt1;
		control_offset_y2 = offt2;

		setValue(value);
	}


protected:
	float offset_x;
	float offset_y;

	float max_value;
	float min_value;

	float value;
	float relative_value;

	float control_offset_x1;
	float control_offset_x2;
	float control_offset_y1;
	float control_offset_y2;
};

class ThresholdGraph : public Wdgt::Object
{
public:
	ThresholdGraph(SlidingControl *_threshold_control, SlidingControl *_ratio_control)
		: threshold_control(_threshold_control)
		, ratio_control(_ratio_control)
	{
		image_graph_bg      = load_png("graph_bg.png");
		image_threshold     = load_png("graph_bg_threshold.png");
		
		// This is unnecessary as the graph is always redrawn when
		// the threshold control is. I'll keep it here to document the relationship
		// for future reference.
		//_threshold_control->dependents.push_back(this);
		_ratio_control    ->dependents.push_back(this);
	}
	
	~ThresholdGraph()
	{
		cairo_surface_destroy(image_graph_bg);
		cairo_surface_destroy(image_threshold);
	}

	virtual void drawWidget(bool hover, cairo_t *cr) const
	{
		float cutoff_x = (x2-x1) * (threshold_control->getRelativeValue());
		float cutoff_y = (y2-y1) * (1.0 - threshold_control->getRelativeValue());

		// draw the background
		cairo_set_source_surface(cr, image_graph_bg, x1, y1);
		cairo_paint(cr);

		// then the "compression triangle"
		cairo_set_source_surface(cr, image_threshold, x1, y1);

		float offset_y_min_ratio = cutoff_y - cutoff_y / 1.5;
		float offset_y_max_ratio = cutoff_y - cutoff_y / 20.0;

		cairo_move_to(cr, x2, y1 + offset_y_min_ratio);
		cairo_line_to(cr, x1 + cutoff_x, y1 + cutoff_y);
		cairo_line_to(cr, x2, y1 + offset_y_max_ratio);
		cairo_close_path(cr);

		cairo_fill(cr);

		// the line from the bottom left corner up to the threshold
		cairo_set_line_width(cr, 1.0);
		cairo_set_source_rgb(cr, 0.6, 0.6, 0.6);
		cairo_move_to(cr, x1, y2);
		cairo_line_to(cr, x1 + cutoff_x, y1 + cutoff_y);
		cairo_stroke(cr);

		// the line from the threshold point to the right side, as affected by ratio
		cairo_set_source_rgb(cr, 0.412, 0.820, 0.976);
		cairo_move_to(cr, x1 + cutoff_x, y1 + cutoff_y);
		cairo_line_to(cr, x2, y1 + cutoff_y - cutoff_y / ratio_control->getValue());
		cairo_stroke(cr);
	}

private:
	cairo_surface_t *image_graph_bg;
	cairo_surface_t *image_threshold;

	SlidingControl *threshold_control;
	SlidingControl *ratio_control;
};

class ThresholdControl : public SlidingControl
{
public:
	ThresholdControl(float _min_value, float _max_value, double _clip_x1, double _clip_x2)
		: SlidingControl(_min_value, _max_value)
	{
		image_thr_cntrl          = load_png("threshold.png");
		image_thr_cntrl_prelight = load_png("threshold_prelight.png");

		control_w = 15.0;

		clip_x1 = _clip_x1;
		clip_x2 = _clip_x2;
	}

	~ThresholdControl()
	{
		cairo_surface_destroy(image_thr_cntrl);
		cairo_surface_destroy(image_thr_cntrl_prelight);
	}

	virtual void drawWidget(bool hover, cairo_t *cr) const
	{
		cairo_surface_t *tmp = NULL;

		if (hover) {
			tmp = image_thr_cntrl_prelight;
		} else {
			tmp = image_thr_cntrl;
		}

		double _tx1, _tx2;
		_tx1 = x1 + offset_x - control_w/2;
		_tx2 = x1 + offset_x + control_w/2;

		cairo_set_source_surface(cr, tmp, _tx1, y1);

		if (_tx1 < clip_x1) {
			_tx1 = clip_x1;
		} else if (_tx2 > clip_x2) {
			_tx2 = clip_x2;
		}

		cairo_rectangle(cr, _tx1, y1, _tx2 - _tx1, y2 - y1);
		cairo_fill(cr);
	}

	bool intersectsRectangle(double x, double y, double w, double h) const
	{
		return 	((x+w) >= (x1+offset_x - control_w/2.0) && 
			  x    <  (x1+offset_x + control_w/2.0) &&
                         (y+h) >= y1 && 
                          y    < y2);
	};
private:
	double control_w;

	double clip_x1;
	double clip_x2;

	cairo_surface_t *image_thr_cntrl;
	cairo_surface_t *image_thr_cntrl_prelight;
};


class RatioBackground : public Wdgt::Object
{
public:
	RatioBackground()
	{
		image_bg = load_png("ratio_trough.png");
	}
	
	~RatioBackground()
	{
		cairo_surface_destroy(image_bg);
	}

	virtual void drawWidget(bool hover, cairo_t *cr) const
	{
		cairo_set_source_surface(cr, image_bg, x1, y1);
		cairo_paint(cr);
	}

private:
	cairo_surface_t *image_bg;
};


class RatioControl : public SlidingControl
{
public:
	RatioControl(float _min_value, float _max_value)
		: SlidingControl(_min_value, _max_value)
	{
		image_ratio_cntrl          = load_png("ratio_thumb.png");
		image_ratio_cntrl_prelight = load_png("ratio_thumb_prelight.png");

		control_h = 12.0;

		setControlOffsetY(0, -control_h);
	}

	~RatioControl()
	{
		cairo_surface_destroy(image_ratio_cntrl);
		cairo_surface_destroy(image_ratio_cntrl_prelight);
	}

	virtual void drawWidget(bool hover, cairo_t *cr) const
	{
		cairo_surface_t *tmp = NULL;

		if (hover) {
			tmp = image_ratio_cntrl_prelight;
		} else {
			tmp = image_ratio_cntrl;
		}

		double _ty1, _ty2;
		_ty1 = y1 + offset_y;
		_ty2 = y1 + offset_y + control_h;

		cairo_set_source_surface(cr, tmp, x1, _ty1);

		cairo_rectangle(cr, x1, _ty1, x2 - x1, _ty2 - _ty1);
		cairo_fill(cr);
	}

	bool intersectsRectangle(double x, double y, double w, double h) const
	{
		return 	((x+w) >= x1 && 
			  x    < x2 &&
			 (y+h) >= (y1 + offset_y) &&
			  y    <  (y1 + offset_y + control_h));
	};

private:
	double control_h;

	cairo_surface_t *image_ratio_cntrl;
	cairo_surface_t *image_ratio_cntrl_prelight;
};


class DryWetControl : public SlidingControl
{
public:
	DryWetControl(float _min_value, float _max_value)
		: SlidingControl(_min_value, _max_value)
	{
		image_drywet_cntrl          = load_png("dry-wet_thumb.png");
		image_drywet_cntrl_prelight = load_png("dry-wet_thumb_prelight.png");

		control_h = 12.0;

		setControlOffsetY(0, -control_h);
	}

	~DryWetControl()
	{
		cairo_surface_destroy(image_drywet_cntrl);
		cairo_surface_destroy(image_drywet_cntrl_prelight);
	}

	virtual void drawWidget(bool hover, cairo_t *cr) const
	{
		cairo_surface_t *tmp = NULL;

		if (hover) {
			tmp = image_drywet_cntrl_prelight;
		} else {
			tmp = image_drywet_cntrl;
		}

		double _ty1, _ty2;
		_ty1 = y1 + offset_y;
		_ty2 = y1 + offset_y + control_h;

		cairo_set_source_surface(cr, tmp, x1, _ty1);

		cairo_rectangle(cr, x1, _ty1, x2 - x1, _ty2 - _ty1);
		cairo_fill(cr);
	}

	bool intersectsRectangle(double x, double y, double w, double h) const
	{
		return 	((x+w) >= x1 && 
			  x    < x2 &&
			 (y+h) >= (y1 + offset_y) &&
			  y    <  (y1 + offset_y + control_h));
	};

private:
	double control_h;

	cairo_surface_t *image_drywet_cntrl;
	cairo_surface_t *image_drywet_cntrl_prelight;
};



class HorizontalColorSlider : public SlidingControl
{
public:
	HorizontalColorSlider(float _min_value, float _max_value, std::string pngBase)
		: SlidingControl(_min_value, _max_value)
	{
		std::string png("slider_");

		std::string zero = png + "zero.png";
		std::string zero_prelight = png + "zero_prelight.png";

		//std::cerr << "opening: '" << zero << "'" << std::endl;
		//std::cerr << "opening: '" << zero_prelight << "'" << std::endl;
		
		slider_background          = load_png(zero);
		slider_background_prelight = load_png(zero_prelight);

		png += pngBase;

		std::string color = png + ".png";
		std::string color_prelight = png + "_prelight.png";

		//std::cerr << "opening: '" << color << "'" << std::endl;
		//std::cerr << "opening: '" << color_prelight << "'" << std::endl;

		slider_color               = load_png(color);
		slider_color_prelight      = load_png(color_prelight);

		setControlOffsetX(3.0, -0.0);
	}

	~HorizontalColorSlider()
	{
		cairo_surface_destroy(slider_background);
		cairo_surface_destroy(slider_background_prelight);
		cairo_surface_destroy(slider_color);
		cairo_surface_destroy(slider_color_prelight);
		
	}

	virtual void drawWidget(bool hover, cairo_t *cr) const
	{
		cairo_surface_t *tmp1 = NULL;
		cairo_surface_t *tmp2 = NULL;

		if (hover) {
			tmp1 = slider_background_prelight;
			tmp2 = slider_color_prelight;
		} else {
			tmp1 = slider_background;
			tmp2 = slider_color;
		}

		cairo_set_source_surface(cr, tmp1, x1, y1);
		cairo_paint(cr);

		cairo_set_source_surface(cr, tmp2, x1, y1);

		cairo_rectangle(cr, x1, y1, offset_x, y2-y1);
		cairo_fill(cr);
	}

private:
	cairo_surface_t *slider_background;
	cairo_surface_t *slider_background_prelight;
	cairo_surface_t *slider_color;
	cairo_surface_t *slider_color_prelight;
};

class TimingGraph : public Wdgt::Object
{
public:
	TimingGraph(HorizontalColorSlider *_attack, HorizontalColorSlider *_release, 
		    SlidingControl *_ratio)
		: attack (_attack)
		, release(_release)
		, ratio  (_ratio)
	{ 
		relative_attack_point  = 0.06;
		relative_release_point = 0.6; // TODO: this could be dropped to 0.4 ..

		_attack ->dependents.push_back(this);
		_release->dependents.push_back(this);
		_ratio  ->dependents.push_back(this);
	}


	~TimingGraph()
	{
	}

	
	virtual void drawWidget(bool hover, cairo_t *cr) const
	{
		cairo_rectangle(cr, x1, y1, x2-x1, y2-y1);
		cairo_clip(cr);

		// draw center line
		cairo_set_source_rgb(cr, 0.525, 0.843, 0.0);
		cairo_set_line_width(cr, 1.0);

		double w = x2-x1;
		double h = y2-y1;

		cairo_move_to(cr, x1, y1 + h/2.0);
		cairo_line_to(cr, x2, y1 + h/2.0);
		cairo_stroke(cr);

		// draw black "input" strength
		cairo_set_source_rgb(cr, 0.553, 0.533, 0.502);
		cairo_move_to(cr, x1, y1 + h*3.0/4.0);
		cairo_line_to(cr, x1 + w * relative_attack_point, y1 + h*3.0/4.0);
		cairo_line_to(cr, x1 + w * relative_attack_point, y1 + h*1.0/4.0);
		cairo_line_to(cr, x1 + w * relative_release_point, y1 + h*1.0/4.0);
		cairo_line_to(cr, x1 + w * relative_release_point, y1 + h*3.0/4.0);
		cairo_line_to(cr, x2, y1 + h*3.0/4.0);
		cairo_stroke(cr);

		// draw blue "compressed" strength
		cairo_set_source_rgb(cr, 0.243, 0.745, 0.945);
		cairo_move_to(cr, x1, y1 + h*3.0/4.0);	
		cairo_line_to(cr, x1 + w * relative_attack_point, y1 + h*3.0/4.0);
		cairo_line_to(cr, x1 + w * relative_attack_point, y1 + h*1.0/4.0);
		// attack curve
		// note that attack relative value is scaled to 0.1 .. 1.0 so that release will
		// never appear to be 0
		double attack_w = w * (relative_release_point - relative_attack_point) * (0.05 + attack->getRelativeValue() * 0.95);
		double attack_h = h/2.0 * (ratio->getValue() / 20.0);

		cairo_line_to(cr, x1 + w * relative_attack_point + attack_w, y1 + h*1.0/4.0+attack_h);
		cairo_line_to(cr, x1 + w * relative_release_point, y1 + h*1.0/4.0+attack_h);
		cairo_line_to(cr, x1 + w * relative_release_point, y1 + h*3.0/4.0+attack_h);

		// note that release relative value is scaled to 0.1 .. 1.0 so that release will
		// never appear to be 0
		double release_w = w * (1.0 - relative_release_point) * (0.1 + release->getRelativeValue() * 0.9);
		cairo_line_to(cr, x1 + w * relative_release_point + release_w, y1 + h*3.0/4.0);

		cairo_line_to(cr, x2, y1 + h*3.0/4.0);
		cairo_stroke(cr);

		// draw dashed black "input" strength to make blue/black overlapping parts dashed
		double dash_length = 2.0;
		cairo_set_dash(cr, &dash_length, 1, 0);
		cairo_set_source_rgb(cr, 0.553, 0.533, 0.502);
		cairo_move_to(cr, x1, y1 + h*3.0/4.0);
		cairo_line_to(cr, x1 + w * relative_attack_point, y1 + h*3.0/4.0);
		cairo_line_to(cr, x1 + w * relative_attack_point, y1 + h*1.0/4.0);
		cairo_line_to(cr, x1 + w * relative_release_point, y1 + h*1.0/4.0);
		cairo_line_to(cr, x1 + w * relative_release_point, y1 + h*3.0/4.0);
		cairo_line_to(cr, x2, y1 + h*3.0/4.0);
		cairo_stroke(cr);

		cairo_set_dash(cr, NULL, 0, 0);

		// attack gradient
		cairo_pattern_t *attack_gradient = cairo_pattern_create_linear(
			x1 + w * relative_attack_point,            y1,
			x1 + w * relative_attack_point + attack_w, y2);
		cairo_pattern_add_color_stop_rgba (attack_gradient, 0, 1.0, 0.875, 0.298, 0.8);
		cairo_pattern_add_color_stop_rgba (attack_gradient, h, 1.0, 0.875, 0.298, 0.1);

		cairo_set_source(cr, attack_gradient);
		cairo_rectangle(cr, 
                                x1 + w * relative_attack_point,            y1, 
                                attack_w, h);
		cairo_fill(cr);

		cairo_pattern_destroy(attack_gradient);

		// release gradient
		cairo_pattern_t *release_gradient = cairo_pattern_create_linear(
			x1 + w * relative_release_point,             y1,
			x1 + w * relative_release_point + release_w, y2);
		cairo_pattern_add_color_stop_rgba (release_gradient, 0, 0.882, 0.523, 1.0, 0.8);
		cairo_pattern_add_color_stop_rgba (release_gradient, h, 0.882, 0.523, 1.0, 0.1);

		cairo_set_source(cr, release_gradient);
		cairo_rectangle(cr, 
                                x1 + w * relative_release_point,            y1, 
                                release_w, h);
		cairo_fill(cr);

		cairo_pattern_destroy(release_gradient);
		
		cairo_reset_clip(cr);
	}

private:
	HorizontalColorSlider *attack;
	HorizontalColorSlider *release;
	SlidingControl        *ratio;

	float relative_attack_point;
	float relative_release_point;
};

class AttenuationMeter : public Wdgt::Object
{
public:
	AttenuationMeter(float _minimum, float _maximum)
		: min_value(_minimum)
		, max_value(_maximum)
		, value(0)
		, relative_value(0)
	{
		meter_image = load_png("make-up_full.png");
		meter_bg    = load_png("make-up.png");
	}

	~AttenuationMeter()
	{
		cairo_surface_destroy(meter_image);
		cairo_surface_destroy(meter_bg);
	}
	
	virtual void drawWidget(bool hover, cairo_t *cr) const
	{
		cairo_set_source_surface(cr, meter_bg, x1, y1);
		cairo_paint(cr);

		float width = relative_value * (x2-x1);
		float zero = x1 + (-min_value) / (max_value - min_value) * (x2-x1);

		if (fabsf(width) < 1.0) {
			//cairo_rectangle(cr, zero - 0.75, y1, 1.5, (y2-y1));
		} else if (value <=0.0) {
			cairo_set_source_surface(cr, meter_image, x1, y1);
			cairo_rectangle(cr, zero + width, y1, -width, (y2-y1));
			cairo_fill(cr);
		} else {
			cairo_set_source_surface(cr, meter_image, x1, y1);
			cairo_rectangle(cr, zero,         y1,  width, (y2-y1));
			cairo_fill(cr);
		}

		/* TODO: this bit draws a line at 0dB
		cairo_set_source_rgb(cr, 0.639, 0.494, 0.373);
		cairo_set_line_width(cr, 1.0);
		cairo_move_to(cr, zero + 1, y1);
		cairo_line_to(cr, zero + 1, y2);
		cairo_stroke(cr);
		*/
	}

	void setValue(float _value)
	{
		value = _value;

		// Relative_value is not normalized to 0..1
		// instead 0 means a value of 0dB and it ranges from x..y where y-x = 1

		relative_value = value / (max_value - min_value);
	}

	float getValue() const { return value; }

protected:
	float min_value;
	float max_value;
	
	float value;
	float relative_value;

	cairo_surface_t *meter_image;
	cairo_surface_t *meter_bg;
};

class VerticalMeter : public Wdgt::Object
{
public:
	VerticalMeter(float _minimum, float _maximum)
		: min_value(_minimum)
		, max_value(_maximum)
		, value(0)
		, relative_value(0)
	{
		meter_image = load_png("gain_v_full.png");
		meter_bg    = load_png("gain_v.png");
	}

	~VerticalMeter()
	{
		cairo_surface_destroy(meter_image);
		cairo_surface_destroy(meter_bg);
	}
	
	virtual void drawWidget(bool hover, cairo_t *cr) const
	{
		float meter_h = (y2-y1) * relative_value;
	
		// draw the meter background
		cairo_set_source_surface(cr, meter_bg, x1, y1);
		cairo_paint(cr);

		// draw the meter
		cairo_set_source_surface(cr, meter_image, x1, y1);
		cairo_rectangle(cr, x1, y2 - meter_h, (x2-x1), meter_h);
		cairo_fill(cr);
	}

	void setValue(float _value)
	{
		value = _value;

		relative_value = (value - min_value) / (max_value - min_value);
	}

	float getValue() const { return value; }

protected:
	float min_value;
	float max_value;
	
	float value;
	float relative_value;

	cairo_surface_t *meter_image;
	cairo_surface_t *meter_bg;
};


class HorizontalMeter : public Wdgt::Object
{
public:
	HorizontalMeter(float _minimum, float _maximum)
		: min_value(_minimum)
		, max_value(_maximum)
		, value(0)
		, relative_value(0)
	{
		meter_image = load_png("gain_h_full.png");
		meter_bg    = load_png("gain_h.png");
	}

	~HorizontalMeter()
	{
		cairo_surface_destroy(meter_image);
		cairo_surface_destroy(meter_bg);
	}
	
	virtual void drawWidget(bool hover, cairo_t *cr) const
	{
		float meter_w = (x2-x1) * relative_value;
	
		// draw the meter background
		cairo_set_source_surface(cr, meter_bg, x1, y1);
		cairo_paint(cr);

		// draw the meter
		cairo_set_source_surface(cr, meter_image, x1, y1);
		cairo_rectangle(cr, x1, y1, meter_w, (y2-y1));

		cairo_fill(cr);
	}

	void setValue(float _value)
	{
		value = _value;

		relative_value = (value - min_value) / (max_value - min_value);
	}

	float getValue() const { return value; }

protected:
	float min_value;
	float max_value;
	
	float value;
	float relative_value;

	cairo_surface_t *meter_image;
	cairo_surface_t *meter_bg;
};

class HoverLabel : public Wdgt::Object
{
public:
	HoverLabel()
	{
	}

	~HoverLabel()
	{
	}
	
	virtual void drawWidget(bool hover, cairo_t *cr) const
	{
		cairo_set_font_size(cr, 9);
		cairo_set_source_rgb(cr, 0.482, 0.361, 0.263);
		/*
		cairo_rectangle(cr, x1, y1, (x2-x1), (y2-y1));
		cairo_fill(cr);
		*/
       		cairo_font_extents_t extents;
		cairo_font_extents(cr, &extents);

		cairo_text_extents_t t_ext;
		cairo_text_extents(cr, label.c_str(), &t_ext);
		cairo_move_to(cr, x1 + (x2-x1-t_ext.width)/2, y1 + extents.height);
		cairo_show_text(cr, label.c_str());
	
		cairo_stroke(cr);
		
	}

	void setLabel(std::string _label)
	{
		label = _label;
	}

	std::string &getLabel() { return label; }

protected:
	std::string label;
};



}

#endif /* _SCHMOOZ_UI_WDGTS_H */

