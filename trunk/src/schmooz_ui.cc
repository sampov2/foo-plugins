/*
    GUI for the schmooz mono compressor. Work in progress.
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
#include <gtkmm.h>
#include <ui.h>
#include <cassert>
#include <iostream>
#include <list>

#include "schmooz_ui_wdgts.h"

// These need to match the indexes in manifest.ttl
// TODO: port macros need to be put in a common header
#define PORT_AUDIO_INPUT        0
#define PORT_AUDIO_OUTPUT       1
#define PORT_THRESHOLD          2
#define PORT_SIDECHAIN_HPF      3
#define PORT_ATTACK             4
#define PORT_RELEASE            5
#define PORT_RATIO              6
#define PORT_MAKEUP             7
#define PORT_DRYWET             8

#define PORT_OUTPUT_ATTENUATION 9

#define SCHMOOZ_UI_URI "http://studionumbersix.com/foo/lv2/schmooz-mono/ui"

#define WDGT_HPF_X 23
#define WDGT_HPF_Y 206
#define WDGT_HPF_W 34
#define WDGT_HPF_H 73

#define WDGT_GRAPH_X 93
#define WDGT_GRAPH_Y 31
#define WDGT_GRAPH_W 200
#define WDGT_GRAPH_H 200

#define WDGT_THRESH_CONTROL_W 15
#define WDGT_THRESH_CONTROL_CLIP_X1 94
#define WDGT_THRESH_CONTROL_CLIP_X2 292

class SchmoozMonoUI
{
public:
	
	SchmoozMonoUI(const struct _LV2UI_Descriptor *descriptor, 
		      const char *bundle_path, 
		      LV2UI_Write_Function write_function, 
		      LV2UI_Controller controller, 
		      LV2UI_Widget *widget, 
		      const LV2_Feature *const *features);

	~SchmoozMonoUI();

	void port_event(uint32_t port_index, uint32_t buffer_size, 
                        uint32_t format, const void *buffer);

private:

	bool _ready_to_draw;

	void hpf_toggled();
	void threshold_changed();
	void ratio_changed();
/*
	void attack_changed();
	void release_changed();
	void makeup_changed();
	void drywet_changed();

	void redraw_attenuation();
	bool expose_attenuation(GdkEventExpose *);
*/

//	float _current_attenuation;

	float _predrag_value;

	LV2UI_Write_Function _write_function;
	LV2UI_Controller _controller;

	// cairo primitives
	cairo_surface_t *_image_background;

	// Wdgts
	HPFButton *hpf;

	ThresholdGraph *threshold;
	ThresholdControl *threshold_control;

	RatioBackground *ratio_bg;
	RatioControl *ratio_control;

	// Gtk essentials
	void size_request(Gtk::Requisition *);
	bool expose(GdkEventExpose *);

	// Eventhandlers
	Gtk::DrawingArea _drawingArea;
	bool motion_notify_event(GdkEventMotion *);
	bool button_press_event(GdkEventButton *);
	bool button_release_event(GdkEventButton *);

	Wdgt::Object *identifyWdgt(GdkEventMotion *);

	Wdgt::Object *_hoverWdgt;
	Wdgt::Object *_dragWdgt;
	Wdgt::Object *_buttonPressWdgt;

	int _dragStartX;
	int _dragStartY;

	std::list<Wdgt::Object *> wdgts;

	
};

#define PNG_DIR "/home/v2/dev/foo/foo-plugins/graphics/"

