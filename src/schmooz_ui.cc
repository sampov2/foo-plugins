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
	void attack_changed();
	void release_changed();
	void redraw_attenuation();
	bool expose_attenuation(GdkEventExpose *);

	LV2UI_Write_Function _write_function;
	LV2UI_Controller _controller;

	Gtk::Table _table;


	Gtk::ToggleButton 	*_hpf_toggle;
	Gtk::HScale		*_threshold;

	Gtk::HScale 		*_attack_ms;
	Gtk::HScale 		*_release_ms;

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

	// Attack and release widgets
	_attack_ms = Gtk::manage( new Gtk::HScale(0.1, 120.0, 1.0) );
	_attack_ms->signal_value_changed().connect( sigc::mem_fun(*this, &SchmoozMonoUI::attack_changed) );

	_release_ms = Gtk::manage( new Gtk::HScale(50, 1200.0, 10.0) );
	_release_ms->signal_value_changed().connect( sigc::mem_fun(*this, &SchmoozMonoUI::release_changed) );

	// Attenuation widget
	_attenuation = Gtk::manage( new Gtk::DrawingArea() );
	_current_attenuation = 0.0;
	_attenuation_width = 220.0;
	_attenuation_height = 16.0;
        _attenuation->set_size_request(_attenuation_width, _attenuation_height);

        _attenuation->signal_expose_event().connect( sigc::mem_fun (*this, &SchmoozMonoUI::expose_attenuation));
        //_attenuation->signal_size_allocate()


	_table.attach( *Gtk::manage( new Gtk::Label("Threshold")), 0, 1, 0, 1);
	_table.attach( *_hpf_toggle, 2, 3, 0, 1);
	_table.attach( *_threshold,  1, 2, 0, 1);

	_table.attach( *Gtk::manage( new Gtk::Label("Attack (ms)")), 0, 1, 1, 2);
	_table.attach( *_attack_ms,  1, 2, 1, 2);

	_table.attach( *Gtk::manage( new Gtk::Label("Release (ms)")), 0, 1, 2, 3);
	_table.attach( *_release_ms, 1, 2, 2, 3);

	_table.attach( *Gtk::manage( new Gtk::Label("Attenuation")), 0, 1, 3, 4);
	_table.attach( *_attenuation,   1, 2, 3, 4);


	 *(GtkWidget **)(widget) = GTK_WIDGET(_table.gobj());
}

void
SchmoozMonoUI::hpf_toggled()
{
	// TODO: port macros need to be put in a common header
	float hpf_value = (_hpf_toggle->property_active() ? 1.0 : 0.0);
	_write_function(_controller, 3, sizeof(float), 0, &hpf_value);
}

void
SchmoozMonoUI::threshold_changed()
{
	// TODO: port macros need to be put in a common header
	float threshold = (float)_threshold->get_value();
	_write_function(_controller, 2, sizeof(float), 4, &threshold);
}

void
SchmoozMonoUI::attack_changed()
{
	// TODO: port macros need to be put in a common header
	float attack = (float)_attack_ms->get_value();
	_write_function(_controller, 4, sizeof(float), 4, &attack);
}

void
SchmoozMonoUI::release_changed()
{
	// TODO: port macros need to be put in a common header
	float release = (float)_release_ms->get_value();
	_write_function(_controller, 5, sizeof(float), 4, &release);
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

	// TODO: port macros need to be put in a common header
	switch(port_index) {
	case 2: // threshold
		_threshold->set_value( (double) *(float *)buffer);
		break;

	case 3:	// HPF
		_hpf_toggle->set_active( (*(float *)buffer > 0.5 ? TRUE : FALSE));
		break;

	case 4: // attack
		_attack_ms->set_value( (double) *(float *)buffer);
		break;

	case 5: // release
		_release_ms->set_value( (double) *(float *)buffer);
		break;

	case 9: // attenuation
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
