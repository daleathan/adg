/* CPML - Cairo Path Manipulation Library
 * Copyright (C) 2008, 2009  Nicola Fontana <ntd at entidi.it>
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

#ifndef __CPML_ARC_H__
#define __CPML_ARC_H__

#include <cpml/cpml-primitive.h>
#include <cpml/cpml-macros.h>

#ifndef CAIRO_PATH_ARC_TO
#define CAIRO_PATH_ARC_TO       100
#undef  CAIRO_HAS_ARC_SUPPORT
#else
#define CAIRO_HAS_ARC_SUPPORT   1
#endif


CAIRO_BEGIN_DECLS

int     cpml_arc_type_get_npoints       (void) CPML_GNUC_CONST;
void    cpml_arc_center                 (const CpmlPrimitive    *arc,
                                         CpmlPair               *pair);
void    cpml_arc_pair_at                (const CpmlPrimitive    *arc,
                                         CpmlPair               *pair,
                                         double                  pos);
void    cpml_arc_vector_at              (const CpmlPrimitive    *arc,
                                         CpmlVector             *vector,
                                         double                  pos);
int     cpml_arc_intersection           (const CpmlPrimitive    *arc,
                                         const CpmlPrimitive    *arc2,
                                         CpmlPair               *dest);
int     cpml_arc_intersection_with_line (const CpmlPrimitive    *arc,
                                         const CpmlPrimitive    *line,
                                         CpmlPair               *dest);
void    cpml_arc_offset                 (CpmlPrimitive          *arc,
                                         double                  offset);

CAIRO_END_DECLS


#endif /* __CPML_ARC_H__ */