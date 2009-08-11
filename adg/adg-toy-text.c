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
 * SECTION:toy-text
 * @title: AdgToyText
 * @short_description: Simple text entity that use the cairo "toy" text API
 *
 * The #AdgToyText class is a basic class to show simple text. It internally
 * uses the so called cairo "toy" API and it shares the same limitations.
 **/

/**
 * AdgToyText:
 *
 * All fields are privates and should not be used directly.
 * Use its public methods instead.
 **/


#include "adg-toy-text.h"
#include "adg-toy-text-private.h"
#include "adg-translatable.h"
#include "adg-rotable.h"
#include "adg-font-style.h"
#include "adg-intl.h"


enum {
    PROP_0,
    PROP_ORIGIN,
    PROP_ANGLE,
    PROP_LABEL
};

static void     translatable_init       (AdgTranslatableIface *iface);
static void     get_origin              (AdgTranslatable*translatable,
                                         AdgPoint       *dest);
static void     set_origin              (AdgTranslatable*translatable,
                                         const AdgPoint *origin);

static void     rotable_init            (AdgRotableIface*iface);
static gdouble  get_angle               (AdgRotable     *rotable);
static void     set_angle               (AdgRotable     *rotable,
                                         gdouble         angle);

static void     finalize                (GObject        *object);
static void     get_property            (GObject        *object,
                                         guint           param_id,
                                         GValue         *value,
                                         GParamSpec     *pspec);
static void     set_property            (GObject        *object,
                                         guint           param_id,
                                         const GValue   *value,
                                         GParamSpec     *pspec);
static void     model_matrix_changed    (AdgEntity      *entity,
                                         AdgMatrix      *parent_matrix);
static void     invalidate              (AdgEntity      *entity);
static void     render                  (AdgEntity      *entity,
                                         cairo_t        *cr);
static gboolean update_origin_cache     (AdgToyText     *toy_text,
                                         cairo_t        *cr);
static gboolean update_label_cache      (AdgToyText     *toy_text,
                                         cairo_t        *cr);
static void     clear_label_cache       (AdgToyText     *toy_text);
static void     clear_origin_cache      (AdgToyText     *toy_text);


G_DEFINE_TYPE_WITH_CODE(AdgToyText, adg_toy_text, ADG_TYPE_ENTITY,
                        G_IMPLEMENT_INTERFACE(ADG_TYPE_TRANSLATABLE,
                                              translatable_init)
                        G_IMPLEMENT_INTERFACE(ADG_TYPE_ROTABLE, rotable_init))


static void
translatable_init(AdgTranslatableIface *iface)
{
    iface->get_origin = get_origin;
    iface->set_origin = set_origin;
}

static void
get_origin(AdgTranslatable *translatable, AdgPoint *dest)
{
    AdgToyText *toy_text;
    AdgToyTextPrivate *data;

    toy_text = (AdgToyText *) translatable;
    data = toy_text->data;

    adg_point_copy(dest, &data->origin);
}

static void
set_origin(AdgTranslatable *translatable, const AdgPoint *origin)
{
    AdgToyText *toy_text;
    AdgToyTextPrivate *data;

    toy_text = (AdgToyText *) translatable;
    data = toy_text->data;

    adg_point_copy(&data->origin, origin);
}


static void
rotable_init(AdgRotableIface *iface)
{
    iface->get_angle = get_angle;
    iface->set_angle = set_angle;
}

static gdouble
get_angle(AdgRotable *rotable)
{
    AdgToyText *toy_text;
    AdgToyTextPrivate *data;

    toy_text = (AdgToyText *) rotable;
    data = toy_text->data;

    return data->angle;
}

static void
set_angle(AdgRotable *rotable, gdouble angle)
{
    AdgToyText *toy_text;
    AdgToyTextPrivate *data;

    toy_text = (AdgToyText *) rotable;
    data = toy_text->data;

    data->angle = angle;
}


