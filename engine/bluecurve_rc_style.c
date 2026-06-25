/* Bluecurve theme engine
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

#include "bluecurve_style.h"
#include "bluecurve_rc_style.h"

static void      bluecurve_rc_style_init         (BluecurveRcStyle      *style);
static void      bluecurve_rc_style_class_init   (BluecurveRcStyleClass *klass);
static GtkStyle *bluecurve_rc_style_create_style (GtkRcStyle             *rc_style);
static guint     bluecurve_rc_style_parse        (GtkRcStyle             *rc_style,
						   GtkSettings            *settings,
						   GScanner               *scanner);
static void      bluecurve_rc_style_merge        (GtkRcStyle             *dest,
						   GtkRcStyle             *src);


static GtkRcStyleClass *parent_class;

GType bluecurve_type_rc_style = 0;

enum
{
  TOKEN_SPOTCOLOR = G_TOKEN_LAST + 1,
  TOKEN_CONTRAST
};

static struct
  {
    const gchar        *name;
    guint               token;
  }
theme_symbols[] =
{
  { "spotcolor", 		TOKEN_SPOTCOLOR  },
  { "contrast", 		TOKEN_CONTRAST  },
};


void
bluecurve_rc_style_register_type (GTypeModule *module)
{
  static const GTypeInfo object_info =
  {
    sizeof (BluecurveRcStyleClass),
    (GBaseInitFunc) NULL,
    (GBaseFinalizeFunc) NULL,
    (GClassInitFunc) bluecurve_rc_style_class_init,
    NULL,           /* class_finalize */
    NULL,           /* class_data */
    sizeof (BluecurveRcStyle),
    0,              /* n_preallocs */
    (GInstanceInitFunc) bluecurve_rc_style_init,
    NULL
  };
  
  bluecurve_type_rc_style = g_type_module_register_type (module,
						     GTK_TYPE_RC_STYLE,
						     "BluecurveRcStyle",
						     &object_info, 0);
}

static void
bluecurve_rc_style_init (BluecurveRcStyle *bluecurve_rc)
{
  bluecurve_rc->has_spot_color = FALSE;
  bluecurve_rc->contrast = 1.0;
}

static void
bluecurve_rc_style_class_init (BluecurveRcStyleClass *klass)
{
  GtkRcStyleClass *rc_style_class = GTK_RC_STYLE_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  rc_style_class->parse = bluecurve_rc_style_parse;
  rc_style_class->create_style = bluecurve_rc_style_create_style;
  rc_style_class->merge = bluecurve_rc_style_merge;
}

static guint
theme_parse_color(GtkSettings  *settings,
		  GScanner     *scanner,
		  GdkColor     *color)
{
  guint token;

  /* Skip 'blah_color' */
  token = g_scanner_get_next_token(scanner);

  token = g_scanner_get_next_token(scanner);
  if (token != G_TOKEN_EQUAL_SIGN)
    return G_TOKEN_EQUAL_SIGN;

  return gtk_rc_parse_color (scanner, color);
}

static guint
theme_parse_contrast(GtkSettings  *settings,
		     GScanner     *scanner,
		     double       *contrast)
{
  guint token;

  /* Skip 'contrast' */
  token = g_scanner_get_next_token(scanner);

  token = g_scanner_get_next_token(scanner);
  if (token != G_TOKEN_EQUAL_SIGN)
    return G_TOKEN_EQUAL_SIGN;

  token = g_scanner_get_next_token(scanner);
  if (token != G_TOKEN_FLOAT)
    return G_TOKEN_FLOAT;

  *contrast = scanner->value.v_float;
  
  return G_TOKEN_NONE;
}

static guint
bluecurve_rc_style_parse (GtkRcStyle *rc_style,
			   GtkSettings  *settings,
			   GScanner   *scanner)
		     
{
  static GQuark scope_id = 0;
  BluecurveRcStyle *bluecurve_style = BLUECURVE_RC_STYLE (rc_style);

  guint old_scope;
  guint token;
  guint i;
  
  /* Set up a new scope in this scanner. */

  if (!scope_id)
    scope_id = g_quark_from_string("bluecurve_theme_engine");

  /* If we bail out due to errors, we *don't* reset the scope, so the
   * error messaging code can make sense of our tokens.
   */
  old_scope = g_scanner_set_scope(scanner, scope_id);

  /* Now check if we already added our symbols to this scope
   * (in some previous call to bluecurve_rc_style_parse for the
   * same scanner.
   */

  if (!g_scanner_lookup_symbol(scanner, theme_symbols[0].name))
    {
      g_scanner_freeze_symbol_table(scanner);
      for (i = 0; i < G_N_ELEMENTS (theme_symbols); i++)
	g_scanner_scope_add_symbol(scanner, scope_id,
				   theme_symbols[i].name,
				   GINT_TO_POINTER(theme_symbols[i].token));
      g_scanner_thaw_symbol_table(scanner);
    }

  /* We're ready to go, now parse the top level */

  token = g_scanner_peek_next_token(scanner);
  while (token != G_TOKEN_RIGHT_CURLY)
    {
      switch (token)
	{
	case TOKEN_SPOTCOLOR:
	  token = theme_parse_color(settings, scanner, &bluecurve_style->spot_color);
	  bluecurve_style->has_spot_color = TRUE;
	  break;
	case TOKEN_CONTRAST:
	  token = theme_parse_contrast(settings, scanner, &bluecurve_style->contrast);
	  break;
	default:
	  g_scanner_get_next_token(scanner);
	  token = G_TOKEN_RIGHT_CURLY;
	  break;
	}

      if (token != G_TOKEN_NONE)
	return token;

      token = g_scanner_peek_next_token(scanner);
    }

  g_scanner_get_next_token(scanner);

  g_scanner_set_scope(scanner, old_scope);

  return G_TOKEN_NONE;
}

static void
bluecurve_rc_style_merge (GtkRcStyle *dest,
			   GtkRcStyle *src)
{
  BluecurveRcStyle *dest_w, *src_w;
  
  parent_class->merge (dest, src);

  if (!BLUECURVE_IS_RC_STYLE (src))
    return;
  
  src_w = BLUECURVE_RC_STYLE (src);
  dest_w = BLUECURVE_RC_STYLE (dest);

  dest_w->contrast = src_w->contrast;
  if (src_w->has_spot_color)
    {
      dest_w->has_spot_color = TRUE;
      dest_w->spot_color = src_w->spot_color;
    }
}


/* Create an empty style suitable to this RC style
 */
static GtkStyle *
bluecurve_rc_style_create_style (GtkRcStyle *rc_style)
{
  return GTK_STYLE (g_object_new (BLUECURVE_TYPE_STYLE, NULL));
}
