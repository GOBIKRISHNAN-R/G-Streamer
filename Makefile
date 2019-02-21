CC = gcc

# -lc -lpthread -lgio-2.0 -lgstbase-1.0 -lgstreamer-1.0 -lgobject-2.0 -lgmodule-2.0  -lgthread-2.0 -lglib-2.0 -I/usr/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/glib-2.0/include -I/usr/include/gstreamer-1.0 -I/usr/lib/i386-linux-gnu/glib-2.0/include/ -I/usr/lib/x86_64-linux-gnu/glib-2.0/include/ -I/usr/include/gtk-3.0/gtk/ -I/usr/include/gtk-2.0/gtk/ -I/usr/include/libxml2/ -lxml2

LIBS = `pkg-config --cflags --libs gstreamer-1.0`

.PHONY: single multi clean 

av :
	$(CC) vid_aud.c $(LIBS) -o av

dynamic :
	$(CC) dyn_vid.c $(LIBS) -o dynamic

single:
	$(CC) single_vide0.c $(LIBS) -o single

multi :
	$(CC) multi_video.c $(LIBS) -o multi

clean :
	rm -r single multi *.mp4
