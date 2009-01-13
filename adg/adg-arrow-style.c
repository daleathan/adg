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

/**
 * SECTION:arrow-style
 * @title: AdgArrowStyle
 * @short_description: Arrow rendering related stuff
 *
 * Contains parameters on how to draw arrows, providing a way to register a
 * custom rendering callback.
 */

#include "adg-arrow-style.h"
#include "adg-arrow-style-private.h"
#include "adg-intl.h"
#include "adg-util.h"
#include <math.h>

#define PARENT_CLASS ((AdgStyleClass *) adg_arrow_style_parent_class)


enum {
    PROP_0,
    PROP_SIZE,
    PROP_ANGLE,
    PROP_MARGIN,
    PROP_RENDERER
};


static void             get_property            (GObject        *object,
                                                 guint           prop_id,
                                                 GValue         *value,
                                                 GParamSpec     *pspec);
static void             set_property            (GObject        *object,
                                                 guint           prop_id,
                                                 const GValue   *value,
                                                 GParamSpec     *pspec);
static GPtrArray *      get_pool                (void);
static void             arrow_renderer          (AdgArrowStyle  *arrow_style,
                                                 cairo_t        *cr,
                                                 CpmlSegment    *segment);
static void             triangle_renderer       (AdgArrowStyle  *arrow_style,
                                                 cairo_t        *cr,
                                                 CpmlSegment    *segment);
static void             dot_renderer            (AdgArrowStyle  *arrow_style,
                                                 cairo_t        *cr,
                                                 CpmlSegment    *segment);
static void             circle_renderer         (AdgArrowStyle  *arrow_style,
                                                 cairo_t        *cr,
                                                 CpmlSegment    *segment);
static void             block_renderer          (AdgArrowStyle  *arrow_style,
                                                 cairo_t        *cr,
                                                 CpmlSegment    *segment);
static void             square_renderer         (AdgArrowStyle  *arrow_style,
                                                 cairo_t        *cr,
                                                 CpmlSegment    *segment);
static void             tick_renderer           (AdgArrowStyle  *arrow_style,
                                                 cairo_t        *cr,
                                                 CpmlSegment    *segment);
static void             draw_triangle           (cairo_t        *cr,
                                                 AdgArrowStyle  *arrow_style,
                                                 CpmlSegment    *segment);
static void             draw_circle             (cairo_t        *cr,
                                                 AdgArrowStyle  *arrow_style,
                                                 CpmlSegment    *segment);


G_DEFINE_TYPE(AdgArrowStyle, adg_arrow_style, ADG_TYPE_STYLE)


static void
adg_arrow_style_class_init(AdgArrowStyleClass *klass)
{
    GObjectClass *gobject_class;
    AdgStyleClass *style_class;
    GParamSpec *param;

    gobject_class = (GObjectClass *) klass;
    style_class = (AdgStyleClass *) klass;

    g_type_class_add_private(klass, sizeof(AdgArrowStylePrivate));

    gobject_class->get_property = get_property;
    gobject_class->set_property = set_property;

    style_class->get_pool = get_pool;

    param = g_param_spec_double("size",
                                P_("Arrow Size"),
                                P_
                                ("The size of the arrow, a renderer dependent parameter"),
                                -G_MAXDOUBLE, G_MAXDOUBLE, 14.,
                                G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_SIZE, param);

    param = g_param_spec_double("angle",
                                P_("Arrow Angle"),
                                P_
                                ("The angle of the arrow, a renderer dependent parameter"),
                                -G_MAXDOUBLE, G_MAXDOUBLE, G_PI / 6.,
                                G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_ANGLE, param);

    param = g_param_spec_double("margin",
                                P_("Arrow Margin"),
                                P_
                                ("The margin of the arrow, a renderer dependent parameter"),
                                -G_MAXDOUBLE, G_MAXDOUBLE, 14.,
                                G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_MARGIN, param);

    param = g_param_spec_pointer("renderer",
                                 P_("Renderer Callback"),
                                 P_
                                 ("The callback to call to renderer this arrow type"),
                                 G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_RENDERER, param);
}

