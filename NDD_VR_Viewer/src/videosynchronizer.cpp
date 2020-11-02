#include "videosynchronizer.hpp"

namespace msi_vr
{

    VideoSynchronizer::VideoSynchronizer()
    {
        gst_init(NULL, NULL);

        loop = g_main_loop_new(NULL, false);

        pipeline = gst_pipeline_new(NULL);
        gst_element_set_state(pipeline, GST_STATE_PAUSED);
    }

    VideoSynchronizer::~VideoSynchronizer()
    {
        stop();
    }

    void VideoSynchronizer::start()
    {
        if(videothread.joinable())
        {
            return;
        }
        gst_element_set_state(pipeline, GST_STATE_PAUSED);

        videothread = std::thread([this]()
        {
            g_main_loop_run(loop);
        });
        seek(std::chrono::nanoseconds(0));
        
        this->play();
    }

    void VideoSynchronizer::play()
    {
        gst_element_set_state(pipeline, GST_STATE_PLAYING);
    }

    void VideoSynchronizer::pause()
    {
        gst_element_set_state(pipeline, GST_STATE_PAUSED);
    }

    void VideoSynchronizer::stop()
    {
        if (videothread.joinable())
        {
            g_main_loop_quit(loop);
            videothread.join();
        }

        seek(std::chrono::nanoseconds(0));
        gst_element_set_state(pipeline, GST_STATE_NULL);
    }

    void VideoSynchronizer::seek(std::chrono::nanoseconds time)
    {
        gst_element_seek_simple(pipeline, GST_FORMAT_TIME, static_cast<GstSeekFlags>(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT), time.count());
    }

    void VideoSynchronizer::update()
    {
        this->pause();
        changeBuffer = false;
        for(auto &video: videos)
        {
            video->update();
        }
        changeBuffer = true;
        this->play();
    }

    int VideoSynchronizer::addVideo(VideoTexture *video)
    {
        video->stop();
        this->stop();

        gst_bin_remove_many(GST_BIN(video->pipeline), video->source, video->decoder, video->scaler, video->converter, video->sink, NULL);
        video->initElements();

        gst_bin_add_many(GST_BIN(pipeline), video->source, video->decoder, video->scaler, video->converter, video->sink, NULL);
        video->linkElements();
        GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
        gst_bus_add_watch(bus, (GstBusFunc)bus_callback, this);

        video->changeBuffer = &changeBuffer;
        videos.push_back(video);

        return 0;
    }

    void VideoSynchronizer::removeVideo(VideoTexture *video)
    {
    }

    gboolean VideoSynchronizer::bus_callback(GstBus *bus, GstMessage *message, VideoSynchronizer *videosyncer)
    {
        switch (GST_MESSAGE_TYPE(message))
        {
        case GST_MESSAGE_EOS:
            //std::cout << "woot" << std::endl;
            videosyncer->seek(std::chrono::nanoseconds(0));
            if (!videosyncer->repeat)
            {
                videosyncer->pause();
            }
            break;
        }
        return true;
    }

} // namespace msi_vr