/* Bluecurve Theme Engine
 * Copyright (C) 2001 Red Hat, Inc.
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
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Written by Owen Taylor <otaylor@redhat.com>
 * modified by Alexander Larsson <alexl@redhat.com>
 */

#include <gtk/gtkrc.h>

typedef struct _BluecurveRcStyle BluecurveRcStyle;
typedef struct _BluecurveRcStyleClass BluecurveRcStyleClass;

extern GType bluecurve_type_rc_style;

#define BLUECURVE_TYPE_RC_STYLE              bluecurve_type_rc_style
#define BLUECURVE_RC_STYLE(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), BLUECURVE_TYPE_RC_STYLE, BluecurveRcStyle))
#define BLUECURVE_RC_STYLE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), BLUECURVE_TYPE_RC_STYLE, BluecurveRcStyleClass))
#define BLUECURVE_IS_RC_STYLE(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), BLUECURVE_TYPE_RC_STYLE))
#define BLUECURVE_IS_RC_STYLE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), BLUECURVE_TYPE_RC_STYLE))
#define BLUECURVE_RC_STYLE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), BLUECURVE_TYPE_RC_STYLE, BluecurveRcStyleClass))

struct _BluecurveRcStyle
{
  GtkRcStyle parent_instance;

  GdkColor spot_color;
  gboolean has_spot_color;
  double contrast;
};

struct _BluecurveRcStyleClass
{
  GtkRcStyleClass parent_class;
};

void bluecurve_rc_style_register_type (GTypeModule *module);
