
#include <iostream>

#include <gtkmm/main.h>
#include <gtkmm/window.h>
#include <gtkmm/drawingarea.h>

#include <list>
#include <set>


#define max(x,y) (((x)>(y)) ? (x) : (y))
#define min(x,y) (((x)<(y)) ? (x) : (y))

#include "gen/foo-yc20-minimal.cpp"

#include "wdgt.h"
#include "yc20_wdgts.h"

// initialize statics
namespace Wdgt {
	cairo_surface_t *DrawbarWhite::images[] = {
		load_png("white_0.png"), 
		load_png("white_1.png"), 
		load_png("white_2.png"), 
		load_png("white_3.png") };

	cairo_surface_t *DrawbarBlack::images[] = {
		load_png("black_0.png"), 
		load_png("black_1.png"), 
		load_png("black_2.png"), 
		load_png("black_3.png") };

	cairo_surface_t *DrawbarGreen::images[] = {
		load_png("green_0.png"), 
		load_png("green_1.png"), 
		load_png("green_2.png"), 
		load_png("green_3.png") };
};



class YC20UI :  public UI
{
	public:
		YC20UI();

		~YC20UI();

		Gtk::Widget *getWidget() { return &_drawingArea; }

		// from Faust UI
		void addButton(const char* label, float* zone) {};
		void addToggleButton(const char* label, float* zone) {};
		void addCheckButton(const char* label, float* zone) {};
		void addVerticalSlider(const char* label, float* zone, float init, float min, float max, float step);
		void addHorizontalSlider(const char* label, float* zone, float init, float min, float max, float step);
		void addNumEntry(const char* label, float* zone, float init, float min, float max, float step) {};

		void openFrameBox(const char* label) {};
		void openTabBox(const char* label) {};
		void openHorizontalBox(const char* label) {};
		void openVerticalBox(const char* label) {};
		void closeBox() {};

		void declare(float* zone, const char* key, const char* value) {};

		void controlChanged(Wdgt::Object *);
	

	private:


		// Gtk essentials
		void size_request(Gtk::Requisition *);
		bool exposeWdgt(Wdgt::Object *);
		bool expose(GdkEventExpose *);

		Gtk::DrawingArea _drawingArea;
		bool motion_notify_event(GdkEventMotion *);
		bool button_press_event(GdkEventButton *);
		bool button_release_event(GdkEventButton *);

		bool draw_queue();

		Wdgt::Object *identifyWdgt(GdkEventMotion *);

		Wdgt::Object *_hoverWdgt;
		Wdgt::Lever *_dragWdgt;
		Wdgt::Object *_buttonPressWdgt;

		int _dragStartX;
		int _dragStartY;
		float _predrag_value;

		std::list<Wdgt::Object *> wdgts;

		std::map<std::string, float *> processorValuePerLabel;


		bool _ready_to_draw;

};


