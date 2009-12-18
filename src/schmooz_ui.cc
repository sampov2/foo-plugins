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

class SchmoozMonoUI
{
public:
	// This controls the Z-order, so overlapping and superimposed wdgts must be in
	// the correct order
	enum Wdgt
	{
		SIDECHAIN_HPF,
		THRESHOLD_CONTROL,
		THRESHOLD_GRAPH,
		
		NUM_WDGTS
	};

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

	bool _hpf_status;
	void hpf_toggled();
/*
	void threshold_changed();
	void ratio_changed();
	void attack_changed();
	void release_changed();
	void makeup_changed();
	void drywet_changed();

	void redraw_attenuation();
	bool expose_attenuation(GdkEventExpose *);
*/

	float _current_attenuation;

	float _current_threshold;
	float _predrag_threshold;

	void set_threshold(float newvalue);

	LV2UI_Write_Function _write_function;
	LV2UI_Controller _controller;

	// cairo primitives
	cairo_surface_t *_image_background;
	cairo_surface_t *_image_hpf_on;
	cairo_surface_t *_image_hpf_on_hover;
	cairo_surface_t *_image_hpf_off;
	cairo_surface_t *_image_hpf_off_hover;

	cairo_surface_t *_image_graph_bg;
	cairo_surface_t *_image_threshold;
	cairo_surface_t *_image_thr_cntrl;
	cairo_surface_t *_image_thr_cntrl_hover;


	// Gtk essentials
	void size_request(Gtk::Requisition *);
	bool expose(GdkEventExpose *);

	// Widget and eventhandlers
	Gtk::DrawingArea _drawingArea;
	bool motion_notify_event(GdkEventMotion *);
	bool button_press_event(GdkEventButton *);
	bool button_release_event(GdkEventButton *);

	int identifyWdgt(GdkEventMotion *);

	int _hoverWdgt;
	int _dragWdgt;
	int _buttonPressWdgt;

	int _dragStartX;
	int _dragStartY;

	gdouble **_wdgtPositions;

	
};

#define PNG_DIR "/home/v2/dev/foo/foo-plugins/graphics/"

