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

	void hpf_toggled();
	void threshold_changed();
	void ratio_changed();
	void attack_changed();
	void release_changed();
	void makeup_changed();
	void drywet_changed();

	void redraw_attenuation();
	bool expose_attenuation(GdkEventExpose *);

	LV2UI_Write_Function _write_function;
	LV2UI_Controller _controller;

	Gtk::Table _table;


	Gtk::ToggleButton 	*_hpf_toggle;
	Gtk::HScale		*_threshold;
	Gtk::HScale		*_ratio;

	Gtk::HScale 		*_attack_ms;
	Gtk::HScale 		*_release_ms;

	Gtk::HScale		*_makeup;
	Gtk::HScale		*_drywet;

	Gtk::DrawingArea	*_attenuation;
	float			_current_attenuation;
	float			_attenuation_width;
	float			_attenuation_height;
};

SchmoozMonoUI::SchmoozMonoUI(const struct _LV2UI_Descriptor *descriptor, 
			     const char *bundle_path, 
			     LV2UI_Write_Function write_function, 
			     LV2UI_Controller controller, 
			     LV2UI_Widget *widget, 
			     const LV2_Feature *const *features)
	: _write_function(write_function)
	, _controller(controller)
	, _table(3, 3)
{
	std::cerr << "SchmoozMonoUI::SchmoozMonoUI()" << std::endl;

	// HPF sidechain
	_hpf_toggle = Gtk::manage( new Gtk::ToggleButton("HPF"));
	_hpf_toggle->signal_toggled().connect( sigc::mem_fun(*this, &SchmoozMonoUI::hpf_toggled));

	// Threshold
	_threshold = Gtk::manage( new Gtk::HScale(-60.0, 10.0, 1.0) );
	_threshold->signal_value_changed().connect( sigc::mem_fun(*this, &SchmoozMonoUI::threshold_changed) );

	// Ratio
	_ratio = Gtk::manage( new Gtk::HScale(1.5, 20.0, 0.5) );
	_ratio->signal_value_changed().connect( sigc::mem_fun(*this, &SchmoozMonoUI::ratio_changed) );

	// Attack and release widgets
	_attack_ms = Gtk::manage( new Gtk::HScale(0.1, 120.0, 1.0) );
	_attack_ms->signal_value_changed().connect( sigc::mem_fun(*this, &SchmoozMonoUI::attack_changed) );

	_release_ms = Gtk::manage( new Gtk::HScale(50, 1200.0, 10.0) );
	_release_ms->signal_value_changed().connect( sigc::mem_fun(*this, &SchmoozMonoUI::release_changed) );

	// Makeup gain
	_makeup = Gtk::manage( new Gtk::HScale(0.0, 20.0, 0.5) );
	_makeup->signal_value_changed().connect( sigc::mem_fun(*this, &SchmoozMonoUI::makeup_changed) );

	// Dry/wet balance
	_drywet = Gtk::manage( new Gtk::HScale(0.0, 1.0, 0.05));
	_drywet->signal_value_changed().connect( sigc::mem_fun(*this, &SchmoozMonoUI::drywet_changed) );

	// Attenuation widget
	_attenuation = Gtk::manage( new Gtk::DrawingArea() );
	_current_attenuation = 0.0;
	_attenuation_width = 220.0;
	_attenuation_height = 16.0;
        _attenuation->set_size_request(_attenuation_width, _attenuation_height);

        _attenuation->signal_expose_event().connect( sigc::mem_fun (*this, &SchmoozMonoUI::expose_attenuation));
        //_attenuation->signal_size_allocate()

	int row = 0;

	_table.attach( *Gtk::manage( new Gtk::Label("Threshold (dB)")), 0, 1, row, row+1);
	_table.attach( *_hpf_toggle, 2, 3, row, row+1);
	_table.attach( *_threshold,  1, 2, row, row+1);
	++row;

	_table.attach( *Gtk::manage( new Gtk::Label("Ratio (x:1)")), 0, 1, row, row+1);
	_table.attach( *_ratio, 1, 2, row, row+1);
	++row;

	_table.attach( *Gtk::manage( new Gtk::Label("Attack (ms)")), 0, 1, row, row+1);
	_table.attach( *_attack_ms,  1, 2, row, row+1);
	++row;

	_table.attach( *Gtk::manage( new Gtk::Label("Release (ms)")), 0, 1, row, row+1);
	_table.attach( *_release_ms, 1, 2, row, row+1);
	++row;

	_table.attach( *Gtk::manage( new Gtk::Label("Makeup gain (dB)")), 0, 1, row, row+1);
	_table.attach( *_makeup, 1, 2, row, row+1);
	++row;

	_table.attach( *Gtk::manage( new Gtk::Label("Dry/wet")), 0, 1, row, row+1);
	_table.attach( *_drywet, 1, 2, row, row+1);
	++row;

	_table.attach( *Gtk::manage( new Gtk::Label("Attenuation")), 0, 1, row, row+1);
	_table.attach( *_attenuation,   1, 2, row, row+1);
	++row;

	 *(GtkWidget **)(widget) = GTK_WIDGET(_table.gobj());
}

void
SchmoozMonoUI::hpf_toggled()
{
	float hpf_value = (_hpf_toggle->property_active() ? 1.0 : 0.0);
	_write_function(_controller, PORT_SIDECHAIN_HPF, sizeof(float), 0, &hpf_value);
}

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

SchmoozMonoUI::~SchmoozMonoUI()
{
	std::cerr << "SchmoozMonoUI::~SchmoozMonoUI()" << std::endl;
}

void
SchmoozMonoUI::port_event(uint32_t port_index, uint32_t buffer_size, 
                        uint32_t format, const void *buffer)
{

	switch(port_index) {
	case PORT_THRESHOLD:
		_threshold->set_value( (double) *(float *)buffer);
		break;

	case PORT_RATIO:
		_ratio->set_value( (double) *(float *)buffer);
		break;

	case PORT_SIDECHAIN_HPF:
		_hpf_toggle->set_active( (*(float *)buffer > 0.5 ? TRUE : FALSE));
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
