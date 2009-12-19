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
#include "wdgt.h"

#define PNG_DIR "/home/v2/dev/foo/foo-plugins/graphics/"


class HPFButton : public Wdgt::Object
{
public:
	HPFButton()
	{
		image_hpf_on           = cairo_image_surface_create_from_png (PNG_DIR "high-pass_on.png");
		image_hpf_on_prelight  = cairo_image_surface_create_from_png (PNG_DIR "high-pass_on_prelight.png");
		image_hpf_off          = cairo_image_surface_create_from_png (PNG_DIR "high-pass_off.png");
		image_hpf_off_prelight = cairo_image_surface_create_from_png (PNG_DIR "high-pass_off_prelight.png");
	}

	~HPFButton()
	{
		cairo_surface_destroy(image_hpf_on);
		cairo_surface_destroy(image_hpf_on_prelight);
		cairo_surface_destroy(image_hpf_off);
		cairo_surface_destroy(image_hpf_off_prelight);
	}

	virtual void drawWidget(bool hover, cairo_t *cr) const
	{
		cairo_surface_t *tmp = NULL;
		switch( (hover ? 1 : 0) | (hpf_status ? 2: 0)) {
			case 0: tmp = image_hpf_off;
				break;
			case 1: tmp = image_hpf_off_prelight;
				break;
			case 2: tmp = image_hpf_on;
				break;
			case 3: tmp = image_hpf_on_prelight;
				break;
		}

		cairo_set_source_surface(cr, tmp, x1, y1);
		cairo_paint(cr);
	}

	bool get_status() const   { return hpf_status; }
	void set_status(bool tmp) { hpf_status = tmp; }
	bool toggle_status()      { hpf_status = !hpf_status; return hpf_status; }

private:
	bool hpf_status;

	cairo_surface_t *image_hpf_on;
	cairo_surface_t *image_hpf_on_prelight;
	cairo_surface_t *image_hpf_off;
	cairo_surface_t *image_hpf_off_prelight;
};

class ThresholdGraph : public Wdgt::Object
{
public:
	ThresholdGraph()
	{
		image_graph_bg      = cairo_image_surface_create_from_png (PNG_DIR "graph_bg.png");
		image_threshold     = cairo_image_surface_create_from_png (PNG_DIR "graph_bg_threshold.png");
	}
	
	~ThresholdGraph()
	{
		cairo_surface_destroy(image_graph_bg);
		cairo_surface_destroy(image_threshold);
	}

	virtual void drawWidget(bool hover, cairo_t *cr) const
	{
		cairo_set_source_surface(cr, image_threshold, x1, y1);
		cairo_paint(cr);
	}

private:
	cairo_surface_t *image_graph_bg;
	cairo_surface_t *image_threshold;
};

class ThresholdControl : public Wdgt::Object
{
public:
	ThresholdControl(double _clip_x1, double _clip_x2)
	{
		image_thr_cntrl          = cairo_image_surface_create_from_png (PNG_DIR "threshold.png");
		image_thr_cntrl_prelight = cairo_image_surface_create_from_png (PNG_DIR "threshold_prelight.png");
		control_w = 15.0; // TODO: read from the images

		clip_x1 = _clip_x1;
		clip_x2 = _clip_x2;
	}

	~ThresholdControl()
	{
		cairo_surface_destroy(image_thr_cntrl);
		cairo_surface_destroy(image_thr_cntrl_prelight);
	}

