#ifndef __MSI_VR__VIDEOSYNCHRONIZER_HPP__
#define __MSI_VR__VIDEOSYNCHRONIZER_HPP__

#include <vector>
#include <chrono>

#include "videotexture.hpp"

namespace msi_vr
{
    class VideoSynchronizer
    {
        public:
        bool repeat = true;

        VideoSynchronizer();
        ~VideoSynchronizer();

        void start();
        void play();
        void pause();
        void stop();

        void seek(std::chrono::nanoseconds time);

        void update();

        int addVideo(VideoTexture* video);
        void removeVideo(VideoTexture* video);

        std::vector<VideoTexture*> videos;

        bool changeBuffer = true;
        private:
        GstElement *pipeline=NULL;
        GMainLoop* loop; // cant have unique_ptr for some reason
        std::thread videothread;

        static gboolean bus_callback(GstBus *bus, GstMessage *message, VideoSynchronizer *videosyncer);
    };

} // namespace msi_vr

#endif