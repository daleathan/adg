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
 * SECTION:widget
 * @title: AdgWidget
 * @short_description: A #GtkWidget specifically designed to contain
 *                     an #AdgCanvas entity
 **/

#include "adg-widget.h"
#include "adg-widget-private.h"
#include "adg-intl.h"


enum {
    PROP_0,
    PROP_CANVAS
};

enum {
    CANVAS_CHANGED,
    LAST_SIGNAL
};


static void             dispose                 (GObject        *object);
static void             get_property            (GObject        *object,
                                                 guint           prop_id,
                                                 GValue         *value,
                                                 GParamSpec     *pspec);
static void             set_property            (GObject        *object,
                                                 guint           prop_id,
                                                 const GValue   *value,
                                                 GParamSpec     *pspec);
static void             set_canvas              (AdgWidget      *widget,
                                                 AdgCanvas      *canvas);
static gboolean         expose_event            (GtkWidget      *widget,
                                                 GdkEventExpose *event);
static gboolean         scroll_event            (GtkWidget      *widget,
                                                 GdkEventScroll *event);
static gboolean         button_press_event      (GtkWidget      *widget,
                                                 GdkEventButton *event);
static gboolean         motion_notify_event     (GtkWidget      *widget,
                                                 GdkEventMotion *event);
static gboolean         get_transformation      (GtkWidget      *widget,
                                                 AdgMatrix      *model,
                                                 AdgMatrix      *inverted);
static void             set_transformation      (GtkWidget      *widget,
                                                 const AdgMatrix *model);

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
                                G_PARAM_CONSTRUCT|G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_CANVAS, param);

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
                                           g_cclosure_marshal_VOID__VOID,
                                           G_TYPE_NONE, 0);
}

static void
adg_widget_init(AdgWidget *widget)
{
    AdgWidgetPrivate *priv = G_TYPE_INSTANCE_GET_PRIVATE(widget,
                                                         ADG_TYPE_WIDGET,
                                                         AdgWidgetPrivate);

    priv->canvas = NULL;
    priv->x_event = 0;
    priv->y_event = 0;

    widget->priv = priv;

    /* Enable GDK events to catch wheel rotation and drag */
    gtk_widget_add_events((GtkWidget *) widget,
                          GDK_BUTTON2_MOTION_MASK|GDK_SCROLL_MASK);
}

static void
dispose(GObject *object)
{
    AdgWidgetPrivate *priv = ((AdgWidget *) object)->priv;

    if (priv->canvas != NULL) {
        g_object_unref(priv->canvas);
        priv->canvas = NULL;
    }

    G_OBJECT_CLASS(adg_widget_parent_class)->dispose(object);
}

static void
get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
    AdgWidget *widget = (AdgWidget *) object;

    switch (prop_id) {

    case PROP_CANVAS:
        g_value_set_object(value, widget->priv->canvas);
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

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}


/**
 * adg_widget_new:
 * @path: the #AdgPath to stroke
 *
 * Creates a new #AdgWidget.
 *
 * Return value: the newly created widget
 **/
GtkWidget *
adg_widget_new(AdgCanvas *canvas)
{
    g_return_val_if_fail(ADG_IS_CANVAS(canvas), NULL);

    return (GtkWidget *) g_object_new(ADG_TYPE_WIDGET, "canvas", canvas, NULL);
}


/**
 * adg_widget_get_canvas:
 * @widget: an #AdgWidget
 *
 * Gets the canvas associated to @widget.
 *
 * Return value: the requested #AdgCanvas object or %NULL on errors
 */
AdgCanvas *
adg_widget_get_canvas(AdgWidget *widget)
{
    g_return_val_if_fail(ADG_IS_WIDGET(widget), NULL);

    return widget->priv->canvas;
}

/**
 * adg_widget_set_canvas:
 * @widget: an #AdgWidget
 * @canvas: the new #AdgCanvas
 *
 * Sets a new canvas on @widget. The old canvas, if presents, is
 * unreferenced.
 */
void
adg_widget_set_canvas(AdgWidget *widget, AdgCanvas *canvas)
{
    g_return_if_fail(ADG_IS_WIDGET(widget));

    set_canvas(widget, canvas);

    g_object_notify((GObject *) widget, "canvas");
}


