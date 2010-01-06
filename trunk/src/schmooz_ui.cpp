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
#include <set>


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
#define PORT_OUTPUT_INPUT_POWER 10
#define PORT_OUTPUT_COMP_POWER  11

#define PORT_BYPASS             12


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

//#define IDENTIFY_THREAD(func) { std::cerr << "Thread at " func "() is: " << pthread_self() << std::endl; }
#define IDENTIFY_THREAD(func) { }

bool
check_cairo_png(cairo_surface_t *s)
{
	cairo_status_t _stat = cairo_surface_status(s);
	return !(_stat == CAIRO_STATUS_NO_MEMORY ||
		 _stat == CAIRO_STATUS_FILE_NOT_FOUND ||
		 _stat == CAIRO_STATUS_READ_ERROR);
	
}

#include "schmooz_ui_wdgts.h"

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
	void bypass_toggled();
	void threshold_changed();
	void ratio_changed();
	void attack_changed();
	void release_changed();
	void makeup_changed();
	void drywet_changed();

/*
	void redraw_attenuation();
	bool expose_attenuation(GdkEventExpose *);
*/

	float _predrag_value;

	LV2UI_Write_Function _write_function;
	LV2UI_Controller _controller;

	// cairo primitives
	cairo_surface_t *_image_background;

	// Wdgts
	Wdgt::Button *hpf;
	Wdgt::Button *bypass;

	Wdgt::ThresholdGraph *threshold;
	Wdgt::ThresholdControl *threshold_control;

	Wdgt::RatioBackground *ratio_bg;
	Wdgt::RatioControl *ratio_control;

	Wdgt::HorizontalColorSlider *attack_control;
	Wdgt::HorizontalColorSlider *release_control;

	Wdgt::HorizontalColorSlider *makeup_control;

	Wdgt::TimingGraph *timing_graph;

	Wdgt::DryWetControl *drywet_control;

	// Meters, meters, meters...
	Wdgt::AttenuationMeter *attenuation_meter;
	Wdgt::VerticalMeter *input_meter;
	Wdgt::HorizontalMeter *comp_meter;

	Wdgt::HoverLabel *hover_label;


	// Gtk essentials
	void size_request(Gtk::Requisition *);
	bool exposeWdgt(Wdgt::Object *);
	bool expose(GdkEventExpose *);

	// Eventhandlers
	Gtk::DrawingArea _drawingArea;
	bool motion_notify_event(GdkEventMotion *);
	bool button_press_event(GdkEventButton *);
	bool button_release_event(GdkEventButton *);

	bool draw_queue();

	Wdgt::Object *identifyWdgt(GdkEventMotion *);

	Wdgt::Object *_hoverWdgt;
	Wdgt::SlidingControl *_dragWdgt;
	Wdgt::Object *_buttonPressWdgt;

	int _dragStartX;
	int _dragStartY;

	std::list<Wdgt::Object *> wdgts;

	std::set<Wdgt::Object *> redraw_queue;
	sigc::connection redraw_timer_connection;
};

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
	_image_background    = cairo_image_surface_create_from_png (SCHMOOZ_PNG_DIR "background.png");
	if (!check_cairo_png(_image_background)) {
		std::cerr << "SchmoozUI: could not open " << SCHMOOZ_PNG_DIR "background.png" << std::endl;
	}


	_drawingArea.signal_size_request().connect( sigc::mem_fun(*this, &SchmoozMonoUI::size_request));
        _drawingArea.signal_expose_event().connect( sigc::mem_fun (*this, &SchmoozMonoUI::expose));

 	_drawingArea.signal_motion_notify_event().connect ( sigc::mem_fun (*this, &SchmoozMonoUI::motion_notify_event) );
	_drawingArea.signal_button_press_event().connect  ( sigc::mem_fun (*this, &SchmoozMonoUI::button_press_event));
	_drawingArea.signal_button_release_event().connect( sigc::mem_fun (*this, &SchmoozMonoUI::button_release_event));


	Gdk::EventMask mask = _drawingArea.get_events();

	mask |= Gdk::POINTER_MOTION_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK;

	_drawingArea.set_events(mask);


	hpf = new Wdgt::Button("high-pass");
	hpf->setName("Sidechain high-pass filter");
	wdgts.push_back(hpf);

	bypass = new Wdgt::Button("bypass");
	bypass->setName("Effect bypass");
	wdgts.push_back(bypass);

	threshold_control = new Wdgt::ThresholdControl(-60.0, 10.0,
						 WDGT_THRESH_CONTROL_CLIP_X1,
						 WDGT_THRESH_CONTROL_CLIP_X2);
	threshold_control->setName("Compression threshold");
	ratio_control = new Wdgt::RatioControl(1.5,20.0);
	ratio_control->setName("Compression ratio");
	threshold = new Wdgt::ThresholdGraph(threshold_control, ratio_control);

	wdgts.push_back(threshold_control);
	wdgts.push_back(threshold);

	wdgts.push_back(ratio_control);

	ratio_bg = new Wdgt::RatioBackground();
	wdgts.push_back(ratio_bg);

	attack_control  = new Wdgt::HorizontalColorSlider( 0.1,  120.0, "attack");
	attack_control->setName("Attack time");
	wdgts.push_back(attack_control);

	release_control = new Wdgt::HorizontalColorSlider(50.0, 1200.0, "release");
	release_control->setName("Release time");
	wdgts.push_back(release_control);

	makeup_control = new Wdgt::HorizontalColorSlider(0.0, 40.0, "make-up");
	makeup_control->setName("Make-up gain");
	wdgts.push_back(makeup_control);

	timing_graph = new Wdgt::TimingGraph(attack_control, release_control, ratio_control);
	wdgts.push_back(timing_graph);

	drywet_control = new Wdgt::DryWetControl(0.0, 1.0);
	drywet_control->setName("Dry/wet balance");
	wdgts.push_back(drywet_control); // 330, 24, 24, 337

	// meters
	attenuation_meter = new Wdgt::AttenuationMeter(-70.0, 20.0);
	attenuation_meter->setName("Gain applied by the effect");
	wdgts.push_back(attenuation_meter);

	input_meter = new Wdgt::VerticalMeter(-60.0, 10.0);
	input_meter->setName("Input signal");
	wdgts.push_back(input_meter);

	comp_meter = new Wdgt::HorizontalMeter(-60.0, 10.0);
	comp_meter->setName("Compressed signal");
	wdgts.push_back(comp_meter);

	hover_label = new Wdgt::HoverLabel();
	wdgts.push_back(hover_label);

	hpf->setPosition( WDGT_HPF_X, WDGT_HPF_Y, WDGT_HPF_W, WDGT_HPF_H );

	bypass->setPosition(25, 414, 30, 24);

	threshold_control->setPosition(WDGT_GRAPH_X + 1, WDGT_GRAPH_Y - 1, 
				       WDGT_GRAPH_W - 3, WDGT_GRAPH_H );

	threshold->setPosition( WDGT_GRAPH_X, WDGT_GRAPH_Y, WDGT_GRAPH_W, WDGT_GRAPH_H );

	ratio_control  ->setPosition(300, 33,  12, 196);
	ratio_bg       ->setPosition(298, 31,  16, 200);

	attack_control ->setPosition(93, 265, 200,  16);
	release_control->setPosition(93, 287, 200,  16);

	makeup_control ->setPosition(93, 375, 200,  16);

	timing_graph   ->setPosition(94, 310, 198,  46);

	drywet_control ->setPosition(331, 24,  24, 337);

	attenuation_meter->setPosition(93,418, 253,   6);
	input_meter      ->setPosition(81, 31,   6, 200);
	comp_meter       ->setPosition(93,237, 200,   6);

	//hover_label      ->setPosition(0, 442, 355,  18);
	hover_label      ->setPosition(0, 442, 355,  36);
	// Set widget for host
	*(GtkWidget **)(widget) = GTK_WIDGET(_drawingArea.gobj());

	redraw_timer_connection = Glib::signal_timeout().connect( sigc::mem_fun(*this, &SchmoozMonoUI::draw_queue), 50 );
