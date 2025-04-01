#include <gtk/gtk.h>
#include "bluecurve_style.h"
#include "bluecurve_rc_style.h"
#include <math.h>
#include <string.h>


#include "bits.c"


/* #define DEBUG 1 */

#define RADIO_SIZE 13
#define CHECK_SIZE 13

#define SCALE_SIZE 5

#define DETAIL(xx)   ((detail) && (!strcmp(xx, detail)))

/**************************************************************************
* GTK Bluecurve Theme
*
* Designed by Garrett LeSage
* Implemented by Alexander Larsson
*
* Parts of the code based on Metal theme, Copyright:
* Randy Gordon, randy@integrand.com
* Owen Taylor <otaylor@redhat.com>
*
* License: GPL (Gnu Public License)
*
**************************************************************************/

/* TODO:
 *  Handle box on toolbars is wider in screenshots, hard to do in gtk+.
 *  Sometimes the grippies repaint wrong. One pixel is missing in the end of the lines.
 *  How to handle defaults now?
 */

static void draw_box               (GtkStyle       *style,
				    GdkWindow      *window,
				    GtkStateType    state_type,
				    GtkShadowType   shadow_type,
				    GdkRectangle   *area,
				    GtkWidget      *widget,
				    const gchar    *detail,
				    gint            x,
				    gint            y,
				    gint            width,
				    gint            height);
static void bluecurve_button      (GtkStyle       *style,
				    GdkWindow      *window,
				    GtkStateType    state_type,
				    GtkShadowType   shadow_type,
				    GdkRectangle   *area,
				    GtkWidget      *widget,
				    const gchar    *detail,
				    gint            x,
				    gint            y,
				    gint            width,
				    gint            height);

static GtkStyleClass *parent_class;

static GtkRequisition default_option_indicator_size = { 7, 13 };
static GtkBorder default_option_indicator_spacing = { 7, 5, 2, 2 };

static GtkTextDirection
get_direction (GtkWidget *widget)
{
  GtkTextDirection dir;
  
  if (widget)
    dir = gtk_widget_get_direction (widget);
  else
    dir = GTK_TEXT_DIR_LTR;
  
  return dir;
}

static GdkPixbuf *
generate_bit (unsigned char alpha[], GdkColor *color, double mult)
{
  guint r, g, b;
  GdkPixbuf *pixbuf;
  unsigned char *pixels;
  int w, h, rs;
  int x, y;
  
  r = (color->red >> 8) * mult;
  r = MIN(r, 255);
  g = (color->green >> 8) * mult;
  g = MIN(g, 255);
  b = (color->blue >> 8) * mult;
  b = MIN(b, 255);

  pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, RADIO_SIZE, RADIO_SIZE);

  w = gdk_pixbuf_get_width (pixbuf);
  h = gdk_pixbuf_get_height (pixbuf);
  rs = gdk_pixbuf_get_rowstride (pixbuf);
  pixels = gdk_pixbuf_get_pixels (pixbuf);


  for (y=0; y < h; y++)
    {
      for (x=0; x < w; x++)
	{
	  pixels[y*rs + x*4 + 0] = r;
	  pixels[y*rs + x*4 + 1] = g;
	  pixels[y*rs + x*4 + 2] = b;
	  if (alpha)
	    pixels[y*rs + x*4 + 3] = alpha[y*w + x];
	  else
	    pixels[y*rs + x*4 + 3] = 255;
	}
    }

  return pixbuf;
}

#define CLAMP_UCHAR(v) ((guchar) (CLAMP (((int)v), (int)0, (int)255)))

static GdkPixbuf *
colorize_bit (unsigned char *bit,
	      unsigned char *alpha,
	      GdkColor  *new_color)
{
  GdkPixbuf *pixbuf;
  double intensity;
  int x, y;
  const guchar *src, *asrc;
  guchar *dest;
  int dest_rowstride;
  int width, height;
  guchar *dest_pixels;
  
  pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8,
			   RADIO_SIZE, RADIO_SIZE);

  if (pixbuf == NULL)
    return NULL;
  
  dest_rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  width = gdk_pixbuf_get_width (pixbuf);
  height = gdk_pixbuf_get_height (pixbuf);
  dest_pixels = gdk_pixbuf_get_pixels (pixbuf);
  
  for (y = 0; y < RADIO_SIZE; y++)
    {
      src = bit + y * RADIO_SIZE;
      asrc = alpha + y * RADIO_SIZE;
      dest = dest_pixels + y * dest_rowstride;

      for (x = 0; x < RADIO_SIZE; x++)
        {
          double dr, dg, db;
          
          intensity = (src[x] + 0 )/ 255.0;

          if (intensity <= 0.5)
            {
              /* Go from black at intensity = 0.0 to new_color at intensity = 0.5 */
              dr = (new_color->red * intensity * 2.0) / 65535.0;
              dg = (new_color->green * intensity * 2.0) / 65535.0;
              db = (new_color->blue * intensity * 2.0) / 65535.0;
            }
          else
            {
              /* Go from new_color at intensity = 0.5 to white at intensity = 1.0 */
              dr = (new_color->red + (65535 - new_color->red) * (intensity - 0.5) * 2.0) / 65535.0;
              dg = (new_color->green + (65535 - new_color->green) * (intensity - 0.5) * 2.0) / 65535.0;
              db = (new_color->blue + (65535 - new_color->blue) * (intensity - 0.5) * 2.0) / 65535.0;
            }
          
          dest[0] = CLAMP_UCHAR (255 * dr);
          dest[1] = CLAMP_UCHAR (255 * dg);
          dest[2] = CLAMP_UCHAR (255 * db);

	  dest[3] = asrc[x];
	  dest += 4;
        }
    }

  return pixbuf;
}

static GdkPixmap *
pixbuf_to_pixmap (GtkStyle  *style,
		  GdkPixbuf *pixbuf,
		  GdkScreen *screen)
{
  GdkGC *tmp_gc;
  GdkPixmap *pixmap;
  
  pixmap = gdk_pixmap_new (gdk_screen_get_root_window (screen),
			   gdk_pixbuf_get_width (pixbuf),
			   gdk_pixbuf_get_height (pixbuf),
			   style->depth);
  gdk_drawable_set_colormap (pixmap, style->colormap);

  tmp_gc = gdk_gc_new (pixmap);
  
  gdk_pixbuf_render_to_drawable (pixbuf, pixmap, tmp_gc, 0, 0, 0, 0,
				 gdk_pixbuf_get_width (pixbuf),
				 gdk_pixbuf_get_height (pixbuf),
				 GDK_RGB_DITHER_NORMAL, 0, 0);
  gdk_gc_unref (tmp_gc);

  return pixmap;
}

static gboolean 
sanitize_size (GdkWindow      *window,
	       gint           *width,
	       gint           *height)
{
  gboolean set_bg = FALSE;

  if ((*width == -1) && (*height == -1))
    {
      set_bg = GDK_IS_WINDOW (window);
      gdk_window_get_size (window, width, height);
    }
  else if (*width == -1)
    gdk_window_get_size (window, width, NULL);
  else if (*height == -1)
    gdk_window_get_size (window, NULL, height);

  return set_bg;
}

static GdkColor *
bluecurve_get_spot_color (BluecurveRcStyle *bluecurve_rc)
{
  GtkRcStyle *rc = GTK_RC_STYLE (bluecurve_rc);
  
  if (bluecurve_rc->has_spot_color)
    return &bluecurve_rc->spot_color;
  else
    return &rc->base[GTK_STATE_SELECTED];
}

static void
option_menu_get_props (GtkWidget      *widget,
		       GtkRequisition *indicator_size,
		       GtkBorder      *indicator_spacing)
{
  GtkRequisition *tmp_size = NULL;
  GtkBorder *tmp_spacing = NULL;
  
  if (widget)
    gtk_widget_style_get (widget, 
			  "indicator_size", &tmp_size,
			  "indicator_spacing", &tmp_spacing,
			  NULL);

  if (tmp_size)
    {
      *indicator_size = *tmp_size;
      g_free (tmp_size);
    }
  else
    *indicator_size = default_option_indicator_size;

  if (tmp_spacing)
    {
      *indicator_spacing = *tmp_spacing;
      g_free (tmp_spacing);
    }
  else
    *indicator_spacing = default_option_indicator_spacing;
}

static void
rgb_to_hls (gdouble *r,
            gdouble *g,
            gdouble *b)
{
  gdouble min;
  gdouble max;
  gdouble red;
  gdouble green;
  gdouble blue;
  gdouble h, l, s;
  gdouble delta;
  
  red = *r;
  green = *g;
  blue = *b;
  
  if (red > green)
    {
      if (red > blue)
        max = red;
      else
        max = blue;
      
      if (green < blue)
        min = green;
      else
        min = blue;
    }
  else
    {
      if (green > blue)
        max = green;
      else
        max = blue;
      
      if (red < blue)
        min = red;
      else
        min = blue;
    }
  
  l = (max + min) / 2;
  s = 0;
  h = 0;
  
  if (max != min)
    {
      if (l <= 0.5)
        s = (max - min) / (max + min);
      else
        s = (max - min) / (2 - max - min);
      
      delta = max -min;
      if (red == max)
        h = (green - blue) / delta;
      else if (green == max)
        h = 2 + (blue - red) / delta;
      else if (blue == max)
        h = 4 + (red - green) / delta;
      
      h *= 60;
      if (h < 0.0)
        h += 360;
    }
  
  *r = h;
  *g = l;
  *b = s;
}

static void
hls_to_rgb (gdouble *h,
            gdouble *l,
            gdouble *s)
{
  gdouble hue;
  gdouble lightness;
  gdouble saturation;
  gdouble m1, m2;
  gdouble r, g, b;
  
  lightness = *l;
  saturation = *s;
  
  if (lightness <= 0.5)
    m2 = lightness * (1 + saturation);
  else
    m2 = lightness + saturation - lightness * saturation;
  m1 = 2 * lightness - m2;
  
  if (saturation == 0)
    {
      *h = lightness;
      *l = lightness;
      *s = lightness;
    }
  else
    {
      hue = *h + 120;
      while (hue > 360)
        hue -= 360;
      while (hue < 0)
        hue += 360;
      
      if (hue < 60)
        r = m1 + (m2 - m1) * hue / 60;
      else if (hue < 180)
        r = m2;
      else if (hue < 240)
        r = m1 + (m2 - m1) * (240 - hue) / 60;
      else
        r = m1;
      
      hue = *h;
      while (hue > 360)
        hue -= 360;
      while (hue < 0)
        hue += 360;
      
      if (hue < 60)
        g = m1 + (m2 - m1) * hue / 60;
      else if (hue < 180)
        g = m2;
      else if (hue < 240)
        g = m1 + (m2 - m1) * (240 - hue) / 60;
      else
        g = m1;
      
      hue = *h - 120;
      while (hue > 360)
        hue -= 360;
      while (hue < 0)
        hue += 360;
      
      if (hue < 60)
        b = m1 + (m2 - m1) * hue / 60;
      else if (hue < 180)
        b = m2;
      else if (hue < 240)
        b = m1 + (m2 - m1) * (240 - hue) / 60;
      else
        b = m1;
      
      *h = r;
      *l = g;
      *s = b;
    }
}

static void
shade (GdkColor * a, GdkColor * b, float k)
{
  gdouble red;
  gdouble green;
  gdouble blue;
  
  red = (gdouble) a->red / 65535.0;
  green = (gdouble) a->green / 65535.0;
  blue = (gdouble) a->blue / 65535.0;
  
  rgb_to_hls (&red, &green, &blue);
  
  green *= k;
  if (green > 1.0)
    green = 1.0;
  else if (green < 0.0)
    green = 0.0;
  
  blue *= k;
  if (blue > 1.0)
    blue = 1.0;
  else if (blue < 0.0)
    blue = 0.0;
  
  hls_to_rgb (&red, &green, &blue);
  
  b->red = red * 65535.0;
  b->green = green * 65535.0;
  b->blue = blue * 65535.0;
}


/**************************************************************************/

static void
draw_hgradient (GdkDrawable *drawable, GdkGC *gc, GdkColormap *colormap,
		int x, int y, int width, int height,
		GdkColor *top_color, GdkColor *bottom_color)
{
  int i;
  GdkColor col;
  int dr, dg, db;
  GdkGCValues old_values;

  col = *top_color;
  dr = (bottom_color->red - top_color->red) / (height == 0 ? 1 : height);
  dg = (bottom_color->green - top_color->green) / (height == 0 ? 1 : height);
  db = (bottom_color->blue - top_color->blue) / (height == 0 ? 1 : height);
  
  gdk_gc_get_values (gc, &old_values);
  
  for (i = 0; i < height; i++)
    {
      gdk_rgb_find_color (colormap,
			  &col);
      
      gdk_gc_set_foreground (gc, &col);
      gdk_draw_line (drawable, gc,
		     x, y + i,
		     x + width - 1, y + i);
					   
      col.red += dr;
      col.green += dg;
      col.blue += db;
    }

  gdk_gc_set_foreground (gc, &old_values.foreground);
}

static void
draw_vgradient (GdkDrawable *drawable, GdkGC *gc, GdkColormap *colormap,
		int x, int y, int width, int height,
		GdkColor *left_color, GdkColor *right_color)
{
  int i;
  GdkColor col;
  int dr, dg, db;
  GdkGCValues old_values;

  col = *left_color;
  dr = (right_color->red - left_color->red) / (width == 0 ? 1 : width);
  dg = (right_color->green - left_color->green) / (width == 0 ? 1 : width);
  db = (right_color->blue - left_color->blue) / (width == 0 ? 1 : width);
  
  gdk_gc_get_values (gc, &old_values);
  
  for (i = 0; i < width; i++)
    {
      gdk_rgb_find_color (colormap,
			  &col);
      
      gdk_gc_set_foreground (gc, &col);
      gdk_draw_line (drawable, gc,
		     x + i, y,
		     x + i, y + height - 1);
					   
      col.red += dr;
      col.green += dg;
      col.blue += db;
    }

  gdk_gc_set_foreground (gc, &old_values.foreground);
}



