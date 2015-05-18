#include "glcv.h"
#include <GL\glut.h>


cv::Mat depth_to_z(cv::Mat& depth, const cv::Mat& projection){
	float A = projection.ptr<float>(2)[2];
	float B = projection.ptr<float>(2)[3];
	int numpix = depth.rows * depth.cols;
	cv::Mat out(depth.rows, depth.cols, CV_32F);

	float * depth_ptr = depth.ptr<float>();
	float * out_ptr = out.ptr<float>();

	float zNear = -B / (1.0 - A);
	float zFar = B / (1.0 + A);

	for (int i = 0; i < numpix; ++i){
		float d = *depth_ptr;

		if (d == 1) //nothing
		{
			*out_ptr = 0;
		}
		else{

			//*out_ptr = 0.5 * (-A*d + B) + 0.5;
			//*out_ptr = -2 * B / (d + 1 + 2 * A);
			float z_b = d;
			float z_n = 2.0 * z_b - 1.0;
			float z_e = -2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
			*out_ptr = z_e;

		}
		++depth_ptr;
		++out_ptr;
	}

	return out;
}

void display_mat(cv::Mat texmat, bool fullscreen){
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glLoadIdentity();


	GLuint texture[1];

	//load in cv mat as texture
	glEnable(GL_TEXTURE_2D);

	if (texmat.data == NULL){
		std::cout << "failed to load texture...\n";
	}
	else{
		glGenTextures(1, texture);


		glBindTexture(GL_TEXTURE_2D, texture[0]);


		// select modulate to mix texture with color for shading
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		// when texture area is small, bilinear filter the closest mipmap
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if wrap is true, the texture wraps over at the edges (repeat)
		//       ... false, the texture ends at the edges (clamp)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexImage2D(GL_TEXTURE_2D, 0, 4, texmat.cols, texmat.rows, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, texmat.data);

		glColor3f(1, 1, 1);


		if (fullscreen){
			glBegin(GL_QUADS);
			glTexCoord2d(0.0, 0.0); glVertex2d(-1.0, -1.0);
			glTexCoord2d(1.0, 0.0); glVertex2d(1.0, -1.0);
			glTexCoord2d(1.0, 1.0); glVertex2d(1.0, 1.0);
			glTexCoord2d(0.0, 1.0); glVertex2d(-1.0, 1.0);
			glEnd();
		}
		else{

		}

		glDeleteTextures(1, texture);

	}



	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

cv::Mat gl_read_color(unsigned int win_width, unsigned int win_height){

	cv::Mat render_pretexture_4(win_height, win_width, CV_8UC4);
	glReadPixels(0, 0, win_width, win_height, GL_RGBA, GL_UNSIGNED_BYTE, render_pretexture_4.ptr<unsigned char>());

	cv::Mat render_pretexture_;
	cv::cvtColor(render_pretexture_4, render_pretexture_, CV_BGRA2BGR);

	cv::Mat render_pretexture;
	cv::flip(render_pretexture_, render_pretexture, 0);

	return render_pretexture;
}

cv::Mat gl_read_depth(unsigned int win_width, unsigned int win_height, const cv::Mat& opengl_projection){
	cv::Mat render_depth_(win_height, win_width, CV_32F);
	glReadPixels(0, 0, win_width, win_height, GL_DEPTH_COMPONENT, GL_FLOAT, render_depth_.ptr<float>());
	
	render_depth_ = depth_to_z(render_depth_, opengl_projection);

	cv::Mat render_depth;

	cv::flip(render_depth_, render_depth, 0);

	return render_depth;
}

/**
@brief basic function to produce an OpenGL projection matrix and associated viewport parameters
which match a given set of camera intrinsics. 
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
cv::Mat build_opengl_projection_for_intrinsics(int *viewport, double alpha, double beta, double skew, double u0, double v0, int img_width, int img_height, double near_clip, double far_clip){

	// These parameters define the final viewport that is rendered into by
	// the camera.
	double L = 0;
	double R = img_width;
	double B = 0;
	double T = img_height;

	// near and far clipping planes, these only matter for the mapping from
	// world-space z-coordinate into the depth coordinate for OpenGL
	double N = near_clip;
	double F = far_clip;

	// set the viewport parameters
	viewport[0] = L;
	viewport[1] = B;
	viewport[2] = R - L;
	viewport[3] = T - B;

	// construct an orthographic matrix which maps from projected
	// coordinates to normalized device coordinates in the range
	// [-1, 1].  OpenGL then maps coordinates in NDC to the current
	// viewport
	cv::Mat ortho = cv::Mat::zeros(4,4,CV_32F);
	ortho.ptr<float>(0)[0] = 2.0 / (R - L); ortho.ptr<float>(0)[3] = -(R + L) / (R - L);
	ortho.ptr<float>(1)[1] = 2.0 / (T - B); ortho.ptr<float>(1)[3] = -(T + B) / (T - B);
	ortho.ptr<float>(2)[2] = -2.0 / (F - N); ortho.ptr<float>(2)[3] = -(F + N) / (F - N);
	ortho.ptr<float>(3)[3] = 1.0;

	// construct a projection matrix, this is identical to the 
	// projection matrix computed for the intrinsicx, except an
	// additional row is inserted to map the z-coordinate to
	// OpenGL. 
	cv::Mat tproj = cv::Mat::zeros(4,4,CV_32F);
	tproj.ptr<float>(0)[0] = alpha; tproj.ptr<float>(0)[1] = skew; tproj.ptr<float>(0)[2] = -u0;
	tproj.ptr<float>(1)[1] = beta; tproj.ptr<float>(1)[2] = -v0;
	tproj.ptr<float>(2)[2] = (N + F); tproj.ptr<float>(2)[3] = N*F;
	tproj.ptr<float>(3)[2] = -1.0;

	// resulting OpenGL frustum is the product of the orthographic
	// mapping to normalized device coordinates and the augmented
	// camera intrinsic matrix
	cv::Mat frustum = ortho*tproj;
	return frustum;
}