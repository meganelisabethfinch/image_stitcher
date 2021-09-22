#include <opencv2/core/mat.hpp>

class Stitcher {
    private:
        double ratio;

    public:
        // constructor
        Stitcher(double ratio = 0.6);
        
        int stitch(cv::Mat leftImg, cv::Mat rightImg, cv::Mat& out);

        int stitch(std::vector<cv::Mat> images, cv::Mat& out);
};