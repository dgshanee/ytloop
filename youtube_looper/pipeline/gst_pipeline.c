/*
 *  -----GST PIPELINE-----
 *
 *  The self-contained GStreamer pipeline demonstrates how video data is moved
 * from the media file to the Raylib buffer.
 * */

#include "../include/pipeline.h"

#include <assert.h>

void create_gstreamer_pipeline(RaylibVideo *stream) {
  /*
   *  The starting point for the GStreamer pipeline. It handles the
   * initialization of the GStreamer pipeline and its related appsink.
   * If everything works out, it starts the pipeline and the video plays.
   *
   * Parameters
   * ----------
   *  stream : *RaylibVideo
   *    At the time of calling, RaylibVideo should just be the file path
   *    and its metadata.
   * */

  GError *err = NULL;

  // loads the media file into the gstreamer pipeline with an appsink
  char *call_str =
      "filesrc location=%s ! decodebin name=decode ! queue ! videoconvert"
      "! video/x-raw,format=RGB,width=%i,height=%i,colorimetry=sRGB ! appsink "
      "name=appsink sync=true decode."
      "! queue ! audioconvert !volume volume=0.1 ! audioresample ! "
      "autoaudiosink";

  // load the call_str with provided variables
  char *pipeline_str =
      malloc((sizeof(char) * strlen(call_str)) + (sizeof(char) * 100));
  sprintf(pipeline_str, call_str, stream->file_path, stream->width,
          stream->height);

  // parse the string and launch GStreamer with the provided options
  GstElement *pipeline_ref = gst_parse_launch(pipeline_str, &err);
  assert(err == NULL);

  free(pipeline_str);

  GstPipeline *pipeline = GST_PIPELINE(pipeline_ref);

  /*
   *  ---SET UP GSTREAMER APPSINK---
   * */
  GstElement *appsink_ref = gst_bin_get_by_name(GST_BIN(pipeline), "appsink");
  GstAppSink *appsink = GST_APP_SINK(appsink_ref);
  assert(appsink != NULL);

  gst_object_unref(appsink_ref);

  PipelineContext *pipeline_ctx =
      set_appsink_callback(*stream, pipeline, appsink);

  /*
   *  ---PLAY VIDEO---
   * */
  GstStateChangeReturn ret;
  ret = gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);
  assert(ret != GST_STATE_CHANGE_FAILURE);

  stream->thread_data = pipeline_ctx;
}

PipelineContext *set_appsink_callback(RaylibVideo stream, GstPipeline *pipeline,
                                      GstAppSink *appsink) {
  /*
   *  GStreamer appsinks are designed to emit every time something happens.
   *  In this case, every time a new sample is loaded from the media file, the
   *  appsink will perform a workflow to handle it.
   *
   *  The appsink here connects to the callback on_new_sample on each sample.
   *
   *  Parameters
   *  ----------
   *  stream : RaylibVideo
   *    We use the stream for its metadata so we can provide proficient
   *    information to create the workflow.
   *  pipeline : *GstPipeline
   *    To create the weak pipeline
   *  appsink : *GstAppSink
   *    To create the callback system
   *
   *  Returns
   *  -------
   *  PipelineContext*
   *    Data about the pipeline created during this function.
   *
   *
   * */
  gst_app_sink_set_emit_signals(appsink, TRUE);

  GWeakRef weak_pipeline;
  g_weak_ref_init(&weak_pipeline, G_OBJECT(pipeline));
  assert(&weak_pipeline != NULL);

  volatile gint timestamp = 0;
  gint64 timestamp_ref = g_atomic_int_get(&timestamp);

  // initialize shared frame
  SharedFrame *sf = malloc(sizeof(SharedFrame));
  init_frame_mutex(sf, stream.height, stream.width);

  // create the pipeline for appsink events
  PipelineContext *pipeline_ctx = malloc(sizeof(PipelineContext));
  atomic_init(&pipeline_ctx->dirty, false);
  pipeline_ctx->frame = sf;
  pipeline_ctx->timestamp_ref = timestamp_ref;
  pipeline_ctx->pipeline = pipeline;
  pipeline_ctx->appsink = appsink;

  g_signal_connect(appsink, "new-sample", G_CALLBACK(on_new_sample),
                   pipeline_ctx);

  return pipeline_ctx;
}

void init_frame_mutex(SharedFrame *sf, int64_t video_height,
                      int64_t video_width) {
  /*
   *  Creates the mutex of the frame inside of the SharedFrame.
   *
   *  Parameters
   *  ----------
   *  sf : *SharedFrame
   *    Struct of the frame and the mutex.
   *  video_height : int64_t
   *    Height of the frame
   *  video_width : int64_t
   *    Width of the frame
   * */
  sf->data = malloc(sizeof(uint8_t) * video_height * video_width * 3); // rgb
  assert(sf->data != NULL);

  pthread_mutex_init(&sf->mutex, NULL);
  assert(&sf->mutex != NULL);
}

GstFlowReturn on_new_sample(GstAppSink *appsink, gpointer user_data) {
  /*
   *  Callback function to be called when the signal "new-sample" is emitted
   *  from the GStreamer pipeline. It essentially just grabs the new frame,
   *  processes it, and pushes it to the frame data while letting the pipeline
   *  know it's dirty.
   *
   *  Returns
   *  -------
   *  GstFlowReturn
   *    Success indicator of the workflow performed
   *
   *  Parameters
   *  ----------
   *  appsink : *GstAppSink
   *    The appsink that originally emits the signal to call this function
   *  user_data : gpointer
   *    Basically a void pointer that we cast to our pipeline context
   * */

  // since this is a generic function header, we need to cast the user_data to
  // what we know it is.
  PipelineContext *pipeline_context = (PipelineContext *)user_data;
  GstPipeline *pipeline = pipeline_context->pipeline;
  assert(pipeline);

  // pull the new frame
  GstSample *sample = gst_app_sink_pull_sample(appsink);
  assert(sample);

  // from the sample, get the buffer
  GstBuffer *buffer = gst_sample_get_buffer(sample);
  assert(buffer);

  // from the docs:
  // Fills info with the GstMapInfo of all merged memory blocks in buffer
  GstMapInfo map;
  if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) {
    return GST_FLOW_ERROR;
  }

  // video position
  gint64 pos_ns = GST_CLOCK_TIME_NONE;

  if (gst_element_query_position(GST_ELEMENT(pipeline), GST_FORMAT_TIME,
                                 &pos_ns)) {
    volatile gint ts = (gint)pipeline_context->timestamp_ref;
    g_atomic_int_set(&ts, pos_ns);
  }

  // write the frame data to the frame and let the pipeline know it's a new
  // sample
  write_frame_data(pipeline_context->frame, map.data, map.size);
  atomic_store_explicit(&pipeline_context->dirty, true, memory_order_relaxed);

  // all done!
  gst_buffer_unmap(buffer, &map);
  gst_sample_unref(sample);

  return GST_FLOW_OK;
}

void write_frame_data(SharedFrame *sf, uint8_t *src, size_t size) {
  /*
   *  Writes the mapped buffer to the SharedFrame within the context of a mutex.
   *
   *  Parameters
   *  ----------
   *  sf : *SharedFrame
   *    The shared frame containing the mutex and frame data
   *  src : *uint8_t
   *    The mapped data buffer from on_new_sample
   *  size : size_t
   *    The size of the mapped data buffer
   * */
  pthread_mutex_lock(&sf->mutex);
  memcpy(sf->data, src, size);
  pthread_mutex_unlock(&sf->mutex);
}
