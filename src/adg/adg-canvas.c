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


/**
 * SECTION:adg-canvas
 * @short_description: The drawing container
 *
 * The canvas is the toplevel entity of an ADG drawing. It can be
 * bound to a GTK+ widget, such as #AdgGtkArea, or manually rendered
 * to a custom surface.
 **/

/**
 * AdgCanvas:
 *
 * All fields are private and should not be used directly.
 * Use its public methods instead.
 **/


#include "adg-internal.h"
#include "adg-canvas.h"
#include "adg-canvas-private.h"
#include "adg-dress-builtins.h"
#include "adg-color-style.h"

#define PARENT_ENTITY_CLASS  ((AdgEntityClass *) adg_canvas_parent_class)


G_DEFINE_TYPE(AdgCanvas, adg_canvas, ADG_TYPE_CONTAINER);

enum {
    PROP_0,
    PROP_BACKGROUND_DRESS,
    PROP_FRAME_DRESS,
    PROP_TOP_MARGIN,
    PROP_RIGHT_MARGIN,
    PROP_BOTTOM_MARGIN,
    PROP_LEFT_MARGIN,
    PROP_HAS_FRAME,
    PROP_TOP_PADDING,
    PROP_RIGHT_PADDING,
    PROP_BOTTOM_PADDING,
    PROP_LEFT_PADDING
};


static void             _adg_get_property       (GObject        *object,
                                                 guint           param_id,
                                                 GValue         *value,
                                                 GParamSpec     *pspec);
static void             _adg_set_property       (GObject        *object,
                                                 guint           param_id,
                                                 const GValue   *value,
                                                 GParamSpec     *pspec);
static void             _adg_arrange            (AdgEntity      *entity);
static void             _adg_render             (AdgEntity      *entity,
                                                 cairo_t        *cr);


static void
adg_canvas_class_init(AdgCanvasClass *klass)
{
    GObjectClass *gobject_class;
    AdgEntityClass *entity_class;
    GParamSpec *param;

    gobject_class = (GObjectClass *) klass;
    entity_class = (AdgEntityClass *) klass;

    g_type_class_add_private(klass, sizeof(AdgCanvasPrivate));

    gobject_class->get_property = _adg_get_property;
    gobject_class->set_property = _adg_set_property;

    entity_class->arrange = _adg_arrange;
    entity_class->render = _adg_render;

    param = adg_param_spec_dress("background-dress",
                                 P_("Background Dress"),
                                 P_("The color dress to use for the canvas background"),
                                 ADG_DRESS_COLOR_BACKGROUND,
                                 G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_BACKGROUND_DRESS, param);

    param = adg_param_spec_dress("frame-dress",
                                 P_("Frame Dress"),
                                 P_("Line dress to use while drawing the frame around the canvas"),
                                 ADG_DRESS_LINE_FRAME,
                                 G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_FRAME_DRESS, param);

    param = g_param_spec_double("top-margin",
                                P_("Top Margin"),
                                P_("The margin (in identity space) to leave above the frame"),
                                G_MINDOUBLE, G_MAXDOUBLE, 15,
                                G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_TOP_MARGIN, param);

    param = g_param_spec_double("right-margin",
                                P_("Right Margin"),
                                P_("The margin (in identity space) to leave empty at the right of the frame"),
                                G_MINDOUBLE, G_MAXDOUBLE, 15,
                                G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_RIGHT_MARGIN, param);

    param = g_param_spec_double("bottom-margin",
                                P_("Bottom Margin"),
                                P_("The margin (in identity space) to leave empty below the frame"),
                                G_MINDOUBLE, G_MAXDOUBLE, 15,
                                G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_BOTTOM_MARGIN, param);

    param = g_param_spec_double("left-margin",
                                P_("Left Margin"),
                                P_("The margin (in identity space) to leave empty at the left of the frame"),
                                G_MINDOUBLE, G_MAXDOUBLE, 15,
                                G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_LEFT_MARGIN, param);

    param = g_param_spec_boolean("has-frame",
                                 P_("Has Frame Flag"),
                                 P_("If enabled, a frame using the frame dress will be drawn around the canvas extents, taking into account the margins"),
                                 TRUE,
                                 G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_HAS_FRAME, param);

    param = g_param_spec_double("top-padding",
                                P_("Top Padding"),
                                P_("The padding (in identity space) to leave empty above between the drawing and the frame"),
                                G_MINDOUBLE, G_MAXDOUBLE, 15,
                                G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_TOP_PADDING, param);

    param = g_param_spec_double("right-padding",
                                P_("Right Padding"),
                                P_("The padding (in identity space) to leave empty at the right between the drawing and the frame"),
                                G_MINDOUBLE, G_MAXDOUBLE, 15,
                                G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_RIGHT_PADDING, param);

    param = g_param_spec_double("bottom-padding",
                                P_("Bottom Padding"),
                                P_("The padding (in identity space) to leave empty below between the drawing and the frame"),
                                G_MINDOUBLE, G_MAXDOUBLE, 15,
                                G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_BOTTOM_PADDING, param);

    param = g_param_spec_double("left-padding",
                                P_("Left Padding"),
                                P_("The padding (in identity space) to leave empty at the left between the drawing and the frame"),
                                G_MINDOUBLE, G_MAXDOUBLE, 15,
                                G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_LEFT_PADDING, param);
}