SchmoozMonoUI::SchmoozMonoUI(const struct _LV2UI_Descriptor *descriptor, 
			     const char *bundle_path, 
			     LV2UI_Write_Function write_function, 
			     LV2UI_Controller controller, 
			     LV2UI_Widget *widget, 
			     const LV2_Feature *const *features)
	: _ready_to_draw(false)
	, _write_function(write_function)
	, _controller(controller)
	, _drawingArea()
	, _hoverWdgt(NULL)
	, _dragWdgt(NULL)
	, _buttonPressWdgt(NULL)
{
	std::cerr << "SchmoozMonoUI::SchmoozMonoUI()" << std::endl;
	_image_background    = cairo_image_surface_create_from_png (PNG_DIR "background.png");


	_drawingArea.signal_size_request().connect( sigc::mem_fun(*this, &SchmoozMonoUI::size_request));
        _drawingArea.signal_expose_event().connect( sigc::mem_fun (*this, &SchmoozMonoUI::expose));

 	_drawingArea.signal_motion_notify_event().connect ( sigc::mem_fun (*this, &SchmoozMonoUI::motion_notify_event) );
	_drawingArea.signal_button_press_event().connect  ( sigc::mem_fun (*this, &SchmoozMonoUI::button_press_event));
	_drawingArea.signal_button_release_event().connect( sigc::mem_fun (*this, &SchmoozMonoUI::button_release_event));

	Gdk::EventMask mask = _drawingArea.get_events();

	mask |= Gdk::POINTER_MOTION_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK;

	_drawingArea.set_events(mask);


	hpf = new HPFButton();
	wdgts.push_back(hpf);


	threshold_control = new ThresholdControl(-60.0, 10.0,
						 WDGT_THRESH_CONTROL_CLIP_X1,
						 WDGT_THRESH_CONTROL_CLIP_X2);
	wdgts.push_back(threshold_control);

	threshold = new ThresholdGraph();
	wdgts.push_back(threshold);

	ratio_control = new RatioControl(1.5,20.0);
	wdgts.push_back(ratio_control);

	ratio_bg = new RatioBackground();
	wdgts.push_back(ratio_bg);

	hpf->setPosition( WDGT_HPF_X, WDGT_HPF_Y, WDGT_HPF_W, WDGT_HPF_H );

	threshold_control->setPosition(WDGT_GRAPH_X + 1, WDGT_GRAPH_Y - 1, 
				       WDGT_GRAPH_W - 3, WDGT_GRAPH_H );

	threshold->setPosition( WDGT_GRAPH_X, WDGT_GRAPH_Y, WDGT_GRAPH_W, WDGT_GRAPH_H );

	ratio_control->setPosition(300, 33, 12, 196 - 12);
	ratio_bg->setPosition(298, 31, 16, 200);

	// Set widget for host
	*(GtkWidget **)(widget) = GTK_WIDGET(_drawingArea.gobj());
}

void
SchmoozMonoUI::size_request(Gtk::Requisition *req)
{
	req->width  = 355;
	req->height = 465;
}

bool 
SchmoozMonoUI::motion_notify_event(GdkEventMotion *evt)
{
	if (_dragWdgt != NULL) {
		if (_dragWdgt == threshold_control) {
			threshold_control->set_value_from_horizontal_drag(_predrag_value, _dragStartX, evt->x);
			threshold_changed();
			expose(NULL);
		} else if (_dragWdgt == ratio_control) {
			ratio_control->set_value_from_vertical_drag(_predrag_value, _dragStartY, evt->y);
			ratio_changed();
			expose(NULL);
		}

		return true;
	}

	Wdgt::Object *newHover = identifyWdgt(evt);
	if (newHover == _hoverWdgt) {
		return true;
	}

	_hoverWdgt = newHover;
	expose(NULL);

	return true;
}

bool
SchmoozMonoUI::button_press_event(GdkEventButton *evt)
{
	std::cerr << "button press" << std::endl;
	_buttonPressWdgt = _hoverWdgt;

	if (_buttonPressWdgt == threshold_control) {
		_predrag_value = threshold_control->get_value();
	} else if (_buttonPressWdgt == ratio_control) {
		_predrag_value = ratio_control->get_value();
	} else {
		return true;
	}

	_dragWdgt = _buttonPressWdgt;
	_dragStartX = evt->x;
	_dragStartY = evt->y;
	return true;
}

bool 
SchmoozMonoUI::button_release_event(GdkEventButton *evt)
{
	std::cerr << "button release" << std::endl;

	if (_hoverWdgt == _buttonPressWdgt) {
		if (_buttonPressWdgt == hpf) {
			hpf->toggle_status();
			hpf_toggled();
		}
	
	}

	_buttonPressWdgt = NULL;
	_dragWdgt = NULL;

	expose(NULL);

	return true;
}