/**************************************************************************/

static void
arrow_draw_hline (GdkWindow     *window,
		  GdkGC         *gc,
		  int            x1,
		  int            x2,
		  int            y,
		  gboolean       last)
{
  if (x2 - x1 < 7 && !last) /* 7 to get garretts pixels, otherwise 6 */
    gdk_draw_line (window, gc, x1, y, x2, y);
  else if (last)
    {
      if (x2 - x1 <= 7)
	{
	  gdk_draw_line (window, gc, x1+1, y, x1+1, y);
	  gdk_draw_line (window, gc, x2-1, y, x2-1, y); 
	}
      else
	{
	  gdk_draw_line (window, gc, x1+2, y, x1+2, y);
	  gdk_draw_line (window, gc, x2-2, y, x2-2, y);
	}
    }
  else
    {
      gdk_draw_line (window, gc, x1, y, x1+2, y);
      gdk_draw_line (window, gc, x2-2, y, x2, y); 
    }
}

static void
arrow_draw_vline (GdkWindow     *window,
		  GdkGC         *gc,
		  int            y1,
		  int            y2,
		  int            x,
		  gboolean       last)
{
  if (y2 - y1 < 7 && !last) /* 7 to get garretts pixels */
    gdk_draw_line (window, gc, x, y1, x, y2);
  else if (last)
    {
      gdk_draw_line (window, gc, x, y1+2, x, y1+2);
      gdk_draw_line (window, gc, x, y2-2, x, y2-2); 
    }
  else
    {
      gdk_draw_line (window, gc, x, y1, x, y1+2);
      gdk_draw_line (window, gc, x, y2-2, x, y2); 
    }
}



static void
draw_arrow (GdkWindow     *window,
	    GdkGC         *gc,
	    GdkRectangle  *area,
	    GtkArrowType   arrow_type,
	    gint           x,
	    gint           y,
	    gint           width,
	    gint           height)
{
  gint i, j;

  if (area)
    gdk_gc_set_clip_rectangle (gc, area);

  if (arrow_type == GTK_ARROW_DOWN)
    {
      for (i = 0, j = -1; i < height; i++, j++)
	arrow_draw_hline (window, gc, x + j, x + width - j - 1, y + i, i == 0);
	  
    }
  else if (arrow_type == GTK_ARROW_UP)
    {
      for (i = height - 1, j = -1; i >= 0; i--, j++)
	arrow_draw_hline (window, gc, x + j, x + width - j - 1, y + i, i == height - 1);
    }
  else if (arrow_type == GTK_ARROW_LEFT)
    {
      for (i = width - 1, j = -1; i >= 0; i--, j++)
	arrow_draw_vline (window, gc, y + j, y + height - j - 1, x + i, i == width - 1);
    }
  else if (arrow_type == GTK_ARROW_RIGHT)
    {
      for (i = 0, j = -1; i < width; i++, j++)
	arrow_draw_vline (window, gc, y + j, y + height - j - 1,  x + i, i == 0);
    }

  if (area)
    gdk_gc_set_clip_rectangle (gc, NULL);
}

static void
calculate_arrow_geometry (GtkArrowType  arrow_type,
			  gint         *x,
			  gint         *y,
			  gint         *width,
			  gint         *height)
{
  gint w = *width;
  gint h = *height;
  
  switch (arrow_type)
    {
    case GTK_ARROW_UP:
    case GTK_ARROW_DOWN:
      w += (w % 2) - 1;
      h = (w / 2 + 1) + 1;

      if (h > *height)
	{
	  h = *height;
	  w = 2 * (h - 1) - 1;
	}
      
      if (arrow_type == GTK_ARROW_DOWN)
	{
	  if (*height % 2 == 1 || h % 2 == 0)
	    *height += 1;
	}
      else
	{
	  if (*height % 2 == 0 || h % 2 == 0)
	    *height -= 1;
	}
      break;

    case GTK_ARROW_RIGHT:
    case GTK_ARROW_LEFT:
      h += (h % 2) - 1;
      w = (h / 2 + 1) + 1; 
      
      if (w > *width)
	{
	  w = *width;
	  h = 2 * (w - 1) - 1;
	}
      
      if (arrow_type == GTK_ARROW_RIGHT)
	{
	  if (*width % 2 == 1 || w % 2 == 0)
	    *width += 1;
	}
      else
	{
	  if (*width % 2 == 0 || w % 2 == 0)
	    *width -= 1;
	}
      break;
      
    default:
      /* should not be reached */
      break;
    }

  *x += (*width - w) / 2;
  *y += (*height - h) / 2;
  *height = h;
  *width = w;
}

static void
bluecurve_draw_arrow (GtkStyle      *style,
		       GdkWindow     *window,
		       GtkStateType   state,
		       GtkShadowType  shadow,
		       GdkRectangle  *area,
		       GtkWidget     *widget,
		       const gchar   *detail,
		       GtkArrowType   arrow_type,
		       gboolean       fill,
		       gint           x,
		       gint           y,
		       gint           width,
		       gint           height)
{
  BluecurveStyle *bluecurve_style = BLUECURVE_STYLE (style);
  gint original_width, original_x;
  GdkGC *gc;

  sanitize_size (window, &width, &height);

  original_width = width;
  original_x = x;

  /* Make spinbutton arrows and arrows in menus
   * slightly larger to get the right pixels drawn */
  if (DETAIL ("spinbutton"))
    height += 1;

  if (DETAIL("menuitem"))
    {
      width = 6;
      height = 7;
    }
  
  calculate_arrow_geometry (arrow_type, &x, &y, &width, &height);
  
  if (detail && strcmp (detail, "menuitem") == 0)
    x = original_x + original_width - width;

  if (DETAIL ("spinbutton") &&
      (arrow_type == GTK_ARROW_DOWN))
    y += 1;
  
  if (state == GTK_STATE_INSENSITIVE)
    draw_arrow (window, style->white_gc, area, arrow_type,
		x + 1, y + 1, width, height);
  if (state == GTK_STATE_NORMAL)
    gc = bluecurve_style->gray_gc[7];
  else
    gc = style->fg_gc[state];
  draw_arrow (window, gc, area, arrow_type,
	      x, y, width, height);
}


/**************************************************************************/

static void
draw_tab (GtkStyle      *style,
	  GdkWindow     *window,
	  GtkStateType   state_type,
	  GtkShadowType  shadow_type,
	  GdkRectangle  *area,
	  GtkWidget     *widget,
	  const gchar   *detail,
	  gint           x,
	  gint           y,
	  gint           width,
	  gint           height)
{
#define ARROW_SPACE 2
#define ARROW_LINE_HEIGHT 2
#define ARROW_LINE_WIDTH 5

  BluecurveStyle *bluecurve_style = BLUECURVE_STYLE (style);
  GtkRequisition indicator_size;
  GtkBorder indicator_spacing;
  gint arrow_height;
  
  option_menu_get_props (widget, &indicator_size, &indicator_spacing);

  indicator_size.width += (indicator_size.width % 2) - 1;
  arrow_height = indicator_size.width / 2 + 2;

  x += (width - indicator_size.width) / 2;
  y += (height - (arrow_height + ARROW_SPACE + ARROW_LINE_HEIGHT )) / 2;

  if (state_type == GTK_STATE_INSENSITIVE)
    {
      draw_arrow (window, style->white_gc, area,
		  GTK_ARROW_DOWN, x + 1, y + 1,
		  indicator_size.width, arrow_height);
    }
  
  draw_arrow (window, bluecurve_style->gray_gc[7], area,
	      GTK_ARROW_DOWN, x, y,
	      indicator_size.width, arrow_height);

  gdk_draw_rectangle (window,
		      bluecurve_style->gray_gc[3],
		      TRUE,
		      x + (indicator_size.width - ARROW_LINE_WIDTH) / 2,
		      y + arrow_height + ARROW_SPACE,
		      ARROW_LINE_WIDTH,
		      ARROW_LINE_HEIGHT);
  
}



/**************************************************************************/

