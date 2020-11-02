#include "videotexture.hpp"

namespace msi_vr
{

    VideoTexture::VideoTexture()
        : Texture()
    {

    }

    VideoTexture::VideoTexture(std::string const &filename, int width, int height)
        : filename(filename), Texture()
    {
        loadVideo(filename, width, height);
    }

    VideoTexture::~VideoTexture()
    {
        if(videoLoaded) stop();
    }

    bool VideoTexture::loadVideo(std::string const &filename, int width, int height)
    {
        if(videoLoaded) return false;

        this->filename = filename;
        if(width > 0 && height > 0)
        {
            initialize(width, height, GL_RGBA);
            sizeSet = true;
        }
        initElements();
        initPipeline();
        //start();
        return videoLoaded = true;
    }

    void VideoTexture::start()
    {
        if(videothread.joinable())
        {
            return;
        }

        videothread = std::thread([this]()
        {
            g_main_loop_run(loop);
        });
        seek(std::chrono::nanoseconds(0));
        
        this->play();
    }

    void VideoTexture::play()
    {
        gst_element_set_state(pipeline, GST_STATE_PLAYING);
    }

    void VideoTexture::pause()
    {
        gst_element_set_state(pipeline, GST_STATE_PAUSED);
    }

    void VideoTexture::stop()
    {
        if(videothread.joinable())
        {
            g_main_loop_quit(loop);
            videothread.join();
        }

        seek(std::chrono::nanoseconds(0));
        gst_element_set_state(pipeline, GST_STATE_NULL);
    }

    void VideoTexture::seek(std::chrono::nanoseconds time)
    {
        gst_element_seek_simple(pipeline, GST_FORMAT_TIME, static_cast<GstSeekFlags>(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT), time.count());
    }


    void VideoTexture::update()
    {
        if(!sizeSet && width > 0 && height > 0)
        {
                initialize(width, height, GL_RGBA);
                sizeSet = true;
        }
        if (bufferinfo.buffer)
        {
            gst_buffer_unmap(bufferinfo.buffer, &bufferinfo.mapped_info);
            gst_buffer_unref(bufferinfo.buffer);
        }

        if (incoming.load())
        {
            if (!gst_buffer_map(incoming, &bufferinfo.mapped_info, GST_MAP_READ))
                std::cerr << "could not map gstreamer buffer for reading" << std::endl;

            bufferinfo.buffer = incoming;
            incoming = nullptr;
        }
        else
        {
            bufferinfo.buffer = nullptr;
            return;
        }

        if(bufferinfo.buffer) upload(bufferinfo.mapped_info.data);
    }

    int VideoTexture::initElements()
    {
        gst_init(NULL, NULL);

        loop = g_main_loop_new(NULL, false);

        source = gst_element_factory_make("filesrc", NULL);
        g_object_set(G_OBJECT(source), "location", filename.c_str(), NULL);

        decoder = gst_element_factory_make("decodebin", NULL);
        scaler = gst_element_factory_make("videoscale", NULL);
        converter = gst_element_factory_make("videoconvert", NULL);


        sink = gst_element_factory_make("fakesink", NULL);
        g_object_set(G_OBJECT(sink), "sync", true, NULL);
        g_object_set(G_OBJECT(sink), "signal-handoffs", true, NULL);
        g_signal_connect(sink, "handoff", G_CALLBACK(handoff_callback), this);

        if (!source || !sink)
        {
            std::cerr << "GStreamer couldnt initialize all GstElements!" << std::endl;
            return -1;
        }
        return 0;
    }

    int VideoTexture::initPipeline()
    {
        pipeline = gst_pipeline_new(NULL);

        // build the pipeline
        gst_element_set_state(pipeline, GST_STATE_PAUSED);
        gst_bin_add_many(GST_BIN(pipeline), source, decoder, scaler, converter, sink, NULL);

        int linked = linkElements();
        if(linked != 0) return linked;

        bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
        gst_bus_add_watch(bus, (GstBusFunc)bus_callback, this);
        gst_object_unref(bus);

        gst_element_set_state(pipeline, GST_STATE_PAUSED);

        return 0;
    }

    int VideoTexture::linkElements()
    {
        if (gst_element_link(source, decoder) != true)
        {
            std::cerr << "GStreamer could not link source and decoder!" << std::endl;
            return -1;
        }
        if(sizeSet)
        {
            caps = gst_caps_new_simple("video/x-raw", "width", G_TYPE_INT, this->width,
                                                      "height", G_TYPE_INT, this->height, NULL);
            if(gst_element_link_filtered(scaler, converter, caps) != true)
            {
                std::cerr << "GStreamer could not link scaler and converter!" << std::endl;
            }
            gst_caps_unref(caps);
        }
        else
        {
            if(gst_element_link(scaler, converter) != true)
            {
                std::cerr << "GStreamer could not link scaler and converter!" << std::endl;
            }
        }

        caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "RGBA", NULL);
        if(gst_element_link_filtered(converter, sink, caps) != true)
        {
            std::cerr << "GStreamer could not link converter and sink!" << std::endl;
            return -1;
        }
        gst_caps_unref(caps);

        g_signal_connect(decoder, "pad-added", G_CALLBACK(pad_added_handler), this);

        return 0;
    }

    gboolean VideoTexture::bus_callback(GstBus *bus, GstMessage *message, VideoTexture *video)
    {
        switch (GST_MESSAGE_TYPE(message))
        {
        case GST_MESSAGE_EOS:
            //std::cout << "woot" << std::endl;
            if (video->repeat)
            {
                gst_element_seek_simple(video->pipeline, GST_FORMAT_TIME, 
                static_cast<GstSeekFlags>(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT), 0 * GST_SECOND);
            }
            else
            {
                video->pause();
            }
            break;
        }
        return true;
    }

    void VideoTexture::pad_added_handler(GstElement *src, GstPad *newPad, VideoTexture *video)
    {
        GstPad *sinkPad = gst_element_get_static_pad(video->scaler, "sink");

        if(gst_pad_is_linked(sinkPad)) return;

        GstCaps *newPadCaps = gst_pad_get_current_caps(newPad);
        GstStructure *newPadStruct = gst_caps_get_structure(newPadCaps, 0);
        const gchar *newPadType = gst_structure_get_name(newPadStruct);

        if(!g_strrstr(newPadType, "video")) return;

        GstPadLinkReturn ret = gst_pad_link(newPad, sinkPad);
        if(GST_PAD_LINK_FAILED(ret))
        {
            std::cerr << "Type is '" << newPadType << "' but link failed." << std::endl;
        }

    }

    void VideoTexture::handoff_callback(GstElement *src, GstBuffer *buffer, GstPad *pad, VideoTexture *video)
    {
        if (!video->sizeSet)
        {
            GstCaps *caps = gst_pad_get_current_caps(pad);
            if (caps)
            {
                GstStructure *structure = gst_caps_get_structure(caps, 0);
                gst_structure_get_int(structure, "width", &(video->width));
                gst_structure_get_int(structure, "height", &(video->height));
            }
        }

        if(video->changeBuffer == NULL || *video->changeBuffer)
        {
            gst_buffer_ref(buffer);
            video->incoming = buffer;
        }
    }

} // namespace msi_vr