static void
set_canvas(AdgWidget *widget, AdgCanvas *canvas)
{
    if (widget->priv->canvas != NULL)
        g_object_unref(widget->priv->canvas);

    widget->priv->canvas = canvas;

    if (canvas != NULL)
        g_object_ref(widget->priv->canvas);

    g_signal_emit(widget, signals[CANVAS_CHANGED], 0);
}

static gboolean
expose_event(GtkWidget *widget, GdkEventExpose *event)
{
    AdgCanvas *canvas;
    GtkWidgetClass *parent_class;

    canvas = ((AdgWidget *) widget)->priv->canvas;
    parent_class = (GtkWidgetClass *) adg_widget_parent_class;

    if (canvas != NULL) {
        cairo_t *cr = gdk_cairo_create(widget->window);
        adg_entity_render((AdgEntity *) canvas, cr);
        cairo_destroy(cr);
    }

    if (parent_class->expose_event == NULL)
        return FALSE;

    return parent_class->expose_event(widget, event);
}

static gboolean
scroll_event(GtkWidget *widget, GdkEventScroll *event)
{
    GtkWidgetClass *parent_class;
    AdgWidgetPrivate *priv;
    AdgMatrix model, inverted;

    parent_class = (GtkWidgetClass *) adg_widget_parent_class;
    priv = ((AdgWidget *) widget)->priv;

    if ((event->direction == GDK_SCROLL_UP ||
         event->direction == GDK_SCROLL_DOWN) &&
        get_transformation(widget, &model, &inverted)) {
        double factor, x, y;

        if (event->direction == GDK_SCROLL_UP) {
            factor = 1.05;
        } else {
            factor = 1. / 1.05;
        }

        x = event->x;
        y = event->y;

        cairo_matrix_transform_point(&inverted, &x, &y);

        cairo_matrix_scale(&model, factor, factor);
        cairo_matrix_translate(&model, x/factor - x, y/factor - y);

        set_transformation(widget, &model);

        gtk_widget_queue_draw(widget);
    }

    if (parent_class->scroll_event == NULL)
        return FALSE;

    return parent_class->scroll_event(widget, event);
}

static gboolean
button_press_event(GtkWidget *widget, GdkEventButton *event)
{
    GtkWidgetClass *parent_class;
    AdgWidgetPrivate *priv;

    parent_class = (GtkWidgetClass *) adg_widget_parent_class;
    priv = ((AdgWidget *) widget)->priv;

    if (event->type == GDK_BUTTON_PRESS && event->button == 2) {
        priv->x_event = event->x;
        priv->y_event = event->y;
    }

    if (parent_class->button_press_event == NULL)
        return FALSE;

    return parent_class->button_press_event(widget, event);
}

static gboolean
motion_notify_event(GtkWidget *widget, GdkEventMotion *event)
{
    GtkWidgetClass *parent_class;
    AdgWidgetPrivate *priv;
    AdgMatrix model, inverted;

    parent_class = (GtkWidgetClass *) adg_widget_parent_class;
    priv = ((AdgWidget *) widget)->priv;

    if ((event->state & GDK_BUTTON2_MASK) > 0 &&
        get_transformation(widget, &model, &inverted)) {
        double x, y;

        x = event->x - priv->x_event;
        y = event->y - priv->y_event;

        cairo_matrix_transform_distance(&inverted, &x, &y);
        cairo_matrix_translate(&model, x, y);
        priv->x_event = event->x;
        priv->y_event = event->y;

        set_transformation(widget, &model);

        gtk_widget_queue_draw(widget);
    }

    if (parent_class->motion_notify_event == NULL)
        return FALSE;

    return parent_class->motion_notify_event(widget, event);
}

static gboolean
get_transformation(GtkWidget *widget, AdgMatrix *model, AdgMatrix *inverted)
{
    AdgCanvas *canvas;
    const AdgMatrix *src;

    canvas = ((AdgWidget *) widget)->priv->canvas;
    if (canvas == NULL)
        return FALSE;

    src = adg_container_get_model_transformation((AdgContainer *) canvas);

    adg_matrix_copy(model, src);
    adg_matrix_copy(inverted, src);
    return cairo_matrix_invert(inverted) == CAIRO_STATUS_SUCCESS;
}

static void
set_transformation(GtkWidget *widget, const AdgMatrix *model)
{
    AdgCanvas *canvas = ((AdgWidget *) widget)->priv->canvas;

    if (canvas != NULL)
        adg_container_set_model_transformation((AdgContainer *) canvas, model);
}
