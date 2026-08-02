/*
 *  -----GST PIPELINE-----
 *
 *  The self-contained GStreamer pipeline demonstrates how video data is moved
 * from the media file to the Raylib buffer.
 * */

#include "include/pipeline.h"

#include <assert.h>

void create_gstreamer_pipeline(RaylibVideo *stream) {
  char *call_str =
      "filesrc location=%s ! decodebin name=decode ! queue ! videoconvert"
      "! video/x-raw,format=RGB,width=%i,height=%i,colorimetry=sRGB ! appsink "
      "name=appsink sync=true decode."
      "! queue ! audioconvert !volume volume=0.1 ! audioresample ! "
      "autoaudiosink";

  char *pipeline_str =
      malloc((sizeof(char) * strlen(call_str)) + (sizeof(char) * 100));
  sprintf(pipeline_str, call_str, stream->file_path, stream->width,
          stream->height);

  GError *err = NULL;
  GstElement *pipeline_ref = gst_parse_launch(pipeline_str, &err);
  assert(err == NULL);

  free(pipeline_str);

  GstPipeline *pipeline = GST_PIPELINE(pipeline_ref);

  GstElement *appsink_ref = gst_bin_get_by_name(GST_BIN(pipeline), "appsink");
  GstAppSink *appsink = GST_APP_SINK(appsink_ref);
  assert(appsink != NULL);

  gst_object_unref(appsink_ref);

  UserData *ud = set_appsink_callback(*stream, pipeline, appsink);

  // play video
  GstStateChangeReturn ret;
  ret = gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);
  assert(ret != GST_STATE_CHANGE_FAILURE);

  stream->thread_data = ud;
  return;
}

UserData *set_appsink_callback(RaylibVideo stream, GstPipeline *pipeline,
                               GstAppSink *appsink) {
  gst_app_sink_set_emit_signals(appsink, TRUE);

  GWeakRef weak_pipeline;
  g_weak_ref_init(&weak_pipeline, G_OBJECT(pipeline));
  assert(&weak_pipeline != NULL);

  volatile gint ts = 0;
  gint64 ts_ref = g_atomic_int_get(&ts);

  SharedFrame *sf = malloc(sizeof(SharedFrame));
  init_frame_mutex(sf, stream.height, stream.width);

  UserData *ud = malloc(sizeof(UserData));
  atomic_init(&ud->dirty, false);
  ud->frame = sf;
  ud->ts_ref = ts_ref;
  ud->pipeline = pipeline;
  ud->appsink = appsink;

  g_signal_connect(appsink, "new-sample", G_CALLBACK(on_new_sample), ud);

  return ud;
}

void init_frame_mutex(SharedFrame *sf, int64_t video_height,
                      int64_t video_width) {
  sf->data = malloc(sizeof(uint8_t) * video_height * video_width * 3); // rgb
  assert(sf->data != NULL);

  pthread_mutex_init(&sf->mutex, NULL);
  assert(&sf->mutex != NULL);
}

GstFlowReturn on_new_sample(GstAppSink *appsink, gpointer user_data) {
  UserData *ud = (UserData *)user_data;
  GstPipeline *pipeline = ud->pipeline;
  assert(pipeline);

  GstSample *sample = gst_app_sink_pull_sample(appsink);
  assert(sample);

  GstBuffer *buffer = gst_sample_get_buffer(sample);
  assert(buffer);

  GstMapInfo map;
  if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) {
    return GST_FLOW_ERROR;
  }

  gint64 pos_ns = GST_CLOCK_TIME_NONE;

  if (gst_element_query_position(GST_ELEMENT(pipeline), GST_FORMAT_TIME,
                                 &pos_ns)) {
    volatile gint ts = (gint)ud->ts_ref;
    g_atomic_int_set(&ts, pos_ns);
  }

  write_frame_data(ud->frame, map.data, map.size);
  atomic_store_explicit(&ud->dirty, true, memory_order_relaxed);

  gst_buffer_unmap(buffer, &map);
  gst_sample_unref(sample);

  return GST_FLOW_OK;
}

void write_frame_data(SharedFrame *sf, uint8_t *src, size_t size) {
  pthread_mutex_lock(&sf->mutex);
  memcpy(sf->data, src, size);
  pthread_mutex_unlock(&sf->mutex);
}
