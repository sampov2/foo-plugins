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

#define NUM_PORTS 10

#define SCHMOOZ_UI_URI "http://studionumbersix.com/foo/lv2/schmooz-mono/ui"


#define WDGT_HPF_X 23
#define WDGT_HPF_Y 206
#define WDGT_HPF_W 34
#define WDGT_HPF_H 73

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

	LV2UI_Write_Function _write_function;
	LV2UI_Controller _controller;

	// cairo primitives
	cairo_surface_t *_image_background;
	cairo_surface_t *_image_hpf_on;
	cairo_surface_t *_image_hpf_on_hover;
	cairo_surface_t *_image_hpf_off;
	cairo_surface_t *_image_hpf_off_hover;

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
	int _buttonPressWdgt;
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
	, _buttonPressWdgt(-1)
{
	std::cerr << "SchmoozMonoUI::SchmoozMonoUI()" << std::endl;
	_image_background    = cairo_image_surface_create_from_png (PNG_DIR "background.png");

	_image_hpf_on        = cairo_image_surface_create_from_png (PNG_DIR "high-pass_on.png");
	_image_hpf_on_hover  = cairo_image_surface_create_from_png (PNG_DIR "high-pass_on_prelight.png");
	_image_hpf_off       = cairo_image_surface_create_from_png (PNG_DIR "high-pass_off.png");
	_image_hpf_off_hover = cairo_image_surface_create_from_png (PNG_DIR "high-pass_off_prelight.png");


	_drawingArea.signal_size_request().connect( sigc::mem_fun(*this, &SchmoozMonoUI::size_request));
        _drawingArea.signal_expose_event().connect( sigc::mem_fun (*this, &SchmoozMonoUI::expose));

 	_drawingArea.signal_motion_notify_event().connect ( sigc::mem_fun (*this, &SchmoozMonoUI::motion_notify_event) );
	_drawingArea.signal_button_press_event().connect  ( sigc::mem_fun (*this, &SchmoozMonoUI::button_press_event));
	_drawingArea.signal_button_release_event().connect( sigc::mem_fun (*this, &SchmoozMonoUI::button_release_event));

	Gdk::EventMask mask = _drawingArea.get_events();

	mask |= Gdk::POINTER_MOTION_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK;

	_drawingArea.set_events(mask);


	_wdgtPositions = new gdouble*[NUM_PORTS];
	for (int i = 0; i<NUM_PORTS; ++i) {
		_wdgtPositions[i] = new gdouble[4];

		switch (i) {
		case PORT_SIDECHAIN_HPF:
			_wdgtPositions[i][0] = WDGT_HPF_X;
			_wdgtPositions[i][1] = WDGT_HPF_Y;
			_wdgtPositions[i][2] = WDGT_HPF_W+WDGT_HPF_X;
			_wdgtPositions[i][3] = WDGT_HPF_H+WDGT_HPF_Y;
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
	return true;
}

bool 
SchmoozMonoUI::button_release_event(GdkEventButton *evt)
{
	std::cerr << "button release" << std::endl;

	if (_hoverWdgt == _buttonPressWdgt) {
		switch(_buttonPressWdgt) {
		case PORT_SIDECHAIN_HPF:
			_hpf_status = !_hpf_status;
			hpf_toggled();
			break;
		
		}
	
	}

	_buttonPressWdgt = -1;
	expose(NULL);

	return true;
}

bool 
SchmoozMonoUI::expose(GdkEventExpose *)
{
	cairo_t *cr;

	cr = gdk_cairo_create(GDK_DRAWABLE(_drawingArea.get_window()->gobj()));

	cairo_copy_page(cr);

	cairo_set_source_surface(cr, _image_background, 0.0, 0.0);
	cairo_paint(cr);


	cairo_surface_t *hpf_surface = NULL;
	switch( (_hoverWdgt == PORT_SIDECHAIN_HPF ? 1 : 0) | (_hpf_status ? 2: 0)) {
	case 0: hpf_surface = _image_hpf_off;
		break;
	case 1: hpf_surface = _image_hpf_off_hover;
		break;
	case 2: hpf_surface = _image_hpf_on;
		break;
	case 3: hpf_surface = _image_hpf_on_hover;
		break;
	}
	
	cairo_set_source_surface(cr, hpf_surface, 
                                 _wdgtPositions[PORT_SIDECHAIN_HPF][0], 
                                 _wdgtPositions[PORT_SIDECHAIN_HPF][1]);
	cairo_paint(cr);


	// TODO: hover label
/*
	cairo_rectangle(cr, 
			22.0,  206.0, 
			34.0, 73.0);
	cairo_set_fill_rule(cr,CAIRO_FILL_RULE_EVEN_ODD);
*/
/*
	cairo_set_source_rgb(cr, 0.2, 9.0, 0.0);

	cairo_rectangle(cr, 
			0.0, 0.0, 
			_attenuation_width * ( ( _current_attenuation + 70.0) / 90.0 ), 
			_attenuation_height);
	cairo_fill(cr);
*/
        cairo_destroy(cr);

	return true;
}


void
SchmoozMonoUI::hpf_toggled()
{
	float hpf_value = (_hpf_status ? 1.0 : 0.0);
	_write_function(_controller, PORT_SIDECHAIN_HPF, sizeof(float), 0, &hpf_value);
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
	for (int i = 0; i < NUM_PORTS; ++i) {
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

/*
	case PORT_THRESHOLD:
		_threshold->set_value( (double) *(float *)buffer);
		break;

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
	assert(strcmp(SCHMOOZ_UI_URI,plugin_uri)==0);

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