static void
draw_shadow (GtkStyle        *style,
	     GdkWindow       *window,
	     GtkStateType     state_type,
	     GtkShadowType    shadow_type,
	     GdkRectangle    *area,
	     GtkWidget       *widget,
	     const gchar     *detail,
	     gint             x,
	     gint             y,
	     gint             width,
	     gint             height)
{
  BluecurveStyle *bluecurve_style = BLUECURVE_STYLE (style);
  GdkGC *outer_gc = NULL;
  GdkGC *gc1 = NULL;
  GdkGC *gc2 = NULL;
  gint thickness_light;
  gint thickness_dark;
  gint i;


  if (DETAIL ("frame") && widget && widget->parent && GTK_IS_STATUSBAR (widget->parent))
    {
      gtk_style_apply_default_background (style, window,
					  widget && !GTK_WIDGET_NO_WINDOW (widget),
					  state_type, area, x, y, width, height);
      
      if (area)
	{
	  gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[3], area);
	  gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[0], area);
	}
      
      gdk_draw_line (window, bluecurve_style->gray_gc[3],
		     x, y, x + width, y);
      gdk_draw_line (window, bluecurve_style->gray_gc[0],
		     x, y + 1, x + width, y + 1);
      if (area)
	{
	  gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[3], NULL);
	  gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[0], NULL);
	}
      return;
    }

  
  sanitize_size (window, &width, &height);

  /* Make sure stepper and slider outlines "overlap" */
  if (DETAIL("slider") && widget && GTK_IS_RANGE (widget))
    {
      GtkAdjustment *adj = GTK_RANGE (widget)->adjustment;

      if (adj->value <= adj->lower &&
	  (GTK_RANGE (widget)->has_stepper_a ||
	   GTK_RANGE (widget)->has_stepper_b))
	{
	  if (GTK_IS_VSCROLLBAR (widget))
	    {
	      height += 1;
	      y -= 1;
	    }
	  else if (GTK_IS_HSCROLLBAR (widget))
	    {
	      width += 1;
	      x -= 1;
	    }
	}
      if (adj->value >= adj->upper - adj->page_size &&
	  (GTK_RANGE (widget)->has_stepper_c ||
	   GTK_RANGE (widget)->has_stepper_d))
	{
	  if (GTK_IS_VSCROLLBAR (widget))
	    {
	      height += 1;
	    }
	  else if (GTK_IS_HSCROLLBAR (widget))
	    {
	      width += 1;
	    }
	}
    }
  
  if (detail &&
      ((strcmp (detail, "button") == 0) ||
       (strcmp (detail, "togglebutton") == 0) ||
       (strcmp (detail, "optionmenu") == 0) ||
       (strcmp (detail, "slider") == 0) ||
       (strcmp (detail, "vscrollbar") == 0) ||
       (strcmp (detail, "hscrollbar") == 0)))
    outer_gc = bluecurve_style->gray_gc[6];
  else if (DETAIL ("menuitem"))
    outer_gc = bluecurve_style->spot3_gc;
  else
    outer_gc = bluecurve_style->gray_gc[5];
    
  switch (shadow_type)
    {
    case GTK_SHADOW_NONE:
      return;
    case GTK_SHADOW_IN:
    case GTK_SHADOW_ETCHED_IN:
      gc1 = style->white_gc;
      gc2 = bluecurve_style->gray_gc[2];
      break;
    case GTK_SHADOW_OUT:
    case GTK_SHADOW_ETCHED_OUT:
      if (DETAIL ("menuitem"))
	{
	  gc1 = bluecurve_style->spot2_gc;
	  gc2 = bluecurve_style->spot1_gc;
	}
      else
	{
	  gc1 = bluecurve_style->gray_gc[2];
	  gc2 = style->white_gc;
	}
      break;
    }
  
  if (area)
    {
      gdk_gc_set_clip_rectangle (gc1, area);
      gdk_gc_set_clip_rectangle (gc2, area);
      if (shadow_type == GTK_SHADOW_IN || 
          shadow_type == GTK_SHADOW_OUT)
	{
	  gdk_gc_set_clip_rectangle (outer_gc, area);
	  gdk_gc_set_clip_rectangle (style->black_gc, area);
	}
    }
  
  switch (shadow_type)
    {
    case GTK_SHADOW_NONE:
      break;
      
    case GTK_SHADOW_IN:
      if (widget && GTK_WIDGET_HAS_DEFAULT (widget) &&
	  detail && (strcmp (detail, "button") == 0))
	gdk_draw_rectangle (window, style->black_gc, FALSE,
			    x, y, width - 1, height - 1);
      else
	gdk_draw_rectangle (window, outer_gc, FALSE,
			    x, y, width - 1, height - 1);


      if (detail &&
	  ((strcmp (detail, "button") == 0) ||
	   (strcmp (detail, "togglebutton") == 0) ||
	   (strcmp (detail, "handlebox") == 0) ||
	   (strcmp (detail, "optionmenu") == 0) ||
	   (strcmp (detail, "notebook") == 0) ||
	   (strcmp (detail, "menu") == 0) ||
	   (strcmp (detail, "slider") == 0) ||
	   (strcmp (detail, "spinbutton_up") == 0) ||
	   (strcmp (detail, "spinbutton_down") == 0) ||
	   (strcmp (detail, "vscrollbar") == 0) ||
	   (strcmp (detail, "hscrollbar") == 0) ||
	   (strcmp (detail, "stepper") == 0)))
	{
      
      /* Light around right and bottom edge */

      if (style->ythickness > 0)
        gdk_draw_line (window, gc1,
                       x + 1, y + height - 2, x + width - 2, y + height - 2);
      if (style->xthickness > 0)
        gdk_draw_line (window, gc1,
                       x + width - 2, y + 1, x + width - 2, y + height - 2);

      /* Dark around left and top */

      if (style->ythickness > 0)
        gdk_draw_line (window, gc2,
                       x + 1, y + 1, x + width - 3, y + 1);
      if (style->xthickness > 0)
        gdk_draw_line (window, gc2,
                       x + 1, y + 1, x + 1, y + height - 3);
	}
      break;
      
    case GTK_SHADOW_OUT:
      if (widget && GTK_WIDGET_HAS_DEFAULT (widget) &&
	  detail && (strcmp (detail, "button") == 0))
	gdk_draw_rectangle (window, style->black_gc, FALSE,
			    x, y, width - 1, height - 1);
      else
	gdk_draw_rectangle (window, outer_gc, FALSE,
			    x, y, width - 1, height - 1);

      if (detail &&
	  ((strcmp (detail, "button") == 0) ||
	   (strcmp (detail, "togglebutton") == 0) ||
	   (strcmp (detail, "handlebox") == 0) ||
	   (strcmp (detail, "optionmenu") == 0) ||
	   (strcmp (detail, "notebook") == 0) ||
	   (strcmp (detail, "slider") == 0) ||
	   (strcmp (detail, "spinbutton_up") == 0) ||
	   (strcmp (detail, "spinbutton_down") == 0) ||
	   (strcmp (detail, "menu") == 0) ||
	   (strcmp (detail, "menuitem") == 0) ||
	   (strcmp (detail, "vscrollbar") == 0) ||
	   (strcmp (detail, "hscrollbar") == 0) ||
	   (strcmp (detail, "stepper") == 0)))
	{
	  /* Dark around right and bottom edge */
	  if (style->ythickness > 0)
	    gdk_draw_line (window, gc1,
			   x + 2, y + height - 2, x + width - 2, y + height - 2);
	  if (style->xthickness > 0)
	    gdk_draw_line (window, gc1,
			   x + width - 2, y + 2, x + width - 2, y + height - 2);
	  
	  /* Light around top and left */
	  
	  if (style->ythickness > 0)
	    gdk_draw_line (window, gc2,
			   x + 1, y + 1, x + width - 2, y + 1);
	  if (style->xthickness > 0)
	    gdk_draw_line (window, gc2,
			   x + 1, y + 1, x + 1, y + height - 2);
	}      
      break;
      
    case GTK_SHADOW_ETCHED_IN:
    case GTK_SHADOW_ETCHED_OUT:
      if (style->xthickness > 0)
        {
          if (style->xthickness > 1)
            {
              thickness_light = 1;
              thickness_dark = 1;
      
              for (i = 0; i < thickness_dark; i++)
                {
                  gdk_draw_line (window, gc1,
                                 x + width - i - 1,
                                 y + i,
                                 x + width - i - 1,
                                 y + height - i - 1);
                  gdk_draw_line (window, gc2,
                                 x + i,
                                 y + i,
                                 x + i,
                                 y + height - i - 2);
                }
      
              for (i = 0; i < thickness_light; i++)
                {
                  gdk_draw_line (window, gc1,
                                 x + thickness_dark + i,
                                 y + thickness_dark + i,
                                 x + thickness_dark + i,
                                 y + height - thickness_dark - i - 1);
                  gdk_draw_line (window, gc2,
                                 x + width - thickness_light - i - 1,
                                 y + thickness_dark + i,
                                 x + width - thickness_light - i - 1,
                                 y + height - thickness_light - 1);
                }
            }
          else
            {
              gdk_draw_line (window, 
                             style->dark_gc[state_type],
                             x, y, x, y + height);                         
              gdk_draw_line (window, 
                             style->dark_gc[state_type],
                             x + width, y, x + width, y + height);
            }
        }

      if (style->ythickness > 0)
        {
          if (style->ythickness > 1)
            {
              thickness_light = 1;
              thickness_dark = 1;
      
              for (i = 0; i < thickness_dark; i++)
                {
                  gdk_draw_line (window, gc1,
                                 x + i,
                                 y + height - i - 1,
                                 x + width - i - 1,
                                 y + height - i - 1);
          
                  gdk_draw_line (window, gc2,
                                 x + i,
                                 y + i,
                                 x + width - i - 2,
                                 y + i);
                }
      
              for (i = 0; i < thickness_light; i++)
                {
                  gdk_draw_line (window, gc1,
                                 x + thickness_dark + i,
                                 y + thickness_dark + i,
                                 x + width - thickness_dark - i - 1,
                                 y + thickness_dark + i);
          
                  gdk_draw_line (window, gc2,
                                 x + thickness_dark + i,
                                 y + height - thickness_light - i - 1,
                                 x + width - thickness_light - 1,
                                 y + height - thickness_light - i - 1);
                }
            }
          else
            {
              gdk_draw_line (window, 
                             style->dark_gc[state_type],
                             x, y, x + width, y);
              gdk_draw_line (window, 
                             style->dark_gc[state_type],
                             x, y + height, x + width, y + height);
            }
        }
      
      break;
    }

  if (area)
    {
      gdk_gc_set_clip_rectangle (gc1, NULL);
      gdk_gc_set_clip_rectangle (gc2, NULL);
      if (shadow_type == GTK_SHADOW_IN || 
          shadow_type == GTK_SHADOW_OUT)
	{
	  gdk_gc_set_clip_rectangle (outer_gc, NULL);
	  gdk_gc_set_clip_rectangle (style->black_gc, NULL);
	}
    }
    

  if (shadow_type == GTK_SHADOW_IN &&
      widget && DETAIL ("entry"))
    {
      gc1 = bluecurve_style->gray_gc[1];
      if (area)
	gdk_gc_set_clip_rectangle (gc1, area);
      
      gdk_draw_line (window, 
		     gc1,
		     x + 1, y + 1, x + width - 2, y + 1);
      
      gdk_draw_line (window, 
		     gc1,
		     x + 1, y + 1, x + 1, y + height - 2);
      if (area)
	gdk_gc_set_clip_rectangle (gc1, NULL);
    }
  
  if (shadow_type == GTK_SHADOW_IN &&
      widget && GTK_IS_SPIN_BUTTON (widget) &&
      detail && strcmp (detail, "entry") == 0)
    {
      gc1 = style->base_gc[GTK_WIDGET_STATE(widget)];
      if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_LTR)
	{
	  gdk_draw_line (window,
			 gc1,
			 x + width - 1, y + 1,
			 x + width - 1, y + height - 2);
	  gdk_draw_point (window,
			  bluecurve_style->gray_gc[1],
			  x + width - 1, y + 1);
	}
      else
	{
	  gdk_draw_line (window,
			 bluecurve_style->gray_gc[1],
			 x, y + 1,
			 x, y + height - 2);
	  gdk_draw_line (window,
			 gc1,
			 x + 1, y + 2,
			 x + 1, y + height - 2); 
	}
    }
}

/**************************************************************************/

static void 
draw_box_gap (GtkStyle       *style,
	      GdkWindow      *window,
	      GtkStateType    state_type,
	      GtkShadowType   shadow_type,
	      GdkRectangle   *area,
	      GtkWidget      *widget,
	      const gchar    *detail,
	      gint            x,
	      gint            y,
	      gint            width,
	      gint            height,
	      GtkPositionType gap_side,
	      gint            gap_x,
	      gint            gap_width)
{
  BluecurveStyle *bluecurve_style = BLUECURVE_STYLE (style);
  GdkGC *outer_gc = NULL;
  GdkGC *gc1 = NULL;
  GdkGC *gc2 = NULL;
  
  g_return_if_fail (GTK_IS_STYLE (style));
  g_return_if_fail (window != NULL);
  
  gtk_style_apply_default_background (style, window,
                                      widget && !GTK_WIDGET_NO_WINDOW (widget),
                                      state_type, area, x, y, width, height);
  
  sanitize_size (window, &width, &height);

  outer_gc = bluecurve_style->gray_gc[6];

  switch (shadow_type)
    {
    case GTK_SHADOW_NONE:
      return;
    case GTK_SHADOW_IN:
      gc1 = bluecurve_style->gray_gc[2];
      gc2 = style->white_gc;
      break;
    case GTK_SHADOW_ETCHED_IN:
      gc1 = bluecurve_style->gray_gc[2];
      gc2 = style->white_gc;
      break;
    case GTK_SHADOW_OUT:
      gc1 = style->white_gc;
      gc2 = bluecurve_style->gray_gc[2];
      break;
    case GTK_SHADOW_ETCHED_OUT:
      gc1 = style->white_gc;
      gc2 = bluecurve_style->gray_gc[2];
      break;
    }

  if (area)
    {
      gdk_gc_set_clip_rectangle (outer_gc, area);
      gdk_gc_set_clip_rectangle (gc1, area);
      gdk_gc_set_clip_rectangle (gc2, area);
    }
  
  switch (shadow_type)
    {
    case GTK_SHADOW_NONE:
    case GTK_SHADOW_IN:
    case GTK_SHADOW_OUT:
    case GTK_SHADOW_ETCHED_IN:
    case GTK_SHADOW_ETCHED_OUT:
      switch (gap_side)
        {
        case GTK_POS_TOP:
          gdk_draw_line (window, outer_gc,
                         x, y, x, y + height - 1);
          gdk_draw_line (window, gc1,
                         x + 1, y, x + 1, y + height - 2);
          
          gdk_draw_line (window, gc2,
                         x + 1, y + height - 2, x + width - 2, y + height - 2);
          gdk_draw_line (window, gc2,
                         x + width - 2, y, x + width - 2, y + height - 2);
          gdk_draw_line (window, outer_gc,
                         x, y + height - 1, x + width - 1, y + height - 1);
          gdk_draw_line (window, outer_gc,
                         x + width - 1, y, x + width - 1, y + height - 1);
          if (gap_x > 0)
            {
              gdk_draw_line (window, outer_gc,
                             x, y, x + gap_x, y);
              gdk_draw_line (window, gc1,
                             x + 1, y + 1, x + gap_x + 1, y + 1);
              gdk_draw_line (window, gc1,
                             x + gap_x + 1, y, x + gap_x + 1, y);
            }
          if ((width - (gap_x + gap_width)) > 0)
            {
              gdk_draw_line (window, outer_gc,
                             x + gap_x + gap_width - 1, y, x + width - 2, y);
              gdk_draw_line (window, gc1,
                             x + gap_x + gap_width - 1, y + 1, x + width - 2, y + 1);
              gdk_draw_line (window, gc2,
                             x + gap_x + gap_width - 2, y, x + gap_x + gap_width - 2, y + 1);
            }
          break;
        case  GTK_POS_BOTTOM:
          gdk_draw_line (window, outer_gc,
                         x, y, x + width - 1, y);
          gdk_draw_line (window, outer_gc,
                         x, y, x, y + height - 1);
          gdk_draw_line (window, gc1,
                         x + 1, y + 1, x + width - 2, y + 1);
          gdk_draw_line (window, gc1,
                         x + 1, y + 1, x + 1, y + height - 1);
          
          gdk_draw_line (window, gc2,
                         x + width - 2, y + 1, x + width - 2, y + height - 1);
          gdk_draw_line (window, outer_gc,
                         x + width - 1, y, x + width - 1, y + height - 1);
          if (gap_x > 0)
            {
              gdk_draw_line (window, outer_gc,
                             x, y + height - 1, x + gap_x, y + height - 1);
              gdk_draw_line (window, gc2,
                             x + 1, y + height - 2, x + gap_x, y + height - 2);
              gdk_draw_line (window, gc1,
                             x + gap_x + 1, y + height - 2, x + gap_x + 1, y + height - 1);
            }
          if ((width - (gap_x + gap_width)) > 0)
            {
              gdk_draw_line (window, outer_gc,
                             x + gap_x + gap_width - 1, y + height - 1, x + width - 2, y + height - 1);
              gdk_draw_line (window, gc2,
                             x + gap_x + gap_width - 1, y + height - 2, x + width - 2, y + height - 2);
              gdk_draw_line (window, gc2,
                             x + gap_x + gap_width - 2, y + height - 2, x + gap_x + gap_width - 2, y + height - 1);
            }
          break;
        case GTK_POS_LEFT:
          gdk_draw_line (window, outer_gc,
                         x, y, x + width - 1, y);
          gdk_draw_line (window, gc1,
                         x, y + 1, x + width - 2, y + 1);
          
          gdk_draw_line (window, gc2,
                         x, y + height - 2, x + width - 2, y + height - 2);
          gdk_draw_line (window, gc2,
                         x + width - 2, y + 1, x + width - 2, y + height - 2);
          gdk_draw_line (window, outer_gc,
                         x, y + height - 1, x + width - 1, y + height - 1);
          gdk_draw_line (window, outer_gc,
                         x + width - 1, y, x + width - 1, y + height - 1);
          if (gap_x > 0)
            {
              gdk_draw_line (window, outer_gc,
                             x, y, x, y + gap_x);
              gdk_draw_line (window, gc1,
                             x + 1, y + 1, x + 1, y + gap_x);
              gdk_draw_line (window, gc1,
                             x, y + gap_x + 1, x + 1, y + gap_x + 1);
            }
          if ((width - (gap_x + gap_width)) > 0)
            {
              gdk_draw_line (window, outer_gc,
                             x, y + gap_x + gap_width - 1, x, y + height - 2);
              gdk_draw_line (window, gc1,
                             x + 1, y + gap_x + gap_width - 1, x + 1, y + height - 2);
              gdk_draw_line (window, gc2,
                             x, y + gap_x + gap_width - 2, x + 1, y + gap_x + gap_width - 2);
            }
          break;
        case GTK_POS_RIGHT:
          gdk_draw_line (window, outer_gc,
                         x, y, x + width - 1, y);
          gdk_draw_line (window, outer_gc,
                         x, y, x, y + height - 1);
          gdk_draw_line (window, gc1,
                         x + 1, y + 1, x + width - 1, y + 1);
          gdk_draw_line (window, gc1,
                         x + 1, y + 1, x + 1, y + height - 2);
          
          gdk_draw_line (window, gc2,
                         x + 1, y + height - 2, x + width - 1, y + height - 2);
          gdk_draw_line (window, outer_gc,
                         x, y + height - 1, x + width - 1, y + height - 1);
          if (gap_x > 0)
            {
              gdk_draw_line (window, outer_gc,
                             x + width - 1, y, x + width - 1, y + gap_x);
              gdk_draw_line (window, gc2,
                             x + width - 2, y + 1, x + width - 2, y + gap_x);
              gdk_draw_line (window, gc1,
                             x + width - 2, y + gap_x + 1, x + width - 1, y + gap_x + 1);
            }
          if ((width - (gap_x + gap_width)) > 0)
            {
              gdk_draw_line (window, outer_gc,
                             x + width - 1, y + gap_x + gap_width - 1, x + width - 1, y + height - 2);
              gdk_draw_line (window, gc2,
                             x + width - 2, y + gap_x + gap_width - 1, x + width - 2, y + height - 2);
              gdk_draw_line (window, gc2,
                             x + width - 2, y + gap_x + gap_width - 2, x + width - 1, y + gap_x + gap_width - 2);
            }
          break;
        }
    }

  if (area)
    {
      gdk_gc_set_clip_rectangle (outer_gc, NULL);
      gdk_gc_set_clip_rectangle (gc1, NULL);
      gdk_gc_set_clip_rectangle (gc2, NULL);
    }
}

