
#include "stdafx.h"

void COGLTexture::LoadFromFile(char *filename)
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1,&ID); 
	glBindTexture( GL_TEXTURE_2D, ID);
	Image = auxDIBImageLoadA( (const char*) filename );
	Width = Image->sizeX;
	Height = Image->sizeY;
	gluBuild2DMipmaps(	GL_TEXTURE_2D, 
						3, 
						Image->sizeX,
						Image->sizeY,
						GL_RGB,
						GL_UNSIGNED_BYTE,
						Image->data);
	delete Image;
}

void COGLTexture::SetActive()
{
	glBindTexture( GL_TEXTURE_2D, ID);
}

unsigned int COGLTexture::GetID()
{
	return ID;
}

