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


#ifndef __ADG_MATRIX_H__
#define __ADG_MATRIX_H__

#include <glib-object.h>
#include <cairo.h>


G_BEGIN_DECLS

#define ADG_TYPE_MATRIX  (adg_matrix_get_type ())


typedef cairo_matrix_t AdgMatrix;


GType           adg_matrix_get_type             (void) G_GNUC_CONST;
AdgMatrix *     adg_matrix_dup                  (const AdgMatrix *matrix);
const AdgMatrix*adg_matrix_get_fallback         (void) G_GNUC_CONST;
AdgMatrix *     adg_matrix_copy                 (AdgMatrix       *matrix,
                                                 const AdgMatrix *src);
void            adg_matrix_init_null            (AdgMatrix       *matrix);
gboolean        adg_matrix_is_null              (const AdgMatrix *matrix);
gboolean        adg_matrix_equal                (const AdgMatrix *matrix1,
                                                 const AdgMatrix *matrix2);
void            adg_matrix_init_reflection      (AdgMatrix       *matrix,
                                                 double           radians);

G_END_DECLS


#endif /* __ADG_MATRIX_H__ */