/**************************************************************************/

static void 
draw_extension (GtkStyle       *style,
		GdkWindow      *window,
		GtkStateType    state_type,
		GtkShadowType   shadow_type,
		GdkRectangle   *area,
		GtkWidget      *widget,
		const gchar    *detail,
		gint            x,
		gint            y,
		gint            width,
		gint            height,
		GtkPositionType gap_side)
{
  BluecurveStyle *bluecurve_style = BLUECURVE_STYLE (style);
  GdkGC *outer_gc = NULL;
  GdkGC *gc1 = NULL;
  GdkGC *gc2 = NULL;
  
  g_return_if_fail (GTK_IS_STYLE (style));
  g_return_if_fail (window != NULL);
  
  gtk_style_apply_default_background (style, window,
                                      widget && !GTK_WIDGET_NO_WINDOW (widget),
                                      GTK_STATE_NORMAL, area, x, y, width, height);
  
  sanitize_size (window, &width, &height);
  
  outer_gc = bluecurve_style->gray_gc[6];

  switch (shadow_type)
    {
    case GTK_SHADOW_NONE:
      return;
    case GTK_SHADOW_IN:
      gc1 = bluecurve_style->gray_gc[2];
      gc2 = style->white_gc;
      break;
    case GTK_SHADOW_ETCHED_IN:
      gc1 = bluecurve_style->gray_gc[2];
      gc2 = style->white_gc;
      break;
    case GTK_SHADOW_OUT:
      gc1 = style->white_gc;
      gc2 = bluecurve_style->gray_gc[2];
      break;
    case GTK_SHADOW_ETCHED_OUT:
      gc1 = style->white_gc;
      gc2 = bluecurve_style->gray_gc[2];
      break;
    }

  if (area)
    {
      gdk_gc_set_clip_rectangle (outer_gc, area);
      gdk_gc_set_clip_rectangle (gc1, area);
      gdk_gc_set_clip_rectangle (gc2, area);
    }

  switch (shadow_type)
    {
    case GTK_SHADOW_NONE:
    case GTK_SHADOW_IN:
    case GTK_SHADOW_OUT:
    case GTK_SHADOW_ETCHED_IN:
    case GTK_SHADOW_ETCHED_OUT:
      switch (gap_side)
        {
        case GTK_POS_TOP:
          gtk_style_apply_default_background (style, window,
                                              widget && !GTK_WIDGET_NO_WINDOW (widget),
                                              state_type, area,
                                              x + style->xthickness, 
                                              y, 
                                              width - (2 * style->xthickness), 
                                              height - (style->ythickness));
          gdk_draw_line (window, outer_gc,
                         x, y, x, y + height - 2);
          gdk_draw_line (window, gc1,
                         x + 1, y, x + 1, y + height - 2);
          
          gdk_draw_line (window, gc2,
                         x + 2, y + height - 2, x + width - 2, y + height - 2);
          gdk_draw_line (window, gc2,
                         x + width - 2, y, x + width - 2, y + height - 2);
          gdk_draw_line (window, outer_gc,
                         x, y + height - 1, x + width - 1, y + height - 1);
          gdk_draw_line (window, outer_gc,
                         x + width - 1, y, x + width - 1, y + height - 2);
          break;
        case GTK_POS_BOTTOM:
          gtk_style_apply_default_background (style, window,
                                              widget && !GTK_WIDGET_NO_WINDOW (widget),
                                              state_type, area,
                                              x + style->xthickness, 
                                              y + style->ythickness, 
                                              width - (2 * style->xthickness), 
                                              height - (style->ythickness));
          gdk_draw_line (window, outer_gc,
                         x, y, x + width - 1, y);
          gdk_draw_line (window, outer_gc,
                         x, y + 1, x, y + height - 1);
          gdk_draw_line (window, gc1,
                         x + 1, y + 1, x + width - 2, y + 1);
          gdk_draw_line (window, gc1,
                         x + 1, y + 1, x + 1, y + height - 1);
          
          gdk_draw_line (window, gc2,
                         x + width - 2, y + 1, x + width - 2, y + height - 1);
          gdk_draw_line (window, outer_gc,
                         x + width - 1, y + 1, x + width - 1, y + height - 1);
          break;
        case GTK_POS_LEFT:
          gtk_style_apply_default_background (style, window,
                                              widget && !GTK_WIDGET_NO_WINDOW (widget),
                                              state_type, area,
                                              x, 
                                              y + style->ythickness, 
                                              width - (style->xthickness), 
                                              height - (2 * style->ythickness));
          gdk_draw_line (window, outer_gc,
                         x, y, x + width - 2, y);
          gdk_draw_line (window, gc1,
                         x, y + 1, x + width - 2, y + 1);
          
          gdk_draw_line (window, gc2,
                         x, y + height - 2, x + width - 2, y + height - 2);
          gdk_draw_line (window, gc2,
                         x + width - 2, y + 2, x + width - 2, y + height - 2);
          gdk_draw_line (window, outer_gc,
                         x, y + height - 1, x + width - 2, y + height - 1);
          gdk_draw_line (window, outer_gc,
                         x + width - 1, y, x + width - 1, y + height - 1);
          break;
        case GTK_POS_RIGHT:
          gtk_style_apply_default_background (style, window,
                                              widget && !GTK_WIDGET_NO_WINDOW (widget),
                                              state_type, area,
                                              x + style->xthickness, 
                                              y + style->ythickness, 
                                              width - (style->xthickness), 
                                              height - (2 * style->ythickness));
          gdk_draw_line (window, outer_gc,
                         x + 1, y, x + width - 1, y);
          gdk_draw_line (window, outer_gc,
                         x, y, x, y + height - 1);
          gdk_draw_line (window, gc1,
                         x + 1, y + 1, x + width - 1, y + 1);
          gdk_draw_line (window, gc1,
                         x + 1, y + 1, x + 1, y + height - 1);
          
          gdk_draw_line (window, gc2,
                         x + 1, y + height - 2, x + width - 1, y + height - 2);
          gdk_draw_line (window, outer_gc,
                         x + 1, y + height - 1, x + width - 1, y + height - 1);
          break;
        }
    }

  if (area)
    {
      gdk_gc_set_clip_rectangle (outer_gc, NULL);
      gdk_gc_set_clip_rectangle (gc1, NULL);
      gdk_gc_set_clip_rectangle (gc2, NULL);
    }
}
    

/**************************************************************************/

static void 
draw_handle (GtkStyle      *style,
	     GdkWindow     *window,
	     GtkStateType   state_type,
	     GtkShadowType  shadow_type,
	     GdkRectangle  *area,
	     GtkWidget     *widget,
	     const gchar   *detail,
	     gint           x,
	     gint           y,
	     gint           width,
	     gint           height,
	     GtkOrientation orientation)
{
  BluecurveStyle *bluecurve_style = BLUECURVE_STYLE (style);
  gint xx, yy;
  gint xthick, ythick;
  GdkGC *light_gc, *dark_gc;
  GdkRectangle rect;
  GdkRectangle dest;
  gint intersect;
  gint h;
  int i;
  int n_lines;
  int offset;
  
  g_return_if_fail (GTK_IS_STYLE (style));
  g_return_if_fail (window != NULL);
  
  sanitize_size (window, &width, &height);
  
  gtk_style_apply_default_background (style, window,
				      widget && !GTK_WIDGET_NO_WINDOW (widget),
				      state_type, area, x, y, width, height);

  if (DETAIL ("dockitem") ||
      (widget && strcmp (g_type_name (G_TYPE_FROM_INSTANCE (widget)), "PanelAppletFrame") == 0))
    {
      /* Work around orientation bugs */
      if (orientation == GTK_ORIENTATION_VERTICAL)
	orientation = GTK_ORIENTATION_HORIZONTAL;
      else
	orientation = GTK_ORIENTATION_VERTICAL;
      
    }
  
  if (!strcmp (detail, "paned"))
    {
      /* we want to ignore the shadow border in paned widgets */
      xthick = 0;
      ythick = 0;
    }
  else
    {
      xthick = style->xthickness;
      ythick = style->ythickness;
    }

  if ( (DETAIL ("handlebox") && widget && GTK_IS_HANDLE_BOX (widget)) ||
       DETAIL ("dockitem"))
    {
      /* The line in the toolbar */

      light_gc = style->white_gc;
      dark_gc = bluecurve_style->gray_gc[3];
      
      if (area)
	{
	  gdk_gc_set_clip_rectangle (light_gc, area);
	  gdk_gc_set_clip_rectangle (dark_gc, area);
	}
     
      if (area)
	{
	  gdk_gc_set_clip_rectangle (light_gc, NULL);
	  gdk_gc_set_clip_rectangle (dark_gc, NULL);
	}

      if (area)
	{
	  gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[0], area);
	  gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[3], area);
	}
      
      gdk_draw_line (window, bluecurve_style->gray_gc[0],
		     x, y, x + width, y);
      gdk_draw_line (window, bluecurve_style->gray_gc[3],
		     x, y + height - 1, x + width, y + height - 1);
      
      if (area)
	{
	  gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[0], NULL);
	  gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[3], NULL);
	}
    }

	light_gc = style->white_gc;
	dark_gc = bluecurve_style->gray_gc[5];
  
  rect.x = x + xthick;
  rect.y = y + ythick;
  rect.width = width - (xthick * 2);
  rect.height = height - (ythick * 2);

  if (area)
      intersect = gdk_rectangle_intersect (area, &rect, &dest);
  else
    {
      intersect = TRUE;
      dest = rect;
    }

  if (!intersect)
    return;

  gdk_gc_set_clip_rectangle (light_gc, &dest);
  gdk_gc_set_clip_rectangle (dark_gc, &dest);


  n_lines = 5;

  if (orientation == GTK_ORIENTATION_VERTICAL)
    {
      h = width - 2 * xthick;
      h = MAX (3, h - 6);
      
      xx = x + (width - h) / 2;
      offset = (height - 2 * ythick - 4 * n_lines)/2 - 1;
      if (offset < 0)
	offset = 0;
      
      for (i = 0, yy = y + ythick + offset; yy <= (y + height - ythick - h) && i < n_lines; yy += h + 2, i++)
	{
	  gdk_draw_line (window, dark_gc,
			 xx, yy + h,
			 xx + h, yy);
	  gdk_draw_line (window, light_gc,
			 xx, yy + h + 1,
			 xx + h, yy + 1);
	}
    }
  else
    {
      h = height - 2 * ythick;
      h = MAX (3, h - 6);
      
      yy = y + (height - h) / 2;
      offset = (width - 2 * xthick - 4 * n_lines)/2;
      if (offset < 0)
	offset = 0;
      
      for (i = 0, xx = x + xthick + offset; xx <= (x + width - xthick - h) && i < n_lines; xx += h + 2, i++)
	{
	  gdk_draw_line (window, dark_gc,
			 xx, yy + h,
			 xx + h, yy);
	  gdk_draw_line (window, light_gc,
			 xx + 1, yy + h,
			 xx + h + 1, yy);
	}
    }
  
  gdk_gc_set_clip_rectangle (light_gc, NULL);
  gdk_gc_set_clip_rectangle (dark_gc, NULL);
}

/**************************************************************************/

static void 
draw_flat_box (GtkStyle      *style,
	       GdkWindow     *window,
	       GtkStateType   state_type,
	       GtkShadowType  shadow_type,
	       GdkRectangle  *area,
	       GtkWidget     *widget,
	       const gchar   *detail,
	       gint           x,
	       gint           y,
	       gint           width,
	       gint           height)
{
  parent_class->draw_box (style, window, state_type, shadow_type, area,
			  widget, detail, x, y, width, height);
}

