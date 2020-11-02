#ifndef __MSI_VR__VIDEOTEXTURE_HPP__
#define __MSI_VR__VIDEOTEXTURE_HPP__

#include <memory>
#include <thread>
#include <atomic>
#include <mutex>

#include <gst/gst.h>

#include "texture.hpp"

namespace msi_vr
{
    class VideoSynchronizer;
    class VideoTexture : public Texture
    {
        friend class VideoSynchronizer;
        public:
        std::string filename;
        bool repeat = true;
        bool sizeSet = false;
        bool videoLoaded = false;

        bool* changeBuffer = NULL;

        VideoTexture();
        VideoTexture(std::string const &filename, int width = -1, int height = -1);
        ~VideoTexture();

        bool loadVideo(std::string const &filename, int width = -1, int height = -1);

        void start();
        void play();
        void pause();
        void stop();

        void seek(std::chrono::nanoseconds time);

        void update();

        private:
        int initElements();
        int initPipeline();
        int linkElements();
        GstElement *pipeline=NULL, *source=NULL, *decoder=NULL, *scaler=NULL, *converter=NULL, *sink=NULL;
        GstCaps *caps; // to determine the output of the converter
        GstBus *bus;
        GMainLoop* loop; // cant have unique_ptr for some reason
        std::atomic<GstBuffer*> incoming{nullptr};

        std::thread videothread;
        struct BufferInfo
        {
            GstMapInfo mapped_info;
            GstBuffer *buffer{nullptr};
        } bufferinfo;

        static gboolean bus_callback(GstBus *bus, GstMessage *message, VideoTexture *video);
        static void pad_added_handler(GstElement *src, GstPad *newPad, VideoTexture *video);
        static void handoff_callback(GstElement *src, GstBuffer *buffer, GstPad *pad, VideoTexture *video);

    };
}

#endif