/****************************************************************************
 *   ProjectName : 
 *
 *   COMPONENT   : 
 *
 *   Author      :
 *
 *   DATE        : 
 *
 *   Description : 
 *
 *  pipeline     : gst-launch-0.10 -e v4l2src device=/dev/video0 ! x264enc ! mux. audiotestsrc  ! voaacenc ! mux. flvmux streamable=true name=mux ! filesink location=auto.mp4 sync=false  
gst-launch-0.10 -e v4l2src device=/dev/video0  ! x264enc ! h264parse ! queue !  mux. audiotestsrc  ! audioconvert ! lamemp3enc ! queue ! mux. flvmux name=mux ! filesink  location=camera.mp4
*******************************************************************************/

/******************************************************************************
* Include Files
******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <gst/gst.h>
#include <signal.h>

/******************************************************************************
* Macro Definition
******************************************************************************/
//common return values
#define G_PASS 0
#define G_FAIL -1

//shared mem key
#define SHARED_KEY 1234

//string sizes
#define STRG_64  64
#define STRG_128 128
#define STRG_256 256
#define STRG_512 512
#define MAXSTR 1024



/******************************************************************************
* Global variables
******************************************************************************/
int iQuitRecord;
char acRecfile[STRG_128];
char acPrevfile[STRG_128];
//gst-launch-0.10 -e v4l2src device=/dev/video0 ! x264enc ! mux. audiotestsrc  ! voaacenc ! mux. flvmux streamable=true name=mux ! filesink location=auto.mp4 sync=false
GstElement *rec_apipeline = NULL;
GstElement *rec_avideosrc = NULL;
GstElement *rec_aaudiosrc = NULL;
GstElement *rec_amuxer = NULL;
GstElement *rec_asink = NULL;
GstElement *rec_afilter = NULL;
GstElement *rec_aencoder = NULL;
GstElement *rec_aaudioencoder = NULL;
GstElement *rec_videobin = NULL;
GstElement *rec_audiobin = NULL;
GstCaps *rec_audiofiltercaps = NULL;
GstCaps *rec_afiltercaps = NULL;



GstPad *rec_videosrcpad;
GstPad *rec_audiosrcpad;
GstPad *rec_videosinkpad;
GstPad *rec_audiosinkpad;
GstPad *rec_sinkpad;

bool bStreamStart;

void StopRecord();

/******************************************************************************
* Function Prototypes
******************************************************************************/

/*function to get gstreamer bus messages*/
static gboolean bus_call(GstBus *bus,GstMessage *message,gpointer *data);


/****************************************************************************
 *
 * NAME          :  bus_call
 *
 * PARAMETERS    :  GstBus *bus,GstMessage *message,gpointer *data
 *
 * RETURN VALUES :  gboolean
 *
 * DESCRIPTION   :  callback function and receives gstreamer bus messages
 *
****************************************************************************/
static gboolean bus_call(GstBus *bus,GstMessage *message,gpointer *data)
{
    gchar *debug;
    GError *error;

    GMainLoop *loop = (GMainLoop *)data;
    GST_DEBUG ("got message %s\n",gst_message_type_get_name (GST_MESSAGE_TYPE (message)));

    switch (GST_MESSAGE_TYPE(message))
    {
        case GST_MESSAGE_ERROR:
                  {
                      gst_message_parse_error(message,&error,&debug);
                      g_free(debug);
                      printf("Error Occured %s\n",error->message);
                      g_error_free(error);
                      g_main_loop_quit(loop);
                      break;
                  }
        case GST_MESSAGE_EOS:
                  {
                        g_print ("\n\tGot EOS\n");
        		g_main_loop_quit (loop);
	        	gst_element_set_state (rec_apipeline, GST_STATE_NULL);
		        g_main_loop_unref (loop);
	        	gst_object_unref (rec_apipeline);
		        g_print("\n\tFile Saved...\n");
		        exit(0);
		        break;
                  }
        default:
              break;
    }
    return TRUE;
}

/****************************************************************************
 *
 * NAME          :  main
 *
 * PARAMETERS    :  void
 *
 * RETURN VALUES :  none
 *
 * DESCRIPTION   :  start rtmp stream with audio and video
 *
****************************************************************************/
int main(int argc, char *argv[])
{

    GMainLoop *loop=NULL;
    GstBus *bus=NULL;


    memset(acRecfile, '\0', strlen(acRecfile));

    iQuitRecord = 0;
    loop = g_main_loop_new(NULL,FALSE);

    signal(SIGINT,StopRecord);

    /* Initialize GStreamer */
    gst_init(NULL,NULL);

    rec_videobin = gst_bin_new ("videobin");
    rec_audiobin = gst_bin_new ("audiobin");

    rec_apipeline = gst_pipeline_new("rec-pipeline");
    rec_avideosrc = gst_element_factory_make ("videotestsrc", "camerasource");
    rec_aaudiosrc = gst_element_factory_make ("audiotestsrc", "audiosource");
    rec_afilter   = gst_element_factory_make("capsfilter", NULL) ;
    rec_aencoder  = gst_element_factory_make ("x264enc", "H264encoder");
    rec_aaudioencoder = gst_element_factory_make ("voaacenc", "audioencoder");
    rec_asink     = gst_element_factory_make ("filesink", "sinkvideo");
    rec_amuxer    = gst_element_factory_make ("qtmux", "muxer");

    if(  rec_apipeline == NULL || rec_amuxer == NULL|| rec_avideosrc == NULL || rec_aencoder == NULL || rec_asink == NULL || rec_aaudioencoder == NULL || rec_aaudiosrc == NULL ||rec_afilter == NULL )
    {
        printf("Error in gstreamer elements creation\n");
        return G_FAIL;
    }
    printf("Start capture all gstreamer elements created\n");

    g_object_set (G_OBJECT(rec_asink),"location","movie.mp4", (const char*)NULL);

    gst_bin_add_many (GST_BIN(rec_apipeline),rec_avideosrc,rec_aencoder,rec_amuxer,rec_asink,rec_aaudiosrc,rec_aaudioencoder,NULL);


    if (gst_element_link_many (rec_avideosrc,rec_aencoder,rec_amuxer,NULL) != TRUE)
    {
        printf("Elements rec_avideosrc,encode,muxer could not be linked.\n");
        return G_FAIL;
    }
        

    if (gst_element_link_many (rec_aaudiosrc,rec_aaudioencoder,rec_amuxer,NULL) != TRUE)
    {
        printf("audio_src, audio_encoder muxer could not be linked.\n");
        return G_FAIL;
    }
    

    if (gst_element_link_many (rec_amuxer,rec_asink,NULL) != TRUE)
    {
        printf("muxer and sink  could not be linked.\n");
        return G_FAIL;
    }
   
	loop = g_main_loop_new(NULL, FALSE);

	bus = gst_pipeline_get_bus(GST_PIPELINE (rec_apipeline));
	gst_bus_add_signal_watch(bus);
	g_signal_connect(G_OBJECT(bus), "message", G_CALLBACK(bus_call), NULL);
	gst_object_unref(GST_OBJECT(bus));

	gst_element_set_state(rec_apipeline, GST_STATE_PLAYING);

	g_print("\n\tStarting loop...\n");
	g_main_loop_run(loop);
   
}

/****************************************************************************
 *
 * NAME          :  iStopRecord
 *
 * PARAMETERS    :  void
 *
 * RETURN VALUES :  none
 *
 * DESCRIPTION   :  stop capturing data from camera and close pipeline
 *
****************************************************************************/
void StopRecord()
{
    gst_element_send_event(rec_apipeline, gst_event_new_eos());
}