/**************************************************************************/

static void
draw_box (GtkStyle      *style,
	  GdkWindow     *window,
	  GtkStateType   state_type,
	  GtkShadowType  shadow_type,
	  GdkRectangle  *area,
	  GtkWidget     *widget,
	  const gchar   *detail,
	  gint           x,
	  gint           y,
	  gint           width,
	  gint           height)
{
  BluecurveStyle *bluecurve_style = BLUECURVE_STYLE (style);
  
  g_return_if_fail (style != NULL);
  g_return_if_fail (window != NULL);

  if ((width == -1) && (height == -1))
    gdk_window_get_size (window, &width, &height);
  else if (width == -1)
    gdk_window_get_size (window, &width, NULL);
  else if (height == -1)
    gdk_window_get_size (window, NULL, &height);

#if DEBUG
  printf ("draw_box: %p %p %s %i %i %i %i\n", widget, window, detail, x, y,
	  width, height);
#endif

  /* ===================================================================== */
  /*
  if (DETAIL ("button") || DETAIL ("togglebutton"))
    {
      bluecurve_button (style, window, state_type, shadow_type,
			 area, widget, detail, x, y, width, height);
    }
    else */
  if (widget && DETAIL ("trough")) /* Scrollbars */
    {
      if (area)
	{
	  gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[3], area);
	  gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[4], area);
	  gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[5], area);
	}

      if GTK_IS_HSCALE (widget)
	{
	  y += (height - SCALE_SIZE) / 2;
	  height = SCALE_SIZE;
	}
      
      if GTK_IS_VSCALE (widget)
	{
	  x += (width - SCALE_SIZE) / 2;
	  width = SCALE_SIZE;
	}
      
      gdk_draw_rectangle (window, bluecurve_style->gray_gc[3],
			  TRUE, x, y, width, height);
      gdk_draw_rectangle (window, bluecurve_style->gray_gc[5],
			  FALSE, x, y, width - 1, height - 1);

      if (GTK_IS_HSCALE (widget) || GTK_IS_VSCALE (widget))
	{
	  gdk_draw_line (window, bluecurve_style->gray_gc[4],
			 x + 1, y + 1,
			 x + width - 2, y + 1);
	  gdk_draw_line (window, bluecurve_style->gray_gc[4],
			 x + 1, y + 1,
			 x + 1, y + height - 2);
	}

      
      if (area)
	{
	  gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[3], NULL);
	  gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[4], NULL);
	  gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[5], NULL);
	}

    }
  else if (widget && (DETAIL ("menubar") || DETAIL ("toolbar") || DETAIL ("dockitem_bin"))
	   && shadow_type != GTK_SHADOW_NONE)
    {
      /* Toolbars and menus */
      gtk_style_apply_default_background (style, window,
					  widget && !GTK_WIDGET_NO_WINDOW (widget),
					  state_type, area, x, y, width, height);
      
      if (area)
	{
	  gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[0], area);
	  gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[3], area);
	}

      if (!DETAIL ("menubar")) /* Only toolbars have the top line */
	gdk_draw_line (window, bluecurve_style->gray_gc[0],
		       x, y, x + width, y);
      gdk_draw_line (window, bluecurve_style->gray_gc[3],
		     x, y + height - 1, x + width, y + height - 1);
      
      if (area)
	{
	  gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[0], NULL);
	  gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[3], NULL);
	}
    }
  else if (widget && DETAIL ("bar") && GTK_IS_PROGRESS_BAR (widget))
    {
      GdkColor *spot_color = bluecurve_get_spot_color (BLUECURVE_RC_STYLE (style->rc_style));
      GdkColor upper_color, lower_color;
      GtkProgressBarOrientation orientation; 

      shade (spot_color, &upper_color, 0.92);
      shade (spot_color, &lower_color, 1.66);
      
      if (area)
	{
	  gdk_gc_set_clip_rectangle (style->base_gc[state_type], area);
	  gdk_gc_set_clip_rectangle (bluecurve_style->spot1_gc, area);
	  gdk_gc_set_clip_rectangle (bluecurve_style->spot2_gc, area);
	  gdk_gc_set_clip_rectangle (bluecurve_style->spot3_gc, area);
	}
      
      gdk_draw_rectangle (window,
			  bluecurve_style->spot2_gc,
			  FALSE,
			  x - 1, y - 1,
			  width + 1,  height + 1);


      gdk_draw_line (window,
		     bluecurve_style->spot1_gc,
		     x, y,
		     x + width - 1,  y);
      gdk_draw_line (window,
		     bluecurve_style->spot1_gc,
		     x, y,
		     x, y + height - 1);

      gdk_draw_line (window,
		     bluecurve_style->spot3_gc,
		     x + width - 1, y + 1,
		     x + width - 1, y + height - 1);
      gdk_draw_line (window,
		     bluecurve_style->spot3_gc,
		     x + 1, y + height - 1,
		     x + width - 1, y + height - 1);
      
      orientation = gtk_progress_bar_get_orientation (GTK_PROGRESS_BAR (widget));
      
      if (orientation == GTK_PROGRESS_LEFT_TO_RIGHT ||
	  orientation == GTK_PROGRESS_RIGHT_TO_LEFT)
	{
	  if (width > 2)
	    draw_hgradient (window, style->base_gc[state_type], style->colormap,
			    x + 1, y + 1, width - 2, height - 2,
			    &upper_color, &lower_color);
	}
      else
	{
	  if (height > 2)
	    draw_vgradient (window, style->base_gc[state_type], style->colormap,
			    x + 1, y + 1, width - 2, height - 2,
			    &upper_color, &lower_color);
	}
	
      
      if (area)
	{
	  gdk_gc_set_clip_rectangle (style->base_gc[state_type], NULL);
	  gdk_gc_set_clip_rectangle (bluecurve_style->spot1_gc, NULL);
	  gdk_gc_set_clip_rectangle (bluecurve_style->spot2_gc, NULL);
	  gdk_gc_set_clip_rectangle (bluecurve_style->spot3_gc, NULL);
	}
    }
  else if (widget && DETAIL ("menuitem"))
    {
      GdkColor *spot_color = bluecurve_get_spot_color (BLUECURVE_RC_STYLE (style->rc_style));
      GdkColor upper_color, lower_color;

      shade (spot_color, &upper_color, 0.9);
      shade (spot_color, &lower_color, 1.2);
      
      if (area)
	gdk_gc_set_clip_rectangle (style->base_gc[state_type], area);

      draw_hgradient (window, style->base_gc[state_type], style->colormap,
		      x, y, width, height,
		      &upper_color, &lower_color);
      
      if (area)
	gdk_gc_set_clip_rectangle (style->base_gc[state_type], NULL);

      
      gtk_paint_shadow (style, window, state_type, shadow_type, area, widget, detail,
			x, y, width, height);
      
    }
  else if (widget && DETAIL ("optionmenu"))
    {
      GtkRequisition indicator_size;
      GtkBorder indicator_spacing;
      int line_pos;
      
      if (!style->bg_pixmap[state_type] || 
	  GDK_IS_PIXMAP (window))
	{
	  if (area)
	    gdk_gc_set_clip_rectangle (style->bg_gc[state_type], area);
	  
	  gdk_draw_rectangle (window, style->bg_gc[state_type], TRUE,
			      x, y, width, height);
	  if (area)
	    gdk_gc_set_clip_rectangle (style->bg_gc[state_type], NULL);
	}
      else
	gtk_style_apply_default_background (style, window,
					    widget && !GTK_WIDGET_NO_WINDOW (widget),
					    state_type, area, x, y, width, height);


      gtk_paint_shadow (style, window, state_type, shadow_type, area, widget, detail,
			x, y, width, height);
      
      option_menu_get_props (widget, &indicator_size, &indicator_spacing);

      if (area)
	{
	  gdk_gc_set_clip_rectangle (style->white_gc, area);
	  gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[3], area);
	}
      
      if (get_direction (widget) == GTK_TEXT_DIR_RTL)
	line_pos = x + (indicator_size.width + indicator_spacing.left + indicator_spacing.right) + style->xthickness;
      else
	line_pos = x + width - (indicator_size.width + indicator_spacing.left + indicator_spacing.right) - style->xthickness;
	
      gdk_draw_line (window, bluecurve_style->gray_gc[3],
		     line_pos,
		     y + style->ythickness - 1,
		     line_pos,
		     y + height - style->ythickness);
	
      gdk_draw_line (window, style->white_gc,
		     line_pos + 1,
		     y + style->ythickness,
		     line_pos + 1,
		     y + height - style->ythickness - 1);
	
		     
      if (area)
	{
	  gdk_gc_set_clip_rectangle (style->white_gc, NULL);
	  gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[3], NULL);
	}
    }
  else if (widget &&
	   (DETAIL ("hscale") || DETAIL ("vscale")))
    {
      GdkGC *outer_gc = NULL;
      GdkGC *gc1 = NULL;
      GdkGC *gc2 = NULL;

      
      gtk_style_apply_default_background (style, window,
					  widget && !GTK_WIDGET_NO_WINDOW (widget),
					  state_type, area, x + 1, y + 1, width - 2, height - 2);


      outer_gc = bluecurve_style->gray_gc[6];

      gc1 = style->white_gc;
      gc2 = bluecurve_style->gray_gc[2];

      if (area)
	{
	  gdk_gc_set_clip_rectangle (outer_gc, area);
	  gdk_gc_set_clip_rectangle (gc1, area);
	  gdk_gc_set_clip_rectangle (gc2, area);
	}

      gdk_draw_line (window, outer_gc,
		     x + 2, y, x + width - 3, y);
      gdk_draw_point (window, gc2,
		      x + 1, y);
      gdk_draw_point (window, gc2,
		      x + width - 2, y);
      
      gdk_draw_point (window, gc2,
		      x, y + 1);
      gdk_draw_point (window, outer_gc,
		      x + 1, y + 1);
      gdk_draw_line (window, gc1,
		     x + 2, y + 1, x + width - 3, y + 1);
      gdk_draw_point (window, outer_gc,
		      x + width - 2, y + 1);
      gdk_draw_point (window, gc2,
		      x + width - 1, y + 1);
      
      gdk_draw_line (window, outer_gc,
		     x, y + 2, x, y + height - 3);
      gdk_draw_line (window, gc1,
		     x + 1, y + 2, x + 1, y + height - 3);
      
      gdk_draw_line (window, gc2,
		     x + width - 2, y + 2, x + width - 2, y + height - 3);
      gdk_draw_line (window, outer_gc,
		     x + width - 1, y + 2, x + width - 1, y + height - 3);
      
      gdk_draw_point (window, gc2,
		      x, y + height - 2);
      gdk_draw_point (window, outer_gc,
		      x + 1, y + height - 2);
      gdk_draw_line (window, gc2,
		     x + 2, y + height - 2, x + width - 2, y + height - 2);
      gdk_draw_point (window, outer_gc,
		      x + width - 2, y + height - 2);
      gdk_draw_point (window, gc2,
		      x + width - 1, y + height - 2);
      
      gdk_draw_point (window, gc2,
		      x + 1, y + height - 1);
      gdk_draw_line (window, outer_gc,
		     x + 2, y + height - 1, x + width - 3, y + height - 1);
      gdk_draw_point (window, gc2,
		      x + width - 2, y + height - 1);

      if (area)
	{
	  gdk_gc_set_clip_rectangle (outer_gc, NULL);
	  gdk_gc_set_clip_rectangle (gc1, NULL);
	  gdk_gc_set_clip_rectangle (gc2, NULL);
	}

    }
  else if (DETAIL ("spinbutton_up") || DETAIL ("spinbutton_down"))
    {
      /* Make the top button overlap the first line of the bottom button */
      if (strcmp (detail, "spinbutton_up") == 0)
	height += 1;
      
      gtk_style_apply_default_background (style, window,
					  widget && !GTK_WIDGET_NO_WINDOW (widget),
					  state_type, area, x, y, width, height);
      gtk_paint_shadow (style, window, state_type, shadow_type, area, widget, detail,
			x, y, width, height);
    }
  else
    {
      parent_class->draw_box (style, window, state_type, shadow_type, area,
			      widget, detail, x, y, width, height);
    }
}

/**************************************************************************/