/*
	_redrawTimer = Glib::TimeoutSource::create(1);
	_redrawTimer->connect( sigc::mem_fun (*this, &SchmoozMonoUI::draw_queue));
*/
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
	IDENTIFY_THREAD("motion_notify_event");

	if (_dragWdgt != NULL) {
		if (_dragWdgt == threshold_control) {
			threshold_control->setValueFromHorizontalDrag(_predrag_value, _dragStartX, evt->x);
			threshold_changed();
		} else if (_dragWdgt == ratio_control) {
			ratio_control->setValueFromVerticalDrag(_predrag_value, _dragStartY, evt->y);
			ratio_changed();
		} else if (_dragWdgt == attack_control) {
			attack_control->setValueFromHorizontalDrag(_predrag_value, _dragStartX, evt->x);
			attack_changed();
		} else if (_dragWdgt == release_control) {
			release_control->setValueFromHorizontalDrag(_predrag_value, _dragStartX, evt->x);
			release_changed();
		} else if (_dragWdgt == makeup_control) {
			makeup_control->setValueFromHorizontalDrag(_predrag_value, _dragStartX, evt->x);
			makeup_changed();
		} else if (_dragWdgt == drywet_control) {
			drywet_control->setValueFromVerticalDrag(_predrag_value, _dragStartY, evt->y);
			drywet_changed();
		} else {
			// don't expose
			return true;
		}
		exposeWdgt(_dragWdgt);

		return true;
	}

	Wdgt::Object *newHover = identifyWdgt(evt);
	if (newHover == _hoverWdgt) {
		return true;
	}


	Wdgt::Object *oldHover = _hoverWdgt;

	_hoverWdgt = newHover;

	// Redraw ex-hover-widget
	if (oldHover != NULL) {
		exposeWdgt(oldHover);
	}

	// Redraw new hover-widget
	if (_hoverWdgt != NULL) {
		exposeWdgt(_hoverWdgt);
		hover_label->setLabel(_hoverWdgt->getName());
	} else {
		hover_label->setLabel("");
	}

	exposeWdgt(hover_label);


	return true;
}

