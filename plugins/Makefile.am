plugin_LTLIBRARIES = libgstssd1331.la

# sources used to compile this plug-in
libgstssd1331_la_SOURCES = gstssd1331plugin.c gstssd1331.c gstssd1331.h \
                            ftdi_util.c ftdi_util.h \
                            ssd_displ.c ssd_displ.h ssd_displ_graph.h ssd_displ_graph.c

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstssd1331_la_CFLAGS = $(GST_CFLAGS)
libgstssd1331_la_LIBADD = $(GST_LIBS) -lgstvideo-1.0 -lftdi
libgstssd1331_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