static void
ensure_check_pixmaps (GtkStyle     *style,
		      GtkStateType  state,
		      GdkScreen    *screen)
{
  BluecurveStyle *bluecurve_style = BLUECURVE_STYLE (style);
  BluecurveRcStyle *bluecurve_rc = BLUECURVE_RC_STYLE (style->rc_style);
  GdkPixbuf *check, *base, *inconsistent, *composite;
  GdkColor *spot_color = bluecurve_get_spot_color (bluecurve_rc);
  
  if (bluecurve_style->check_pixmap_nonactive[state] != NULL)
    return;

  check = generate_bit (check_alpha, spot_color, 1.0);
  inconsistent = generate_bit (check_inconsistent_alpha, spot_color, 1.0);

  if (state == GTK_STATE_ACTIVE)
    base = generate_bit (check_base_alpha, &style->bg[state], 1.0);
  else
    base = generate_bit (check_base_alpha, &style->white, 1.0);
  
  composite = generate_bit (NULL, &bluecurve_style->gray[6], 1.0);
  
  gdk_pixbuf_composite (base, composite,
			0, 0, RADIO_SIZE, RADIO_SIZE, 0, 0,
			1.0, 1.0, GDK_INTERP_NEAREST, 255);
  
  bluecurve_style->check_pixmap_nonactive[state] =
    pixbuf_to_pixmap (style, composite, screen);
  
  gdk_pixbuf_composite (check, composite,
			0, 0, RADIO_SIZE, RADIO_SIZE, 0, 0,
			1.0, 1.0, GDK_INTERP_NEAREST, 255);
  
  bluecurve_style->check_pixmap_active[state] =
    pixbuf_to_pixmap (style, composite, screen);
  
  g_object_unref (composite);
  
  composite = generate_bit (NULL, &bluecurve_style->gray[6], 1.0);
  
  gdk_pixbuf_composite (base, composite,
			0, 0, RADIO_SIZE, RADIO_SIZE, 0, 0,
			1.0, 1.0, GDK_INTERP_NEAREST, 255);
  gdk_pixbuf_composite (inconsistent, composite,
			0, 0, RADIO_SIZE, RADIO_SIZE, 0, 0,
			1.0, 1.0, GDK_INTERP_NEAREST, 255);
  
  bluecurve_style->check_pixmap_inconsistent[state] =
    pixbuf_to_pixmap (style, composite, screen);
  
  g_object_unref (composite);
  g_object_unref (base);
  g_object_unref (check);
  g_object_unref (inconsistent);
}

static void 
draw_check (GtkStyle      *style,
	    GdkWindow     *window,
	    GtkStateType   state_type,
	    GtkShadowType  shadow_type,
	    GdkRectangle  *area,
	    GtkWidget     *widget,
	    const gchar   *detail,
	    gint           x,
	    gint           y,
	    gint           width,
	    gint           height)
{
  BluecurveStyle *bluecurve_style = BLUECURVE_STYLE (style);
  GdkGC *gc = style->base_gc[state_type];
  GdkPixmap *pixmap;
  
  if (DETAIL ("check")) /* Menu item */
    {
      parent_class->draw_check (style, window, state_type, shadow_type, area,
				 widget, detail, x, y, width, height);
      return;
    }

  ensure_check_pixmaps (style,
			state_type,
			gtk_widget_get_screen (widget));
  
  if (area)
    gdk_gc_set_clip_rectangle (gc, area);

  if (shadow_type == GTK_SHADOW_IN)
    pixmap = bluecurve_style->check_pixmap_active[state_type];
  else if (shadow_type == GTK_SHADOW_ETCHED_IN) /* inconsistent */
    pixmap = bluecurve_style->check_pixmap_inconsistent[state_type];
  else
    pixmap = bluecurve_style->check_pixmap_nonactive[state_type];

  x += (width - CHECK_SIZE)/2;
  y += (height - CHECK_SIZE)/2;
  
  gdk_draw_drawable (window, gc,
		     pixmap,
		     0, 0,
		     x, y,
		     CHECK_SIZE, CHECK_SIZE);
  
  if (area)
    gdk_gc_set_clip_rectangle (gc, NULL);
}

/**************************************************************************/
static void
draw_slider (GtkStyle      *style,
	     GdkWindow     *window,
	     GtkStateType   state_type,
	     GtkShadowType  shadow_type,
	     GdkRectangle  *area,
	     GtkWidget     *widget,
	     const gchar   *detail,
	     gint           x,
	     gint           y,
	     gint           width,
	     gint           height,
	     GtkOrientation orientation)
{
  BluecurveStyle *bluecurve_style = BLUECURVE_STYLE (style);
  int x1, y1;
  
  g_return_if_fail (GTK_IS_STYLE (style));
  g_return_if_fail (window != NULL);
  
  sanitize_size (window, &width, &height);
  
  gtk_paint_box (style, window, state_type, shadow_type,
                 area, widget, detail, x, y, width, height);

  if (detail &&
      strcmp ("slider", detail) == 0)
    {
      if (area)
	{
	  gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[5], area);
	  gdk_gc_set_clip_rectangle (style->white_gc, area);
	}
      if (orientation == GTK_ORIENTATION_HORIZONTAL)
	{
	  x1 = x + width / 2 - 8;
	  y1 = y + (height - 6) / 2;
	  gdk_draw_line (window, bluecurve_style->gray_gc[5],
                         x1 + 5, y1,
			 x1, y1 + 5);
	  gdk_draw_line (window, style->white_gc,
                         x1 + 5, y1 + 1,
			 x1 + 1, y1 + 5);
	  gdk_draw_line (window, bluecurve_style->gray_gc[5],
                         x1 + 5 + 5, y1,
			 x1 + 5, y1 + 5);
	  gdk_draw_line (window, style->white_gc,
                         x1 + 5 + 5, y1 + 2,
			 x1 + 5 + 1, y1 + 5);
	  gdk_draw_line (window, bluecurve_style->gray_gc[5],
                         x1 + 5 + 5*2, y1,
			 x1 + 5*2, y1 + 5);
	  gdk_draw_line (window, style->white_gc,
                         x1 + 5 + 5*2, y1 + 1,
			 x1 + 5*2 + 1, y1 + 5);
	}
      else
	{
	  x1 = x + (width - 6) / 2;
	  y1 = y + height / 2 - 8;
	  gdk_draw_line (window, bluecurve_style->gray_gc[5],
                         x1 + 5, y1,
			 x1, y1 + 5);
	  gdk_draw_line (window, style->white_gc,
                         x1 + 5, y1 + 1,
			 x1 + 1, y1 + 5);
	  gdk_draw_line (window, bluecurve_style->gray_gc[5],
                         x1 + 5, y1 + 5,
			 x1, y1 + 5 + 5);
	  gdk_draw_line (window, style->white_gc,
                         x1 + 5, y1 + 5 + 1,
			 x1 + 1, y1 + 5 + 5);
	  gdk_draw_line (window, bluecurve_style->gray_gc[5],
                         x1 + 5, y1 + 5*2,
			 x1, y1 + 5 + 5*2);
	  gdk_draw_line (window, style->white_gc,
                         x1 + 5, y1 + 5*2 + 1,
			 x1 + 1, y1 + 5 + 5*2);
	}
      if (area)
	{
	  gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[5], NULL);
	  gdk_gc_set_clip_rectangle (style->white_gc, NULL);
	}
    }
  if (detail &&
      (strcmp ("hscale", detail) == 0 ||
       strcmp ("vscale", detail) == 0))
    {
      if (area)
	{
	  gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[5], area);
	  gdk_gc_set_clip_rectangle (style->white_gc, area);
	}
      
      if (orientation == GTK_ORIENTATION_HORIZONTAL)
	{
	  x1 = x + width / 2 - 5;
	  y1 = y + (height - 7) / 2;
	  gdk_draw_line (window, bluecurve_style->gray_gc[5],
                         x1 + 0, y1 + 4,
			 x1 + 3, y1 + 1);
	  gdk_draw_line (window, style->white_gc,
                         x1 + 1, y1 + 4,
			 x1 + 3, y1 + 2);
	  gdk_draw_line (window, bluecurve_style->gray_gc[5],
                         x1 + 2, y1 + 6,
			 x1 + 8, y1 + 0);
	  gdk_draw_line (window, style->white_gc,
                         x1 + 3, y1 + 6,
			 x1 + 8, y1 + 1);
	  gdk_draw_line (window, bluecurve_style->gray_gc[5],
                         x1 + 7,  y1 + 5,
			 x1 + 10, y1 + 2);
	  gdk_draw_line (window, style->white_gc,
                         x1 + 8,  y1 + 5,
			 x1 + 10, y1 + 3);
	}
      else
	{
	  x1 = x + (width - 7) / 2;
	  y1 = y + height / 2 - 5;
	  gdk_draw_line (window, bluecurve_style->gray_gc[5],
                         x1 + 4, y1 + 0,
			 x1 + 1, y1 + 3);
	  gdk_draw_line (window, style->white_gc,
                         x1 + 4, y1 + 1,
			 x1 + 2, y1 + 3);
	  gdk_draw_line (window, bluecurve_style->gray_gc[5],
                         x1 + 6, y1 + 2,
			 x1 + 0, y1 + 8);
	  gdk_draw_line (window, style->white_gc,
                         x1 + 6, y1 + 3,
			 x1 + 1, y1 + 8);
	  gdk_draw_line (window, bluecurve_style->gray_gc[5],
                         x1 + 5, y1 + 7,
			 x1 + 2, y1 + 10);
	  gdk_draw_line (window, style->white_gc,
                         x1 + 5, y1 + 8,
			 x1 + 3, y1 + 10);
	}
      if (area)
	{
	  gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[5], NULL);
	  gdk_gc_set_clip_rectangle (style->white_gc, NULL);
	}
    }
  
}

/**************************************************************************/
static void
ensure_radio_pixmaps (GtkStyle     *style,
		      GtkStateType  state,
		      GdkScreen    *screen)
{
  BluecurveStyle *bluecurve_style = BLUECURVE_STYLE (style);
  BluecurveRcStyle *bluecurve_rc = BLUECURVE_RC_STYLE (style->rc_style);
  GdkPixbuf *dot, *circle, *outline, *inconsistent, *composite;
  GdkColor *spot_color = bluecurve_get_spot_color (bluecurve_rc);
  GdkColor *composite_color;
  
  if (bluecurve_style->radio_pixmap_nonactive[state] != NULL)
    return;

  dot = colorize_bit (dot_intensity, dot_alpha, spot_color);
  inconsistent = generate_bit (inconsistent_alpha, spot_color, 1.0);
  outline = generate_bit (outline_alpha, &bluecurve_style->gray[6], 1.0);
  
  if (bluecurve_style->radio_pixmap_mask == NULL) 
    gdk_pixbuf_render_pixmap_and_mask (outline,
				       NULL,
				       &bluecurve_style->radio_pixmap_mask,
				       1);

  if (state == GTK_STATE_ACTIVE)
    {
      composite_color = &style->bg[GTK_STATE_PRELIGHT];
      circle = generate_bit (circle_alpha, &style->bg[state], 1.0);
    }
  else
    {
      composite_color = &style->bg[state];
      circle = generate_bit (circle_alpha, &style->white, 1.0);
    }

  composite = generate_bit (NULL, composite_color, 1.0);
  
  gdk_pixbuf_composite (outline, composite,
			0, 0, RADIO_SIZE, RADIO_SIZE, 0, 0,
			1.0, 1.0, GDK_INTERP_NEAREST, 255);
  gdk_pixbuf_composite (circle, composite,
			0, 0, RADIO_SIZE, RADIO_SIZE, 0, 0,
			1.0, 1.0, GDK_INTERP_NEAREST, 255);
  
  bluecurve_style->radio_pixmap_nonactive[state] =
    pixbuf_to_pixmap (style, composite, screen);
  
  gdk_pixbuf_composite (dot, composite,
			0, 0, RADIO_SIZE, RADIO_SIZE, 0, 0,
			1.0, 1.0, GDK_INTERP_NEAREST, 255);
  
  bluecurve_style->radio_pixmap_active[state] =
    pixbuf_to_pixmap (style, composite, screen);
  
  g_object_unref (composite);
  
  composite = generate_bit (NULL, composite_color,1.0);
  
  gdk_pixbuf_composite (outline, composite,
			0, 0, RADIO_SIZE, RADIO_SIZE, 0, 0,
			1.0, 1.0, GDK_INTERP_NEAREST, 255);
  gdk_pixbuf_composite (circle, composite,
			0, 0, RADIO_SIZE, RADIO_SIZE, 0, 0,
			1.0, 1.0, GDK_INTERP_NEAREST, 255);
  gdk_pixbuf_composite (inconsistent, composite,
			0, 0, RADIO_SIZE, RADIO_SIZE, 0, 0,
			1.0, 1.0, GDK_INTERP_NEAREST, 255);
  
  bluecurve_style->radio_pixmap_inconsistent[state] =
    pixbuf_to_pixmap (style, composite, screen);
  
  g_object_unref (composite);
  g_object_unref (circle);
  g_object_unref (dot);
  g_object_unref (inconsistent);
  g_object_unref (outline);
}
		     
static void 
draw_option (GtkStyle      *style,
	     GdkWindow     *window,
	     GtkStateType   state_type,
	     GtkShadowType  shadow_type,
	     GdkRectangle  *area,
	     GtkWidget     *widget,
	     const gchar   *detail,
	     gint           x,
	     gint           y,
	     gint           width,
	     gint           height)
{
  BluecurveStyle *bluecurve_style = BLUECURVE_STYLE (style);
  GdkGC *gc = style->base_gc[state_type];
  GdkPixmap *pixmap;

  if (DETAIL ("option")) /* Menu item */
    {
      parent_class->draw_option (style, window, state_type, shadow_type, area,
				 widget, detail, x, y, width, height);
      return;
    }
    
  ensure_radio_pixmaps (style,
			state_type,
			gtk_widget_get_screen (widget));

  if (area)
    gdk_gc_set_clip_rectangle (gc, area);

  if (shadow_type == GTK_SHADOW_IN)
    pixmap = bluecurve_style->radio_pixmap_active[state_type];
  else if (shadow_type == GTK_SHADOW_ETCHED_IN) /* inconsistent */
    pixmap = bluecurve_style->radio_pixmap_inconsistent[state_type];
  else
    pixmap = bluecurve_style->radio_pixmap_nonactive[state_type];

  x += (width - RADIO_SIZE)/2;
  y += (height - RADIO_SIZE)/2;

  gdk_gc_set_clip_mask (gc,
			bluecurve_style->radio_pixmap_mask);
  gdk_gc_set_clip_origin (gc, x, y);

  gdk_draw_drawable (window, gc,
		     pixmap,
		     0, 0,
		     x, y,
		     RADIO_SIZE, RADIO_SIZE);

  gdk_gc_set_clip_origin (gc, 0, 0);
  gdk_gc_set_clip_mask (gc,
			NULL);

  if (area)
    gdk_gc_set_clip_rectangle (gc, NULL);
}

