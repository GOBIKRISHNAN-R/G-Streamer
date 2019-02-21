/****************************************************************************
 *   ProjectName : capture video and split it into multiple files
 *
 *   COMPONENT   : G-Streamer
 *
 *   Author      : GOBIKRISHNAN R
 *
 *   DATE        : 29.01.2019
 *
 *   Description : used to record videos in multiple files using gstreamer framework.
 *
 *   Pipeline    : gst-launch-1.0 -e v4l2src device=/dev/video0 ! 'video/x-raw,width=1280,heigt=720' ! timeoverlay ! x264enc ! mp4mux ! filesink location=gobi.mp4
*
*******************************************************************************/

/******************************************************************************
* Include Files
******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <gst/gst.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

/******************************************************************************
* Global variables
******************************************************************************/

static GMainLoop *loop;
static GstElement *pipeline, *src, *format, *queue, *timelay,  *encoder, *parser,  *muxer, *sink;
static GstBus *bus;

/******************************************************************************
* Function Prototypes
******************************************************************************/
int sigintHandler(int unused);
static gboolean message_cb (GstBus * bus, GstMessage * message, gpointer user_data);
int capture(char *name);

/****************************************************************************
 *
 * NAME          :  main
 *
 * PARAMETERS    :  void
 *
 * RETURN VALUES :  none
 *
 * DESCRIPTION   :  call the capturefunction repeatedly depending on the time duration.
 *                  
 *
****************************************************************************/

int main()
{
    char p[120];
    printf("\n\tEnter the Filename : ");
    scanf("%s",p);
    printf("\n\tfile name is : %s\n");
    capture(p);
    /*int i;
    for (i =0; i < 333333333333333333333333333333333; i++)
    {
        sprintf(p,"video%02d.mp4",i);
        capture(p);
    }*/
    return 0;
}

/****************************************************************************
 *
 * NAME          :  capture
 *
 * PARAMETERS    :  name of the video
 *
 * RETURN VALUES :  none
 *
 * DESCRIPTION   :  start capturing data from camera and creates a mp4 file
 *                  
 *
****************************************************************************/
int capture(char name[20])
{

	signal(SIGINT, sigintHandler);
	//signal(SIGALRM, sigintHandler);
        //alarm(10);

	gst_init ( NULL, NULL);

        printf("\n\n\tPipeline Initialized Successfully...\n");

	pipeline = gst_pipeline_new(NULL);
	src      = gst_element_factory_make("v4l2src", NULL);
        queue    = gst_element_factory_make("queue", NULL);
        timelay  = gst_element_factory_make("timeoverlay", NULL);
	encoder  = gst_element_factory_make("x264enc", NULL);
	parser   = gst_element_factory_make("h264parse", NULL);
	muxer    = gst_element_factory_make("mp4mux", NULL);
	sink     = gst_element_factory_make("filesink", NULL);

	if (pipeline == NULL || src == NULL || encoder == NULL || queue == NULL || timelay == NULL || parser == NULL || muxer == NULL || sink == NULL) 
        {
		g_error("\n\tFailed to create elements\n");
		return -1;
	}

        printf("\n\tAll the Elements were Created.....\n");

	g_object_set(src, "device", "/dev/video1", NULL);

        printf("\n\tSource Was Linked...\n");

	g_object_set(sink, "location", name, NULL);

        printf("\n\tSink Was Linked...\n");

	gst_bin_add_many(GST_BIN(pipeline), src, queue, timelay,  encoder, parser, muxer, sink, NULL);
	if (!gst_element_link_many(src, queue, timelay, encoder, parser,  muxer, sink, NULL))
        {
		g_error("\n\tFailed to link elements\n");
		return -2;
	}

	loop = g_main_loop_new(NULL, FALSE);

	bus = gst_pipeline_get_bus(GST_PIPELINE (pipeline));
	gst_bus_add_signal_watch(bus);
	g_signal_connect(G_OBJECT(bus), "message", G_CALLBACK(message_cb), NULL);
	gst_object_unref(GST_OBJECT(bus));

	gst_element_set_state(pipeline, GST_STATE_PLAYING);

	g_print("\n\tStarting loop...\n");
	g_main_loop_run(loop);

	return 0;
}

/****************************************************************************
 *
 * NAME          :  sigintHandler  
 *
 * PARAMETERS    :  void
 *
 * RETURN VALUES :  0 for success...
 *
 * DESCRIPTION   :  stop capturing data from camera and saves a mp4 file
 *                  
 *
****************************************************************************/
int sigintHandler(int unused) 
{
        g_print("\n\tSending EoS\n");
        gst_element_send_event(pipeline, gst_event_new_eos()); 
        return 0;
}

/****************************************************************************
 *
 * NAME          :  message_cb
 *
 * PARAMETERS    :  GstBus *bus,GstMessage *message,gpointer *user_data
 *
 * RETURN VALUES :  gboolean
 *
 * DESCRIPTION   :  callback function and receives gstreamer bus messages
 *
****************************************************************************/
static gboolean message_cb (GstBus * bus, GstMessage * message, gpointer user_data)
{
    switch (GST_MESSAGE_TYPE (message)) 
    {
      case GST_MESSAGE_ERROR:
      {
          GError *err = NULL;
          gchar *name, *debug = NULL;

          name = gst_object_get_path_string (message->src);
          gst_message_parse_error (message, &err, &debug);

          g_printerr ("ERROR: from element %s: %s\n", name, err->message);
          if (debug != NULL)
          {
              g_printerr ("Additional debug info:\n%s\n", debug);
          }

          g_error_free (err);
          g_free (debug);
          g_free (name);

          g_main_loop_quit (loop);
          break;
      }
      case GST_MESSAGE_WARNING:
      {
		GError *err = NULL;
		gchar *name, *debug = NULL;

		name = gst_object_get_path_string (message->src);
		gst_message_parse_warning (message, &err, &debug);

		g_printerr ("ERROR: from element %s: %s\n", name, err->message);
		if (debug != NULL)
         		g_printerr ("Additional debug info:\n%s\n", debug);

		g_error_free (err);
		g_free (debug);
		g_free (name);
		break;
      }
      case GST_MESSAGE_EOS:
      {
		g_print ("Got EOS\n");
		g_main_loop_quit (loop);
		gst_element_set_state (pipeline, GST_STATE_NULL);
		g_main_loop_unref (loop);
		gst_object_unref (pipeline);
		g_print("Saved video file\n");
		exit(0);
		break;
      }
      default:
		break;
   }

   return TRUE;
}