	// The rectangle x1,y1,x2,y2 is the space into which the control must be drawn onto
	// the actual offset is deduced from the internal threshold_value variable
	virtual void drawWidget(bool hover, cairo_t *cr) const
	{
		cairo_surface_t *tmp = NULL;

		if (hover) {
			tmp = image_thr_cntrl_prelight;
		} else {
			tmp = image_thr_cntrl;
		}

		double _tx1, _tx2, _ty1, _ty2;
		_tx1 = x1 + offset_x;
		_tx2 = x1 + offset_x + control_w;
		_ty1 = y1;
		_ty2 = y2; //y1 + WDGT_GRAPH_H;

		cairo_set_source_surface(cr, tmp, _tx1, _ty1);

		if (_tx1 < clip_x1) {
			_tx1 = clip_x1;
		} else if (_tx2 > clip_x2) {
			_tx2 = clip_x2;
		}

		cairo_rectangle(cr, _tx1, _ty1, _tx2 - _tx1, _ty2 - _ty1);
		cairo_fill(cr);
	}

	float get_threshold() const { return threshold_value; }

	void set_threshold_from_drag(float thresholdAtStart, int dragStart, int x)
	{
		float thr = 70.0 * (float)(x - dragStart) / (x2-x1) + thresholdAtStart;
	
		if (thr >= -60 && thr <= 10) {
			set_threshold(thr);
		}
	}	

	void set_threshold(float value)
	{
		threshold_value = value;
		
		offset_x = (x2-x1) * (threshold_value + 60.0)/70.0 - control_w/2.0;
	}

	// The offset value must be taken into account
	// For some reason this isn't called!!
	bool intersectsPoint(double x, double y) {
		return 	(x >= (x1+offset_x) && 
			 x <  (x1+offset_x + control_w) &&
                         y >= y1 && y < y2);
	};
private:
	float threshold_value;

	double offset_x;
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
		image_bg = cairo_image_surface_create_from_png (PNG_DIR "ratio_trough.png");
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


class RatioControl : public Wdgt::Object
{
public:
	RatioControl()
	{
		image_ratio_cntrl          = cairo_image_surface_create_from_png (PNG_DIR "ratio_thumb.png");
		image_ratio_cntrl_prelight = cairo_image_surface_create_from_png (PNG_DIR "ratio_thumb_prelight.png");
		control_h = 12.0;
	}

	~RatioControl()
	{
		cairo_surface_destroy(image_ratio_cntrl);
		cairo_surface_destroy(image_ratio_cntrl_prelight);
	}

	// The rectangle x1,y1,x2,y2 is the space into which the control must be drawn onto
	// the actual offset is deduced from the internal ratio_value variable
	virtual void drawWidget(bool hover, cairo_t *cr) const
	{
		cairo_surface_t *tmp = NULL;

		if (hover) {
			tmp = image_ratio_cntrl_prelight;
		} else {
			tmp = image_ratio_cntrl;
		}

		double _tx1, _tx2, _ty1, _ty2;
		_tx1 = x1;
		_tx2 = x2;
		_ty1 = y1 + offset_y;
		_ty2 = y1 + offset_y + control_h;

		cairo_set_source_surface(cr, tmp, _tx1, _ty1);

		cairo_rectangle(cr, _tx1, _ty1, _tx2 - _tx1, _ty2 - _ty1);
		cairo_fill(cr);
	}

	float get_ratio() const { return ratio_value; }

	void set_ratio_from_drag(float ratioAtStart, int dragStart, int y)
	{
		float thr = 18.5 * (float)(y - dragStart) / (y2-y1) + ratioAtStart;
	
		if (thr >= 1.5 && thr <= 20) {
			set_ratio(thr);
		}
	}	

	void set_ratio(float value)
	{
		ratio_value = value;
		
		offset_y = (y2-y1-control_h) * (ratio_value - 1.5)/18.5;
	}

	// The offset value must be taken into account
	// For some reason this isn't called!!
	bool intersectsPoint(double x, double y) {
		return 	(x >= x1 && x < x2 &&
			 y >= y1 &&
			 y < (y1 + offset_y + control_h));
	};
private:
	float ratio_value;

	double offset_y;
	double control_h;

	cairo_surface_t *image_ratio_cntrl;
	cairo_surface_t *image_ratio_cntrl_prelight;
};