SchmoozMonoUI::SchmoozMonoUI(const struct _LV2UI_Descriptor *descriptor, 
			     const char *bundle_path, 
			     LV2UI_Write_Function write_function, 
			     LV2UI_Controller controller, 
			     LV2UI_Widget *widget, 
			     const LV2_Feature *const *features)
	: _write_function(write_function)
	, _controller(controller)
	, _drawingArea()
	, _hoverWdgt(-1)
	, _dragWdgt(-1)
	, _buttonPressWdgt(-1)
{
	std::cerr << "SchmoozMonoUI::SchmoozMonoUI()" << std::endl;
	_image_background    = cairo_image_surface_create_from_png (PNG_DIR "background.png");

	_image_hpf_on        = cairo_image_surface_create_from_png (PNG_DIR "high-pass_on.png");
	_image_hpf_on_hover  = cairo_image_surface_create_from_png (PNG_DIR "high-pass_on_prelight.png");
	_image_hpf_off       = cairo_image_surface_create_from_png (PNG_DIR "high-pass_off.png");
	_image_hpf_off_hover = cairo_image_surface_create_from_png (PNG_DIR "high-pass_off_prelight.png");

	_image_graph_bg      = cairo_image_surface_create_from_png (PNG_DIR "graph_bg.png");
	_image_threshold     = cairo_image_surface_create_from_png (PNG_DIR "graph_bg_threshold.png");
	_image_thr_cntrl       = cairo_image_surface_create_from_png (PNG_DIR "threshold.png");
	_image_thr_cntrl_hover = cairo_image_surface_create_from_png (PNG_DIR "threshold_prelight.png");


	_drawingArea.signal_size_request().connect( sigc::mem_fun(*this, &SchmoozMonoUI::size_request));
        _drawingArea.signal_expose_event().connect( sigc::mem_fun (*this, &SchmoozMonoUI::expose));

 	_drawingArea.signal_motion_notify_event().connect ( sigc::mem_fun (*this, &SchmoozMonoUI::motion_notify_event) );
	_drawingArea.signal_button_press_event().connect  ( sigc::mem_fun (*this, &SchmoozMonoUI::button_press_event));
	_drawingArea.signal_button_release_event().connect( sigc::mem_fun (*this, &SchmoozMonoUI::button_release_event));

	Gdk::EventMask mask = _drawingArea.get_events();

	mask |= Gdk::POINTER_MOTION_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK;

	_drawingArea.set_events(mask);


	_wdgtPositions = new gdouble*[NUM_WDGTS];
	for (int i = 0; i<NUM_WDGTS; ++i) {
		_wdgtPositions[i] = new gdouble[4];

		switch (i) {
		case SIDECHAIN_HPF:
			_wdgtPositions[i][0] = WDGT_HPF_X;
			_wdgtPositions[i][1] = WDGT_HPF_Y;
			_wdgtPositions[i][2] = WDGT_HPF_W+WDGT_HPF_X;
			_wdgtPositions[i][3] = WDGT_HPF_H+WDGT_HPF_Y;
			break;

		case THRESHOLD_CONTROL:
			_wdgtPositions[i][0] = WDGT_GRAPH_X;
			_wdgtPositions[i][1] = WDGT_GRAPH_Y;
			_wdgtPositions[i][2] = WDGT_GRAPH_X+WDGT_THRESH_CONTROL_W;
			_wdgtPositions[i][3] = WDGT_GRAPH_H+WDGT_GRAPH_Y;
			break;

		case THRESHOLD_GRAPH:
			_wdgtPositions[i][0] = WDGT_GRAPH_X;
			_wdgtPositions[i][1] = WDGT_GRAPH_Y;
			_wdgtPositions[i][2] = WDGT_GRAPH_W+WDGT_GRAPH_X;
			_wdgtPositions[i][3] = WDGT_GRAPH_H+WDGT_GRAPH_Y;
			break;

		default:
			_wdgtPositions[i][0] = -1;
			_wdgtPositions[i][1] = -1;
			_wdgtPositions[i][2] = 0;
			_wdgtPositions[i][3] = 0;
		}
	}

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
	if (_dragWdgt != -1) {
		switch(_dragWdgt) {
		case THRESHOLD_CONTROL:
			float thr = 70.0 * (float)(evt->x - _dragStartX) / (float)WDGT_GRAPH_W + _predrag_threshold;

			if (thr >= -60 && thr <= 10) {
				set_threshold(thr);
				expose(NULL);
			}
			break;
		}

		return true;
	}

	int newHover = identifyWdgt(evt);
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

	if (_buttonPressWdgt == THRESHOLD_CONTROL) {
		_predrag_threshold = _current_threshold;
		_dragWdgt = _buttonPressWdgt;
		_dragStartX = evt->x;
		_dragStartY = evt->y;
	}
	return true;
}

bool 
SchmoozMonoUI::button_release_event(GdkEventButton *evt)
{
	std::cerr << "button release" << std::endl;

	if (_hoverWdgt == _buttonPressWdgt) {
		switch(_buttonPressWdgt) {
		case SIDECHAIN_HPF:
			_hpf_status = !_hpf_status;
			hpf_toggled();
			break;
		
		}
	
	}

	_buttonPressWdgt = -1;
	_dragWdgt = -1;

	expose(NULL);

	return true;
}

