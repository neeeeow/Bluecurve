#include <string.h>
#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

int
main (int argc, char *argv[])
{
  GdkPixbuf *pixbuf;
  unsigned char *pixels;
  char *base, *dot;
  int w, h, rs;
  int x, y;
  int type = 0;

  g_type_init();

  if (argc < 2)
    {
      g_print ("bad args\n");
      return 1;
    }

  if (argc >= 3)
    type = atoi(argv[2]);

  base = g_path_get_basename(argv[1]);
  dot = strchr (base, '.');
  if (dot)
    *dot = 0;
  
  pixbuf = gdk_pixbuf_new_from_file (argv[1], NULL);

  w = gdk_pixbuf_get_width (pixbuf);
  h = gdk_pixbuf_get_height (pixbuf);
  rs = gdk_pixbuf_get_rowstride (pixbuf);
  pixels = gdk_pixbuf_get_pixels (pixbuf);


  g_print ("static unsigned char %s_alpha[] = {\n", base);
  for (y=0; y < h; y++)
    {
      for (x=0; x < w; x++)
	{
	  int pos = y*rs + 4*x;
	  int pix;
	  
	  if (type == 1)
	    {
	      int a1, a2, a3;

	      a1 = (255 - pixels[pos + 0]) * 255 / (255 - 76);
	      a2 = (255 - pixels[pos + 1]) * 255 / (255 - 89);
	      a3 = (255 - pixels[pos + 2]) * 255 / (255 - 166);

	      //g_print ("(%.2x,%.2x,%.2x)", a1,a2,a3);
	      
	      pix = ((a1 + a2 + a3) / 3) * 0xff / 0x141;
	    }
	  else if (type == 2)
	    {
	      int a1, a2, a3;

	      a1 = pixels[pos + 0];
	      a2 = pixels[pos + 1];
	      a3 = pixels[pos + 2];

	      //g_print ("(%.2x,%.2x,%.2x)", a1,a2,a3);
	      
	      pix = ((a1 + a2 + a3) / 3);
	    }
	  else
	    pix = pixels[pos + 3];
	    
	  g_print ("0x%.2x,", pix);
	}
      g_print ("\n");
    }
  g_print ("};\n");
}
