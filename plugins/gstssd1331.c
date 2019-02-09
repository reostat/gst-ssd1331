/* GStreamer
 * Copyright (C) 2019 FIXME <fixme@example.com>
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
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */
/**
 * SECTION:element-gstssd1331
 *
 * The ssd1331 element does FIXME stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v fakesrc ! ssd1331 ! FIXME ! fakesink
 * ]|
 * FIXME Describe what the pipeline does.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideosink.h>
#include <byteswap.h>
#include "ssd_displ_graph.h"
#include "gstssd1331.h"

GST_DEBUG_CATEGORY_STATIC (gst_ssd1331_debug_category);
#define GST_CAT_DEFAULT gst_ssd1331_debug_category

/* prototypes */


static gboolean gst_ssd1331_setcaps(GstBaseSink * basesink, GstCaps * caps);
static gboolean gst_ssd1331_start(GstBaseSink *sink);
static gboolean gst_ssd1331_stop(GstBaseSink *sink);

static void gst_ssd1331_set_property(GObject * object,
    guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_ssd1331_get_property(GObject * object,
    guint property_id, GValue * value, GParamSpec * pspec);

static void gst_ssd1331_dispose(GObject * object);
static void gst_ssd1331_finalize(GObject * object);

static GstFlowReturn gst_ssd1331_show_frame(GstVideoSink * video_sink,
    GstBuffer * buf);

enum
{
  PROP_0
};

/* pad templates */
static GstStaticPadTemplate sink_template = GST_STATIC_PAD_TEMPLATE(
    "sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("video/x-raw, "       \
      "format = (string) RGB16, "        \
      "width = (int) 96, "               \
      "height = (int) 64, "              \
      "framerate = (fraction) [ 0, max ]"
    )
  );

/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstSsd1331, gst_ssd1331, GST_TYPE_VIDEO_SINK,
  GST_DEBUG_CATEGORY_INIT (gst_ssd1331_debug_category, "ssd1331", 0,
  "debug category for ssd1331 element"));

static void gst_ssd1331_class_init (GstSsd1331Class * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstElementClass *gstelement_class = GST_ELEMENT_CLASS (klass);
  GstBaseSinkClass *base_sink_class = GST_BASE_SINK_CLASS (klass);
  GstVideoSinkClass *video_sink_class = GST_VIDEO_SINK_CLASS (klass);

  gst_element_class_add_static_pad_template (gstelement_class, &sink_template);

  gst_element_class_set_static_metadata (GST_ELEMENT_CLASS(klass),
      "SSD1331 Display Sink", "Sink/Video", "Sends RGB565 video to SSD1331 display over FTDI SPI",
      "FIXME <fixme@example.com>");

  gobject_class->set_property = gst_ssd1331_set_property;
  gobject_class->get_property = gst_ssd1331_get_property;
  gobject_class->dispose = gst_ssd1331_dispose;
  gobject_class->finalize = gst_ssd1331_finalize;
  base_sink_class->set_caps = gst_ssd1331_setcaps;
  base_sink_class->start = gst_ssd1331_start;
  base_sink_class->stop = gst_ssd1331_stop;
  video_sink_class->show_frame = GST_DEBUG_FUNCPTR (gst_ssd1331_show_frame);

}


static void gst_ssd1331_init (GstSsd1331 *ssd1331)
{
}

static gboolean gst_ssd1331_start(GstBaseSink *sink)
{
  GstSsd1331 *ssd1331 = GST_SSD1331 (sink);
  
  GST_DEBUG_OBJECT (ssd1331, "Connecting to SSD display");
  ssd1331->ssd = ssd_new();
  if (ssd_open(ssd1331->ssd, 0x0403, 0x6010) != 0) {
    ssd_free(ssd1331->ssd);
    ssd1331->ssd = NULL;
    GST_ELEMENT_ERROR (ssd1331, RESOURCE, NOT_FOUND,
      ("Failed to connect to display"), (NULL));
    return FALSE;
  }

  return TRUE;
}

#define check_ssd_error(expr) if (expr < 0) { \
  GST_ELEMENT_ERROR (ssd1331, RESOURCE, NOT_FOUND, \
    ("Failed to transfer data to to display"), (NULL)); \
  return GST_FLOW_ERROR; \
}

