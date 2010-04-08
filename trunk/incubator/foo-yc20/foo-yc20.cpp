/*
    Foo-YC20 UI
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

#include <iostream>
#include <cstring>
#include <cerrno>
#include <list>
#include <set>
#include <deque>

#include <semaphore.h>

#include <jack/jack.h>
#include <jack/midiport.h>

#include <gtkmm/main.h>
#include <gtkmm/window.h>
#include <gtkmm/drawingarea.h>


#define max(x,y) (((x)>(y)) ? (x) : (y))
#define min(x,y) (((x)<(y)) ? (x) : (y))

#include "gen/foo-yc20-minimal.cpp"

#include "wdgt.h"
#include "yc20_wdgts.h"

#ifdef __SSE__
    #include <xmmintrin.h>
    #ifdef __SSE2__
        #define AVOIDDENORMALS { _mm_setcsr(_mm_getcsr() | 0x8040); std::cerr << "Denormals off" << std::endl; }
    #else
        #define AVOIDDENORMALS { _mm_setcsr(_mm_getcsr() | 0x8000); std::cerr << "Denormals off" << std::endl; }
    #endif
#else
    #define AVOIDDENORMALS 
#endif


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
	cairo_surface_t *Potentiometer::image =
		load_png("potentiometer.png");
};

class MidiCC 
{
public:
        MidiCC(int a, int b) { cc = a; value = b;}

        int cc;
        int value;
};

class YC20UI;

mydsp         *yc20 = NULL;

jack_port_t   *audio_output_port = NULL;
jack_port_t   *midi_input_port = NULL;
jack_client_t *jack_client = NULL;

float         *yc20_keys[61];

// Idle timeout stuff
std::deque<MidiCC *> controlChangeQueue;
YC20UI *idleTimeoutUI;
sem_t controlChangeQueueSem;



class YC20UI :  public UI
{
	public:
		YC20UI();

		~YC20UI();

		Gtk::Widget *getWidget() { return &_drawingArea; }

		// from Faust UI
		void addButton(const char* label, float* zone);
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

		void controlChanged(Wdgt::Draggable *);
	
		void doControlChange(MidiCC *);

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
		Wdgt::Draggable *_dragWdgt;
		Wdgt::Object *_buttonPressWdgt;

		int _dragStartX;
		int _dragStartY;
		float _predrag_value;

		std::list<Wdgt::Object *> wdgts;

		std::map<std::string, float *> processorValuePerLabel;
		Wdgt::Draggable *draggablePerCC[127];

		cairo_surface_t *_image_background;

		bool _ready_to_draw;

};


YC20UI::YC20UI()
{
	memset(draggablePerCC, 0, sizeof(Wdgt::Draggable *)*127);
	_image_background = Wdgt::load_png("background.png");

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

	// Pitch, volume & bass volume
	Wdgt::Potentiometer *pitch  = new Wdgt::Potentiometer(x, y, -1.0, 1.0);
	pitch->setName("pitch");
	pitch->setValue(0.0);
	x += 72.0 + pitch_x_longest;

	Wdgt::Potentiometer *volume = new Wdgt::Potentiometer(x, y, 0.0, 1.0);
	volume->setName("volume");
	x += 72.0 + pitch_x_longest;

	Wdgt::Potentiometer *bass_v = new Wdgt::Potentiometer(x, y, 0.0, 1.0);
	bass_v->setName("bass volume");
	x += 72.0 + pitch_x_longest + pitch_x_long;

	wdgts.push_back(pitch);
	wdgts.push_back(volume);
	wdgts.push_back(bass_v);

	// Vibrato
	Wdgt::SwitchBlack *touch    = new Wdgt::SwitchBlack(x, y);
	touch->setName("touch vibrato");
	x += 40.0 + pitch_x;

	Wdgt::DrawbarBlack *vibrato = new Wdgt::DrawbarBlack(x, y, true);
	vibrato->setName("depth");
	draggablePerCC[12] = vibrato;
	x += 40.0 + pitch_x;

	Wdgt::DrawbarBlack *v_speed = new Wdgt::DrawbarBlack(x, y, true);
	v_speed->setName("speed");
	draggablePerCC[13] = v_speed;
	x += 40.0 + pitch_x_longest;

	wdgts.push_back(touch);
	wdgts.push_back(vibrato);
	wdgts.push_back(v_speed);

	// Bass
	Wdgt::DrawbarWhite *bass_16  = new Wdgt::DrawbarWhite(x, y);
	bass_16->setName("16' b");
	draggablePerCC[14] = bass_16;
	x += 40.0 + pitch_x;

	Wdgt::DrawbarWhite *bass_8   = new Wdgt::DrawbarWhite(x, y);
	bass_8->setName("8' b");
	draggablePerCC[15] = bass_8;
	x += 40.0 + pitch_x;

	Wdgt::SwitchBlack *bass_man = new Wdgt::SwitchBlack(x, y);
	bass_man->setName("bass manual");
	draggablePerCC[23] = bass_man;
	x += 40.0 + pitch_x_longest;

	wdgts.push_back(bass_16);
	wdgts.push_back(bass_8);
	wdgts.push_back(bass_man);

	// Section I
	Wdgt::DrawbarWhite *sect1_16    = new Wdgt::DrawbarWhite(x, y);
	sect1_16->setName("16' i");
	draggablePerCC[2] = sect1_16;
	x += 40.0 + pitch_x;

	Wdgt::DrawbarWhite *sect1_8     = new Wdgt::DrawbarWhite(x, y);
	sect1_8->setName("8' i");
	draggablePerCC[3] = sect1_8;
	x += 40.0 + pitch_x;

	Wdgt::DrawbarWhite *sect1_4     = new Wdgt::DrawbarWhite(x, y);
	sect1_4->setName("4' i");
	draggablePerCC[4] = sect1_4;
	x += 40.0 + pitch_x;

	Wdgt::DrawbarWhite *sect1_2_2p3 = new Wdgt::DrawbarWhite(x, y);
	sect1_2_2p3->setName("2 2/3' i");
	draggablePerCC[5] = sect1_2_2p3;
	x += 40.0 + pitch_x;

	Wdgt::DrawbarWhite *sect1_2     = new Wdgt::DrawbarWhite(x, y);
	sect1_2->setName("2' i");
	draggablePerCC[6] = sect1_2;
	x += 40.0 + pitch_x;

	Wdgt::DrawbarWhite *sect1_1_3p5 = new Wdgt::DrawbarWhite(x, y);
	sect1_1_3p5->setName("1 3/5' i");
	draggablePerCC[8] = sect1_1_3p5;
	x += 40.0 + pitch_x;

	Wdgt::DrawbarWhite *sect1_1     = new Wdgt::DrawbarWhite(x, y);
	sect1_1->setName("1' i");
	draggablePerCC[9] = sect1_1;
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
	draggablePerCC[16] = balance;
	x += 40.0 + pitch_x_long;

	Wdgt::DrawbarBlack *brightness = new Wdgt::DrawbarBlack(x, y, false);
	brightness->setName("bright");
	draggablePerCC[17] = brightness;
	x += 40.0 + pitch_x_long;

	wdgts.push_back(balance);
	wdgts.push_back(brightness);

	// Section II
	Wdgt::DrawbarWhite *sect2_16 = new Wdgt::DrawbarWhite(x, y);
	sect2_16->setName("16' ii");
	draggablePerCC[18] = sect2_16;
	x += 40.0 + pitch_x;

	Wdgt::DrawbarWhite *sect2_8  = new Wdgt::DrawbarWhite(x, y);
	sect2_8->setName("8' ii");
	draggablePerCC[19] = sect2_8;
	x += 40.0 + pitch_x;

	Wdgt::DrawbarWhite *sect2_4  = new Wdgt::DrawbarWhite(x, y);
	sect2_4->setName("4' ii");
	draggablePerCC[20] = sect2_4;
	x += 40.0 + pitch_x;

	Wdgt::DrawbarWhite *sect2_2  = new Wdgt::DrawbarWhite(x, y);
	sect2_2->setName("2' ii");
	draggablePerCC[21] = sect2_2;
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
	draggablePerCC[22] = percussion;

	wdgts.push_back(percussion);


/*
	for (std::list<Wdgt::Object *>::iterator i = wdgts.begin(); i !=  wdgts.end(); ) {
		Wdgt::Draggable *d = dynamic_cast<Wdgt::Draggable *>(*i);
		if (d != NULL) {
			draggablePerLabel[d->getName()] = d;
		}

		++i;
	}
*/
}