static void
adg_arrow_style_init(AdgArrowStyle *arrow_style)
{
    AdgArrowStylePrivate *priv = G_TYPE_INSTANCE_GET_PRIVATE(arrow_style,
                                                             ADG_TYPE_ARROW_STYLE,
                                                             AdgArrowStylePrivate);

    priv->size = 14.;
    priv->angle = G_PI / 6.;
    priv->margin = 14.;
    priv->renderer = NULL;

    arrow_style->priv = priv;
}

static void
get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
    AdgArrowStyle *arrow_style = (AdgArrowStyle *) object;

    switch (prop_id) {
    case PROP_SIZE:
        g_value_set_double(value, arrow_style->priv->size);
        break;
    case PROP_ANGLE:
        g_value_set_double(value, arrow_style->priv->angle);
        break;
    case PROP_MARGIN:
        g_value_set_double(value, arrow_style->priv->margin);
        break;
    case PROP_RENDERER:
        g_value_set_pointer(value, arrow_style->priv->renderer);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void
set_property(GObject *object,
             guint prop_id, const GValue *value, GParamSpec *pspec)
{
    AdgArrowStyle *arrow_style = (AdgArrowStyle *) object;

    switch (prop_id) {
    case PROP_SIZE:
        arrow_style->priv->size = g_value_get_double(value);
        break;
    case PROP_ANGLE:
        arrow_style->priv->angle = g_value_get_double(value);
        break;
    case PROP_MARGIN:
        arrow_style->priv->margin = g_value_get_double(value);
        break;
    case PROP_RENDERER:
        arrow_style->priv->renderer = g_value_get_pointer(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}


/**
 * adg_arrow_style_get_slot:
 *
 * Gets the slot id for this style class.
 *
 * Return value: the slot
 **/
AdgStyleSlot
adg_arrow_style_get_slot(void)
{
    static AdgStyleSlot slot = -1;

    if (G_UNLIKELY(slot < 0))
        slot = adg_context_get_slot(ADG_TYPE_ARROW_STYLE);

    return slot;
}

/**
 * adg_arrow_style_new:
 *
 * Constructs a new arrow style initialized with default params.
 *
 * Return value: a new arrow style
 **/
AdgStyle *
adg_arrow_style_new(void)
{
    return g_object_new(ADG_TYPE_ARROW_STYLE, NULL);
}

/**
 * adg_arrow_style_render:
 * @arrow_style: an #AdgArrowStyle instance
 * @cr: the cairo context to use
 * @segment: the #CpmlSegment where the arrow must be rendered
 *
 * Renders an arrow on @cr at the beginning of @segment in the way
 * specified by @arrow_style.
 **/
void
adg_arrow_style_render(AdgArrowStyle *arrow_style,
                       cairo_t *cr, CpmlSegment *segment)
{
    g_return_if_fail(arrow_style != NULL);
    g_return_if_fail(cr != NULL);
    g_return_if_fail(segment != NULL);

    /* NULL renderer */
    if (arrow_style->priv->renderer == NULL)
        return;

    PARENT_CLASS->apply((AdgStyle *) arrow_style, cr);
    arrow_style->priv->renderer(arrow_style, cr, segment);
}

/**
 * adg_arrow_style_get_size:
 * @arrow_style: an #AdgArrowStyle object
 *
 * Gets the size (in paper units) of the arrow (renderer dependent value).
 *
 * Return value: the size value
 **/
gdouble
adg_arrow_style_get_size(AdgArrowStyle *arrow_style)
{
    g_return_val_if_fail(ADG_IS_ARROW_STYLE(arrow_style), 0.);

    return arrow_style->priv->size;
}

/**
 * adg_arrow_style_set_size:
 * @arrow_style: an #AdgArrowStyle object
 * @size: the new size
 *
 * Sets a new size.
 **/
void
adg_arrow_style_set_size(AdgArrowStyle *arrow_style, gdouble size)
{
    g_return_if_fail(ADG_IS_ARROW_STYLE(arrow_style));

    arrow_style->priv->size = size;
    g_object_notify((GObject *) arrow_style, "size");
}

/**
 * adg_arrow_style_get_angle:
 * @arrow_style: an #AdgArrowStyle object
 *
 * Gets the angle (in degree) of the arrow (renderer dependent value).
 *
 * Return value: the angle value
 **/
gdouble
adg_arrow_style_get_angle(AdgArrowStyle *arrow_style)
{
    g_return_val_if_fail(ADG_IS_ARROW_STYLE(arrow_style), 0.);

    return arrow_style->priv->angle;
}

/**
 * adg_arrow_style_set_angle:
 * @arrow_style: an #AdgArrowStyle object
 * @angle: the new angle
 *
 * Sets a new angle.
 **/
void
adg_arrow_style_set_angle(AdgArrowStyle *arrow_style, gdouble angle)
{
    g_return_if_fail(ADG_IS_ARROW_STYLE(arrow_style));

    arrow_style->priv->angle = angle;
    g_object_notify((GObject *) arrow_style, "angle");
}

/**
 * adg_arrow_style_get_margin:
 * @arrow_style: an #AdgArrowStyle object
 *
 * Gets the margin (in paper units) of this arrow (renderer dependent value).
 * The margin is also used to trim the baseline of this amount.
 *
 * Return value: the margin value
 **/
gdouble
adg_arrow_style_get_margin(AdgArrowStyle *arrow_style)
{
    g_return_val_if_fail(ADG_IS_ARROW_STYLE(arrow_style), 0.);

    return arrow_style->priv->margin;
}

/**
 * adg_arrow_style_set_margin:
 * @arrow_style: an #AdgArrowStyle object
 * @margin: the new margin
 *
 * Sets a new margin.
 **/
void
adg_arrow_style_set_margin(AdgArrowStyle *arrow_style, gdouble margin)
{
    g_return_if_fail(ADG_IS_ARROW_STYLE(arrow_style));

    arrow_style->priv->margin = margin;
    g_object_notify((GObject *) arrow_style, "margin");
}

/**
 * adg_arrow_style_get_renderer:
 * @arrow_style: an #AdgArrowStyle object
 *
 * Gets the renderer of @arrow_style.
 *
 * Return value: the renderer value
 **/
const AdgArrowRenderer
adg_arrow_style_get_renderer(AdgArrowStyle *arrow_style)
{
    g_return_val_if_fail(ADG_IS_ARROW_STYLE(arrow_style), NULL);

    return arrow_style->priv->renderer;
}

/**
 * adg_arrow_style_set_renderer:
 * @arrow_style: an #AdgArrowStyle object
 * @renderer: the new renderer
 *
 * Sets a new renderer.
 **/
void
adg_arrow_style_set_renderer(AdgArrowStyle *arrow_style,
                             AdgArrowRenderer renderer)
{
    g_return_if_fail(ADG_IS_ARROW_STYLE(arrow_style));

    arrow_style->priv->renderer = renderer;
    g_object_notify((GObject *) arrow_style, "renderer");
}


static GPtrArray *
get_pool(void)
{
    static GPtrArray *pool = NULL;

    if (G_UNLIKELY(pool == NULL)) {
        pool = g_ptr_array_sized_new(ADG_ARROW_STYLE_LAST);

        pool->pdata[ADG_ARROW_STYLE_ARROW] =
            g_object_new(ADG_TYPE_ARROW_STYLE, "renderer", arrow_renderer,
                         NULL);
        pool->pdata[ADG_ARROW_STYLE_TRIANGLE] =
            g_object_new(ADG_TYPE_ARROW_STYLE, "renderer",
                         triangle_renderer, NULL);
        pool->pdata[ADG_ARROW_STYLE_DOT] =
            g_object_new(ADG_TYPE_ARROW_STYLE, "size", 5., "angle", 0.,
                         "margin", 2.5, "renderer", dot_renderer, NULL);
        pool->pdata[ADG_ARROW_STYLE_CIRCLE] =
            g_object_new(ADG_TYPE_ARROW_STYLE, "size", 10., "angle", 0.,
                         "margin", 5., "renderer", circle_renderer, NULL);
        pool->pdata[ADG_ARROW_STYLE_BLOCK] =
            g_object_new(ADG_TYPE_ARROW_STYLE, "size", 10., "angle", 0.,
                         "margin", 5., "renderer", block_renderer, NULL);
        pool->pdata[ADG_ARROW_STYLE_SQUARE] =
            g_object_new(ADG_TYPE_ARROW_STYLE, "size", 10., "angle", 0.,
                         "margin", -0.1, "renderer", square_renderer,
                         NULL);
        pool->pdata[ADG_ARROW_STYLE_TICK] =
            g_object_new(ADG_TYPE_ARROW_STYLE, "size", 20., "angle",
                         G_PI / 3., "margin", 0., "renderer",
                         tick_renderer, NULL);

        pool->len = ADG_ARROW_STYLE_LAST;
        }

    return pool;
}

static void
arrow_renderer(AdgArrowStyle *arrow_style, cairo_t *cr, CpmlSegment *segment)
{
    draw_triangle(cr, arrow_style, segment);
    cairo_fill(cr);
}

static void
triangle_renderer(AdgArrowStyle *arrow_style, cairo_t *cr, CpmlSegment *segment)
{
    draw_triangle(cr, arrow_style, segment);
    cairo_stroke(cr);
}

static void
dot_renderer(AdgArrowStyle *arrow_style, cairo_t *cr, CpmlSegment *segment)
{
    draw_circle(cr, arrow_style, segment);
    cairo_fill(cr);
}

static void
circle_renderer(AdgArrowStyle *arrow_style, cairo_t *cr, CpmlSegment *segment)
{
    draw_circle(cr, arrow_style, segment);
    cairo_stroke(cr);
}

static void
block_renderer(AdgArrowStyle *arrow_style, cairo_t *cr, CpmlSegment *segment)
{
    ADG_STUB();
}

static void
square_renderer(AdgArrowStyle *arrow_style, cairo_t *cr, CpmlSegment *segment)
{
    ADG_STUB();
}

static void
tick_renderer(AdgArrowStyle *arrow_style, cairo_t *cr, CpmlSegment *segment)
{
    ADG_STUB();
}

static void
draw_triangle(cairo_t *cr, AdgArrowStyle *arrow_style, CpmlSegment *segment)
{
    double length, height_2;
    double org_x, org_y;
    double tmp;
    CpmlPair tail, tail1, tail2;
    CpmlPair vector;

    length = arrow_style->priv->size;
    height_2 = tan(arrow_style->priv->angle / 2.0) * length;
    cairo_device_to_user_distance(cr, &length, &height_2);
    org_x = segment->path.data[1].point.x;
    org_y = segment->path.data[1].point.y;

    switch (segment->path.data[2].header.type) {
    case CAIRO_PATH_LINE_TO:
        vector.x = segment->path.data[3].point.x - org_x;
        vector.y = segment->path.data[3].point.y - org_y;
        cpml_vector_from_pair(&vector, &vector);

        tail.x = vector.x * length + org_x;
        tail.y = vector.y * length + org_y;

        tmp = vector.x;
        vector.x = -vector.y * height_2;
        vector.y = tmp * height_2;

        tail1.x = tail.x + vector.x;
        tail1.y = tail.y + vector.y;

        tail2.x = tail.x - vector.x;
        tail2.y = tail.y - vector.y;

        cairo_move_to(cr, org_x, org_y);
        cairo_line_to(cr, tail1.x, tail1.y);
        cairo_line_to(cr, tail2.x, tail2.y);
        cairo_close_path(cr);

        break;
    case CAIRO_PATH_CURVE_TO:
        ADG_STUB();
        break;
    default:
        g_assert_not_reached();
    }
}

static void
draw_circle(cairo_t *cr, AdgArrowStyle *arrow_style, CpmlSegment *segment)
{
    double radius = arrow_style->priv->size / 2.;
    double dummy = 0.;

    cairo_device_to_user_distance(cr, &radius, &dummy);
    cairo_new_path(cr);
    cairo_arc(cr,
              segment->path.data[1].point.x, segment->path.data[1].point.y,
              radius, 0., M_PI);
}
