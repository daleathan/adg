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
 * SECTION:adg-gtk-area
 * @short_description: A #GtkWidget specifically designed to contain
 *                     an #AdgCanvas entity
 *
 * This is a #GtkDrawingArea derived object that provides an easy way
 * to show an ADG based canvas. The associated canvas can be set
 * directly with the adg_gtk_area_new_with_canvas() constructor
 * function or by using adg_gtk_area_set_canvas().
 *
 * The default minimum size of this widget will depend on the canvas
 * content. The global matrix of the #AdgCanvas will be adjusted to
 * expose the drawing in the proper position. The empty space around
 * the drawing can be changed by setting the margins of the underlying
 * #AdgCanvas object.
 *
 * The default implementation reacts to some mouse events: if you drag
 * the mouse keeping the wheel pressed the canvas will be translated;
 * if the mouse wheel is rotated the canvas will be scaled up or down
 * (accordingly to the wheel direction) by the factor specified in the
 * #AdgGtkArea:factor property.
 **/


/**
 * AdgGtkArea:
 *
 * All fields are private and should not be used directly.
 * Use its public methods instead.
 **/


#include "adg-gtk-internal.h"
#include "adg-gtk-area.h"
#include "adg-gtk-area-private.h"
#include "adg-gtk-marshal.h"

#define _ADG_OLD_OBJECT_CLASS   ((GObjectClass *) adg_gtk_area_parent_class)
#define _ADG_OLD_WIDGET_CLASS   ((GtkWidgetClass *) adg_gtk_area_parent_class)


G_DEFINE_TYPE(AdgGtkArea, adg_gtk_area, GTK_TYPE_DRAWING_AREA);

enum {
    PROP_0,
    PROP_CANVAS,
    PROP_FACTOR,
    PROP_AUTOZOOM
};

enum {
    CANVAS_CHANGED,
    EXTENTS_CHANGED,
    LAST_SIGNAL
};


static void             _adg_dispose            (GObject         *object);
static void             _adg_get_property       (GObject         *object,
                                                 guint            prop_id,
                                                 GValue          *value,
                                                 GParamSpec      *pspec);
static void             _adg_set_property       (GObject         *object,
                                                 guint            prop_id,
                                                 const GValue    *value,
                                                 GParamSpec      *pspec);
static void             _adg_size_request       (GtkWidget       *widget,
                                                 GtkRequisition  *requisition);
static void             _adg_size_allocate      (GtkWidget       *widget,
                                                 GtkAllocation   *allocation);
static gboolean         _adg_expose_event       (GtkWidget       *widget,
                                                 GdkEventExpose  *event);
static gboolean         _adg_scroll_event       (GtkWidget       *widget,
                                                 GdkEventScroll  *event);
static gboolean         _adg_button_press_event (GtkWidget       *widget,
                                                 GdkEventButton  *event);
static gboolean         _adg_motion_notify_event(GtkWidget       *widget,
                                                 GdkEventMotion  *event);
static gboolean         _adg_get_map            (GtkWidget       *widget,
                                                 gboolean         local_space,
                                                 AdgMatrix       *map,
                                                 AdgMatrix       *inverted);
static void             _adg_set_map            (GtkWidget       *widget,
                                                 gboolean         local_space,
                                                 const AdgMatrix *map);
static const CpmlExtents *
                        _adg_get_extents        (AdgGtkArea      *area);

static guint            _adg_signals[LAST_SIGNAL] = { 0 };


