/* ADG - Automatic Drawing Generation
 * Copyright (C) 2007,2008,2009,2010  Nicola Fontana <ntd at entidi.it>
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


#if !defined(__ADG_GTK_H__)
#error "Only <adg-gtk.h> can be included directly."
#endif


#ifndef __ADG_GTK_AREA_H__
#define __ADG_GTK_AREA_H__


G_BEGIN_DECLS

#define ADG_GTK_TYPE_AREA             (adg_gtk_area_get_type())
#define ADG_GTK_AREA(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), ADG_GTK_TYPE_AREA, AdgGtkArea))
#define ADG_GTK_AREA_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), ADG_GTK_TYPE_AREA, AdgGtkAreaClass))
#define ADG_GTK_IS_AREA(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), ADG_GTK_TYPE_AREA))
#define ADG_GTK_IS_AREA_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), ADG_GTK_TYPE_AREA))
#define ADG_GTK_AREA_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), ADG_GTK_TYPE_AREA, AdgGtkAreaClass))

typedef struct _AdgGtkArea        AdgGtkArea;
typedef struct _AdgGtkAreaClass   AdgGtkAreaClass;

struct _AdgGtkArea {
    /*< private >*/
    GtkDrawingArea       parent;
    gpointer             data;
};

struct _AdgGtkAreaClass {
    /*< private >*/
    GtkDrawingAreaClass  parent_class;
    /*< public >*/
    /* Virtual Table */
    void                (*canvas_changed)       (AdgGtkArea     *area);
};


GType           adg_gtk_area_get_type           (void) G_GNUC_CONST;

GtkWidget *     adg_gtk_area_new                (void);
GtkWidget *     adg_gtk_area_new_with_canvas    (AdgCanvas      *canvas);
void            adg_gtk_area_set_canvas         (AdgGtkArea     *area,
                                                 AdgCanvas      *canvas);
AdgCanvas *     adg_gtk_area_get_canvas         (AdgGtkArea     *area);
void            adg_gtk_area_set_factor         (AdgGtkArea     *area,
                                                 gdouble         factor);
gdouble         adg_gtk_area_get_factor         (AdgGtkArea     *area);

G_END_DECLS


#endif /* __ADG_GTK_AREA_H__ */