YC20UI::YC20UI()
{
	_drawingArea.signal_size_request().connect( sigc::mem_fun(*this, &YC20UI::size_request));
	_drawingArea.signal_expose_event().connect( sigc::mem_fun (*this, &YC20UI::expose));

	_drawingArea.signal_motion_notify_event().connect ( sigc::mem_fun (*this, &YC20UI::motion_notify_event) );
	_drawingArea.signal_button_press_event().connect  ( sigc::mem_fun (*this, &YC20UI::button_press_event));
	_drawingArea.signal_button_release_event().connect( sigc::mem_fun (*this, &YC20UI::button_release_event));



	Gdk::EventMask mask = _drawingArea.get_events();

	mask |= Gdk::POINTER_MOTION_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK;

	_drawingArea.set_events(mask);


	_dragWdgt = NULL;

	// Widgets
	float pitch_x = 6.0;
	float pitch_x_long = 10.0;
	float pitch_x_longest = 20.0;

	float x = 5;
	float y = 15.0;

	// Vibrato
	Wdgt::SwitchBlack *touch    = new Wdgt::SwitchBlack(x, y);
	touch->setName("touch vibrato");
	x += 40.0 + pitch_x;

	Wdgt::DrawbarBlack *vibrato = new Wdgt::DrawbarBlack(x, y, true);
	vibrato->setName("depth");
	x += 40.0 + pitch_x;

	Wdgt::DrawbarBlack *v_speed = new Wdgt::DrawbarBlack(x, y, true);
	v_speed->setName("speed");
	x += 40.0 + pitch_x_longest;

	wdgts.push_back(touch);
	wdgts.push_back(vibrato);
	wdgts.push_back(v_speed);

	// Bass
	Wdgt::DrawbarWhite *bass_16  = new Wdgt::DrawbarWhite(x, y);
	bass_16->setName("16' b");
	x += 40.0 + pitch_x;

	Wdgt::DrawbarWhite *bass_8   = new Wdgt::DrawbarWhite(x, y);
	bass_8->setName("8' b");
	x += 40.0 + pitch_x;

	Wdgt::SwitchBlack *bass_man = new Wdgt::SwitchBlack(x, y);
	bass_man->setName("bass manual");
	x += 40.0 + pitch_x_longest;

	wdgts.push_back(bass_16);
	wdgts.push_back(bass_8);
	wdgts.push_back(bass_man);

	// Section I
	Wdgt::DrawbarWhite *sect1_16    = new Wdgt::DrawbarWhite(x, y);
	sect1_16->setName("16' i");
	x += 40.0 + pitch_x;

	Wdgt::DrawbarWhite *sect1_8     = new Wdgt::DrawbarWhite(x, y);
	sect1_8->setName("8' i");
	x += 40.0 + pitch_x;

	Wdgt::DrawbarWhite *sect1_4     = new Wdgt::DrawbarWhite(x, y);
	sect1_4->setName("4' i");
	x += 40.0 + pitch_x;

	Wdgt::DrawbarWhite *sect1_2_2p3 = new Wdgt::DrawbarWhite(x, y);
	sect1_2_2p3->setName("2 2/3' i");
	x += 40.0 + pitch_x;

	Wdgt::DrawbarWhite *sect1_2     = new Wdgt::DrawbarWhite(x, y);
	sect1_2->setName("2' i");
	x += 40.0 + pitch_x;

	Wdgt::DrawbarWhite *sect1_1_3p5 = new Wdgt::DrawbarWhite(x, y);
	sect1_1_3p5->setName("1 3/5' i");
	x += 40.0 + pitch_x;

	Wdgt::DrawbarWhite *sect1_1     = new Wdgt::DrawbarWhite(x, y);
	sect1_1->setName("1' i");
	x += 40.0 + pitch_x_long;

	wdgts.push_back(sect1_16);
	wdgts.push_back(sect1_8);
	wdgts.push_back(sect1_4);
	wdgts.push_back(sect1_2_2p3);
	wdgts.push_back(sect1_2);
	wdgts.push_back(sect1_1_3p5);
	wdgts.push_back(sect1_1);

	// Balance & Brightness
	Wdgt::DrawbarBlack *balance    = new Wdgt::DrawbarBlack(x, y, false);
	balance->setName("balance");
	x += 40.0 + pitch_x_long;

	Wdgt::DrawbarBlack *brightness = new Wdgt::DrawbarBlack(x, y, false);
	brightness->setName("bright");
	x += 40.0 + pitch_x_long;

	wdgts.push_back(balance);
	wdgts.push_back(brightness);

	// Section II
	Wdgt::DrawbarWhite *sect2_16 = new Wdgt::DrawbarWhite(x, y);
	sect2_16->setName("16' ii");
	x += 40.0 + pitch_x;

	Wdgt::DrawbarWhite *sect2_8  = new Wdgt::DrawbarWhite(x, y);
	sect2_8->setName("8' ii");
	x += 40.0 + pitch_x;

	Wdgt::DrawbarWhite *sect2_4  = new Wdgt::DrawbarWhite(x, y);
	sect2_4->setName("4' ii");
	x += 40.0 + pitch_x;

	Wdgt::DrawbarWhite *sect2_2  = new Wdgt::DrawbarWhite(x, y);
	sect2_2->setName("2' ii");
	x += 40.0 + pitch_x_long;

	sect2_16->setValue(1.0);
	sect2_8 ->setValue(0.66);
	sect2_4 ->setValue(0.33);
	sect2_2 ->setValue(0.0);

	wdgts.push_back(sect2_16);
	wdgts.push_back(sect2_8);
	wdgts.push_back(sect2_4);
	wdgts.push_back(sect2_2);

	// Percussion
	Wdgt::DrawbarGreen *percussion = new Wdgt::DrawbarGreen(x, y);
	percussion->setName("percussion");

	wdgts.push_back(percussion);
}

void
YC20UI::addVerticalSlider(const char* label, float* zone, float init, float min, float max, float step)
{
	std::string name(label);

	processorValuePerLabel[name] = zone;

	for (std::list<Wdgt::Object *>::iterator i = wdgts.begin(); i != wdgts.end(); ) {
                Wdgt::Lever *obj = dynamic_cast<Wdgt::Lever *>(*i);
		if (obj != NULL && obj->getName() == name) {
			obj->setValue( init);
		
			break;
		}

                ++i;
        }
}

