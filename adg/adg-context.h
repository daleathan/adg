/* ADG - Automatic Drawing Generation
 * Copyright (C) 2007-2008, Nicola Fontana <ntd at entidi.it>
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


#ifndef __ADG_CONTEXT_H__
#define __ADG_CONTEXT_H__

#include <adg/adg-style.h>


G_BEGIN_DECLS


#define ADG_TYPE_CONTEXT             (adg_context_get_type ())
#define ADG_CONTEXT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ADG_TYPE_CONTEXT, AdgContext))
#define ADG_CONTEXT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), ADG_TYPE_CONTEXT, AdgContextClass))
#define ADG_IS_CONTEXT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ADG_TYPE_CONTEXT))
#define ADG_IS_CONTEXT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), ADG_TYPE_CONTEXT))
#define ADG_CONTEXT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), ADG_TYPE_CONTEXT, AdgContextClass))


typedef struct _AdgContext        AdgContext;
typedef struct _AdgContextClass   AdgContextClass;
typedef struct _AdgContextPrivate AdgContextPrivate;

struct _AdgContext
{
  GObject		 object;

  /*< private >*/
  AdgContextPrivate	*priv;
};

struct _AdgContextClass
{
  GObjectClass	 object_class;
};


GType		adg_context_get_type		(void) G_GNUC_CONST;


G_END_DECLS


#endif /* __ADG_CONTEXT_H__ */
