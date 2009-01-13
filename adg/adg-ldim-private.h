/* ADG - Automatic Drawing Generation
 * Copyright (C) 2007,2008,2009 Nicola Fontana <ntd at entidi.it>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */


#ifndef __ADG_LDIM_PRIVATE_H__
#define __ADG_LDIM_PRIVATE_H__

#include <cairo.h>


G_BEGIN_DECLS

/* The path_data cache is structured as following:
 * [0]  = MOVE_TO
 * [1]  = baseline start
 * [2]  = LINE_TO
 * [3]  = baseline end
 * [4]  = MOVE_TO
 * [5]  = first extension line start
 * [6]  = LINE_TO
 * [7]  = first extension line end
 * [8]  = MOVE_TO
 * [9]  = second extension line start
 * [10] = LINE_TO
 * [11] = second extension line end
 *
 * The director_data cache is as following:
 * [0]  = MOVE_TO
 * [1]  = start of arrow director
 * [2]  = LINE_TO
 * [3]  = end of arrow director
 */

struct _AdgLDimPrivate {
    /* Properties */
    double              direction;

    /* Cache */
    cairo_path_t        path;
    cairo_path_data_t   path_data[12];
    CpmlSegment         director;
    cairo_path_data_t   director_data[4];
};

G_END_DECLS


#endif /* __ADG_LDIM_PRIVATE_H__ */
