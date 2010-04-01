/* CPML - Cairo Path Manipulation Library
 * Copyright (C) 2008,2009,2010  Nicola Fontana <ntd at entidi.it>
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


#if !defined(__CPML_H__)
#error "Only <cpml/cpml.h> can be included directly."
#endif


#ifndef __CPML_SEGMENT_H__
#define __CPML_SEGMENT_H__


CAIRO_BEGIN_DECLS

typedef cairo_path_t CpmlPath;
typedef struct _CpmlSegment CpmlSegment;

struct _CpmlSegment {
        CpmlPath          *path;
        cairo_path_data_t *data;
        int                num_data;
};


cairo_bool_t
        cpml_segment_from_cairo         (CpmlSegment            *segment,
                                         CpmlPath               *path);
void    cpml_segment_copy               (CpmlSegment            *segment,
                                         const CpmlSegment      *src);
#define cpml_path_is_empty(cpml_path) \
    ((cpml_path) == NULL || (cpml_path)->data == NULL || \
     (cpml_path)->num_data <= 0)
void    cpml_segment_reset              (CpmlSegment            *segment);
cairo_bool_t
        cpml_segment_next               (CpmlSegment            *segment);
double  cpml_segment_get_length         (const CpmlSegment      *segment);
void    cpml_segment_put_extents        (const CpmlSegment      *segment,
                                         CpmlExtents            *extents);
void    cpml_segment_put_pair_at        (const CpmlSegment      *segment,
                                         double                  pos,
                                         CpmlPair               *pair);
void    cpml_segment_put_vector_at      (const CpmlSegment      *segment,
                                         double                  pos,
                                         CpmlVector             *vector);
size_t  cpml_segment_put_intersections  (const CpmlSegment      *segment,
                                         const CpmlSegment      *segment2,
                                         size_t                  n_dest,
                                         CpmlPair               *dest);
void    cpml_segment_offset             (CpmlSegment            *segment,
                                         double                  offset);
void    cpml_segment_transform          (CpmlSegment            *segment,
                                         const cairo_matrix_t   *matrix);
void    cpml_segment_reverse            (CpmlSegment            *segment);
void    cpml_segment_to_cairo           (const CpmlSegment      *segment,
                                         cairo_t                *cr);
void    cpml_segment_dump               (const CpmlSegment      *segment);

CAIRO_END_DECLS


#endif /* __CPML_SEGMENT_H__ */