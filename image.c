/****************************************************************************
 *   ProjectName : capture image 
 *
 *   COMPONENT   : G-Streamer
 *
 *   Author      : GOBIKRISHNAN R
 *
 *   DATE        : 29.01.2019
 *
 *   Description : capturing the image from the webcam
 *
 *   Pipeline    : gst-launch-1.0 -e v4l2src device=/dev/video0 num-buffers=1 ! 'video/x-raw,width=1280,heigt=720' ! videoconvert ! jpegenc ! filesink location=image.jpg
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
static GstElement *pipeline, *src, *convert, *encoder, *sink;
static GstBus *bus;

/******************************************************************************
* Function Prototypes
******************************************************************************/
int sigintHandler(int unused);

/****************************************************************************
 *
 * NAME          :  main
 *
 * PARAMETERS    :  void
 *
 * RETURN VALUES :  none
 *
****************************************************************************/

int main()
{
        signal(SIGALRM, sigintHandler);
        alarm(5);

	gst_init ( NULL, NULL);

        printf("\n\n\tPipeline Initialized Successfully...\n");

	pipeline = gst_pipeline_new(NULL);
	src      = gst_element_factory_make("v4l2src", NULL);
        convert  = gst_element_factory_make("videoconvert", NULL);
	encoder  = gst_element_factory_make("jpegenc", NULL);
	sink     = gst_element_factory_make("filesink", NULL);

	if (pipeline == NULL || src == NULL || encoder == NULL ||  convert == NULL || sink == NULL) 
        {
		g_error("\n\tFailed to create elements\n");
		return -1;
	}

        printf("\n\tAll the Elements were Created.....\n");

	g_object_set(src, "num-buffers", "1", NULL);
	g_object_set(src, "device", "/dev/video1", NULL);

        printf("\n\tSource Was Linked...\n");

	g_object_set(sink, "location", "image.jpg", NULL);

        printf("\n\tSink Was Linked...\n");

	gst_bin_add_many(GST_BIN(pipeline), src, convert,  encoder, sink, NULL);
	if (!gst_element_link_many(src, convert, encoder, sink, NULL))
        {
		g_error("\n\tFailed to link elements\n");
		return -2;
	}

	loop = g_main_loop_new(NULL, FALSE);

	bus = gst_pipeline_get_bus(GST_PIPELINE (pipeline));
	gst_bus_add_signal_watch(bus);
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
 * DESCRIPTION   :  stop capturing data from camera and saves a jpg file
 *                  
 *
****************************************************************************/
int sigintHandler(int unused)
{
        g_print("\n\tImage Taken...\n");
        gst_element_send_event(pipeline, gst_event_new_eos());
        exit(0);
}