bool
SchmoozMonoUI::button_press_event(GdkEventButton *evt)
{
	IDENTIFY_THREAD("button_press_event");

	//std::cerr << "button press" << std::endl;

	_buttonPressWdgt = _hoverWdgt;

	Wdgt::SlidingControl *slider = dynamic_cast<Wdgt::SlidingControl *>(_buttonPressWdgt);

	if (slider == NULL) {
		return true;
	}

	_predrag_value = slider->getValue();

	_dragWdgt = slider;
	_dragStartX = evt->x;
	_dragStartY = evt->y;
	return true;
}

bool 
SchmoozMonoUI::button_release_event(GdkEventButton *evt)
{
	IDENTIFY_THREAD("button_release_event");

	//std::cerr << "button release" << std::endl;

	Wdgt::Object *exposeObj = NULL;

	if (_hoverWdgt == _buttonPressWdgt) {
		if (_buttonPressWdgt == hpf) {
			hpf->toggleStatus();
			hpf_toggled();
			exposeObj = hpf;
		} else if (_buttonPressWdgt == bypass) {
			bypass->toggleStatus();
			bypass_toggled();
			exposeObj = bypass;
		}
	
	}

	_buttonPressWdgt = NULL;
	_dragWdgt = NULL;

	if (exposeObj != NULL) {
		exposeWdgt(exposeObj);
	}

	return true;
}

bool
SchmoozMonoUI::draw_queue()
{
	IDENTIFY_THREAD("draw_queue");

	if (!_ready_to_draw || redraw_queue.empty()) {
		return TRUE;
	}

	// TODO: this is in desperate need of synchronization
	for (std::set<Wdgt::Object *>::iterator i = redraw_queue.begin(); i != redraw_queue.end(); ++i)
	{
		exposeWdgt(*i);
	}
	redraw_queue.erase( redraw_queue.begin(), redraw_queue.end() );

	return TRUE;
}

bool 
SchmoozMonoUI::exposeWdgt(Wdgt::Object *obj)
{
	GdkEventExpose evt;
	evt.area.x = obj->x1;
	evt.area.y = obj->y1;
	evt.area.width = obj->x2 - evt.area.x;
	evt.area.height = obj->y2 - evt.area.y;

	expose(&evt);

	for (std::list<Wdgt::Object *>::iterator i = obj->dependents.begin(); i != obj->dependents.end(); ) {
		Wdgt::Object *dep = *i;

		exposeWdgt(dep);

		++i;
	}


	return true;
}

bool 
SchmoozMonoUI::expose(GdkEventExpose *evt)
{
	bool clip = (evt != NULL);

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
	
		if (evt == NULL || obj->intersectsEvent(evt)) {
			obj->drawWidget( (_hoverWdgt == obj), cr);
		}
	}

	// finish drawing (retrieve double-buffer & draw it)
	cairo_pattern_t *bg = cairo_pop_group(cr);

	cairo_copy_page(cr);

	if (clip) {
		cairo_rectangle(cr,
				evt->area.x, evt->area.y, 
				evt->area.width, evt->area.height);
		cairo_clip(cr);
	}


	cairo_set_source(cr,bg);
	cairo_paint(cr);

	if (clip) {
		cairo_reset_clip(cr);
	}

	cairo_pattern_destroy(bg);


        cairo_destroy(cr);

	return true;
}


