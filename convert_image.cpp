// convert_image.cc
// BMP���瑼�̉摜�`���ɕϊ�������A���̋t��

#include "kazesomiso.h"

#include "imgctl.h"

BOOL ToBmp(const char * lpImageFile, const char * lpBmpFile)
{
  BOOL bRes = FALSE;
  HDIB hDIB = NULL;

  hDIB = ToDIB(lpImageFile);
  if (hDIB != NULL)
  {
    bRes = DIBtoBMP(lpBmpFile, hDIB);
    DeleteDIB(hDIB);
  }

  return bRes;
}


BOOL Bmp2Jpg(const char * lpImageFile, const char * lpJpegFile, int iQuality)
{
  BOOL bProgression = FALSE;

  BOOL bRes = FALSE;
  HDIB hDIB = NULL;

  hDIB = ToDIB(lpImageFile);
  if (hDIB != NULL)
  {
    bRes = DIBtoJPG(lpJpegFile, hDIB, iQuality, bProgression);
    DeleteDIB(hDIB);
  }
  return bRes;
}


BOOL Bmp2Png(const char * lpImageFile, const char * lpPngFile)
{
  BOOL bInterlacing = FALSE;

  BOOL bRes = FALSE;
  HDIB hDIB = NULL;

  hDIB = ToDIB(lpImageFile);
  if (hDIB != NULL)
  {
    bRes = DIBtoPNG(lpPngFile, hDIB, bInterlacing);
    DeleteDIB(hDIB);
  }

  return bRes;
}


/* �摜�t�@�C����GIF�t�@�C���ɂ��� */
BOOL Bmp2Gif(const char *  lpImageFile, const char * lpGifFile)
{
  BOOL bInterlacing = FALSE;

  BOOL bRes = FALSE;
  HDIB hDIB = NULL;

  hDIB = ToDIB(lpImageFile);
  if (hDIB != NULL)
  {
    bRes = DIBtoGIF(lpGifFile, hDIB, bInterlacing);
    DeleteDIB(hDIB);
  }

  return bRes;
}
