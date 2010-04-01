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
 * SECTION:adg-widget
 * @short_description: A #GtkWidget specifically designed to contain
 *                     an #AdgCanvas entity
 *
 * This is a #GtkDrawingArea derived object that provides an easy way
 * to show an ADG based canvas. Its default implementation reacts to
 * some mouse events: if you drag the mouse by keepeng the wheel pressed
 * the canvas is translated by translating its local map; if the mouse
 * wheel is rotated, the local map of the canvas is scaled up or down
 * (accordling to the wheel direction) by a factor specified in the
 * #AdgWidget:factor property.
 **/

/**
 * AdgWidget:
 *
 * All fields are private and should not be used directly.
 * Use its public methods instead.
 **/


#include <adg/adg-internal.h>
#include <adg/adg-canvas.h>
#include <adg/adg-marshal.h>
#include "adg-widget.h"
#include "adg-widget-private.h"

#define PARENT_WIDGET_CLASS  ((GtkWidgetClass *) adg_widget_parent_class)


enum {
    PROP_0,
    PROP_CANVAS,
    PROP_FACTOR
};

enum {
    CANVAS_CHANGED,
    LAST_SIGNAL
};


static void             dispose                 (GObject         *object);
static void             get_property            (GObject         *object,
                                                 guint            prop_id,
                                                 GValue          *value,
                                                 GParamSpec      *pspec);
static void             set_property            (GObject         *object,
                                                 guint            prop_id,
                                                 const GValue    *value,
                                                 GParamSpec      *pspec);
static gboolean         set_canvas              (AdgWidget       *widget,
                                                 AdgCanvas       *canvas);
static gboolean         set_factor              (AdgWidget       *widget,
                                                 gdouble          factor);
static gboolean         expose_event            (GtkWidget       *widget,
                                                 GdkEventExpose  *event);
static gboolean         scroll_event            (GtkWidget       *widget,
                                                 GdkEventScroll  *event);
static gboolean         button_press_event      (GtkWidget       *widget,
                                                 GdkEventButton  *event);
static gboolean         motion_notify_event     (GtkWidget       *widget,
                                                 GdkEventMotion  *event);
static gboolean         get_local_map           (GtkWidget       *widget,
                                                 AdgMatrix       *map,
                                                 AdgMatrix       *inverted);
static void             set_local_map           (GtkWidget       *widget,
                                                 const AdgMatrix *map);

static guint signals[LAST_SIGNAL] = { 0 };


G_DEFINE_TYPE(AdgWidget, adg_widget, GTK_TYPE_DRAWING_AREA);