void
SchmoozMonoUI::hpf_toggled()
{
	float hpf_value = (hpf->getStatus() ? 1.0 : 0.0);
	_write_function(_controller, PORT_SIDECHAIN_HPF, sizeof(float), 0, &hpf_value);
}

void
SchmoozMonoUI::bypass_toggled()
{
	float bypass_value = (bypass->getStatus() ? 1.0 : 0.0);
	_write_function(_controller, PORT_BYPASS, sizeof(float), 0, &bypass_value);
}

void
SchmoozMonoUI::threshold_changed()
{
	float threshold = (float)threshold_control->getValue();
	_write_function(_controller, PORT_THRESHOLD, sizeof(float), 0, &threshold);
}

void
SchmoozMonoUI::ratio_changed()
{
	float ratio = (float)ratio_control->getValue();
	_write_function(_controller, PORT_RATIO, sizeof(float), 0, &ratio);
}

void
SchmoozMonoUI::attack_changed()
{
	float attack = (float)attack_control->getValue();
	_write_function(_controller, PORT_ATTACK, sizeof(float), 0, &attack);
}

void
SchmoozMonoUI::release_changed()
{
	float release = (float)release_control->getValue();
	_write_function(_controller, PORT_RELEASE, sizeof(float), 0, &release);
}

void
SchmoozMonoUI::makeup_changed()
{
	float makeup = (float)makeup_control->getValue();
	_write_function(_controller, PORT_MAKEUP, sizeof(float), 0, &makeup);
}

void
SchmoozMonoUI::drywet_changed()
{
	float drywet = (float)drywet_control->getValue();
	_write_function(_controller, PORT_DRYWET, sizeof(float), 0, &drywet);
}

/*
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
	cairo_surface_destroy(_image_background);
	redraw_timer_connection.disconnect();
	// TODO: lots'n'lots of unallocation
	// cairo, wdgt stuff
}

void
SchmoozMonoUI::port_event(uint32_t port_index, uint32_t buffer_size, 
                        uint32_t format, const void *buffer)
{
	IDENTIFY_THREAD("port_event");

	Wdgt::Object *exposeObj = NULL;
	Wdgt::SlidingControl *slider = NULL;

	bool new_status;
	float new_value;

	switch(port_index) {
	case PORT_SIDECHAIN_HPF:
		new_status = (*(float *)buffer > 0.5 ? TRUE : FALSE);
		if (new_status != hpf->getStatus()) {
			exposeObj = hpf;
			hpf->setStatus((*(float *)buffer > 0.5 ? TRUE : FALSE));
		}
		break;

	case PORT_BYPASS:
		new_status = (*(float *)buffer > 0.5 ? TRUE : FALSE);
		if (new_status != bypass->getStatus()) {
			exposeObj = bypass;
			bypass->setStatus((*(float *)buffer > 0.5 ? TRUE : FALSE));
		}
		break;


	case PORT_THRESHOLD:
		slider = threshold_control;
		break;

	case PORT_RATIO:
		slider = ratio_control;
		break;

	case PORT_ATTACK:
		slider = attack_control;
		break;

	case PORT_RELEASE:
		slider = release_control;
		break;

	case PORT_MAKEUP:
		slider = makeup_control;
		break;

	case PORT_DRYWET:
		slider = drywet_control;
		break;

	case PORT_OUTPUT_ATTENUATION:
		attenuation_meter->setValue(*(float *)buffer);
		// perhaps a timer event would be better..
		exposeObj = attenuation_meter;
		break;

	case PORT_OUTPUT_INPUT_POWER:
		input_meter->setValue(*(float *)buffer);
		exposeObj = input_meter;
		break;

	case PORT_OUTPUT_COMP_POWER:
		comp_meter->setValue(*(float *)buffer);
		exposeObj = comp_meter;
		break;

	default:
		std::cerr << "unknown port event: SchmoozMonoUI::port_event(" << port_index << ", " << buffer_size << ", " << format << ", " << *(float *)buffer << ")" << std::endl;
		return;
	}

	// 
	if (slider != NULL) {

		new_value = *(float *)buffer;

		// note that we prevent event feedback from screwing with drag "smoothness"
		// by not calling set_threshold() when dragging on said control
		if (new_value != slider->getValue() &&
		    _dragWdgt != slider) {
			exposeObj = slider;
			slider->setValue(new_value);
		}

	}

	if (_ready_to_draw && exposeObj != NULL) {
		redraw_queue.insert(exposeObj);
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