static void
adg_toy_text_class_init(AdgToyTextClass *klass)
{
    GObjectClass *gobject_class;
    AdgEntityClass *entity_class;
    GParamSpec *param;

    gobject_class = (GObjectClass *) klass;
    entity_class = (AdgEntityClass *) klass;

    g_type_class_add_private(klass, sizeof(AdgToyTextPrivate));

    gobject_class->finalize = finalize;
    gobject_class->get_property = get_property;
    gobject_class->set_property = set_property;
 
    entity_class->model_matrix_changed = model_matrix_changed;
    entity_class->invalidate = invalidate;
    entity_class->render = render;

    g_object_class_override_property(gobject_class, PROP_ORIGIN, "origin");
    g_object_class_override_property(gobject_class, PROP_ANGLE, "angle");

    param = g_param_spec_string("label",
                                P_("Label"),
                                P_("The label to display"),
                                NULL, G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_LABEL, param);
}

static void
adg_toy_text_init(AdgToyText *toy_text)
{
    AdgToyTextPrivate *data = G_TYPE_INSTANCE_GET_PRIVATE(toy_text,
                                                          ADG_TYPE_TOY_TEXT,
                                                          AdgToyTextPrivate);

    data->label = NULL;
    adg_point_unset(&data->origin);

    data->origin_cached = FALSE;
    data->glyphs = NULL;

    toy_text->data = data;
}

static void
finalize(GObject *object)
{
    AdgToyText *toy_text;
    AdgToyTextPrivate *data;
    GObjectClass *object_class;

    toy_text = (AdgToyText *) object;
    data = toy_text->data;
    object_class = (GObjectClass *) adg_toy_text_parent_class;

    g_free(data->label);
    clear_label_cache(toy_text);
    clear_origin_cache(toy_text);

    if (object_class->finalize != NULL)
        object_class->finalize(object);
}

