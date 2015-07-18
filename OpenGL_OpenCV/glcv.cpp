#include "glcv.h"


cv::Mat depth_to_z(cv::Mat& depth, const cv::Mat& projection){
	int depth_mult = 1;
	float A = projection.ptr<float>(2)[2];
	float B = projection.ptr<float>(2)[3];
	int numpix = depth.rows * depth.cols;
	cv::Mat out(depth.rows, depth.cols, CV_32F);

	float * depth_ptr = depth.ptr<float>();
	float * out_ptr = out.ptr<float>();

	float zNear = -B / (1.0 - A);
	float zFar = B / (1.0 + A);

	if (zNear < 0 && zFar < 0){
		float zNear_ = -zFar;
		float zFar_ = -zNear;
		zNear = zNear_;
		zFar = zFar_;
		depth_mult = -1;
	}

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
			*out_ptr = depth_mult * z_e;

		}
		++depth_ptr;
		++out_ptr;
	}

	return out;
}

GLuint mat_to_texture(cv::Mat texmat){
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	GLuint texture[1];
	//load in cv mat as texture
	

	if (texmat.data == NULL){
		std::cout << "failed to load texture...\n";
		return 0;
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

		if (texmat.channels() == 3){
			glTexImage2D(GL_TEXTURE_2D, 0, 4, texmat.cols, texmat.rows, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, texmat.data);
		}
		else if (texmat.channels() == 4){
			glTexImage2D(GL_TEXTURE_2D, 0, 4, texmat.cols, texmat.rows, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, texmat.data);

		}

		return texture[0];
	}

}



void mat_to_texture(cv::Mat texmat, GLuint texture){
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (texmat.data == NULL){
		std::cout << "failed to load texture...\n";
		return;
	}
	else{
		glBindTexture(GL_TEXTURE_2D, texture);

		// select modulate to mix texture with color for shading
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		// when texture area is small, bilinear filter the closest mipmap
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if wrap is true, the texture wraps over at the edges (repeat)
		//       ... false, the texture ends at the edges (clamp)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		if (texmat.channels() == 3){
			glTexImage2D(GL_TEXTURE_2D, 0, 4, texmat.cols, texmat.rows, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, texmat.data);
		}
		else if (texmat.channels() == 4){
			glTexImage2D(GL_TEXTURE_2D, 0, 4, texmat.cols, texmat.rows, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, texmat.data);

		}
	}

}

void display_mat(cv::Mat texmat, bool fullscreen){
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glLoadIdentity();




	if (texmat.data == NULL){
		std::cout << "failed to load texture...\n";
	}
	else{

		GLuint texture[1];

		texture[0] = mat_to_texture(texmat);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture[0]);

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
cv::Mat build_opengl_projection_for_intrinsics(int *viewport, double alpha, double beta, double skew, double u0, double v0, int img_width, int img_height, double near_clip, double far_clip, int z_neg){

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
	tproj.ptr<float>(0)[0] = alpha; tproj.ptr<float>(0)[1] = skew; tproj.ptr<float>(0)[2] = z_neg * u0;
	tproj.ptr<float>(1)[1] = beta; tproj.ptr<float>(1)[2] = z_neg * v0;
	tproj.ptr<float>(2)[2] = -1 * z_neg *(N + F); tproj.ptr<float>(2)[3] = N*F;
	tproj.ptr<float>(3)[2] = z_neg * 1.0;

	// resulting OpenGL frustum is the product of the orthographic
	// mapping to normalized device coordinates and the augmented
	// camera intrinsic matrix
	cv::Mat frustum = ortho*tproj;
	return frustum;
}

cv::Mat build_opengl_projection_for_intrinsics_2(int *viewport, double alpha, double beta, double skew, double u0, double v0, int img_width, int img_height, double near_clip, double far_clip){
	cv::Mat out = cv::Mat::zeros(4, 4, CV_32F);
	out.ptr<float>(0)[0] = alpha / (0.5f * img_width);
	out.ptr<float>(1)[1] = beta / (0.5f * img_height);
	out.ptr<float>(2)[2] = -(near_clip + far_clip) / (far_clip - near_clip);
	out.ptr<float>(2)[3] = -2 * near_clip*far_clip / (far_clip - near_clip);
	out.ptr<float>(3)[2] = -1.0f;

	return out;
}

cv::Mat draw_normals(const cv::Mat& depth_img, const cv::Mat& camera_matrix){
	cv::Mat camera_matrix_inv = camera_matrix.inv();
	cv::Mat normal_display(depth_img.size(), CV_32FC3);
	cv::Vec3f half(0.5, 0.5, 0.5);
	for (int y = 0; y < depth_img.rows-1; ++y){
		for (int x = 0; x < depth_img.cols-1; ++x){
			
			cv::Vec3f v(0, 0, 0);

			if (depth_img.ptr<float>(y)[x] == 0 || depth_img.ptr<float>(y + 1)[x] == 0 || depth_img.ptr<float>(y)[x + 1] == 0){
			}
			else{
				depth_img.ptr<float>(y)[x];

				cv::Mat pts_screen = cv::Mat::ones(4,3,CV_32F);
				pts_screen.ptr<float>(0)[0] = x;
				pts_screen.ptr<float>(1)[0] = y;
				pts_screen.ptr<float>(0)[1] = x+1;
				pts_screen.ptr<float>(1)[1] = y;
				pts_screen.ptr<float>(0)[2] = x;
				pts_screen.ptr<float>(1)[2] = y+1;
				cv::Mat pts_camplane = camera_matrix_inv * pts_screen;

				cv::Vec3f pt_camera_0(pts_camplane.ptr<float>(0)[0], 
					pts_camplane.ptr<float>(1)[0], 
					pts_camplane.ptr<float>(2)[0]);
				pt_camera_0 *= depth_img.ptr<float>(y)[x];;

				cv::Vec3f pt_camera_1x(pts_camplane.ptr<float>(0)[1],
					pts_camplane.ptr<float>(1)[1],
					pts_camplane.ptr<float>(2)[1]);
				pt_camera_1x *= depth_img.ptr<float>(y)[x+1];;

				cv::Vec3f pt_camera_1y(pts_camplane.ptr<float>(0)[2],
					pts_camplane.ptr<float>(1)[2],
					pts_camplane.ptr<float>(2)[2]);
				pt_camera_1y *= depth_img.ptr<float>(y+1)[x];;

				v = (pt_camera_1y - pt_camera_0).cross(pt_camera_1x - pt_camera_0);
				if (v(2) == 0){
					v = cv::Vec3f(0, 0, 0);
				}
				else{
					v = cv::normalize(v);
				}
			}

			normal_display.ptr<cv::Vec3f>(y)[x] = v / 2 + half;

		}
	}

	return normal_display;
}