static void
adg_gtk_area_class_init(AdgGtkAreaClass *klass)
{
    GObjectClass *gobject_class;
    GtkWidgetClass *widget_class;
    GParamSpec *param;

    gobject_class = (GObjectClass *) klass;
    widget_class = (GtkWidgetClass *) klass;

    g_type_class_add_private(klass, sizeof(AdgGtkAreaPrivate));

    gobject_class->dispose = _adg_dispose;
    gobject_class->get_property = _adg_get_property;
    gobject_class->set_property = _adg_set_property;

    widget_class->size_request = _adg_size_request;
    widget_class->size_allocate = _adg_size_allocate;
    widget_class->expose_event = _adg_expose_event;
    widget_class->scroll_event = _adg_scroll_event;
    widget_class->button_press_event = _adg_button_press_event;
    widget_class->motion_notify_event = _adg_motion_notify_event;

    param = g_param_spec_object("canvas",
                                P_("Canvas"),
                                P_("The canvas to be shown"),
                                ADG_TYPE_CANVAS,
                                G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
    g_object_class_install_property(gobject_class, PROP_CANVAS, param);

    param = g_param_spec_double("factor",
                                P_("Factor"),
                                P_("The factor used in zooming in and out"),
                                1., G_MAXDOUBLE, 1.05,
                                G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_FACTOR, param);

    param = g_param_spec_boolean("autozoom",
                                 P_("Autozoom"),
                                 P_("When enabled, automatically adjust the zoom factor in global space at every size allocation"),
                                 FALSE,
                                 G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_AUTOZOOM, param);

    /**
     * AdgGtkArea::canvas-changed:
     * @area: an #AdgGtkArea
     * @old_canvas: the old #AdgCanvas object
     *
     * Emitted when the #AdgGtkArea has a new canvas. If the new canvas
     * is the same as the old one, the signal is not emitted.
     **/
    _adg_signals[CANVAS_CHANGED] =
        g_signal_new("canvas-changed", ADG_GTK_TYPE_AREA,
                     G_SIGNAL_RUN_LAST|G_SIGNAL_NO_RECURSE,
                     G_STRUCT_OFFSET(AdgGtkAreaClass, canvas_changed),
                     NULL, NULL,
                     adg_gtk_marshal_VOID__OBJECT,
                     G_TYPE_NONE, 1, ADG_TYPE_CANVAS);

    /**
     * AdgGtkArea::extents-changed:
     * @area: an #AdgGtkArea
     * @old_extents: the old #CpmlExtents struct
     *
     * Emitted when the extents of @area have been changed.
     * The old extents are always compared to the new ones,
     * so when the extents are recalculated but the result
     * is the same the signal is not emitted.
     **/
    _adg_signals[EXTENTS_CHANGED] =
        g_signal_new("extents-changed", ADG_GTK_TYPE_AREA,
                     G_SIGNAL_RUN_LAST|G_SIGNAL_NO_RECURSE,
                     G_STRUCT_OFFSET(AdgGtkAreaClass, extents_changed),
                     NULL, NULL,
                     adg_gtk_marshal_VOID__POINTER,
                     G_TYPE_NONE, 1, G_TYPE_POINTER);
}

static void
adg_gtk_area_init(AdgGtkArea *area)
{
    AdgGtkAreaPrivate *data = G_TYPE_INSTANCE_GET_PRIVATE(area,
                                                          ADG_GTK_TYPE_AREA,
                                                          AdgGtkAreaPrivate);

    data->canvas = NULL;
    data->factor = 1.05;
    data->autozoom = FALSE;

    data->x_event = 0;
    data->y_event = 0;
    data->src_factor = 0;

    area->data = data;

    /* Enable GDK events to catch wheel rotation and drag */
    gtk_widget_add_events((GtkWidget *) area,
                          GDK_BUTTON_PRESS_MASK |
                          GDK_BUTTON2_MOTION_MASK |
                          GDK_SCROLL_MASK);
}

static void
_adg_dispose(GObject *object)
{
    AdgGtkAreaPrivate *data = ((AdgGtkArea *) object)->data;

    if (data->canvas) {
        g_object_unref(data->canvas);
        data->canvas = NULL;
    }

    if (_ADG_OLD_OBJECT_CLASS->dispose)
        _ADG_OLD_OBJECT_CLASS->dispose(object);
}

static void
_adg_get_property(GObject *object, guint prop_id,
                  GValue *value, GParamSpec *pspec)
{
    AdgGtkAreaPrivate *data = ((AdgGtkArea *) object)->data;

    switch (prop_id) {
    case PROP_CANVAS:
        g_value_set_object(value, data->canvas);
        break;
    case PROP_FACTOR:
        g_value_set_double(value, data->factor);
        break;
    case PROP_AUTOZOOM:
        g_value_set_boolean(value, data->autozoom);
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
    AdgGtkArea *area;
    AdgGtkAreaPrivate *data;
    AdgCanvas *canvas;

    area = (AdgGtkArea *) object;
    data = area->data;

    switch (prop_id) {
    case PROP_CANVAS:
        canvas = g_value_get_object(value);
        if (canvas)
            g_object_ref(canvas);
        if (data->canvas)
            g_object_unref(data->canvas);
        if (data->canvas != canvas) {
            data->canvas = canvas;
            g_signal_emit(area, _adg_signals[CANVAS_CHANGED], 0);
        }
        break;
    case PROP_FACTOR:
        data->factor = g_value_get_double(value);
        break;
    case PROP_AUTOZOOM:
        data->autozoom = g_value_get_boolean(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}


/**
 * adg_gtk_area_new:
 *
 * Creates a new empty #AdgGtkArea. The widget is useful only after
 * an #AdgCanvas has been added either using the #AdgGtkArea:canvas
 * property or with adg_gtk_area_set_canvas().
 *
 * Returns: the newly created widget
 **/
GtkWidget *
adg_gtk_area_new(void)
{
    return g_object_new(ADG_GTK_TYPE_AREA, NULL);
}

/**
 * adg_gtk_area_new_with_canvas:
 * @canvas: the #AdgCanvas shown by this widget
 *
 * Creates a new #AdgGtkArea and sets the #AdgGtkArea:canvas property
 * to @canvas.
 *
 * Returns: the newly created widget
 **/
GtkWidget *
adg_gtk_area_new_with_canvas(AdgCanvas *canvas)
{
    g_return_val_if_fail(ADG_IS_CANVAS(canvas), NULL);

    return g_object_new(ADG_GTK_TYPE_AREA, "canvas", canvas, NULL);
}

/**
 * adg_gtk_area_set_canvas:
 * @area: an #AdgGtkArea
 * @canvas: the new #AdgCanvas
 *
 * Sets a new canvas on @area. The old canvas, if presents, is
 * unreferenced.
 **/
void
adg_gtk_area_set_canvas(AdgGtkArea *area, AdgCanvas *canvas)
{
    g_return_if_fail(ADG_GTK_IS_AREA(area));
    g_object_set(area, "canvas", canvas, NULL);
}

/**
 * adg_gtk_area_get_canvas:
 * @area: an #AdgGtkArea
 *
 * Gets the canvas associated to @area.
 *
 * Returns: the requested #AdgCanvas object or %NULL on errors
 **/
AdgCanvas *
adg_gtk_area_get_canvas(AdgGtkArea *area)
{
    AdgGtkAreaPrivate *data;

    g_return_val_if_fail(ADG_GTK_IS_AREA(area), NULL);

    data = area->data;

    return data->canvas;
}

/**
 * adg_gtk_area_get_extents:
 * @area: an #AdgGtkArea
 *
 * Gets the extents of the canvas bound to @area. The returned
 * struct is owned by @area and should not modified or freed.
 * This is a convenient function that gets the extents with
 * adg_entity_get_extents() and add to it the margins of the
 * canvas .
 *
 * If @area still does not have any canvas associated to it or
 * the canvas is invalid or empty, %NULL is returned.
 *
 * The canvas will be updated, meaning the adg_entity_arrange()
 * will be called before the extents computation.
 *
 * Returns: the extents of the @area canvas or %NULL on errors
 **/
const CpmlExtents *
adg_gtk_area_get_extents(AdgGtkArea *area)
{
    g_return_val_if_fail(ADG_GTK_IS_AREA(area), NULL);

    return _adg_get_extents(area);
}

/**
 * adg_gtk_area_set_factor:
 * @area: an #AdgGtkArea
 * @factor: the new zoom factor
 *
 * Sets a new zoom factor to @area. If the factor is less than
 * 1, it will be clamped to 1.
 **/
void
adg_gtk_area_set_factor(AdgGtkArea *area, gdouble factor)
{
    g_return_if_fail(ADG_GTK_IS_AREA(area));
    g_object_set(area, "factor", factor, NULL);
}

/**
 * adg_gtk_area_get_factor:
 * @area: an #AdgGtkArea
 *
 * Gets the zoom factor associated to @area. The zoom factor is
 * directly used to zoom in (that is, the default zoom factor of
 * 1.05 will zoom of 5% every iteration) and it is reversed while
 * zooming out (that is, the default factor will use 1/1.05).
 *
 * Returns: the requested zoom factor or 0 on error
 **/
gdouble
adg_gtk_area_get_factor(AdgGtkArea *area)
{
    AdgGtkAreaPrivate *data;

    g_return_val_if_fail(ADG_GTK_IS_AREA(area), 0.);

    data = area->data;
    return data->factor;
}

/**
 * adg_gtk_area_switch_autozoom:
 * @area: an #AdgGtkArea
 * @state: the new autozoom state
 *
 * Sets the #AdgGtkArea:autozoom property of @area to @state. When the
 * autozoom feature is enabled, @area reacts to any size allocation
 * by adjusting its zoom factor in global space. This means the
 * drawing will fill the available space (keeping its aspect ratio)
 * when resizing the window.
 **/
void
adg_gtk_area_switch_autozoom(AdgGtkArea *area, gboolean state)
{
    g_return_if_fail(ADG_GTK_IS_AREA(area));
    g_object_set(area, "autozoom", state, NULL);
}

/**
 * adg_gtk_area_has_autozoom:
 * @area: an #AdgGtkArea
 *
 * Gets the current state of the #AdgGtkArea:autozoom property on
 * the @area object.
 *
 * Returns: the current autozoom state
 **/
gboolean
adg_gtk_area_has_autozoom(AdgGtkArea *area)
{
    AdgGtkAreaPrivate *data;

    g_return_val_if_fail(ADG_GTK_IS_AREA(area), FALSE);

    data = area->data;
    return data->autozoom;
}

/**
 * adg_gtk_area_canvas_changed:
 * @area: an #AdgGtkArea
 * @canvas: the old canvas bound to @area
 *
 * Emits the #AdgGtkArea::canvas-changed signal on @area.
 **/
void
adg_gtk_area_canvas_changed(AdgGtkArea *area, AdgCanvas *old_canvas)
{
    g_return_if_fail(ADG_GTK_IS_AREA(area));

    g_signal_emit(area, _adg_signals[CANVAS_CHANGED], 0, old_canvas);
}

/**
 * adg_gtk_area_extents_changed:
 * @area: an #AdgGtkArea
 * @old_extents: the old extents of @area
 *
 * Emits the #AdgGtkArea::extents-changed signal on @area.
 **/
void
adg_gtk_area_extents_changed(AdgGtkArea *area, const CpmlExtents *old_extents)
{
    g_return_if_fail(ADG_GTK_IS_AREA(area));

    g_signal_emit(area, _adg_signals[EXTENTS_CHANGED], 0, old_extents);
}


static void
_adg_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
    const CpmlExtents *extents = _adg_get_extents((AdgGtkArea *) widget);

    if (extents != NULL && extents->is_defined) {
        requisition->width = extents->size.x;
        requisition->height = extents->size.y;
    }
}

/**
 * _adg_size_allocate:
 * @widget: an #AdgGtkArea widget
 * @allocation: the new allocation struct
 *
 * Scales the drawing according to the new allocation.
 *
 * TODO: the current implementation keeps the point in the center
 * of the #AdgCanvas at the center of the new #AdgGtkArea. This is
 * not what a user would likely expect because all the eventual
 * translations in global space will be lost. In other words the
 * resulting drawing is always centered also when the original
 * drawing (that is, the drawing before the new size allocation)
 * was not.
 **/
static void
_adg_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
    AdgGtkArea *area;
    AdgGtkAreaPrivate *data;
    AdgCanvas *canvas;
    AdgEntity *entity;
    GtkAllocation *src_allocation;
    AdgPair ratio;
    gdouble factor;
    AdgMatrix map;

    if (_ADG_OLD_WIDGET_CLASS->size_allocate)
        _ADG_OLD_WIDGET_CLASS->size_allocate(widget, allocation);

    area = (AdgGtkArea *) widget;
    data = area->data;

    if (!data->autozoom)
        return;

    canvas = data->canvas;

    if (canvas == NULL)
        return;

    /* Check if the allocated space is enough:
     * if not, there is not much we can do... */
    g_return_if_fail(allocation->width > 0 && allocation->height > 0);

    entity = (AdgEntity *) canvas;
    src_allocation = &data->src_allocation;

    adg_matrix_copy(&map, adg_entity_get_global_map(entity));

    if (data->src_factor <= 0) {
        /* First allocation */
        const CpmlExtents *extents = _adg_get_extents(area);

        if (extents == NULL || !extents->is_defined)
            return;

        if (extents->size.x <= 0 || extents->size.y <= 0)
            return;

        ratio.x = (gdouble) allocation->width / extents->size.x;
        ratio.y = (gdouble) allocation->height / extents->size.y;
        factor = MIN(ratio.x, ratio.y);

        /* TODO: check the following formula, especially
         * the final translation */
        map.x0 = allocation->width - extents->size.x * factor;
        map.y0 = allocation->height - extents->size.y * factor;
        map.x0 /= 2;
        map.y0 /= 2;
        map.x0 -= extents->org.x;
        map.y0 -= extents->org.y;

        *src_allocation = *allocation;
        src_allocation->x = map.x0;
        src_allocation->y = map.y0;
        data->src_factor = factor;
    } else {
        /* Scaling with reference to the first allocation */
        ratio.x = data->src_factor * allocation->width / src_allocation->width;
        ratio.y = data->src_factor * allocation->height / src_allocation->height;

        factor = MIN(ratio.x, ratio.y);

        map.x0 = src_allocation->x * factor +
            (allocation->width - src_allocation->width * factor) / 2;
        map.y0 = src_allocation->y * factor +
            (allocation->height - src_allocation->height * factor) / 2;
    }

    map.xx = map.yy = factor;
    adg_entity_set_global_map(entity, &map);
}

static gboolean
_adg_expose_event(GtkWidget *widget, GdkEventExpose *event)
{
    AdgGtkAreaPrivate *data;
    AdgCanvas *canvas;

    data = ((AdgGtkArea *) widget)->data;
    canvas = data->canvas;

    if (canvas && event->window) {
        cairo_t *cr = gdk_cairo_create(event->window);
        adg_entity_render((AdgEntity *) canvas, cr);
        cairo_destroy(cr);
    }

    if (_ADG_OLD_WIDGET_CLASS->expose_event == NULL)
        return FALSE;

    return _ADG_OLD_WIDGET_CLASS->expose_event(widget, event);
}

static gboolean
_adg_scroll_event(GtkWidget *widget, GdkEventScroll *event)
{
    gboolean zoom_in, zoom_out, local_space, global_space;
    AdgMatrix map, inverted;
    AdgGtkAreaPrivate *data;
    double factor, x, y;

    zoom_in = event->direction == GDK_SCROLL_UP;
    zoom_out = event->direction == GDK_SCROLL_DOWN;
    local_space = (event->state & ADG_GTK_MODIFIERS) == 0;
    global_space = (event->state & ADG_GTK_MODIFIERS) == GDK_SHIFT_MASK;

    if ((zoom_in || zoom_out) && (local_space || global_space) &&
        _adg_get_map(widget, local_space, &map, &inverted)) {
        data = ((AdgGtkArea *) widget)->data;
        factor = zoom_in ? data->factor : 1. / data->factor;
        x = event->x;
        y = event->y;

        cairo_matrix_transform_point(&inverted, &x, &y);
        cairo_matrix_scale(&map, factor, factor);
        cairo_matrix_translate(&map, x/factor - x, y/factor - y);

        _adg_set_map(widget, local_space, &map);

        gtk_widget_queue_draw(widget);
    }

    if (_ADG_OLD_WIDGET_CLASS->scroll_event == NULL)
        return FALSE;

    return _ADG_OLD_WIDGET_CLASS->scroll_event(widget, event);
}

static gboolean
_adg_button_press_event(GtkWidget *widget, GdkEventButton *event)
{
    AdgGtkAreaPrivate *data = ((AdgGtkArea *) widget)->data;

    if (event->type == GDK_BUTTON_PRESS && event->button == 2) {
        /* Remember the starting coordinates of a (probable) translation */
        data->x_event = event->x;
        data->y_event = event->y;
    }

    if (_ADG_OLD_WIDGET_CLASS->button_press_event == NULL)
        return FALSE;

    return _ADG_OLD_WIDGET_CLASS->button_press_event(widget, event);
}

static gboolean
_adg_motion_notify_event(GtkWidget *widget, GdkEventMotion *event)
{
    gboolean translating, local_space, global_space;
    AdgMatrix map, inverted;
    AdgGtkAreaPrivate *data;
    double x, y;

    translating = (event->state & GDK_BUTTON2_MASK) == GDK_BUTTON2_MASK;
    local_space = (event->state & ADG_GTK_MODIFIERS) == 0;
    global_space = (event->state & ADG_GTK_MODIFIERS) == GDK_SHIFT_MASK;

    if (translating && (local_space || global_space) &&
        _adg_get_map(widget, local_space, &map, &inverted)) {
        data = ((AdgGtkArea *) widget)->data;
        x = event->x - data->x_event;
        y = event->y - data->y_event;

        cairo_matrix_transform_distance(&inverted, &x, &y);
        cairo_matrix_translate(&map, x, y);
        data->x_event = event->x;
        data->y_event = event->y;

        _adg_set_map(widget, local_space, &map);

        gtk_widget_queue_draw(widget);
    }

    if (_ADG_OLD_WIDGET_CLASS->motion_notify_event == NULL)
        return FALSE;

    return _ADG_OLD_WIDGET_CLASS->motion_notify_event(widget, event);
}

static gboolean
_adg_get_map(GtkWidget *widget, gboolean local_space,
             AdgMatrix *map, AdgMatrix *inverted)
{
    AdgGtkAreaPrivate *data;
    AdgEntity *entity;

    data = ((AdgGtkArea *) widget)->data;
    entity = (AdgEntity *) data->canvas;
    if (entity == NULL)
        return FALSE;

    if (local_space) {
        adg_matrix_copy(map, adg_entity_get_local_map(entity));

        /* The inverted map is subject to the global matrix */
        adg_matrix_copy(inverted, adg_entity_get_global_matrix(entity));
        adg_matrix_transform(inverted, map, ADG_TRANSFORM_BEFORE);
    } else {
        adg_matrix_copy(map, adg_entity_get_global_map(entity));
        adg_matrix_copy(inverted, map);
    }

    return cairo_matrix_invert(inverted) == CAIRO_STATUS_SUCCESS;
}

static void
_adg_set_map(GtkWidget *widget, gboolean local_space, const AdgMatrix *map)
{
    AdgGtkAreaPrivate *data;
    AdgEntity *entity;

    data = ((AdgGtkArea *) widget)->data;
    entity = (AdgEntity *) data->canvas;

    if (entity == NULL)
        return;

    if (local_space)
        adg_entity_set_local_map(entity, map);
    else
        adg_entity_set_global_map(entity, map);

    _adg_get_extents((AdgGtkArea *) widget);
}

static const CpmlExtents *
_adg_get_extents(AdgGtkArea *area)
{
    AdgGtkAreaPrivate *data;
    AdgCanvas *canvas;
    CpmlExtents old_extents;
    gdouble top, right, bottom, left;

    data = area->data;
    old_extents = data->extents;
    data->extents.is_defined = FALSE;

    canvas = data->canvas;

    if (ADG_IS_CANVAS(canvas)) {
        const CpmlExtents *extents;
        AdgEntity *entity;

        entity = (AdgEntity *) canvas;

        adg_entity_arrange(entity);

        extents = adg_entity_get_extents(entity);
        if (extents != NULL && extents->is_defined) {
            top = adg_canvas_get_top_margin(canvas);
            right = adg_canvas_get_right_margin(canvas);
            bottom = adg_canvas_get_bottom_margin(canvas);
            left = adg_canvas_get_left_margin(canvas);

            data->extents = *extents;
            data->extents.org.x -= left;
            data->extents.org.y -= top;
            data->extents.size.x += left + right;
            data->extents.size.y += top + bottom;
            data->extents.is_defined = TRUE;
        }
    }

    if (!cpml_extents_equal(&data->extents, &old_extents))
        g_signal_emit(area, _adg_signals[EXTENTS_CHANGED], 0, &old_extents);

    return &data->extents;
}
