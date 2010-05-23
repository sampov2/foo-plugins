#! /usr/bin/env python
# encoding: utf-8

import autowaf

VERSION	= '2.0'
APPNAME	= 'foo-plugins'
srcdir	= '.'
blddir	= 'build'



def configure(conf):
	autowaf.configure(conf)
	autowaf.check_pkg(conf, 'lv2-plugin', uselib_store='LV2-PLUGIN', atleast_version='1.0.2')
	autowaf.check_pkg(conf, 'gtkmm-2.4', uselib_store='GTKMM', atleast_version='2.8.0')
	conf.env['shlib_PATTERN'] = '%s.so'
	# we need to check whether user provides compiler options, otherwise..
	compiler_options = [ '-Wall', '-O3', '-msse', '-msse2', '-mfpmath=sse', '-ffast-math',
                             '-funroll-loops', '-fomit-frame-pointer' ]
	conf.env.append_value('CXXFLAGS', compiler_options)

def set_options(opt):
	autowaf.set_options(opt)

def build(bld):
	guiobj = bld.new_task_gen('cxx','shlib')
	guiobj.source = '''
		src/schmooz_ui.cpp
	'''

	guiobj.name   = 'schmooz_ui'
	#guiobj.uselib = 'GTKMM'
	guiobj.uselib = [ 'GTKMM', 'm' ]
	guiobj.vnum   = ''
	guiobj.target = 'schmooz_ui'
        guiobj.cxxflags = [ '-DSCHMOOZ_PNG_DIR="' + bld.env['LV2DIR'] + '/foo.lv2/schmooz_ui/"' ];
	guiobj.install_path = '${LV2DIR}/foo.lv2'
	guiobj.includes = 'extra-include'

	obj = bld.new_task_gen('cxx', 'shlib')
	obj.source = '''
		src/schmooz_mono.cpp
		src/schmooz_stereo.cpp
		src/chop.cpp
		src/driver.cpp
		src/limiter.cpp
		src/limiter_v2.cpp
		src/saturator.cpp
		src/transients_v2.cpp
		src/transients_mono_v2.cpp
		src/el_maxim.cpp
		src/t00b_limiter.cpp

		src/rms.cpp
	'''

	obj.name   = 'foo-plugins'
	obj.uselib = 'LV2-PLUGIN'
	obj.vnum   = ''
	obj.target = 'foo-plugins'
	obj.install_path = '${LV2DIR}/foo.lv2'

	bld.install_files('${LV2DIR}/foo.lv2', 'src/ttl/manifest.ttl')
	bld.install_files('${LV2DIR}/foo.lv2', 'src/ttl/schmooz-mono.ttl')
	bld.install_files('${LV2DIR}/foo.lv2', 'src/ttl/schmooz-stereo.ttl')
	bld.install_files('${LV2DIR}/foo.lv2', 'src/ttl/chop.ttl')
	bld.install_files('${LV2DIR}/foo.lv2', 'src/ttl/driver.ttl')
	bld.install_files('${LV2DIR}/foo.lv2', 'src/ttl/limiter.ttl')
	bld.install_files('${LV2DIR}/foo.lv2', 'src/ttl/limiter-v2.ttl')
	bld.install_files('${LV2DIR}/foo.lv2', 'src/ttl/saturator.ttl')
	bld.install_files('${LV2DIR}/foo.lv2', 'src/ttl/transients-v2.ttl')
	bld.install_files('${LV2DIR}/foo.lv2', 'src/ttl/transients-mono-v2.ttl')
	bld.install_files('${LV2DIR}/foo.lv2', 'src/ttl/el-maxim.ttl')
	bld.install_files('${LV2DIR}/foo.lv2', 'src/ttl/t00b-limiter.ttl')

	pngs = '''
		graphics/schmooz/background.png
		graphics/schmooz/high-pass_on.png
		graphics/schmooz/high-pass_on_prelight.png
		graphics/schmooz/high-pass_off.png
		graphics/schmooz/high-pass_off_prelight.png
		graphics/schmooz/bypass_on.png
		graphics/schmooz/bypass_on_prelight.png
		graphics/schmooz/bypass_off.png
		graphics/schmooz/bypass_off_prelight.png
		graphics/schmooz/graph_bg.png
		graphics/schmooz/graph_bg_threshold.png
		graphics/schmooz/threshold.png
		graphics/schmooz/threshold_prelight.png
		graphics/schmooz/ratio_trough.png
		graphics/schmooz/ratio_thumb.png
		graphics/schmooz/ratio_thumb_prelight.png
		graphics/schmooz/slider_zero.png
		graphics/schmooz/slider_zero_prelight.png
		graphics/schmooz/slider_attack.png
		graphics/schmooz/slider_attack_prelight.png
		graphics/schmooz/slider_release.png
		graphics/schmooz/slider_release_prelight.png
		graphics/schmooz/slider_make-up.png
		graphics/schmooz/slider_make-up_prelight.png
		graphics/schmooz/dry-wet_thumb.png
		graphics/schmooz/dry-wet_thumb_prelight.png
		graphics/schmooz/make-up.png
		graphics/schmooz/make-up_full.png
		graphics/schmooz/gain_v.png
		graphics/schmooz/gain_v_full.png
		graphics/schmooz/gain_h.png
		graphics/schmooz/gain_h_full.png
	'''

	bld.install_files('${LV2DIR}/foo.lv2/schmooz_ui', pngs)

	
	
def shutdown():
	autowaf.shutdown()