bool 
SchmoozMonoUI::expose(GdkEventExpose *)
{
	cairo_t *cr;

	cr = gdk_cairo_create(GDK_DRAWABLE(_drawingArea.get_window()->gobj()));

	// double-buffer
	cairo_push_group_with_content(cr, CAIRO_CONTENT_COLOR);

	cairo_set_source_surface(cr, _image_background, 0.0, 0.0);
	cairo_paint(cr);


	cairo_surface_t *tmp = NULL;
	switch( (_hoverWdgt == SIDECHAIN_HPF ? 1 : 0) | (_hpf_status ? 2: 0)) {
	case 0: tmp = _image_hpf_off;
		break;
	case 1: tmp = _image_hpf_off_hover;
		break;
	case 2: tmp = _image_hpf_on;
		break;
	case 3: tmp = _image_hpf_on_hover;
		break;
	}
	
	cairo_set_source_surface(cr, tmp, 
                                 _wdgtPositions[SIDECHAIN_HPF][0], 
                                 _wdgtPositions[SIDECHAIN_HPF][1]);
	cairo_paint(cr);

	// Threshold & compressor curve
	tmp = _image_threshold;

	cairo_set_source_surface(cr, tmp, 
                                 _wdgtPositions[THRESHOLD_GRAPH][0], 
                                 _wdgtPositions[THRESHOLD_GRAPH][1]);
	cairo_paint(cr);

	// Threshold control
	if (_hoverWdgt == THRESHOLD_CONTROL) {
		tmp = _image_thr_cntrl_hover;
	} else {
		tmp = _image_thr_cntrl;
	}

	cairo_set_source_surface(cr, tmp, 
				 _wdgtPositions[THRESHOLD_CONTROL][0],
				 _wdgtPositions[THRESHOLD_CONTROL][1]);
	cairo_paint(cr);

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
	float hpf_value = (_hpf_status ? 1.0 : 0.0);
	_write_function(_controller, PORT_SIDECHAIN_HPF, sizeof(float), 0, &hpf_value);
}

void
SchmoozMonoUI::set_threshold(float newvalue)
{
	_current_threshold = newvalue;
	float x = WDGT_GRAPH_X + WDGT_GRAPH_W * (_current_threshold+60.0)/70.0 - WDGT_THRESH_CONTROL_W/2.0;
	_wdgtPositions[THRESHOLD_CONTROL][0] = x;
	_wdgtPositions[THRESHOLD_CONTROL][2] = x+WDGT_THRESH_CONTROL_W;
}

/*
void
SchmoozMonoUI::threshold_changed()
{
	float threshold = (float)_threshold->get_value();
	_write_function(_controller, PORT_THRESHOLD, sizeof(float), 4, &threshold);
}

void
SchmoozMonoUI::ratio_changed()
{
	float ratio = (float)_ratio->get_value();
	_write_function(_controller, PORT_RATIO, sizeof(float), 4, &ratio);
}

void
SchmoozMonoUI::attack_changed()
{
	float attack = (float)_attack_ms->get_value();
	_write_function(_controller, PORT_ATTACK, sizeof(float), 4, &attack);
}

void
SchmoozMonoUI::release_changed()
{
	float release = (float)_release_ms->get_value();
	_write_function(_controller, PORT_RELEASE, sizeof(float), 4, &release);
}

void
SchmoozMonoUI::makeup_changed()
{
	float makeup = (float)_makeup->get_value();
	_write_function(_controller, PORT_MAKEUP, sizeof(float), 4, &makeup);
}

void
SchmoozMonoUI::drywet_changed()
{
	float drywet = (float)_drywet->get_value();
	_write_function(_controller, PORT_DRYWET, sizeof(float), 4, &drywet);
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

int
SchmoozMonoUI::identifyWdgt(GdkEventMotion *evt)
{
	for (int i = 0; i < NUM_WDGTS; ++i) {
		if ( evt->x >= _wdgtPositions[i][0] &&
		     evt->x <  _wdgtPositions[i][2] &&
		     evt->y >= _wdgtPositions[i][1] &&
		     evt->y <  _wdgtPositions[i][3]) {
			return i;
		}
	}

	return -1;
}

SchmoozMonoUI::~SchmoozMonoUI()
{
	std::cerr << "SchmoozMonoUI::~SchmoozMonoUI()" << std::endl;

	// TODO: lots'n'lots of unallocation
	// cairo, wdgt stuff
}

void
SchmoozMonoUI::port_event(uint32_t port_index, uint32_t buffer_size, 
                        uint32_t format, const void *buffer)
{

	switch(port_index) {
	case PORT_SIDECHAIN_HPF:

		//_hpf_toggle->set_active( (*(float *)buffer > 0.5 ? TRUE : FALSE));
		_hpf_status = ((*(float *)buffer > 0.5 ? TRUE : FALSE));
		break;

	case PORT_THRESHOLD:
		set_threshold(*(float *)buffer);
		break;

/*
	case PORT_RATIO:
		_ratio->set_value( (double) *(float *)buffer);
		break;

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

	// TODO: this is needed, but the UI is not ready in the beginning
	//expose(NULL);
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
