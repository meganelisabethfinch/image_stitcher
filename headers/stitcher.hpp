class Stitcher {
    private:
        std::vector<cv::Mat> workingImages;

        // compute keypoints

        // compute descriptors

        // find matches + filter with ratio test

        // find homography

        // warp right image, add in left image

        // public method that does this, popping first two images and pushing back one
        // then pops/returns final image on stack

    public:
        // constructor - take *copy of* each image
        Stitcher(std::vector<cv::Mat> images);
};