/**************************************************************************/

static void 
draw_shadow_gap (GtkStyle       *style,
		 GdkWindow      *window,
		 GtkStateType    state_type,
		 GtkShadowType   shadow_type,
		 GdkRectangle   *area,
		 GtkWidget      *widget,
		 const gchar    *detail,
		 gint            x,
		 gint            y,
		 gint            width,
		 gint            height,
		 GtkPositionType gap_side,
		 gint            gap_x,
		 gint            gap_width)
{
  BluecurveStyle *bluecurve_style = BLUECURVE_STYLE (style);
  GdkGC *outer_gc = NULL;
  GdkGC *gc1 = NULL;
  GdkGC *gc2 = NULL;
  GdkGC *gc3 = NULL;
  GdkGC *gc4 = NULL;
  
  g_return_if_fail (GTK_IS_STYLE (style));
  g_return_if_fail (window != NULL);
  
  sanitize_size (window, &width, &height);
  
  outer_gc = bluecurve_style->gray_gc[6];
  
  switch (shadow_type)
    {
    case GTK_SHADOW_NONE:
      return;
    case GTK_SHADOW_IN:
      gc1 = bluecurve_style->gray_gc[5];
      gc2 = bluecurve_style->gray_gc[1];
      gc3 = style->white_gc;
      gc4 = bluecurve_style->gray_gc[5];
      break;
    case GTK_SHADOW_ETCHED_IN:
      gc1 = bluecurve_style->gray_gc[3];
      gc2 = bluecurve_style->gray_gc[0];
      gc3 = bluecurve_style->gray_gc[3];
      gc4 = bluecurve_style->gray_gc[0];
      break;
    case GTK_SHADOW_OUT:
      gc1 = bluecurve_style->gray_gc[5];
      gc2 = style->white_gc;
      gc3 = bluecurve_style->gray_gc[1];
      gc4 = bluecurve_style->gray_gc[5];
      break;
    case GTK_SHADOW_ETCHED_OUT:
      gc1 = bluecurve_style->gray_gc[0];
      gc2 = bluecurve_style->gray_gc[3];
      gc3 = bluecurve_style->gray_gc[0];
      gc4 = bluecurve_style->gray_gc[3];
      break;
    }
  if (area)
    {
      gdk_gc_set_clip_rectangle (outer_gc, area);
      gdk_gc_set_clip_rectangle (gc1, area);
      gdk_gc_set_clip_rectangle (gc2, area);
      gdk_gc_set_clip_rectangle (gc3, area);
      gdk_gc_set_clip_rectangle (gc4, area);
    }
  
  switch (shadow_type)
    {
    case GTK_SHADOW_NONE:
    case GTK_SHADOW_IN:
    case GTK_SHADOW_OUT:
    case GTK_SHADOW_ETCHED_IN:
    case GTK_SHADOW_ETCHED_OUT:
      switch (gap_side)
        {
        case GTK_POS_TOP:
          gdk_draw_line (window, gc1,
                         x, y, x, y + height - 1);
          gdk_draw_line (window, gc2,
                         x + 1, y, x + 1, y + height - 2);
          
          gdk_draw_line (window, gc3,
                         x + 1, y + height - 2, x + width - 2, y + height - 2);
          gdk_draw_line (window, gc3,
                         x + width - 2, y, x + width - 2, y + height - 2);
          gdk_draw_line (window, gc4,
                         x, y + height - 1, x + width - 1, y + height - 1);
          gdk_draw_line (window, gc4,
                         x + width - 1, y, x + width - 1, y + height - 1);
          if (gap_x > 0)
            {
              gdk_draw_line (window, gc1,
                             x, y, x + gap_x - 1, y);
              gdk_draw_line (window, gc2,
                             x + 1, y + 1, x + gap_x - 1, y + 1);
            }
          if ((width - (gap_x + gap_width)) > 0)
            {
              gdk_draw_line (window, gc1,
                             x + gap_x + gap_width, y, x + width - 2, y);
              gdk_draw_line (window, gc2,
                             x + gap_x + gap_width, y + 1, x + width - 3, y + 1);
            }
          break;
        case GTK_POS_BOTTOM:
          gdk_draw_line (window, gc1,
                         x, y, x + width - 1, y);
          gdk_draw_line (window, gc1,
                         x, y, x, y + height - 1);
          gdk_draw_line (window, gc2,
                         x + 1, y + 1, x + width - 2, y + 1);
          gdk_draw_line (window, gc2,
                         x + 1, y + 1, x + 1, y + height - 1);
          
          gdk_draw_line (window, gc3,
                         x + width - 2, y + 1, x + width - 2, y + height - 1);
          gdk_draw_line (window, gc4,
                         x + width - 1, y, x + width - 1, y + height - 1);
          if (gap_x > 0)
            {
              gdk_draw_line (window, gc4,
                             x, y + height - 1, x + gap_x - 1, y + height - 1);
              gdk_draw_line (window, gc3,
                             x + 1, y + height - 2, x + gap_x - 1, y + height - 2);
            }
          if ((width - (gap_x + gap_width)) > 0)
            {
              gdk_draw_line (window, gc4,
                             x + gap_x + gap_width, y + height - 1, x + width - 2, y + height - 1);
              gdk_draw_line (window, gc3,
                             x + gap_x + gap_width, y + height - 2, x + width - 2, y + height - 2);
            }
          break;
        case GTK_POS_LEFT:
          gdk_draw_line (window, gc1,
                         x, y, x + width - 1, y);
          gdk_draw_line (window, gc2,
                         x, y + 1, x + width - 2, y + 1);
          
          gdk_draw_line (window, gc3,
                         x, y + height - 2, x + width - 2, y + height - 2);
          gdk_draw_line (window, gc3,
                         x + width - 2, y + 1, x + width - 2, y + height - 2);
          gdk_draw_line (window, gc4,
                         x, y + height - 1, x + width - 1, y + height - 1);
          gdk_draw_line (window, gc4,
                         x + width - 1, y, x + width - 1, y + height - 1);
          if (gap_x > 0)
            {
              gdk_draw_line (window, gc1,
                             x, y, x, y + gap_x - 1);
              gdk_draw_line (window, gc2,
                             x + 1, y + 1, x + 1, y + gap_x - 1);
            }
          if ((width - (gap_x + gap_width)) > 0)
            {
              gdk_draw_line (window, gc1,
                             x, y + gap_x + gap_width, x, y + height - 2);
              gdk_draw_line (window, gc2,
                             x + 1, y + gap_x + gap_width, x + 1, y + height - 2);
            }
          break;
        case GTK_POS_RIGHT:
          gdk_draw_line (window, gc1,
                         x, y, x + width - 1, y);
          gdk_draw_line (window, gc1,
                         x, y, x, y + height - 1);
          gdk_draw_line (window, gc2,
                         x + 1, y + 1, x + width - 1, y + 1);
          gdk_draw_line (window, gc2,
                         x + 1, y + 1, x + 1, y + height - 2);
          
          gdk_draw_line (window, gc3,
                         x + 1, y + height - 2, x + width - 1, y + height - 2);
          gdk_draw_line (window, gc4,
                         x, y + height - 1, x + width - 1, y + height - 1);
          if (gap_x > 0)
            {
              gdk_draw_line (window, gc4,
                             x + width - 1, y, x + width - 1, y + gap_x - 1);
              gdk_draw_line (window, gc3,
                             x + width - 2, y + 1, x + width - 2, y + gap_x - 1);
            }
          if ((width - (gap_x + gap_width)) > 0)
            {
              gdk_draw_line (window, gc4,
                             x + width - 1, y + gap_x + gap_width, x + width - 1, y + height - 2);
              gdk_draw_line (window, gc3,
                             x + width - 2, y + gap_x + gap_width, x + width - 2, y + height - 2);
            }
          break;
        }
    }
  
  if (area)
    {
      gdk_gc_set_clip_rectangle (outer_gc, NULL);
      gdk_gc_set_clip_rectangle (gc1, NULL);
      gdk_gc_set_clip_rectangle (gc2, NULL);
      gdk_gc_set_clip_rectangle (gc3, NULL);
      gdk_gc_set_clip_rectangle (gc4, NULL);
    }
}

/**************************************************************************/

static void
draw_hline (GtkStyle     *style,
	    GdkWindow    *window,
	    GtkStateType  state_type,
	    GdkRectangle  *area,
	    GtkWidget     *widget,
	    const gchar   *detail,
	    gint          x1,
	    gint          x2,
	    gint          y)
{
  BluecurveStyle *bluecurve_style = BLUECURVE_STYLE (style);
  
  g_return_if_fail (GTK_IS_STYLE (style));
  g_return_if_fail (window != NULL);
  
  if (area)
    {
      gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[2], area);
    }
  
  if (detail && !strcmp (detail, "label"))
    {
      if (state_type == GTK_STATE_INSENSITIVE)
        gdk_draw_line (window, style->white_gc, x1 + 1, y + 1, x2 + 1, y + 1);   
      gdk_draw_line (window, style->fg_gc[state_type], x1, y, x2, y);     
    }
  else
    {
      gdk_draw_line (window, bluecurve_style->gray_gc[2],
		     x1, y, x2, y);
      if (DETAIL ("menuitem"))
	gdk_draw_line (window, style->white_gc,
		       x1, y+1, x2, y+1);
      
    }
  
  if (area)
    {
      gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[2], NULL);
    }
}

/**************************************************************************/

static void
draw_vline (GtkStyle     *style,
	    GdkWindow    *window,
	    GtkStateType  state_type,
	    GdkRectangle  *area,
	    GtkWidget     *widget,
	    const gchar   *detail,
	    gint          y1,
	    gint          y2,
	    gint          x)
{
  BluecurveStyle *bluecurve_style = BLUECURVE_STYLE (style);
  gint thickness_light;
  gint thickness_dark;
  
  g_return_if_fail (GTK_IS_STYLE (style));
  g_return_if_fail (window != NULL);
  
  thickness_light = style->xthickness / 2;
  thickness_dark = style->xthickness - thickness_light;
  
  if (area)
    {
      gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[2], area);
    }
  
  gdk_draw_line (window, style->dark_gc[state_type], x, y1, x, y2 - 1);
  
  if (area)
    {
      gdk_gc_set_clip_rectangle (bluecurve_style->gray_gc[2], NULL);
    }
}

/**************************************************************************/

static void 
draw_focus (GtkStyle      *style,
	    GdkWindow     *window,
	    GtkStateType   state_type,
	    GdkRectangle  *area,
	    GtkWidget     *widget,
	    const gchar   *detail,
	    gint           x,
	    gint           y,
	    gint           width,
	    gint           height)
{
  BluecurveStyle *bluecurve_style = BLUECURVE_STYLE (style);
  GdkPoint points[5];
  GdkGC    *gc;
  gboolean free_dash_list = FALSE;
  gint line_width = 1;
  gchar *dash_list = "\1\1";
  gint dash_len;

  gc = bluecurve_style->gray_gc[6];

  if (widget)
    {
      gtk_widget_style_get (widget,
			    "focus-line-width", &line_width,
			    "focus-line-pattern", (gchar *)&dash_list,
			    NULL);

      free_dash_list = TRUE;
  }

  sanitize_size (window, &width, &height);
  
  if (area)
    gdk_gc_set_clip_rectangle (gc, area);

  gdk_gc_set_line_attributes (gc, line_width,
			      dash_list[0] ? GDK_LINE_ON_OFF_DASH : GDK_LINE_SOLID,
			      GDK_CAP_BUTT, GDK_JOIN_MITER);


  if (detail && !strcmp (detail, "add-mode"))
    {
      if (free_dash_list)
	g_free (dash_list);
      
      dash_list = "\4\4";
      free_dash_list = FALSE;
    }

  points[0].x = x + line_width / 2;
  points[0].y = y + line_width / 2;
  points[1].x = x + width - line_width + line_width / 2;
  points[1].y = y + line_width / 2;
  points[2].x = x + width - line_width + line_width / 2;
  points[2].y = y + height - line_width + line_width / 2;
  points[3].x = x + line_width / 2;
  points[3].y = y + height - line_width + line_width / 2;
  points[4] = points[0];

  if (!dash_list[0])
    {
      gdk_draw_lines (window, gc, points, 5);
    }
  else
    {
      /* We go through all the pain below because the X rasterization
       * rules don't really work right for dashed lines if you
       * want continuity in segments that go between top/right
       * and left/bottom. For instance, a top left corner
       * with a 1-1 dash is drawn as:
       *
       *  X X X 
       *  X
       *
       *  X
       *
       * This is because pixels on the top and left boundaries
       * of polygons are drawn, but not on the bottom and right.
       * So, if you have a line going up that turns the corner
       * and goes right, there is a one pixel shift in the pattern.
       *
       * So, to fix this, we drawn the top and right in one call,
       * then the left and bottom in another call, fixing up
       * the dash offset for the second call ourselves to get
       * continuity at the upper left.
       *
       * It's not perfect since we really should have a join at
       * the upper left and lower right instead of two intersecting
       * lines but that's only really apparent for no-dashes,
       * which (for this reason) are done as one polygon and
       * don't to through this code path.
       */
      
      dash_len = strlen (dash_list);
      
      if (dash_list[0])
	gdk_gc_set_dashes (gc, 0, dash_list, dash_len);
      
      gdk_draw_lines (window, gc, points, 3);
      
      /* We draw this line one farther over than it is "supposed" to
       * because of another rasterization problem ... if two 1 pixel
       * unjoined lines meet at the lower right, there will be a missing
       * pixel.
       */
      points[2].x += 1;
      
      if (dash_list[0])
	{
	  gint dash_pixels = 0;
	  gint i;
	  
	  /* Adjust the dash offset for the bottom and left so we
	   * match up at the upper left.
	   */
	  for (i = 0; i < dash_len; i++)
	    dash_pixels += dash_list[i];
      
	  if (dash_len % 2 == 1)
	    dash_pixels *= 2;
	  
	  gdk_gc_set_dashes (gc, dash_pixels - (width + height - 2 * line_width) % dash_pixels, dash_list, dash_len);
	}
      
      gdk_draw_lines (window, gc, points + 2, 3);
    }

  gdk_gc_set_line_attributes (gc, 0, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);

  if (area)
    gdk_gc_set_clip_rectangle (gc, NULL);

  if (free_dash_list)
    g_free (dash_list);
}

