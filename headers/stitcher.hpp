#include <opencv2/core/mat.hpp>

class Stitcher {
    private:


    public:
        // constructor
        Stitcher();

        cv::Mat stitch(std::vector<cv::Mat> images);
};