# Image Stitcher
A simple panorama or image stitcher. The image stitcher uses SIFT to detect keypoints in a pair of images. 

We then apply the ratio test described in [1] to filter for good matches. We match each keypoint descriptor in the first image to its two nearest neighbours in the other. A match is considered 'good' if the distances of the two nearest neighbours differ by less than a ratio; otherwise, the match is likely to be erroneous and is discarded.

## Examples
Input:

<p float="left">
<img src="https://github.com/meganelisabethfinch/image-stitcher/blob/main/images/NewnLeft.jpeg" alt="Peile Building, Newnham College (left side)" width="49%" />
<img src="https://github.com/meganelisabethfinch/image-stitcher/blob/main/images/NewnRight.jpeg" alt="Peile Building, Newnham College (right side)" width="49%"  /> 
</p>

Output:

<p float="left">
<img src="https://github.com/meganelisabethfinch/image-stitcher/blob/main/images/NewnOutColour.jpeg" alt="A stitched image of Peile Building, Newnham College" width=98%" />

<p>

## :books: Sources
[1] Lowe, D. (2004). Distinctive image features from scale-invariant key- points. **International Journal of Computer Vision, 60**(2), 91–110.