void
YC20UI::addButton(const char* label, float* zone)
{
	bool isNote = true;
	int note;
	int octave = atoi(label+1);

	if (strlen(label) != 2) {
		isNote = false;
	}

	if (octave == 0 && errno == EINVAL) {
		isNote = false;
	}

	switch(label[0]) {
		case 'c': note =  0; break;
		case 'C': note =  1; break;
		case 'd': note =  2; break;
		case 'D': note =  3; break;
		case 'e': note =  4; break;
		case 'f': note =  5; break;
		case 'F': note =  6; break;
		case 'g': note =  7; break;
		case 'G': note =  8; break;
		case 'a': note =  9; break;
		case 'A': note = 10; break;
		case 'b': note = 11; break;
		default: isNote = false;
	}

	if (isNote) {
		//std::cerr << "Connected key " << label << ", octave " << octave << ", note " << note << std::endl;
		//fprintf(stderr, "Connected key %s, octave %d, note %d\n", label, octave, note);
		yc20_keys[octave*12 + note] = zone;
		return;
	}

}

void
YC20UI::addVerticalSlider(const char* label, float* zone, float init, float min, float max, float step)
{
	std::string name(label);

	processorValuePerLabel[name] = zone;

	for (std::list<Wdgt::Object *>::iterator i = wdgts.begin(); i != wdgts.end(); ) {
                Wdgt::Lever *lever = dynamic_cast<Wdgt::Lever *>(*i);
		if (lever != NULL && lever->getName() == name) {
			std::cerr << "Setting Lever " << lever->getName() << " to " << init << std::endl;
			lever->setValue(init);
		
			return;
		}

		Wdgt::Potentiometer *pot = dynamic_cast<Wdgt::Potentiometer *>(*i);
		if (pot != NULL && pot->getName() == name) {
			std::cerr << "Setting Pot " << pot->getName() << " to " << init << std::endl;
			pot->setValue(init);

			return;
		}

                ++i;
        }

	std::cerr << "No control for '" << label << "'" << std::endl;
}