static void
adg_canvas_init(AdgCanvas *canvas)
{
    AdgCanvasPrivate *data = G_TYPE_INSTANCE_GET_PRIVATE(canvas,
                                                         ADG_TYPE_CANVAS,
                                                         AdgCanvasPrivate);

    data->background_dress = ADG_DRESS_COLOR_BACKGROUND;
    data->frame_dress = ADG_DRESS_LINE_FRAME;
    data->top_margin = 15;
    data->right_margin = 15;
    data->bottom_margin = 15;
    data->left_margin = 15;
    data->has_frame = TRUE;
    data->top_padding = 15;
    data->right_padding = 15;
    data->bottom_padding = 15;
    data->left_padding = 15;

    canvas->data = data;
}

static void
_adg_get_property(GObject *object, guint prop_id,
                  GValue *value, GParamSpec *pspec)
{
    AdgCanvasPrivate *data = ((AdgCanvas *) object)->data;

    switch (prop_id) {
    case PROP_BACKGROUND_DRESS:
        g_value_set_int(value, data->background_dress);
        break;
    case PROP_FRAME_DRESS:
        g_value_set_int(value, data->frame_dress);
        break;
    case PROP_TOP_MARGIN:
        g_value_set_double(value, data->top_margin);
        break;
    case PROP_RIGHT_MARGIN:
        g_value_set_double(value, data->right_margin);
        break;
    case PROP_BOTTOM_MARGIN:
        g_value_set_double(value, data->bottom_margin);
        break;
    case PROP_LEFT_MARGIN:
        g_value_set_double(value, data->left_margin);
        break;
    case PROP_HAS_FRAME:
        g_value_set_boolean(value, data->has_frame);
        break;
    case PROP_TOP_PADDING:
        g_value_set_double(value, data->top_padding);
        break;
    case PROP_RIGHT_PADDING:
        g_value_set_double(value, data->right_padding);
        break;
    case PROP_BOTTOM_PADDING:
        g_value_set_double(value, data->bottom_padding);
        break;
    case PROP_LEFT_PADDING:
        g_value_set_double(value, data->left_padding);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void
_adg_set_property(GObject *object, guint prop_id,
                  const GValue *value, GParamSpec *pspec)
{
    AdgCanvas *canvas;
    AdgCanvasPrivate *data;

    canvas = (AdgCanvas *) object;
    data = canvas->data;

    switch (prop_id) {
    case PROP_BACKGROUND_DRESS:
        data->background_dress = g_value_get_int(value);
        break;
    case PROP_FRAME_DRESS:
        data->background_dress = g_value_get_int(value);
        break;
    case PROP_TOP_MARGIN:
        data->top_margin = g_value_get_double(value);
        break;
    case PROP_RIGHT_MARGIN:
        data->right_margin = g_value_get_double(value);
        break;
    case PROP_BOTTOM_MARGIN:
        data->bottom_margin = g_value_get_double(value);
        break;
    case PROP_LEFT_MARGIN:
        data->left_margin = g_value_get_double(value);
        break;
    case PROP_HAS_FRAME:
        data->has_frame = g_value_get_boolean(value);
        break;
    case PROP_TOP_PADDING:
        data->top_padding = g_value_get_double(value);
        break;
    case PROP_RIGHT_PADDING:
        data->right_padding = g_value_get_double(value);
        break;
    case PROP_BOTTOM_PADDING:
        data->bottom_padding = g_value_get_double(value);
        break;
    case PROP_LEFT_PADDING:
        data->left_padding = g_value_get_double(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}


/**
 * adg_canvas_new:
 *
 * Creates a new empty canvas object.
 *
 * Returns: the canvas
 **/
AdgCanvas *
adg_canvas_new(void)
{
    return g_object_new(ADG_TYPE_CANVAS, NULL);
}

/**
 * adg_canvas_set_background_dress:
 * @canvas: an #AdgCanvas
 * @dress: the new #AdgDress to use
 *
 * Sets a new background dress for rendering @canvas: the new
 * dress must be a color dress.
 **/
void
adg_canvas_set_background_dress(AdgCanvas *canvas, AdgDress dress)
{
    g_return_if_fail(ADG_IS_CANVAS(canvas));
    g_object_set((GObject *) canvas, "background-dress", dress, NULL);
}

/**
 * adg_canvas_get_background_dress:
 * @canvas: an #AdgCanvas
 *
 * Gets the background dress to be used in rendering @canvas.
 *
 * Returns: the current background dress
 **/
AdgDress
adg_canvas_get_background_dress(AdgCanvas *canvas)
{
    AdgCanvasPrivate *data;

    g_return_val_if_fail(ADG_IS_CANVAS(canvas), ADG_DRESS_UNDEFINED);

    data = canvas->data;

    return data->background_dress;
}

/**
 * adg_canvas_set_frame_dress:
 * @canvas: an #AdgCanvas
 * @dress: the new #AdgDress to use
 *
 * Sets the #AdgCanvas:frame-dress property of @canvas to @dress:
 * the new dress must be a line dress.
 **/
void
adg_canvas_set_frame_dress(AdgCanvas *canvas, AdgDress dress)
{
    g_return_if_fail(ADG_IS_CANVAS(canvas));
    g_object_set((GObject *) canvas, "frame-dress", dress, NULL);
}

/**
 * adg_canvas_get_frame_dress:
 * @canvas: an #AdgCanvas
 *
 * Gets the frame dress to be used in rendering the border of @canvas.
 *
 * Returns: the current frame dress
 **/
AdgDress
adg_canvas_get_frame_dress(AdgCanvas *canvas)
{
    AdgCanvasPrivate *data;

    g_return_val_if_fail(ADG_IS_CANVAS(canvas), ADG_DRESS_UNDEFINED);

    data = canvas->data;
    return data->frame_dress;
}

/**
 * adg_canvas_set_top_margin:
 * @canvas: an #AdgCanvas
 * @value: the new margin, in identity space
 *
 * Changes the top margin of @canvas by setting #AdgCanvas:top-margin
 * to @value. Negative values are allowed.
 **/
void
adg_canvas_set_top_margin(AdgCanvas *canvas, gdouble value)
{
    g_return_if_fail(ADG_IS_CANVAS(canvas));
    g_object_set((GObject *) canvas, "top-margin", value, NULL);
}

/**
 * adg_canvas_get_top_margin:
 * @canvas: an #AdgCanvas
 *
 * Gets the top margin (in identity space) of @canvas.
 *
 * Returns: the requested margin or %0 on error
 **/
gdouble
adg_canvas_get_top_margin(AdgCanvas *canvas)
{
    AdgCanvasPrivate *data;

    g_return_val_if_fail(ADG_IS_CANVAS(canvas), 0.);

    data = canvas->data;
    return data->top_margin;
}

/**
 * adg_canvas_set_right_margin:
 * @canvas: an #AdgCanvas
 * @value: the new margin, in identity space
 *
 * Changes the right margin of @canvas by setting #AdgCanvas:right-margin
 * to @value. Negative values are allowed.
 **/
void
adg_canvas_set_right_margin(AdgCanvas *canvas, gdouble value)
{
    g_return_if_fail(ADG_IS_CANVAS(canvas));
    g_object_set((GObject *) canvas, "right-margin", value, NULL);
}

/**
 * adg_canvas_get_right_margin:
 * @canvas: an #AdgCanvas
 *
 * Gets the right margin (in identity space) of @canvas.
 *
 * Returns: the requested margin or %0 on error
 **/
gdouble
adg_canvas_get_right_margin(AdgCanvas *canvas)
{
    AdgCanvasPrivate *data;

    g_return_val_if_fail(ADG_IS_CANVAS(canvas), 0.);

    data = canvas->data;
    return data->right_margin;
}


/**
 * adg_canvas_set_bottom_margin:
 * @canvas: an #AdgCanvas
 * @value: the new margin, in identity space
 *
 * Changes the bottom margin of @canvas by setting #AdgCanvas:bottom-margin
 * to @value. Negative values are allowed.
 **/
void
adg_canvas_set_bottom_margin(AdgCanvas *canvas, gdouble value)
{
    g_return_if_fail(ADG_IS_CANVAS(canvas));
    g_object_set((GObject *) canvas, "bottom-margin", value, NULL);
}

/**
 * adg_canvas_get_bottom_margin:
 * @canvas: an #AdgCanvas
 *
 * Gets the bottom margin (in identity space) of @canvas.
 *
 * Returns: the requested margin or %0 on error
 **/
gdouble
adg_canvas_get_bottom_margin(AdgCanvas *canvas)
{
    AdgCanvasPrivate *data;

    g_return_val_if_fail(ADG_IS_CANVAS(canvas), 0.);

    data = canvas->data;
    return data->bottom_margin;
}

/**
 * adg_canvas_set_left_margin:
 * @canvas: an #AdgCanvas
 * @value: the new margin, in identity space
 *
 * Changes the left margin of @canvas by setting #AdgCanvas:left-margin
 * to @value. Negative values are allowed.
 **/
void
adg_canvas_set_left_margin(AdgCanvas *canvas, gdouble value)
{
    g_return_if_fail(ADG_IS_CANVAS(canvas));
    g_object_set((GObject *) canvas, "left-margin", value, NULL);
}

/**
 * adg_canvas_get_left_margin:
 * @canvas: an #AdgCanvas
 *
 * Gets the left margin (in identity space) of @canvas.
 *
 * Returns: the requested margin or %0 on error
 **/
gdouble
adg_canvas_get_left_margin(AdgCanvas *canvas)
{
    AdgCanvasPrivate *data;

    g_return_val_if_fail(ADG_IS_CANVAS(canvas), 0.);

    data = canvas->data;
    return data->left_margin;
}

/**
 * adg_canvas_set_margins:
 * @canvas: an #AdgCanvas
 * @top: top margin, in identity space
 * @right: right margin, in identity space
 * @bottom: bottom margin, in identity space
 * @left: left margin, in identity space
 *
 * Convenient function to set all the margins at once.
 **/
void
adg_canvas_set_margins(AdgCanvas *canvas, gdouble top, gdouble right,
                          gdouble bottom, gdouble left)
{
    g_return_if_fail(ADG_IS_CANVAS(canvas));
    g_object_set((GObject *) canvas,
                 "top-margin", top,
                 "right-margin", right,
                 "bottom-margin", bottom,
                 "left-margin", left,
                 NULL);
}

/**
 * adg_canvas_switch_frame:
 * @canvas: an #AdgCanvas
 * @new_state: the new flag status
 *
 * Sets a new status on the #AdgCanvas:has-frame property: %TRUE
 * means a border around the canvas extents (less the margins)
 * should be rendered.
 **/
void
adg_canvas_switch_frame(AdgCanvas *canvas, gboolean new_state)
{
    g_return_if_fail(ADG_IS_CANVAS(canvas));
    g_object_set((GObject *) canvas, "has-frame", new_state, NULL);
}

/**
 * adg_canvas_has_frame:
 * @canvas: an #AdgCanvas
 *
 * Gets the current status of the #AdgCanvas:has-frame property,
 * that is whether a border around the canvas extents (less the
 * margins) should be rendered (%TRUE) or not (%FALSE).
 *
 * Returns: the current status of the frame flag
 **/
gboolean
adg_canvas_has_frame(AdgCanvas *canvas)
{
    AdgCanvasPrivate *data;

    g_return_val_if_fail(ADG_IS_CANVAS(canvas), FALSE);

    data = canvas->data;
    return data->has_frame;
}

/**
 * adg_canvas_set_top_padding:
 * @canvas: an #AdgCanvas
 * @value: the new padding, in identity space
 *
 * Changes the top padding of @canvas by setting #AdgCanvas:top-padding
 * to @value. Negative values are allowed.
 **/
void
adg_canvas_set_top_padding(AdgCanvas *canvas, gdouble value)
{
    g_return_if_fail(ADG_IS_CANVAS(canvas));
    g_object_set((GObject *) canvas, "top-padding", value, NULL);
}

/**
 * adg_canvas_get_top_padding:
 * @canvas: an #AdgCanvas
 *
 * Gets the top padding (in identity space) of @canvas.
 *
 * Returns: the requested padding or %0 on error
 **/
gdouble
adg_canvas_get_top_padding(AdgCanvas *canvas)
{
    AdgCanvasPrivate *data;

    g_return_val_if_fail(ADG_IS_CANVAS(canvas), 0.);

    data = canvas->data;
    return data->top_padding;
}

/**
 * adg_canvas_set_right_padding:
 * @canvas: an #AdgCanvas
 * @value: the new padding, in identity space
 *
 * Changes the right padding of @canvas by setting #AdgCanvas:right-padding
 * to @value. Negative values are allowed.
 **/
void
adg_canvas_set_right_padding(AdgCanvas *canvas, gdouble value)
{
    g_return_if_fail(ADG_IS_CANVAS(canvas));
    g_object_set((GObject *) canvas, "right-padding", value, NULL);
}

/**
 * adg_canvas_get_right_padding:
 * @canvas: an #AdgCanvas
 *
 * Gets the right padding (in identity space) of @canvas.
 *
 * Returns: the requested padding or %0 on error
 **/
gdouble
adg_canvas_get_right_padding(AdgCanvas *canvas)
{
    AdgCanvasPrivate *data;

    g_return_val_if_fail(ADG_IS_CANVAS(canvas), 0.);

    data = canvas->data;
    return data->right_padding;
}


/**
 * adg_canvas_set_bottom_padding:
 * @canvas: an #AdgCanvas
 * @value: the new padding, in identity space
 *
 * Changes the bottom padding of @canvas by setting #AdgCanvas:bottom-padding
 * to @value. Negative values are allowed.
 **/
void
adg_canvas_set_bottom_padding(AdgCanvas *canvas, gdouble value)
{
    g_return_if_fail(ADG_IS_CANVAS(canvas));
    g_object_set((GObject *) canvas, "bottom-padding", value, NULL);
}

/**
 * adg_canvas_get_bottom_padding:
 * @canvas: an #AdgCanvas
 *
 * Gets the bottom padding (in identity space) of @canvas.
 *
 * Returns: the requested padding or %0 on error
 **/
gdouble
adg_canvas_get_bottom_padding(AdgCanvas *canvas)
{
    AdgCanvasPrivate *data;

    g_return_val_if_fail(ADG_IS_CANVAS(canvas), 0.);

    data = canvas->data;
    return data->bottom_padding;
}

/**
 * adg_canvas_set_left_padding:
 * @canvas: an #AdgCanvas
 * @value: the new padding, in identity space
 *
 * Changes the left padding of @canvas by setting #AdgCanvas:left-padding
 * to @value. Negative values are allowed.
 **/
void
adg_canvas_set_left_padding(AdgCanvas *canvas, gdouble value)
{
    g_return_if_fail(ADG_IS_CANVAS(canvas));
    g_object_set((GObject *) canvas, "left-padding", value, NULL);
}

/**
 * adg_canvas_get_left_padding:
 * @canvas: an #AdgCanvas
 *
 * Gets the left padding (in identity space) of @canvas.
 *
 * Returns: the requested padding or %0 on error
 **/
gdouble
adg_canvas_get_left_padding(AdgCanvas *canvas)
{
    AdgCanvasPrivate *data;

    g_return_val_if_fail(ADG_IS_CANVAS(canvas), 0.);

    data = canvas->data;
    return data->left_padding;
}

/**
 * adg_canvas_set_paddings:
 * @canvas: an #AdgCanvas
 * @top: top padding, in identity space
 * @right: right padding, in identity space
 * @bottom: bottom padding, in identity space
 * @left: left padding, in identity space
 *
 * Convenient function to set all the paddings at once.
 **/
void
adg_canvas_set_paddings(AdgCanvas *canvas, gdouble top, gdouble right,
                          gdouble bottom, gdouble left)
{
    g_return_if_fail(ADG_IS_CANVAS(canvas));
    g_object_set((GObject *) canvas,
                 "top-padding", top,
                 "right-padding", right,
                 "bottom-padding", bottom,
                 "left-padding", left,
                 NULL);
}


static void
_adg_arrange(AdgEntity *entity)
{
    CpmlExtents extents;

    if (PARENT_ENTITY_CLASS->arrange)
        PARENT_ENTITY_CLASS->arrange(entity);

    cpml_extents_copy(&extents, adg_entity_get_extents(entity));

    if (extents.is_defined) {
        AdgCanvasPrivate *data = ((AdgCanvas *) entity)->data;

        extents.org.x -= data->left_margin + data->left_padding;
        extents.org.y -= data->top_margin + data->top_padding;
        extents.size.x += data->left_margin + data->left_padding;
        extents.size.x += data->right_margin + data->right_padding;
        extents.size.y += data->top_margin + data->top_padding;
        extents.size.y += data->bottom_margin + data->bottom_padding;

        adg_entity_set_extents(entity, &extents);
    }
}

static void
_adg_render(AdgEntity *entity, cairo_t *cr)
{
    AdgCanvasPrivate *data;
    const CpmlExtents *extents;

    data = ((AdgCanvas *) entity)->data;
    extents = adg_entity_get_extents(entity);

    cairo_save(cr);

    /* Background fill */
    cairo_identity_matrix(cr);
    cairo_rectangle(cr, extents->org.x, extents->org.y,
                    extents->size.x, extents->size.y);
    adg_entity_apply_dress(entity, data->background_dress, cr);
    cairo_fill(cr);

    /* Frame line */
    if (data->has_frame) {
        CpmlExtents frame;
        cpml_extents_copy(&frame, extents);

        frame.org.x += data->left_margin;
        frame.org.y += data->top_margin;
        frame.size.x -= data->left_margin + data->right_margin;
        frame.size.y -= data->top_margin + data->bottom_margin;

        cairo_rectangle(cr, frame.org.x, frame.org.y,
                        frame.size.x, frame.size.y);
        cairo_set_matrix(cr, adg_entity_get_global_matrix(entity));
        adg_entity_apply_dress(entity, data->frame_dress, cr);
        cairo_stroke(cr);
    }

    cairo_restore(cr);

    if (PARENT_ENTITY_CLASS->render)
        PARENT_ENTITY_CLASS->render(entity, cr);
}
