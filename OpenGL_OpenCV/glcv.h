#pragma once
#include <opencv2\opencv.hpp>


//converts from openGL depth (-1, 1) to true depth
cv::Mat depth_to_z(cv::Mat& depth, const cv::Mat& OPENGL_PROJECTION_MATRIX);

void display_mat(cv::Mat texmat, bool fullscreen);

cv::Mat gl_read_color(unsigned int win_width, unsigned int win_height);

cv::Mat gl_read_depth(unsigned int win_width, unsigned int win_height, const cv::Mat& opengl_projection);

/**
@brief basic function to produce an OpenGL projection matrix and associated viewport parameters
which match a given set of camera intrinsics. http://jamesgregson.blogspot.jp/2011/11/matching-calibrated-cameras-with-opengl.html
@param[out] viewport 4-component OpenGL viewport values, as might be retrieved by glGetIntegerv( GL_VIEWPORT, &viewport[0] )
@param[in]  alpha x-axis focal length, from camera intrinsic matrix
@param[in]  alpha y-axis focal length, from camera intrinsic matrix
@param[in]  skew  x and y axis skew, from camera intrinsic matrix
@param[in]  u0 image origin x-coordinate, from camera intrinsic matrix
@param[in]  v0 image origin y-coordinate, from camera intrinsic matrix
@param[in]  img_width image width, in pixels
@param[in]  img_height image height, in pixels
@param[in]  near_clip near clipping plane z-location, can be set arbitrarily > 0, controls the mapping of z-coordinates for OpenGL
@param[in]  far_clip  far clipping plane z-location, can be set arbitrarily > near_clip, controls the mapping of z-coordinate for OpenGL
*/
cv::Mat build_opengl_projection_for_intrinsics(int *viewport, double alpha, double beta, double skew, double u0, double v0, int img_width, int img_height, double near_clip, double far_clip, int z_neg=1);
cv::Mat build_opengl_projection_for_intrinsics_2(int *viewport, double alpha, double beta, double skew, double u0, double v0, int img_width, int img_height, double near_clip, double far_clip);