void
YC20UI::addHorizontalSlider(const char* label, float* zone, float init, float min, float max, float step)
{
	addVerticalSlider(label, zone, init, min, max, step);
}

void
YC20UI::size_request(Gtk::Requisition *req)
{
	req->width  = 1280;
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
YC20UI::controlChanged(Wdgt::Draggable *control)
{
	std::map<std::string, float *>::iterator i = processorValuePerLabel.find(control->getName());

	if (i == processorValuePerLabel.end()) {
		std::cerr << "ERROR: could not find processor for control " << control->getName() << std::endl;
		return;
	}

	*(i->second) = control->getValue();
}

bool 
YC20UI::motion_notify_event(GdkEventMotion *evt)
{
	//IDENTIFY_THREAD("motion_notify_event");

	if (_dragWdgt != NULL) {

		if (!_dragWdgt->setValueFromDrag(_predrag_value, _dragStartY, evt->y)) {
			return true;
		}
		controlChanged(_dragWdgt);
	
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
	}

	return true;
}

bool
YC20UI::button_press_event(GdkEventButton *evt)
{
	//IDENTIFY_THREAD("button_press_event");

	//std::cerr << "button press" << std::endl;

	_buttonPressWdgt = _hoverWdgt;
	Wdgt::Draggable *obj = dynamic_cast<Wdgt::Draggable *>(_buttonPressWdgt);

	if (obj == NULL) {
		return true;
	}


	_predrag_value = obj->getValue();

	_dragWdgt = obj;
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

gboolean 
idleTimeout(gpointer data)
{
        // lock
        sem_wait(&controlChangeQueueSem);
        while (!controlChangeQueue.empty()) {
                // pop list
                MidiCC *evt = controlChangeQueue.front();
                controlChangeQueue.pop_front();
                // unlock
                sem_post(&controlChangeQueueSem);

                // do the slow things
		idleTimeoutUI->doControlChange(evt);
                delete evt;

                // lock again
                sem_wait(&controlChangeQueueSem);
        }
        // unlock
        sem_post(&controlChangeQueueSem);
        return true;
}

YC20UI::~YC20UI()
{
        for (std::list<Wdgt::Object *>::iterator i = wdgts.begin(); i != wdgts.end(); ) {
                Wdgt::Object *obj = *i;
                delete obj;

                ++i;
        }

	cairo_surface_destroy(_image_background);
}

void
YC20UI::doControlChange(MidiCC *evt)
{
	Wdgt::Draggable *control = draggablePerCC[evt->cc];
	
	if (control == NULL) {
		std::cerr << "No control for CC " << evt->cc << std::endl;
		return;
	}


	float newValue = ((float)evt->value)/127.0;
	
	// TODO: the special cases: pitch, bass manual switch, flipped sliders, lever positions

	if (evt->cc >=2 && evt->cc <= 13) {
		newValue = 1.0 - newValue;
	}
	
	control->setValue(newValue);
	controlChanged(control);
	exposeWdgt(control);


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
	cairo_set_source_surface(cr, _image_background, 0.0, 0.0);
	// e4080a
	//cairo_set_source_rgb(cr, 228.0/255.0, 8.0/255.0, 10.0/255.0);
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

int
process (jack_nframes_t nframes, void *arg)
{
	float * output_buffer = (float *)jack_port_get_buffer(audio_output_port, nframes);

        void *midi = jack_port_get_buffer(midi_input_port, nframes);
        jack_midi_event_t event;
        jack_nframes_t n = jack_midi_get_event_count(midi);

        if (n > 0) {

            // TODO: frame accuracy
            // TODO: panic button
            for (uint32_t i = 0; i < n; ++i) {
                jack_midi_event_get(&event, midi, i);

                int note = -1;
                float value = 0.0;

                if( ((*(event.buffer) & 0xf0)) == 0x90 ) {
                        /* note on */
                        note = *(event.buffer + 1) - 36;
                        value = 1.0;
                } else if( ((*(event.buffer)) & 0xf0) == 0x80 ) {
                        /* note off */
                        note = *(event.buffer + 1) - 36;
                        value = 0.0;
                } else if ( ((*(event.buffer)) & 0xf0) ==  0xb0) {
                        int cc    = *(event.buffer+1);
                        int value = *(event.buffer+2);

                        // while one shouldn't lock inside an RT thread, the 
                        // other party aquiring this lock is always O(1) in the
                        // locked state

                        sem_wait(&controlChangeQueueSem);
			// TODO: this 'new' might cause issues
                        controlChangeQueue.push_back( new MidiCC(cc, value));
                        sem_post(&controlChangeQueueSem);
		}

                if (note >= 0 && note < 61) {
                        *yc20_keys[note] = value;
                }

            }

        }

        yc20->compute(nframes, NULL, &output_buffer);


	return 0;
}



void disconnect_from_jack(void *arg)
{
	std::cerr << "Disconnected from jack.. bummer" << std::endl;
	jack_client = NULL;
	midi_input_port = NULL;
	audio_output_port = NULL;

	// TODO: we don't have to quit.. but it's a good idea while
	// there is no way to reconnect
	exit(1);
}


bool connect_to_jack()
{
        jack_options_t options = JackNullOption;
        jack_status_t status;

        jack_client = jack_client_open ("Foo YC20", options, &status, NULL);
	if (jack_client == NULL) {
		std::cerr << "jack_client_open() failed" << std::endl;

		midi_input_port = NULL;
		audio_output_port = NULL;
		return false;
	}


        midi_input_port = jack_port_register (jack_client, "midi in",
                                              JACK_DEFAULT_MIDI_TYPE,
                                              JackPortIsInput, 0);
        audio_output_port = jack_port_register (jack_client, "output",
                                                JACK_DEFAULT_AUDIO_TYPE,
                                                JackPortIsOutput, 0);

        jack_set_process_callback (jack_client, process, 0);

        /* tell the JACK server to call `jack_shutdown()' if
           it ever shuts down, either entirely, or if it
           just decides to stop calling us.
        */

        jack_on_shutdown (jack_client, disconnect_from_jack, 0);


/*
        if (jack_activate (jack_client)) {
                std::cerr << "cannot activate client" << std::endl;
		jack_client_close(jack_client);
		jack_client = NULL;
		midi_input_port = NULL;
		audio_output_port = NULL;
		return false;
        }
*/
	return true;
}



int main(int argc, char **argv)
{

	AVOIDDENORMALS;

        Gtk::Main mymain(argc, argv);


	Gtk::Window *main_window;
	YC20UI      *yc20ui;


	main_window = new Gtk::Window();
	main_window->set_title("Foo YC20");

	yc20ui = new YC20UI();
	idleTimeoutUI = yc20ui;



	main_window->add(*yc20ui->getWidget());

	main_window->show();
	yc20ui->getWidget()->show();

	// Create a semaphore for controlChangeQueue locking
	if (sem_init(&controlChangeQueueSem, 0, 1)) {
		perror("sem_init");
		return 1;
	}

	if (!connect_to_jack()) {
		return 1;
	}

	yc20 = new mydsp();
	yc20->init(jack_get_sample_rate (jack_client));
	yc20->buildUserInterface(yc20ui);

        if (jack_activate (jack_client)) {
                std::cerr << "cannot activate client" << std::endl;
		jack_client_close(jack_client);
		jack_client = NULL;
		midi_input_port = NULL;
		audio_output_port = NULL;
		return 1;
        }

	gint idleSignalTag = g_timeout_add(50, idleTimeout, 0);

	// RUN!
        Gtk::Main::run(*main_window);


	// Cleanup
	g_source_remove(idleSignalTag);

	jack_deactivate(jack_client);

	sem_destroy(&controlChangeQueueSem); 

	delete main_window;
	delete yc20ui;
	delete yc20;

	return 0;
}
