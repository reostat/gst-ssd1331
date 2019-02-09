make

gst-launch-1.0 --gst-plugin-path=$(cd .libs; pwd) \
        filesrc location=sintel_trailer-480p.webm ! decodebin name=decoder \
            decoder. ! queue ! audioconvert ! autoaudiosink \
            decoder. ! tee name=t ! \
                queue ! videoconvert ! autovideosink \
                t. ! queue ! videoscale ! videoconvert ! ssd1331

#    --gst-debug=ssd1331:7 \
#gst-launch-1.0 --gst-plugin-path=$(cd .libs; pwd) \
#    filesrc location=sintel_trailer-480p.webm ! decodebin ! tee name=t ! \
#    queue ! autovideosink \
#    t. ! queue ! videoscale ! videoconvert ! ssd1331