bool 
SchmoozMonoUI::expose(GdkEventExpose *)
{
	_ready_to_draw = true;

	cairo_t *cr;

	cr = gdk_cairo_create(GDK_DRAWABLE(_drawingArea.get_window()->gobj()));

	// double-buffer
	cairo_push_group_with_content(cr, CAIRO_CONTENT_COLOR);

	// background
	cairo_set_source_surface(cr, _image_background, 0.0, 0.0);
	cairo_paint(cr);

	// wdgts
	for (std::list<Wdgt::Object *>::iterator i = wdgts.end(); i != wdgts.begin(); ) {
		--i;

		Wdgt::Object *obj = *i;

		obj->drawWidget( (_hoverWdgt == obj), cr);
	}

	// finish drawing (retrieve double-buffer & draw it)
	cairo_pattern_t *bg = cairo_pop_group(cr);

	cairo_copy_page(cr);
	cairo_set_source(cr,bg);
	cairo_paint(cr);


	cairo_pattern_destroy(bg);


        cairo_destroy(cr);

	return true;
}


void
SchmoozMonoUI::hpf_toggled()
{
	float hpf_value = (hpf->get_status() ? 1.0 : 0.0);
	_write_function(_controller, PORT_SIDECHAIN_HPF, sizeof(float), 0, &hpf_value);
}

void
SchmoozMonoUI::threshold_changed()
{
	float threshold = (float)threshold_control->get_value();
	_write_function(_controller, PORT_THRESHOLD, sizeof(float), 0, &threshold);
}

void
SchmoozMonoUI::ratio_changed()
{
	float ratio = (float)ratio_control->get_value();
	_write_function(_controller, PORT_RATIO, sizeof(float), 0, &ratio);
}

/*
void
SchmoozMonoUI::attack_changed()
{
	float attack = (float)_attack_ms->get_value();
	_write_function(_controller, PORT_ATTACK, sizeof(float), 0, &attack);
}

void
SchmoozMonoUI::release_changed()
{
	float release = (float)_release_ms->get_value();
	_write_function(_controller, PORT_RELEASE, sizeof(float), 0, &release);
}

void
SchmoozMonoUI::makeup_changed()
{
	float makeup = (float)_makeup->get_value();
	_write_function(_controller, PORT_MAKEUP, sizeof(float), 0, &makeup);
}

void
SchmoozMonoUI::drywet_changed()
{
	float drywet = (float)_drywet->get_value();
	_write_function(_controller, PORT_DRYWET, sizeof(float), 0, &drywet);
}

bool
SchmoozMonoUI::expose_attenuation(GdkEventExpose *expose)
{
	redraw_attenuation();

	return true;
}

void
SchmoozMonoUI::redraw_attenuation()
{
	cairo_t *cr;

	cr = gdk_cairo_create(GDK_DRAWABLE(_attenuation->get_window()->gobj()));

	cairo_copy_page(cr);

	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
	cairo_paint(cr);

	cairo_set_source_rgb(cr, 0.2, 9.0, 0.0);

	cairo_rectangle(cr, 
			0.0, 0.0, 
			_attenuation_width * ( ( _current_attenuation + 70.0) / 90.0 ), 
			_attenuation_height);
	cairo_fill(cr);

        cairo_destroy(cr);

}
*/

Wdgt::Object *
SchmoozMonoUI::identifyWdgt(GdkEventMotion *evt)
{
	for (std::list<Wdgt::Object *>::iterator i = wdgts.begin(); i != wdgts.end(); ) {
		Wdgt::Object *obj = *i;

		if (obj->intersectsPoint(evt->x, evt->y))
			return obj;
	
		++i;
	}

	return NULL;
}

SchmoozMonoUI::~SchmoozMonoUI()
{
	std::cerr << "SchmoozMonoUI::~SchmoozMonoUI()" << std::endl;

	for (std::list<Wdgt::Object *>::iterator i = wdgts.begin(); i != wdgts.end(); ) {
		Wdgt::Object *obj = *i;
		delete obj;

		++i;
	}
	// TODO: lots'n'lots of unallocation
	// cairo, wdgt stuff
}

