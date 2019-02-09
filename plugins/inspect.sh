make clean && make
gst-inspect-1.0 --gst-plugin-path=$(cd .libs; pwd) ssd1331

#make clean && make && make DESTDIR=$(cd dest; pwd) install
#gst-inspect-1.0 --gst-plugin-path=$(cd dest/usr/local/lib/gstreamer-1.0; pwd) ssd1331
