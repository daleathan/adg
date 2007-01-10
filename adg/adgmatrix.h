/* ADG - Automatic Drawing Generation.
 * Copyright (C) 2007 - Fontana Nicola <ntd@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */


#ifndef __ADGMATRIX_H__
#define __ADGMATRIX_H__

#include <glib-object.h>
#include <cairo.h>


G_BEGIN_DECLS

#define ADG_TYPE_MATRIX  (adg_matrix_get_type ())

typedef cairo_matrix_t AdgMatrix;


GType                   adg_matrix_get_type             (void) G_GNUC_CONST;

const AdgMatrix *       adg_matrix_get_fallback         (void);
AdgMatrix *             adg_matrix_dup                  (const AdgMatrix *matrix);
AdgMatrix *             adg_matrix_set                  (AdgMatrix       *matrix,
                                                         const AdgMatrix *src);
gboolean                adg_matrix_equal                (const AdgMatrix *matrix1,
                                                         const AdgMatrix *matrix2);

G_END_DECLS


#endif /* __ADGMATRIX_H__ */
