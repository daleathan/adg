#ifndef __ADGPATH_H__
#define __ADGPATH_H__

#include <adg/adgentity.h>
#include <adg/adgpair.h>
#include <adg/adgstyle.h>


G_BEGIN_DECLS

#define ADG_TYPE_PATH             (adg_path_get_type ())
#define ADG_PATH(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ADG_TYPE_PATH, AdgPath))
#define ADG_PATH_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), ADG_TYPE_PATH, AdgPathClass))
#define ADG_IS_PATH(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ADG_TYPE_PATH))
#define ADG_IS_PATH_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), ADG_TYPE_PATH))
#define ADG_PATH_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), ADG_TYPE_PATH, AdgPathClass))

typedef struct _AdgPath       AdgPath;
typedef struct _AdgPathClass  AdgPathClass;

struct _AdgPath
{
  AdgEntity              entity;

  /*< private >*/

  AdgLineStyle          *line_style;
  
  cairo_path_t           cairo_path;
  GArray                *portions;

  /* Current point (cp) */
  gboolean               cp_defined;
  AdgPair                cp;

  AdgCallback            create_func;
  gpointer               user_data;
};

struct _AdgPathClass
{
  AdgEntityClass         parent_class;
};


GType           adg_path_get_type               (void) G_GNUC_CONST;


AdgEntity *     adg_path_new                    (AdgCallback     create_func,
                                                 gpointer        user_data);
void            adg_path_clear                  (AdgPath        *path);
const cairo_path_t *
                adg_path_get_cairo_path         (AdgPath        *path);

void            adg_path_chain_ymirror          (AdgPath        *path);
void            adg_path_dump                   (AdgPath        *path);


/* Cairo wrapper functions */

gboolean        adg_path_get_current_point      (AdgPath        *path,
                                                 double         *x,
                                                 double         *y);
void            adg_path_close                  (AdgPath        *path);
void            adg_path_arc                    (AdgPath        *path,
                                                 double          xc,
                                                 double          yc,
                                                 double          radius,
                                                 double          angle1,
                                                 double          angle2);
void            adg_path_arc_negative           (AdgPath        *path,
                                                 double          xc,
                                                 double          yc,
                                                 double          radius,
                                                 double          angle1,
                                                 double          angle2);
void            adg_path_curve_to               (AdgPath        *path,
                                                 double          x1,
                                                 double          y1,
                                                 double          x2,
                                                 double          y2,
                                                 double          x3,
                                                 double          y3);
void            adg_path_line_to                (AdgPath        *path,
                                                 double          x,
                                                 double          y);
void            adg_path_move_to                (AdgPath        *path,
                                                 double          x,
                                                 double          y);
void            adg_path_rectangle              (AdgPath        *path,
                                                 double          x,
                                                 double          y,
                                                 double          width,
                                                 double          height);
void            adg_path_rel_curve_to           (AdgPath        *path,
                                                 double          dx1,
                                                 double          dy1,
                                                 double          dx2,
                                                 double          dy2,
                                                 double          dx3,
                                                 double          dy3);
void            adg_path_rel_line_to            (AdgPath        *path,
                                                 double          dx,
                                                 double          dy);
void            adg_path_rel_move_to            (AdgPath        *path,
                                                 double          dx,
                                                 double          dy);

G_END_DECLS


#endif /* __ADGPATH_H__ */