void
YC20UI::addHorizontalSlider(const char* label, float* zone, float init, float min, float max, float step)
{
	addVerticalSlider(label, zone, init, min, max, step);
}

void
YC20UI::size_request(Gtk::Requisition *req)
{
	req->width  = 1024;
	req->height = 15.0 + 85.0 + 15.0;
}

Wdgt::Object *
YC20UI::identifyWdgt(GdkEventMotion *evt)
{
	for (std::list<Wdgt::Object *>::iterator i = wdgts.begin(); i != wdgts.end(); ) {
		Wdgt::Object *obj = *i;

		if (obj->intersectsPoint(evt->x, evt->y))
			return obj;
	
		++i;
	}

	return NULL;
}

void
YC20UI::controlChanged(Wdgt::Object *control)
{
	Wdgt::Lever *lever = dynamic_cast<Wdgt::Lever *>(_dragWdgt);
	if (lever == NULL) {
		return;
	}

	std::map<std::string, float *>::iterator i = processorValuePerLabel.find(lever->getName());

	if (i == processorValuePerLabel.end()) {
		std::cerr << "ERROR: could not find processor for control " << lever->getName() << std::endl;
		return;
	}

	*(i->second) = lever->getValue();

	//std::cerr << lever->getName() << ", zone: " << i->second << std::endl;
}

bool 
YC20UI::motion_notify_event(GdkEventMotion *evt)
{
	//IDENTIFY_THREAD("motion_notify_event");

	if (_dragWdgt != NULL) {
		Wdgt::Lever *lever = dynamic_cast<Wdgt::Lever *>(_dragWdgt);
		if (lever == NULL) {
			return true;
		}

		if (!lever->setValueFromDrag(_predrag_value, _dragStartY, evt->y)) {
			return true;
		}
		controlChanged(lever);
	
		exposeWdgt(lever);
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
/*
		hover_label->setLabel(_hoverWdgt->getName());
	} else {
		hover_label->setLabel("");
*/
	}

//	exposeWdgt(hover_label);




	return true;
}

bool
YC20UI::button_press_event(GdkEventButton *evt)
{
	//IDENTIFY_THREAD("button_press_event");

	//std::cerr << "button press" << std::endl;

	_buttonPressWdgt = _hoverWdgt;
	Wdgt::Lever *lever = dynamic_cast<Wdgt::Lever *>(_buttonPressWdgt);

	if (lever == NULL) {
		return true;
	}


	_predrag_value = lever->getValue();

	_dragWdgt = lever;
	_dragStartX = evt->x;
	_dragStartY = evt->y;

	return true;
}

bool 
YC20UI::button_release_event(GdkEventButton *evt)
{
	//IDENTIFY_THREAD("button_release_event");

	Wdgt::Object *exposeObj = NULL;

	if (_dragWdgt != NULL) {
		exposeObj = _dragWdgt;
	}

	_buttonPressWdgt = NULL;
	_dragWdgt = NULL;
	_hoverWdgt = NULL;

	if (exposeObj != NULL) {
		exposeWdgt(exposeObj);
	}

	return true;
	
}
/*
bool
YC20UI::draw_queue()
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
*/

YC20UI::~YC20UI()
{
        for (std::list<Wdgt::Object *>::iterator i = wdgts.begin(); i != wdgts.end(); ) {
                Wdgt::Object *obj = *i;
                delete obj;

                ++i;
        }



}

bool 
YC20UI::exposeWdgt(Wdgt::Object *obj)
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
YC20UI::expose(GdkEventExpose *evt)
{
	bool clip = (evt != NULL);

	//std::cerr << "expose()" << std::endl;

	_ready_to_draw = true;

	cairo_t *cr;

	cr = gdk_cairo_create(GDK_DRAWABLE(_drawingArea.get_window()->gobj()));


	// double-buffer
	cairo_push_group_with_content(cr, CAIRO_CONTENT_COLOR);

	// background
	//cairo_set_source_surface(cr, _image_background, 0.0, 0.0);
	// e4080a
	cairo_set_source_rgb(cr, 228.0/255.0, 8.0/255.0, 10.0/255.0);
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



int main(int argc, char **argv)
{
        Gtk::Main mymain(argc, argv);

	mydsp       yc20;

	Gtk::Window *main_window;
	YC20UI      *yc20ui;


	main_window = new Gtk::Window();
	yc20ui = new YC20UI();

	yc20.init(48000);
	yc20.buildUserInterface(yc20ui);

	main_window->set_title("Foo YC20");


	main_window->add(*yc20ui->getWidget());

	//main_window->show_all();
	main_window->show();
	yc20ui->getWidget()->show();

        Gtk::Main::run(*main_window);

	delete main_window;

	delete yc20ui;


	return 0;
}