static void
adg_widget_class_init(AdgWidgetClass *klass)
{
    GObjectClass *gobject_class;
    GtkWidgetClass *widget_class;
    GParamSpec *param;

    gobject_class = (GObjectClass *) klass;
    widget_class = (GtkWidgetClass *) klass;

    g_type_class_add_private(klass, sizeof(AdgWidgetPrivate));

    gobject_class->dispose = dispose;
    gobject_class->get_property = get_property;
    gobject_class->set_property = set_property;

    widget_class->expose_event = expose_event;
    widget_class->scroll_event = scroll_event;
    widget_class->button_press_event = button_press_event;
    widget_class->motion_notify_event = motion_notify_event;

    param = g_param_spec_object("canvas",
                                P_("Canvas"),
                                P_("The canvas to be shown by this widget"),
                                ADG_TYPE_CANVAS,
                                G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
    g_object_class_install_property(gobject_class, PROP_CANVAS, param);

    param = g_param_spec_double("factor",
                                P_("Factor"),
                                P_("The factor used in zooming in and out"),
                                1., G_MAXDOUBLE, 1.05,
                                G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_FACTOR, param);

    /**
     * AdgWidget::canvas-changed:
     * @widget: an #AdgWidget
     *
     * Emitted when the widget has a new canvas.
     **/
    signals[CANVAS_CHANGED] = g_signal_new("canvas-changed", ADG_TYPE_WIDGET,
                                           G_SIGNAL_RUN_LAST|G_SIGNAL_NO_RECURSE,
                                           G_STRUCT_OFFSET(AdgWidgetClass, canvas_changed),
                                           NULL, NULL,
                                           adg_marshal_VOID__VOID,
                                           G_TYPE_NONE, 0);
}

static void
adg_widget_init(AdgWidget *widget)
{
    AdgWidgetPrivate *data = G_TYPE_INSTANCE_GET_PRIVATE(widget,
                                                         ADG_TYPE_WIDGET,
                                                         AdgWidgetPrivate);

    data->canvas = NULL;
    data->factor = 1.05;
    data->x_event = 0;
    data->y_event = 0;

    widget->data = data;

    /* Enable GDK events to catch wheel rotation and drag */
    gtk_widget_add_events((GtkWidget *) widget,
                          GDK_BUTTON_PRESS_MASK|
                          GDK_BUTTON2_MOTION_MASK|
                          GDK_SCROLL_MASK);
}

static void
dispose(GObject *object)
{
    AdgWidgetPrivate *data = ((AdgWidget *) object)->data;

    if (data->canvas != NULL) {
        g_object_unref(data->canvas);
        data->canvas = NULL;
    }

    G_OBJECT_CLASS(adg_widget_parent_class)->dispose(object);
}

static void
get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
    AdgWidgetPrivate *data = ((AdgWidget *) object)->data;

    switch (prop_id) {
    case PROP_CANVAS:
        g_value_set_object(value, data->canvas);
        break;
    case PROP_FACTOR:
        g_value_set_double(value, data->factor);
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
    AdgWidget *widget = (AdgWidget *) object;

    switch (prop_id) {
    case PROP_CANVAS:
        set_canvas(widget, g_value_get_object(value));
        break;
    case PROP_FACTOR:
        set_factor(widget, g_value_get_double(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}


/**
 * adg_widget_new:
 *
 * Creates a new empty #AdgWidget. The widget is useful only after
 * an #AdgCanvas has been added either using the #AdgWidget:canvas
 * property or with adg_widget_set_canvas().
 *
 * Returns: the newly created widget
 **/
GtkWidget *
adg_widget_new(void)
{
    return g_object_new(ADG_TYPE_WIDGET, NULL);
}

/**
 * adg_widget_new_with_canvas:
 * @canvas: the #AdgCanvas shown by this widget
 *
 * Creates a new #AdgWidget and sets the #AdgWidget:canvas property
 * to @canvas.
 *
 * Returns: the newly created widget
 **/
GtkWidget *
adg_widget_new_with_canvas(AdgCanvas *canvas)
{
    g_return_val_if_fail(ADG_IS_CANVAS(canvas), NULL);

    return g_object_new(ADG_TYPE_WIDGET, "canvas", canvas, NULL);
}

/**
 * adg_widget_set_canvas:
 * @widget: an #AdgWidget
 * @canvas: the new #AdgCanvas
 *
 * Sets a new canvas on @widget. The old canvas, if presents, is
 * unreferenced.
 **/
void
adg_widget_set_canvas(AdgWidget *widget, AdgCanvas *canvas)
{
    g_return_if_fail(ADG_IS_WIDGET(widget));

    if (set_canvas(widget, canvas))
        g_object_notify((GObject *) widget, "canvas");
}

/**
 * adg_widget_get_canvas:
 * @widget: an #AdgWidget
 *
 * Gets the canvas associated to @widget.
 *
 * Returns: the requested #AdgCanvas object or %NULL on errors
 **/
AdgCanvas *
adg_widget_get_canvas(AdgWidget *widget)
{
    AdgWidgetPrivate *data;

    g_return_val_if_fail(ADG_IS_WIDGET(widget), NULL);

    data = widget->data;

    return data->canvas;
}

/**
 * adg_widget_set_factor:
 * @widget: an #AdgWidget
 * @factor: the new zoom factor
 *
 * Sets a new zoom factor to @widget. If the factor is less than
 * 1, it will be clamped to 1.
 **/
void
adg_widget_set_factor(AdgWidget *widget, gdouble factor)
{
    g_return_if_fail(ADG_IS_WIDGET(widget));

    if (set_factor(widget, factor))
        g_object_notify((GObject *) widget, "factor");
}

/**
 * adg_widget_get_factor:
 * @widget: an #AdgWidget
 *
 * Gets the zoom factor associated to @widget. The zoom factor is
 * directly used to zoom in (that is, the default zoom factor of
 * 1.05 will zoom of 5% every iteration) and it is reversed while
 * zooming out (that is, the default factor will use 1/1.05).
 *
 * Returns: the requested zoom factor or 0 on error
 **/
gdouble
adg_widget_get_factor(AdgWidget *widget)
{
    AdgWidgetPrivate *data;

    g_return_val_if_fail(ADG_IS_WIDGET(widget), 0.);

    data = widget->data;

    return data->factor;
}


static gboolean
set_canvas(AdgWidget *widget, AdgCanvas *canvas)
{
    AdgWidgetPrivate *data;

    g_return_val_if_fail(canvas == NULL || ADG_IS_CANVAS(canvas), FALSE);

    data = widget->data;

    if (data->canvas == canvas)
        return FALSE;

    if (canvas)
        g_object_ref(canvas);

    if (data->canvas)
        g_object_unref(data->canvas);

    data->canvas = canvas;
    g_signal_emit(widget, signals[CANVAS_CHANGED], 0);
    return TRUE;
}

static gboolean
set_factor(AdgWidget *widget, gdouble factor)
{
    AdgWidgetPrivate *data;

    /* A better approach would be to use the GParamSpec of this property */
    g_return_val_if_fail(factor >= 1, FALSE);

    data = widget->data;

    if (data->factor == factor)
        return FALSE;

    data->factor = factor;
    return TRUE;
}

static gboolean
expose_event(GtkWidget *widget, GdkEventExpose *event)
{
    AdgWidgetPrivate *data;
    AdgCanvas *canvas;

    data = ((AdgWidget *) widget)->data;
    canvas = data->canvas;

    if (canvas != NULL) {
        cairo_t *cr = gdk_cairo_create(widget->window);
        adg_entity_render((AdgEntity *) canvas, cr);
        cairo_destroy(cr);
    }

    if (PARENT_WIDGET_CLASS->expose_event == NULL)
        return FALSE;

    return PARENT_WIDGET_CLASS->expose_event(widget, event);
}

static gboolean
scroll_event(GtkWidget *widget, GdkEventScroll *event)
{
    AdgWidgetPrivate *data;
    AdgMatrix map, inverted;

    data = ((AdgWidget *) widget)->data;

    if ((event->direction == GDK_SCROLL_UP ||
         event->direction == GDK_SCROLL_DOWN) &&
        get_local_map(widget, &map, &inverted)) {
        double factor, x, y;

        if (event->direction == GDK_SCROLL_UP) {
            factor = data->factor;
        } else {
            factor = 1. / data->factor;
        }

        x = event->x;
        y = event->y;

        cairo_matrix_transform_point(&inverted, &x, &y);

        cairo_matrix_scale(&map, factor, factor);
        cairo_matrix_translate(&map, x/factor - x, y/factor - y);

        set_local_map(widget, &map);

        gtk_widget_queue_draw(widget);
    }

    if (PARENT_WIDGET_CLASS->scroll_event == NULL)
        return FALSE;

    return PARENT_WIDGET_CLASS->scroll_event(widget, event);
}

static gboolean
button_press_event(GtkWidget *widget, GdkEventButton *event)
{
    AdgWidgetPrivate *data = ((AdgWidget *) widget)->data;

    if (event->type == GDK_BUTTON_PRESS && event->button == 2) {
        data->x_event = event->x;
        data->y_event = event->y;
    }

    if (PARENT_WIDGET_CLASS->button_press_event == NULL)
        return FALSE;

    return PARENT_WIDGET_CLASS->button_press_event(widget, event);
}

static gboolean
motion_notify_event(GtkWidget *widget, GdkEventMotion *event)
{
    AdgWidgetPrivate *data;
    AdgMatrix map, inverted;

    data = ((AdgWidget *) widget)->data;

    if ((event->state & GDK_BUTTON2_MASK) > 0 &&
        get_local_map(widget, &map, &inverted)) {
        double x, y;

        x = event->x - data->x_event;
        y = event->y - data->y_event;

        cairo_matrix_transform_distance(&inverted, &x, &y);
        cairo_matrix_translate(&map, x, y);
        data->x_event = event->x;
        data->y_event = event->y;

        set_local_map(widget, &map);

        gtk_widget_queue_draw(widget);
    }

    if (PARENT_WIDGET_CLASS->motion_notify_event == NULL)
        return FALSE;

    return PARENT_WIDGET_CLASS->motion_notify_event(widget, event);
}

static gboolean
get_local_map(GtkWidget *widget, AdgMatrix *map, AdgMatrix *inverted)
{
    AdgWidgetPrivate *data;
    AdgCanvas *canvas;

    data = ((AdgWidget *) widget)->data;
    canvas = data->canvas;
    if (canvas == NULL)
        return FALSE;

    adg_matrix_copy(map, adg_entity_get_local_map((AdgEntity *) canvas));
    adg_matrix_copy(inverted, map);

    return cairo_matrix_invert(inverted) == CAIRO_STATUS_SUCCESS;
}

static void
set_local_map(GtkWidget *widget, const AdgMatrix *map)
{
    AdgWidgetPrivate *data;
    AdgCanvas *canvas;

    data = ((AdgWidget *) widget)->data;
    canvas = data->canvas;

    if (canvas != NULL)
        adg_entity_set_local_map((AdgEntity *) canvas, map);
}