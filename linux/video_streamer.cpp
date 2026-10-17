#include "video_streamer.h"
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/videodev2.h>

namespace whorld {

VideoStreamer::~VideoStreamer() {
    close();
}

void VideoStreamer::close() {
    if (fd >= 0) {
        if (fd != STDOUT_FILENO) {
            ::close(fd);
        }
        fd = -1;
    }
    w = h = 0;
    isV4L2 = false;
}

bool VideoStreamer::openV4L2(const std::string& device, int width, int height, std::string& error) {
    close();
    fd = ::open(device.c_str(), O_RDWR);
    if (fd < 0) {
        error = "Failed to open V4L2 device " + device + ": " + std::strerror(errno);
        return false;
    }

    struct v4l2_format vid_format;
    std::memset(&vid_format, 0, sizeof(vid_format));
    vid_format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    vid_format.fmt.pix.width = width;
    vid_format.fmt.pix.height = height;
    vid_format.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
    vid_format.fmt.pix.sizeimage = width * height * 3;
    vid_format.fmt.pix.field = V4L2_FIELD_NONE;
    vid_format.fmt.pix.colorspace = V4L2_COLORSPACE_SRGB;

    if (ioctl(fd, VIDIOC_S_FMT, &vid_format) < 0) {
        // Fallback to BGR24 if RGB24 is rejected
        vid_format.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
        if (ioctl(fd, VIDIOC_S_FMT, &vid_format) < 0) {
            error = "V4L2 VIDIOC_S_FMT failed: " + std::string(std::strerror(errno));
            ::close(fd);
            fd = -1;
            return false;
        }
    }

    w = width;
    h = height;
    isV4L2 = true;
    error.clear();
    return true;
}

bool VideoStreamer::openPipe(const std::string& path, int width, int height, std::string& error) {
    close();
    if (path == "-") {
        fd = dup(STDOUT_FILENO);
        if (fd < 0) {
            error = "Failed to dup stdout: " + std::string(std::strerror(errno));
            return false;
        }
    } else {
        mkfifo(path.c_str(), 0666);
        fd = ::open(path.c_str(), O_WRONLY);
        if (fd < 0) {
            error = "Failed to open pipe " + path + ": " + std::strerror(errno);
            return false;
        }
    }

    w = width;
    h = height;
    isV4L2 = false;
    error.clear();
    return true;
}

bool VideoStreamer::writeFrame(const unsigned char* rgbData, std::size_t size) {
    if (fd < 0 || !rgbData || size == 0) return false;
    std::size_t written = 0;
    while (written < size) {
        ssize_t ret = ::write(fd, rgbData + written, size - written);
        if (ret < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        written += static_cast<std::size_t>(ret);
    }
    return true;
}

} // namespace whorld
