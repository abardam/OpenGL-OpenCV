#include "glcv.h"
#include <GL\glut.h>
#include <AssimpCV.h>

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