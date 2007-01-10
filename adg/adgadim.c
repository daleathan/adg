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


/**
 * SECTION:adgadim
 * @title: AdgADim
 * @short_description: Angular dimensions
 *
 * The #AdgADim entity represents an angular dimension.
 **/

#include "adgadim.h"
#include "adgdimprivate.h"
#include "adgcontainer.h"
#include "adgutil.h"
#include "adgintl.h"


enum
{
  PROP_0,
  PROP_DIRECTION
};


static void	        adg_adim_finalize               (GObject        *object);

static void             adg_adim_ctm_changed            (AdgEntity      *entity,
                                                         AdgMatrix      *old_matrix);
static void             adg_adim_update                 (AdgEntity      *entity,
                                                         gboolean        recursive);
static void             adg_adim_render                 (AdgEntity      *entity,
                                                         cairo_t        *cr);

static gchar *          adg_adim_default_label          (AdgDim         *dim);


G_DEFINE_TYPE (AdgADim, adg_adim, ADG_TYPE_DIM);

#define PARENT_CLASS ((AdgDimClass *) adg_adim_parent_class)


static void
adg_adim_class_init (AdgADimClass *klass)
{
  GObjectClass   *gobject_class;
  AdgEntityClass *entity_class;
  AdgDimClass    *dim_class;
  GParamSpec     *param;

  gobject_class = (GObjectClass *) klass;
  entity_class = (AdgEntityClass *) klass;
  dim_class = (AdgDimClass *) klass;

  gobject_class->finalize = adg_adim_finalize;

  entity_class->ctm_changed = adg_adim_ctm_changed;
  entity_class->update = adg_adim_update;
  entity_class->render = adg_adim_render;

  g_type_class_add_private (klass, sizeof (_AdgADimPrivate));
  dim_class->default_label = adg_adim_default_label;

  param = g_param_spec_double ("direction",
                              P_("Direction"),
                              P_("The inclination angle of the extension lines"),
                              -G_MAXDOUBLE, G_MAXDOUBLE, ADG_DIR_RIGHT,
                              G_PARAM_READWRITE|G_PARAM_CONSTRUCT);
  g_object_class_install_property (gobject_class, PROP_DIRECTION, param);

}

static void
adg_adim_init (AdgADim *adim)
{
  _AdgADimPrivate *cache = _ADG_ADIM_GET_PRIVATE (adim);

  cache->extension1.status = CAIRO_STATUS_SUCCESS;
  cache->extension1.data = NULL;
  cache->extension1.num_data = 4;

  cache->extension2.status = CAIRO_STATUS_SUCCESS;
  cache->extension2.data = NULL;
  cache->extension2.num_data = 4;

  cache->arrow_path.status = CAIRO_STATUS_SUCCESS;
  cache->arrow_path.data = NULL;
  cache->arrow_path.num_data = 4;

  cache->baseline.status = CAIRO_STATUS_SUCCESS;
  cache->baseline.data = NULL;
  cache->baseline.num_data = 4;
}

static void
adg_adim_finalize (GObject *object)
{
  _AdgADimPrivate *cache = _ADG_ADIM_GET_PRIVATE (object);

  g_free (cache->extension1.data);
  g_free (cache->extension2.data);
  g_free (cache->arrow_path.data);
  g_free (cache->baseline.data);
}

static void
adg_adim_ctm_changed (AdgEntity *entity,
                      AdgMatrix *old_matrix)
{
  AdgContainer *container;
  AdgMatrix    *matrix;

  /* entity is yet outdated, no needs for further checks */
  if (! ADG_ENTITY_UPTODATE (entity))
    return;

  container = (AdgContainer *) entity->parent;
  g_return_if_fail (ADG_IS_CONTAINER (container));

  matrix = &container->ctm;

  if (old_matrix == NULL || old_matrix->xx != matrix->xx || old_matrix->yy != matrix->yy)
    adg_entity_outdate (entity);
}

static void
adg_adim_update (AdgEntity *entity,
                 gboolean   recursive)
{
  /* TODO */
  ((AdgEntityClass *) PARENT_CLASS)->update (entity, recursive);
}

static void
adg_adim_render (AdgEntity *entity,
                 cairo_t   *cr)
{
  /* TODO */
}

static gchar *
adg_adim_default_label (AdgDim *dim)
{
  /* TODO */
  return g_strdup ("TODO");
}



/**
 * adg_adim_new:
 *
 * Creates a new - unreferenced - angular dimension. You must, at least, define
 * the reference points with adg_dim_set_ref() and the position reference using
 * adg_dim_set_pos().
 *
 * Return value: the new entity
 **/
AdgEntity *
adg_adim_new (void)
{
  return (AdgEntity *) g_object_new (ADG_TYPE_ADIM, NULL);
}