/**************************************************************************/

static void
draw_resize_grip (GtkStyle       *style,
		  GdkWindow      *window,
		  GtkStateType    state_type,
		  GdkRectangle   *area,
		  GtkWidget      *widget,
		  const gchar    *detail,
		  GdkWindowEdge   edge,
		  gint            x,
		  gint            y,
		  gint            width,
		  gint            height)
{
  BluecurveStyle *bluecurve_style = BLUECURVE_STYLE (style);
  g_return_if_fail (GTK_IS_STYLE (style));
  g_return_if_fail (window != NULL);
  
  if (area)
    {
      gdk_gc_set_clip_rectangle (style->light_gc[state_type], area);
      gdk_gc_set_clip_rectangle (style->dark_gc[state_type], area);
      gdk_gc_set_clip_rectangle (style->bg_gc[state_type], area);
    }

  switch (edge)
    {
    case GDK_WINDOW_EDGE_NORTH_WEST:
      /* make it square */
      if (width < height)
	{
	  height = width;
	}
      else if (height < width)
	{
	  width = height;
	}
      break;
    case GDK_WINDOW_EDGE_NORTH:
      if (width < height)
	{
	  height = width;
	}
      break;
    case GDK_WINDOW_EDGE_NORTH_EAST:
      /* make it square, aligning to top right */
      if (width < height)
	{
	  height = width;
	}
      else if (height < width)
	{
	  x += (width - height);
	  width = height;
	}
      break;
    case GDK_WINDOW_EDGE_WEST:
      if (height < width)
	{
	   width = height;
	}
      break;
    case GDK_WINDOW_EDGE_EAST:
      /* aligning to right */
      if (height < width)
	{
	  x += (width - height);
	  width = height;
	}
      break;
    case GDK_WINDOW_EDGE_SOUTH_WEST:
      /* make it square, aligning to bottom left */
      if (width < height)
	{
	  y += (height - width);
	  height = width;
	}
      else if (height < width)
	{
	  width = height;
	}
      break;
    case GDK_WINDOW_EDGE_SOUTH:
      /* align to bottom */
      if (width < height)
	{
	  y += (height - width);
	  height = width;
	}
      break;
    case GDK_WINDOW_EDGE_SOUTH_EAST:
      /* make it square, aligning to bottom right */
      if (width < height)
	{
	  y += (height - width);
	  height = width;
	}
      else if (height < width)
	{
	  x += (width - height);
	  width = height;
	}
      break;
    default:
      g_assert_not_reached ();
    }

  /* Clear background */
  gtk_style_apply_default_background (style, window, FALSE,
				      state_type, area,
				      x, y, width, height);   

  switch (edge)
    {
    case GDK_WINDOW_EDGE_WEST:
    case GDK_WINDOW_EDGE_EAST:
      {
	gint xi;

	xi = x;

	while (xi < x + width)
	  {
	    gdk_draw_line (window,
			   style->white_gc,
			   xi, y,
			   xi, y + height);

	    xi++;
	    gdk_draw_line (window,
			   bluecurve_style->gray_gc[5],
			   xi, y,
			   xi, y + height);

	    xi += 2;
	  }
      }
      break;
    case GDK_WINDOW_EDGE_NORTH:
    case GDK_WINDOW_EDGE_SOUTH:
      {
	gint yi;

	yi = y;

	while (yi < y + height)
	  {
	    gdk_draw_line (window,
			   style->white_gc,
			   x, yi,
			   x + width, yi);

	    yi++;
	    gdk_draw_line (window,
			   bluecurve_style->gray_gc[5],
			   x, yi,
			   x + width, yi);

	    yi+= 2;
	  }
      }
      break;
    case GDK_WINDOW_EDGE_NORTH_WEST:
      {
	gint xi, yi;

	xi = x + width;
	yi = y + height;

	while (xi > x + 3)
	  {
	    gdk_draw_line (window,
			   bluecurve_style->gray_gc[5],
			   xi, y,
			   x, yi);

	    --xi;
	    --yi;

	    gdk_draw_line (window,
			   style->white_gc,
			   xi, y,
			   x, yi);

	    xi -= 3;
	    yi -= 3;
	    
	  }
      }
      break;
    case GDK_WINDOW_EDGE_NORTH_EAST:
      {
        gint xi, yi;

        xi = x;
        yi = y + height;

        while (xi < (x + width - 3))
          {
            gdk_draw_line (window,
                           style->white_gc,
                           xi, y,
                           x + width, yi);                           

            ++xi;
            --yi;
            
            gdk_draw_line (window,
                           bluecurve_style->gray_gc[5],
                           xi, y,
                           x + width, yi);

            xi += 3;
            yi -= 3;
          }
      }
      break;
    case GDK_WINDOW_EDGE_SOUTH_WEST:
      {
	gint xi, yi;

	xi = x + width;
	yi = y;

	while (xi > x + 3)
	  {
	    gdk_draw_line (window,
			   bluecurve_style->gray_gc[5],
			   x, yi,
			   xi, y + height);

	    --xi;
	    ++yi;

	    gdk_draw_line (window,
			   style->white_gc,
			   x, yi,
			   xi, y + height);

	    xi -= 3;
	    yi += 3;
	    
	  }
      }
      break;
      
    case GDK_WINDOW_EDGE_SOUTH_EAST:
      {
        gint xi, yi;

        xi = x;
        yi = y;

        while (xi < (x + width - 3))
          {
            gdk_draw_line (window,
                           style->white_gc,
                           xi, y + height,
                           x + width, yi);                           

            ++xi;
            ++yi;
            
            gdk_draw_line (window,
                           bluecurve_style->gray_gc[5],
                           xi, y + height,
                           x + width, yi);                           
            
            xi += 3;
            yi += 3;
          }
      }
      break;
    default:
      g_assert_not_reached ();
      break;
    }
  
  if (area)
    {
      gdk_gc_set_clip_rectangle (style->light_gc[state_type], NULL);
      gdk_gc_set_clip_rectangle (style->dark_gc[state_type], NULL);
      gdk_gc_set_clip_rectangle (style->bg_gc[state_type], NULL);
    }
}


/**************************************************************************/
static void
bluecurve_button (GtkStyle * style,
	      GdkWindow * window,
	      GtkStateType state_type,
	      GtkShadowType shadow_type,
	      GdkRectangle * area,
	      GtkWidget * widget,
	      const gchar * detail,
	      gint x,
	      gint y,
	      gint width,
	      gint height)
{
}

static void
bluecurve_style_init_from_rc (GtkStyle * style,
			       GtkRcStyle * rc_style)
{
  BluecurveStyle *bluecurve_style = BLUECURVE_STYLE (style);
  GdkColor *spot_color;
  double shades[] = {1.065, 0.963, 0.896, 0.85, 0.768, 0.665, 0.4, 0.205};
  int i;
  double contrast;

  parent_class->init_from_rc (style, rc_style);

  contrast = BLUECURVE_RC_STYLE (rc_style)->contrast;

  /* Lighter to darker */
  for (i = 0; i < 8; i++)
    shade (&style->bg[GTK_STATE_NORMAL], &bluecurve_style->gray[i], (shades[i]-0.7) * contrast + 0.7);
  
  spot_color = bluecurve_get_spot_color (BLUECURVE_RC_STYLE (rc_style));

  bluecurve_style->spot_color = *spot_color;
  shade (&bluecurve_style->spot_color, &bluecurve_style->spot1, 1.62);
  shade (&bluecurve_style->spot_color, &bluecurve_style->spot2, 1.05);
  shade (&bluecurve_style->spot_color, &bluecurve_style->spot3, 0.72);
}

static GdkGC *
realize_color (GtkStyle * style,
	       GdkColor * color)
{
  GdkGCValues gc_values;

  gdk_colormap_alloc_color (style->colormap, color,
			    FALSE, TRUE);

  gc_values.foreground = *color;

  return gtk_gc_get (style->depth, style->colormap,
		     &gc_values, GDK_GC_FOREGROUND);
}


static void
bluecurve_style_realize (GtkStyle * style)
{
  BluecurveStyle *bluecurve_style = BLUECURVE_STYLE (style);
  int i;

  parent_class->realize (style);

  for (i = 0; i < 8; i++)
    bluecurve_style->gray_gc[i] = realize_color (style, &bluecurve_style->gray[i]);

  bluecurve_style->spot1_gc = realize_color (style, &bluecurve_style->spot1);
  bluecurve_style->spot2_gc = realize_color (style, &bluecurve_style->spot2);
  bluecurve_style->spot3_gc = realize_color (style, &bluecurve_style->spot3);
}

static void
bluecurve_style_unrealize (GtkStyle * style)
{
  BluecurveStyle *bluecurve_style = BLUECURVE_STYLE (style);
  int i;
  
  /* We don't free the colors, because we don't know if
   * gtk_gc_release() actually freed the GC. FIXME - need
   * a way of ref'ing colors explicitely so GtkGC can
   * handle things properly.
   */
  for (i=0; i < 8; i++)
    gtk_gc_release (bluecurve_style->gray_gc[i]);

  gtk_gc_release (bluecurve_style->spot1_gc);
  gtk_gc_release (bluecurve_style->spot2_gc);
  gtk_gc_release (bluecurve_style->spot3_gc);

  for (i = 0; i < 5; i++)
    {
      if (bluecurve_style->radio_pixmap_nonactive[i] != NULL)
	{
	  g_object_unref (bluecurve_style->radio_pixmap_nonactive[i]);
	  bluecurve_style->radio_pixmap_nonactive[i] = NULL;
	  g_object_unref (bluecurve_style->radio_pixmap_active[i]);
	  bluecurve_style->radio_pixmap_active[i] = NULL;
	  g_object_unref (bluecurve_style->radio_pixmap_inconsistent[i]);
	  bluecurve_style->radio_pixmap_inconsistent[i] = NULL;
	}
      
      if (bluecurve_style->check_pixmap_nonactive[i] != NULL)
	{
	  g_object_unref (bluecurve_style->check_pixmap_nonactive[i]);
	  bluecurve_style->check_pixmap_nonactive[i] = NULL;
	  g_object_unref (bluecurve_style->check_pixmap_active[i]);
	  bluecurve_style->check_pixmap_active[i] = NULL;
	  g_object_unref (bluecurve_style->check_pixmap_inconsistent[i]);
	  bluecurve_style->check_pixmap_inconsistent[i] = NULL;
	}
    }

  if (bluecurve_style->radio_pixmap_mask != NULL)
    g_object_unref (bluecurve_style->radio_pixmap_mask);
  bluecurve_style->radio_pixmap_mask = NULL;
  
  parent_class->unrealize (style);
}

static void
bluecurve_style_init (BluecurveStyle * style)
{
}

static void
bluecurve_style_class_init (BluecurveStyleClass * klass)
{
  GtkStyleClass *style_class = GTK_STYLE_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  style_class->realize = bluecurve_style_realize;
  style_class->unrealize = bluecurve_style_unrealize;
  style_class->init_from_rc = bluecurve_style_init_from_rc;

  /*
  style_class->draw_polygon = draw_polygon;
  style_class->draw_diamond = draw_diamond;
  style_class->draw_string = draw_string;
  */
  style_class->draw_focus = draw_focus;
  style_class->draw_resize_grip = draw_resize_grip;
  style_class->draw_handle = draw_handle;
  style_class->draw_vline = draw_vline;
  style_class->draw_hline = draw_hline;
  style_class->draw_slider = draw_slider;
  style_class->draw_shadow_gap = draw_shadow_gap;
  style_class->draw_tab = draw_tab;
  style_class->draw_arrow = bluecurve_draw_arrow;
  style_class->draw_check = draw_check;
  style_class->draw_box = draw_box;
  /*style_class->draw_flat_box = draw_flat_box;*/
  style_class->draw_shadow = draw_shadow;
  style_class->draw_box_gap = draw_box_gap;
  style_class->draw_extension = draw_extension;
  style_class->draw_option = draw_option;
}

GType bluecurve_type_style = 0;

void
bluecurve_style_register_type (GTypeModule * module)
{
  static const GTypeInfo object_info =
  {
    sizeof (BluecurveStyleClass),
    (GBaseInitFunc) NULL,
    (GBaseFinalizeFunc) NULL,
    (GClassInitFunc) bluecurve_style_class_init,
    NULL,			/* class_finalize */
    NULL,			/* class_data */
    sizeof (BluecurveStyle),
    0,				/* n_preallocs */
    (GInstanceInitFunc) bluecurve_style_init,
    NULL
  };

  bluecurve_type_style = g_type_module_register_type (module,
						  GTK_TYPE_STYLE,
						  "BluecurveStyle",
						  &object_info, 0);
}
