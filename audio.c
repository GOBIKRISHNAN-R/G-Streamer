#include <stdio.h>
#include <string.h>
#include <gst/gst.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

//gst-launch-1.0 -e audiotestsrc wave=1 freq=100 ! audioconvert ! lamemp3enc ! filesink location=audio.mp3

static GMainLoop *loop;
static GstElement *pipeline, *src, *convert, *encoder, *sink;
static GstBus *bus;


int sigintHandler(int unused) 
{
        g_print("\n\tSending EoS\n");
        gst_element_send_event(pipeline, gst_event_new_eos()); 
        return 0;
}

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
              g_printerr ("Additional debug info:\n%s\n", debug);

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
		g_print ("\n\tGot EOS\n");
		g_main_loop_quit (loop);
		gst_element_set_state (pipeline, GST_STATE_NULL);
		g_main_loop_unref (loop);
		gst_object_unref (pipeline);
		g_print("\n\tFile Saved...\n");
		exit(0);
		break;
      }
      default:
		break;
   }

   return TRUE;
}

int main()
{

	signal(SIGINT,sigintHandler);
	//signal(SIGALRM,sigintHandler);
        //alarm(10);

	gst_init ( NULL, NULL);

        printf("\n\n\tPipeline Initialized Successfully...\n");

	pipeline = gst_pipeline_new(NULL);
	src      = gst_element_factory_make("audiotestsrc",     NULL);
        convert  = gst_element_factory_make("audioconvert",       NULL);
        encoder  = gst_element_factory_make("lamemp3enc", NULL);
	sink     = gst_element_factory_make("filesink",    NULL);

	if (pipeline == NULL || src == NULL || encoder == NULL || convert == NULL || sink == NULL) 
        {
		g_error("\n\tFailed to create elements\n");
		return -1;
	}

        printf("\n\tAll the Elements were Created.....\n");

	/*g_object_set(src, "wave", "1", NULL);
	g_object_set(src, "freq", "100", NULL);

        printf("\n\tSource Was Linked...\n");*/

	g_object_set(sink, "location", "audio.mp3", NULL);

        printf("\n\tSink Was Linked...\n");

	gst_bin_add_many(GST_BIN(pipeline), src, convert, encoder, sink, NULL);
	if (!gst_element_link_many(src, convert, encoder, sink, NULL))
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
