#! /usr/bin/env python
# encoding: utf-8

import autowaf

VERSION	= "2.0"
APPNAME	= "foo-plugins"
srcdir	= "."
blddir	= "build"



def configure(conf):
	autowaf.configure(conf)
	autowaf.check_pkg(conf, 'lv2-plugin', uselib_store='LV2-PLUGIN', atleast_version='1.0.2')
	conf.env['shlib_PATTERN'] = '%s.so'
	# we need to check whether user provides compiler options, otherwise..
	compiler_options = [ '-Wall', '-O3', '-msse', '-msse2', '-mfpmath=sse', '-ffast-math' ]
	conf.env.append_value('CXXFLAGS', compiler_options)

def set_options(opt):
	autowaf.set_options(opt)

def build(bld):
	obj = bld.new_task_gen('cxx', 'shlib')
	obj.source = '''
		src/schmooz_mono_lv2.cpp
		src/schmooz_stereo_lv2.cpp
		src/chop.cpp
		src/driver.cpp
	'''

	obj.name   = "schmooz"
	obj.uselib = "LV2-PLUGIN"
	obj.vnum   = ""
	obj.target = "foo-plugins"
	obj.install_path = "/home/v2/.lv2/schmooz.lv2"

	bld.install_files("/home/v2/.lv2/schmooz.lv2", "src/ttl/manifest.ttl")
	bld.install_files("/home/v2/.lv2/schmooz.lv2", "src/ttl/schmooz-mono.ttl")
	bld.install_files("/home/v2/.lv2/schmooz.lv2", "src/ttl/schmooz-stereo.ttl")
	bld.install_files("/home/v2/.lv2/schmooz.lv2", "src/ttl/chop.ttl")
	bld.install_files("/home/v2/.lv2/schmooz.lv2", "src/ttl/driver.ttl")

	
	
def shutdown():
	autowaf.shutdown()