static GstFlowReturn gst_ssd1331_show_frame (GstVideoSink * sink, GstBuffer * buf)
{
  GstSsd1331 *ssd1331 = GST_SSD1331 (sink);
  GstVideoFrame frame;

  GST_DEBUG_OBJECT (ssd1331, "show_frame, buf_size %ld", gst_buffer_get_size(buf));

  if (!gst_video_frame_map(&frame, &ssd1331->info, buf, GST_MAP_READ))
    goto invalid_frame;

  uint16_t *buf_ptr = GST_VIDEO_FRAME_PLANE_DATA(&frame, 0);
  gsize buf_byte_size = GST_VIDEO_FRAME_SIZE(&frame);

  // SSD1331 wants data in big endian over SPI. If we run on little endian arch 
  // then just flip those bytes in the buffer. dirty but effective.
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  for (uint16_t *tmp = buf_ptr; tmp < buf_ptr + buf_byte_size / 2; *tmp = bswap_16(*tmp), tmp++);
#endif

  check_ssd_error(ssd_begin_pixel_transfer(ssd1331->ssd));
  check_ssd_error(ssd_send_pixels(ssd1331->ssd, (uint8_t *)buf_ptr, buf_byte_size));
  check_ssd_error(ssd_end_transfer(ssd1331->ssd));

  gst_video_frame_unmap(&frame);
  return GST_FLOW_OK;

invalid_frame:
  GST_DEBUG_OBJECT(ssd1331, "invalid frame");
  return GST_FLOW_ERROR;
}

static gboolean gst_ssd1331_stop(GstBaseSink *sink)
{
  GstSsd1331 *ssd1331 = GST_SSD1331 (sink);
  GST_DEBUG_OBJECT (ssd1331, "Disconnecting from SSD display");
  if (ssd1331->ssd != NULL)
    ssd_clear_display(ssd1331->ssd);
  ssd_free(ssd1331->ssd);
  ssd1331->ssd = NULL;
  return TRUE;
}

static gboolean gst_ssd1331_setcaps (GstBaseSink * basesink, GstCaps * caps)
{
  GstSsd1331 *ssd1331 = GST_SSD1331 (basesink);
  GstVideoInfo info;

  GST_DEBUG_OBJECT (ssd1331, "Set caps: width %d, height %d, frame size %ld, RGB %d, format 0x%X",
    info.width, info.height, info.size, GST_VIDEO_INFO_IS_RGB(&info), info.finfo->format);

  if (!gst_video_info_from_caps(&info, caps) ||
        info.width != 96  || info.height != 64 || !GST_VIDEO_INFO_IS_RGB(&info))
    goto invalid_caps;

  ssd1331->info = info;
  return TRUE;

invalid_caps:
  GST_DEBUG_OBJECT (ssd1331, "invalid caps");
  return FALSE;
}

void gst_ssd1331_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstSsd1331 *ssd1331 = GST_SSD1331 (object);

  GST_DEBUG_OBJECT (ssd1331, "set_property");

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void gst_ssd1331_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstSsd1331 *ssd1331 = GST_SSD1331 (object);

  GST_DEBUG_OBJECT (ssd1331, "get_property");

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void gst_ssd1331_dispose (GObject * object)
{
  GstSsd1331 *ssd1331 = GST_SSD1331 (object);

  GST_DEBUG_OBJECT (ssd1331, "dispose");

  /* clean up as possible.  may be called multiple times */

  G_OBJECT_CLASS (gst_ssd1331_parent_class)->dispose (object);
}

void gst_ssd1331_finalize (GObject * object)
{
  GstSsd1331 *ssd1331 = GST_SSD1331 (object);

  GST_DEBUG_OBJECT (ssd1331, "finalize");

  /* clean up object here */

  G_OBJECT_CLASS (gst_ssd1331_parent_class)->finalize (object);
}

// Section below is done in gstssd131plugin.c
/*
static gboolean
plugin_init (GstPlugin * plugin)
{

  // FIXME Remember to set the rank if it's an element that is meant
  // to be autoplugged by decodebin.
  return gst_element_register (plugin, "ssd1331", GST_RANK_NONE,
      GST_TYPE_SSD1331);
}

// FIXME: these are normally defined by the GStreamer build system.
// If you are creating an element to be included in gst-plugins-*,
// remove these, as they're always defined.  Otherwise, edit as
// appropriate for your external plugin package.
#ifndef VERSION
#define VERSION "0.0.FIXME"
#endif
#ifndef PACKAGE
#define PACKAGE "FIXME_package"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "FIXME_package_name"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "http://FIXME.org/"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    ssd1331,
    "FIXME plugin description",
    plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)
*/