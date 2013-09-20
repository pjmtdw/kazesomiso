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
  //文字にアンチエイリアスを付ける
  this->SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);
  //画像の拡大縮小の時に最高画質
  this->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
}
