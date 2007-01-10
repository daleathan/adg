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


#ifndef __ADGSTYLE_H__
#define __ADGSTYLE_H__

#include <glib-object.h>
#include <cairo.h>

#include <adg/adgpair.h>


G_BEGIN_DECLS

#define ADG_TYPE_LINE_STYLE             (adg_line_style_get_type ())

typedef struct _AdgLineStyle            AdgLineStyle;

struct _AdgLineStyle
{
  double                 width;
  cairo_line_cap_t       cap;
  cairo_line_join_t      join;
  double                *dashes;        /* Dash style */
  int                    num_dashes;
  double                 dash_offset;
};

/**
 * AdgLineStyleId:
 * @ADG_LINE_STYLE_DRAW: normal drawing
 * @ADG_LINE_STYLE_CENTER: axis and center-lines
 * @ADG_LINE_STYLE_HIDDEN: covered entities
 * @ADG_LINE_STYLE_XATCH: xatches
 * @ADG_LINE_STYLE_DIM: extension and base lines of dimension entities
 * @ADG_LINE_STYLE_LAST: start of user-defined styles
 *
 * Numeric representation of line styles.
 * Some standard line styles are predefined.
 **/
typedef enum
{
  ADG_LINE_STYLE_DRAW,
  ADG_LINE_STYLE_CENTER,
  ADG_LINE_STYLE_HIDDEN,
  ADG_LINE_STYLE_XATCH,
  ADG_LINE_STYLE_DIM,
  ADG_LINE_STYLE_LAST
} AdgLineStyleId;

GType           adg_line_style_get_type (void) G_GNUC_CONST;
AdgLineStyle *  adg_line_style_from_id  (AdgLineStyleId          id);
AdgLineStyleId  adg_line_style_register (AdgLineStyle           *new_style);
void            adg_line_style_apply    (const AdgLineStyle     *style,
                                         cairo_t                *cr);



#define ADG_TYPE_FONT_STYLE             (adg_font_style_get_type ())

typedef struct _AdgFontStyle            AdgFontStyle;

struct _AdgFontStyle
{
  char                  *family;
  cairo_font_slant_t     slant;
  cairo_font_weight_t    weight;
  double                 size;
  cairo_antialias_t      antialias;
  cairo_subpixel_order_t subpixel_order;
  cairo_hint_style_t     hint_style;
  cairo_hint_metrics_t   hint_metrics;
};

/**
 * AdgFontStyleId:
 *
 * Numeric representation of font styles.
 * Some standard font styles are predefined.
 **/
typedef enum
{
  ADG_FONT_STYLE_TEXT,
  ADG_FONT_STYLE_DIMLABEL,
  ADG_FONT_STYLE_DIMTOLERANCE,
  ADG_FONT_STYLE_DIMNOTE,
  ADG_FONT_STYLE_LAST
} AdgFontStyleId;

GType           adg_font_style_get_type (void) G_GNUC_CONST;
AdgFontStyle *  adg_font_style_from_id  (AdgFontStyleId          id);
AdgFontStyleId  adg_font_style_register (AdgFontStyle           *new_style);
void            adg_font_style_apply    (const AdgFontStyle     *style,
                                         cairo_t                *cr);



#define ADG_TYPE_ARROW_STYLE            (adg_arrow_style_get_type ())

typedef struct _AdgArrowStyle           AdgArrowStyle;

/**
 * AdgArrowRenderFunc:
 * @cr: a #cairo_t drawing context
 * @arrow_style: the #AdgArrowStyle structure
 * @poi: point of interest
 * @path_data: data of the path to follow
 *
 * Callback that renders the arrow with the specified @arrow_style following
 * the @path_data path.
 **/
typedef void (*AdgArrowRenderFunc)      (cairo_t           *cr,
                                         AdgArrowStyle     *arrow_style,
                                         AdgPair           *poi,
                                         cairo_path_data_t *path_data);

struct _AdgArrowStyle
{
  double                 size;
  double                 angle;
  double                 margin;
  gboolean               need_path;
  AdgArrowRenderFunc     render_func;
};

/**
 * AdgArrowStyleId:
 * @ADG_ARROW_STYLE_ARROW: the classic arrow to use in technical drawings
 * @ADG_ARROW_STYLE_TRIANGLE: same as above, but not filled
 * @ADG_ARROW_STYLE_DOT: a little size filled circle
 * @ADG_ARROW_STYLE_TICK: an architetural tick
 * @ADG_ARROW_STYLE_CIRCLE: a medium size empty circle
 * @ADG_ARROW_STYLE_SQUARE: a medium size empty square
 * @ADG_ARROW_STYLE_LAST: start of user-defined styles
 *
 * Numeric representation of arrow styles.
 * Some standard arrow styles are predefined.
 **/
typedef enum
{
  ADG_ARROW_STYLE_ARROW,
  ADG_ARROW_STYLE_TRIANGLE,
  ADG_ARROW_STYLE_DOT,
  ADG_ARROW_STYLE_CIRCLE,
  ADG_ARROW_STYLE_SQUARE,
  ADG_ARROW_STYLE_TICK,
  ADG_ARROW_STYLE_LAST
} AdgArrowStyleId;

typedef enum
{
  ADG_PATH_POINT_START,
  ADG_PATH_POINT_MIDDLE,
  ADG_PATH_POINT_END
} AdgPathPoint;

GType           adg_arrow_style_get_type(void) G_GNUC_CONST;
AdgArrowStyle * adg_arrow_style_from_id (AdgArrowStyleId id);
AdgArrowStyleId adg_arrow_style_register(AdgArrowStyle  *new_style);
void            adg_arrow_render        (cairo_t        *cr,
                                         AdgArrowStyle  *arrow_style,
                                         cairo_path_t   *path,
                                         AdgPathPoint    path_point);



#define ADG_TYPE_DIM_STYLE              (adg_dim_style_get_type ())

typedef struct _AdgDimStyle        AdgDimStyle;

struct _AdgDimStyle
{
  cairo_pattern_t       *pattern;

  AdgFontStyle          *label_style;
  AdgFontStyle          *tolerance_style;
  AdgFontStyle          *note_style;
  AdgLineStyle          *line_style;
  AdgArrowStyle         *arrow_style;

  double                 from_offset;
  double                 to_offset;
  double                 baseline_spacing;
  AdgPair                quote_offset;
  AdgPair                tolerance_offset;
  double                 tolerance_spacing;
  AdgPair                note_offset;

  gchar                 *measure_format;
};

/**
 * AdgDimStyleId:
 * @ADG_DIM_STYLE_ISO: ISO standard
 * @ADG_DIM_STYLE_LAST: start of user-defined styles
 *
 * Numeric representation of dimension styles.
 * Some standard dimension styles are predefined.
 **/
typedef enum
{
  ADG_DIM_STYLE_ISO,
  ADG_DIM_STYLE_LAST
} AdgDimStyleId;

GType           adg_dim_style_get_type  (void) G_GNUC_CONST;
AdgDimStyle *   adg_dim_style_from_id   (AdgDimStyleId   id);
AdgDimStyleId   adg_dim_style_register  (AdgDimStyle    *new_style);


G_END_DECLS


#endif /* __ADGSTYLE_H__ */
