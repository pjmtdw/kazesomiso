#include "kazesomiso.h"

void ToriYomiDesign::UpdateSize()
  {
    if(tori_or_yomi == TORI){
      width = tori->width;
      height = tori->height;
    }else{
      width = yomi->width;
      height = yomi->height;
    }

  }
void MyGraphics::Init(){
  if(fudascale > 0.0){
    this->SetPageUnit(fudaunit);
    this->SetPageScale(fudascale);
  }
  this->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
  this->SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
  //�����ɃA���`�G�C���A�X��t����
  this->SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);
  //�摜�̊g��k���̎��ɍō��掿
  this->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
}
