/*
    This header file is part of foo-plugins, a package of ladspa plugins
    Copyright (C) 2006 Sampo Savolainen <v2@iki.fi> (except where credited otherwise)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _FOO_UTILS_H
#define _FOO_UTILS_H

#include <math.h>
#include "rms.h"

/*
    Convert a value in dB's to a coefficent, from swh-plugins
    Copyright (C) 2000 Steve Harris 
*/
#define DB_CO(g) ((g) > -90.0f ? powf(10.0f, (g) * 0.05f) : 0.0f)
#define CO_DB(v) (20.0f * log10f(v))


#endif /* _FOO_UTILS_H */