void
SchmoozMonoUI::port_event(uint32_t port_index, uint32_t buffer_size, 
                        uint32_t format, const void *buffer)
{
	bool redraw = false;


	bool new_status;
	float new_value;

	switch(port_index) {
	case PORT_SIDECHAIN_HPF:
		new_status = (*(float *)buffer > 0.5 ? TRUE : FALSE);
		if (new_status != hpf->get_status()) {
			redraw = true;
			hpf->set_status((*(float *)buffer > 0.5 ? TRUE : FALSE));
		}
		break;

	case PORT_THRESHOLD:
		// note that we prevent event feedback from screwing with drag "smoothness"
		// by not calling set_threshold() when dragging on said control
		new_value = *(float *)buffer;

		if (new_value != threshold_control->get_value() &&
		    _dragWdgt != threshold_control) {
			redraw = true;
			threshold_control->set_value(new_value);
		}

		break;

	case PORT_RATIO:
		new_value = *(float *)buffer;

		if (new_value != ratio_control->get_value() &&
		    _dragWdgt != ratio_control) {
			redraw = true;
			ratio_control->set_value(new_value);
		}

		break;

/*
	case PORT_ATTACK:
		_attack_ms->set_value( (double) *(float *)buffer);
		break;

	case PORT_RELEASE:
		_release_ms->set_value( (double) *(float *)buffer);
		break;

	case PORT_MAKEUP:
		_makeup->set_value( (double) *(float *)buffer);
		break;
	
	case PORT_DRYWET:
		_drywet->set_value( (double) *(float *)buffer);
		break;	

	case PORT_OUTPUT_ATTENUATION:
		_current_attenuation = *(float *)buffer;
		redraw_attenuation();
		break;
*/

	default:
		std::cerr << "unknown port event: SchmoozMonoUI::port_event(" << port_index << ", " << buffer_size << ", " << format << ", " << (int)buffer << ")" << std::endl;
		return;
	}

	if (_ready_to_draw && redraw) {
		expose(NULL);
	}
}


// ui.h -> class SchmoozMonoUI integration. Nothing to see here
void
schmooz_mono_ui_port_event(LV2UI_Handle ui, uint32_t port_index, uint32_t buffer_size, 
                           uint32_t format, const void *buffer)
{
	SchmoozMonoUI *ptr = (SchmoozMonoUI*)ui;
	ptr->port_event(port_index, buffer_size, format, buffer);
}

void
schmooz_mono_ui_cleanup(LV2UI_Handle ui)
{
	std::cerr << "schmooz_mono_ui_cleanup()" << std::endl;
	SchmoozMonoUI *ptr = (SchmoozMonoUI*)ui;
	delete ptr;
}


const void *
schmooz_mono_ui_extension_data(const char *uri)
{
	return NULL;
}

LV2UI_Handle 
schmooz_mono_ui_instantiate(const struct _LV2UI_Descriptor *descriptor, 
			    const char *plugin_uri,
			    const char *bundle_path, 
			    LV2UI_Write_Function write_function, 
			    LV2UI_Controller controller, 
			    LV2UI_Widget *widget, 
			    const LV2_Feature *const *features)
{
	//assert(strcmp(SCHMOOZ_UI_URI,plugin_uri)==0);

	return new SchmoozMonoUI(descriptor, bundle_path, write_function, controller, widget, features);
}

const LV2UI_Descriptor *
lv2ui_descriptor (uint32_t index)
{
	if (index > 0) return NULL;

	LV2UI_Descriptor *ret = new LV2UI_Descriptor();

	ret->URI 		= SCHMOOZ_UI_URI;
	ret->instantiate 	= schmooz_mono_ui_instantiate;
	ret->port_event 	= schmooz_mono_ui_port_event;
	ret->cleanup 		= schmooz_mono_ui_cleanup;
	ret->extension_data 	= schmooz_mono_ui_extension_data;

	return ret;
}