static void
get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
    AdgToyTextPrivate *data = ((AdgToyText *) object)->data;

    switch (prop_id) {
    case PROP_ORIGIN:
        g_value_set_boxed(value, &data->origin);
        break;
    case PROP_ANGLE:
        g_value_set_double(value, data->angle);
        break;
    case PROP_LABEL:
        g_value_set_string(value, data->label);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void
set_property(GObject *object, guint prop_id,
             const GValue *value, GParamSpec *pspec)
{
    AdgToyText *toy_text;
    AdgToyTextPrivate *data;

    toy_text = (AdgToyText *) object;
    data = toy_text->data;

    switch (prop_id) {
    case PROP_ORIGIN:
        adg_point_copy(&data->origin, (AdgPoint *) g_value_get_boxed(value));
        clear_origin_cache(toy_text);
        break;
    case PROP_ANGLE:
        data->angle = g_value_get_double(value);
        clear_origin_cache(toy_text);
        break;
    case PROP_LABEL:
        g_free(data->label);
        data->label = g_value_dup_string(value);
        clear_label_cache(toy_text);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}


/**
 * adg_toy_text_new:
 * @label: the label text
 *
 * Creates a new toy text entity using @label as its text
 *
 * Return value: the new entity
 */
AdgEntity *
adg_toy_text_new(const gchar *label)
{
    return (AdgEntity *) g_object_new(ADG_TYPE_TOY_TEXT, "label", label, NULL);
}

/**
 * adg_toy_text_get_label:
 * @toy_text: an #AdgToyText
 *
 * Gets the label text. The string is internally owned and
 * must not be freed or modified.
 *
 * Return value: the label text
 **/
const gchar *
adg_toy_text_get_label(AdgToyText *toy_text)
{
    AdgToyTextPrivate *data;

    g_return_val_if_fail(ADG_IS_TOY_TEXT(toy_text), NULL);

    data = toy_text->data;

    return data->label;
}

/**
 * adg_toy_text_set_label:
 * @toy_text: an #AdgToyText
 * @label: the label text
 *
 * Explicitely sets the text to use as label.
 **/
void
adg_toy_text_set_label(AdgToyText *toy_text, const gchar *label)
{
    AdgToyTextPrivate *data;

    g_return_if_fail(ADG_IS_TOY_TEXT(label));

    data = toy_text->data;
    g_free(data->label);
    data->label = g_strdup(label);

    g_object_notify((GObject *) toy_text, "label");
    clear_label_cache(toy_text);
}


static void
render(AdgEntity *entity, cairo_t *cr)
{
    AdgToyText *toy_text;
    AdgToyTextPrivate *data;
    AdgEntityClass *entity_class;

    toy_text = (AdgToyText *) entity;
    data = toy_text->data;
    entity_class = (AdgEntityClass *) adg_toy_text_parent_class;

    if (data->label) {
        AdgStyle *font_style;

        font_style = adg_entity_get_style(entity, ADG_SLOT_FONT_STYLE);

        cairo_save(cr);
        cairo_set_matrix(cr, adg_entity_get_paper_matrix(entity));
        adg_style_apply(font_style, cr);
        cairo_rotate(cr, data->angle);

        if (!data->glyphs)
            update_label_cache(toy_text, cr);
        if (!data->origin_cached)
            update_origin_cache(toy_text, cr);

        cairo_show_glyphs(cr, data->glyphs, data->num_glyphs);
        cairo_restore(cr);
    }

    if (entity_class->render != NULL)
        entity_class->render(entity, cr);
}

static void
model_matrix_changed(AdgEntity *entity, AdgMatrix *parent_matrix)
{
    AdgEntityClass *entity_class = (AdgEntityClass *) adg_toy_text_parent_class;

    clear_origin_cache((AdgToyText *) entity);

    if (entity_class->model_matrix_changed != NULL)
        entity_class->model_matrix_changed(entity, parent_matrix);
}

static void
invalidate(AdgEntity *entity)
{
    AdgToyText *toy_text;
    AdgEntityClass *entity_class;

    toy_text = (AdgToyText *) entity;
    entity_class = (AdgEntityClass *) adg_toy_text_parent_class;

    clear_label_cache(toy_text);
    clear_origin_cache(toy_text);

    if (entity_class->invalidate != NULL)
        entity_class->invalidate(entity);
}

static gboolean
update_origin_cache(AdgToyText *toy_text, cairo_t *cr)
{
    AdgToyTextPrivate *data;
    AdgPoint point;
    AdgPair *pair;
    cairo_glyph_t *glyph;
    double x, y;
    int cnt;

    data = toy_text->data;
    adg_point_copy(&point, &data->origin);
    pair = &data->origin_pair;
    glyph = data->glyphs;
    cnt = data->num_glyphs;

    /* On undefined label return error */
    if (glyph == NULL || cnt <= 0)
        return FALSE;

    if (data->angle != 0.) {
        /* Following the less surprise rule, also the paper component
         * of the origin point should rotate with the provided angle */
        cairo_matrix_t rotation;
        cairo_matrix_init_rotate(&rotation, data->angle);
        cpml_pair_transform(&point.paper, &rotation);
    }

    adg_entity_point_to_pair((AdgEntity *) toy_text, &point, pair, cr);
    data->origin_cached = TRUE;

    /* Check if the origin is still the same */
    if (pair->x == glyph->x && pair->y == glyph->y)
        return TRUE;

    x = pair->x - glyph->x;
    y = pair->y - glyph->y;

    while (cnt --) {
        glyph->x += x;
        glyph->y += y;
        ++ glyph;
    }

    return TRUE;
}

static gboolean
update_label_cache(AdgToyText *toy_text, cairo_t *cr)
{
    AdgToyTextPrivate *data = toy_text->data;
    cairo_status_t status;

    status = cairo_scaled_font_text_to_glyphs(cairo_get_scaled_font(cr),
                                              0., 0., data->label, -1,
                                              &data->glyphs,
                                              &data->num_glyphs,
                                              NULL, NULL, NULL);

    if (status != CAIRO_STATUS_SUCCESS) {
        g_error("Unable to build glyphs (cairo message: %s)",
                cairo_status_to_string(status));
        return FALSE;
    }

    cairo_glyph_extents(cr, data->glyphs, data->num_glyphs, &data->extents);

    clear_origin_cache(toy_text);
    return TRUE;
}

static void
clear_origin_cache(AdgToyText *toy_text)
{
    AdgToyTextPrivate *data = toy_text->data;

    data->origin_cached = FALSE;
}

static void
clear_label_cache(AdgToyText *toy_text)
{
    AdgToyTextPrivate *data = toy_text->data;

    if (data->glyphs) {
        cairo_glyph_free(data->glyphs);
        data->glyphs = NULL;
    }

    data->num_glyphs = 0;
    memset(&data->extents, 0, sizeof(data->extents));
}
