#include <opencv2/core/mat.hpp>

class Stitcher {
    private:
        double ratio;

        int stitchAdjacent(cv::Mat leftImg, cv::Mat rightImg, cv::Mat& out);

    public:
        // constructor
        Stitcher(double ratio = 0.6);

        int stitch(std::vector<cv::Mat> images, cv::Mat& out);
};