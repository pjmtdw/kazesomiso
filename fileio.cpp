// fileio.cc

#include "kazesomiso.h"

BOOL LoadTorifuda(){
  std::ifstream ifs;
  ifs.open(TORIFUDAFILE);
  if(!ifs){
    char buf[100];
    wsprintf(buf,"エラー: ファイル %s を開けません。",TORIFUDAFILE);
    MessageBox(NULL,buf,"エラー",MB_OK);
    return FALSE;
  }
  std::string buf;
  while(std::getline(ifs,buf)){
    std::string::size_type pos;
    if(buf[buf.length()-1] == '\n') // いらなかった気がする
      buf = buf.substr(0,buf.length()-1); 
    if(buf[buf.length()-1] == '\r')
      buf = buf.substr(0,buf.length()-1);
    class torifuda f;
    pos = buf.find("\t");
    f.kimariji = buf.substr(0,pos);
    buf = buf.substr(pos+1);
    pos = buf.find("\t");
    f.kaminoku = buf.substr(0,pos);
    buf = buf.substr(pos+1);
    pos = buf.find("\t");
    f.simonoku = buf.substr(0,pos);
    if(pos != std::string::npos){
      buf = buf.substr(pos+1);
      f.explanation = buf;
    }else{
      f.explanation = std::string("");
    }
    f.nigatefuda = TRUE;
    f.saveasimage = TRUE;
    torifuda.push_back(f);
  }
  return TRUE;
}

BOOL LoadYomiFuda(){
  std::ifstream ifs;
  ifs.open(YOMIFUDAFILE);
  if(!ifs){
    char buf[100];
    wsprintf(buf,"エラー: ファイル %s を開けません。",YOMIFUDAFILE);
    MessageBox(NULL,buf,"エラー",MB_OK);
    return FALSE;
  }
  std::string buf;
  std::string b;
  while(std::getline(ifs,b)){
    if(b.length()>0 && b[b.length()-1] == '\n') // いらなかった気がする
      b = b.substr(0,b.length()-1); 
    if(b.length()>0 && b[b.length()-1] == '\r')
      b = b.substr(0,b.length()-1);
    std::string::size_type pos;
    pos = b.find_first_not_of("-");
    if(b.length() > 0 && pos == std::string::npos && buf.length() > 0){
      if(buf[buf.length()-1] == '\n')
        buf = buf.substr(0,buf.length()-1); 
      class yomifuda yom;
      yom.waka = buf;
      yom.saveasimage = TRUE;
      yomifuda.push_back(yom);
      buf = "";
      continue;
    }
    buf += b;
    buf += "\n";
  }

  if(buf.length() > 0){
    if(buf[buf.length()-1] == '\n')
      buf = buf.substr(0,buf.length()-1); 
    class yomifuda yom;
    yom.waka = buf;
    yom.saveasimage = TRUE;
    yomifuda.push_back(yom);
  }

  return TRUE;
}

BOOL SaveToIniFile(int num, const class ToriYomiDesign * s_design){
  char newfile[MAX_PATH];
  int sizeofclass = 0;
  char * filename;
  class BaseDesign * design;
  if(tori_or_yomi == TORI){
    wsprintf(newfile,"bg_image\\tori%d.img",num);
    sizeofclass = sizeof(class ToriDesign);
    filename = (char *)TORIINI;
    design = (class ToriDesign *)malloc(sizeofclass);
    if(s_design != NULL){
      memcpy(design,s_design->tori,sizeofclass);
    }
  }else{
    wsprintf(newfile,"bg_image\\yomi%d.img",num);
    sizeofclass = sizeof(class YomiDesign);
    filename = (char *)YOMIINI;
    design = (class YomiDesign *)malloc(sizeofclass);

    if(s_design != NULL){
      memcpy(design,s_design->yomi,sizeofclass);
    }
  }

  if(s_design != NULL && design->bgimage_mode != 0){
    lstrcpy(design->background_image,newfile);
  }


  DWORD r;

  HANDLE hr = CreateFile(filename,GENERIC_READ,0,NULL,OPEN_EXISTING,0,NULL);
  HANDLE hw = CreateFile("kazesomiso_temp.ini",GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,NULL);
  
  WriteFile(hw,magicword,8,&r,NULL);

  int size = 0;
  int notnigatefuda = 0;

  std::vector<class torifuda>::iterator it;

  if(s_design != NULL){
    if(tori_or_yomi == TORI){
      size += sizeof(int); // class ToriDesignの長さを格納しておく分
      size += sizeof(class ToriDesign);
      size += sizeof(int); // 下の句の長さを格納しておく分
      size += s_design->simonoku.length();
      size += sizeof(int); // 決まり字の長さを格納しておく分
      size += s_design->kimariji.length();
      size += sizeof(int); // 記憶している札の数を格納しておく分
    
      for(it = torifuda.begin(); it != torifuda.end(); it ++){
        if(it -> nigatefuda == FALSE){
          size += sizeof(int);
          notnigatefuda++;
        }
      }
    }else{
      size += sizeof(int); // class YomiDesignの長さを格納しておく分
      size += sizeof(class YomiDesign);
      size += sizeof(int); // 和歌の長さを格納しておく分
      size += s_design->waka.length();
    }
  }
  if(hr != INVALID_HANDLE_VALUE){
    char b[8+1];
    ReadFile(hr,b,8,&r,NULL);
    if(r != 8 || memcmp(b,magicword,8)){
      char buf[1024];
      wsprintf(buf,"エラー: 設定ファイルのバージョンが違います。 この %s は使えません。",filename);
      MessageBox(NULL,buf,"エラー",MB_OK);
      CloseHandle(hr);
      CloseHandle(hw);
      free(design);
      return FALSE;
    }
  }


  for(int i=0; i < num; i++){
    if(hr == INVALID_HANDLE_VALUE){
      int zero = 0;
      WriteFile(hw,&zero,sizeof(int),&r,NULL);
    }else{
      int s = 0;
      ReadFile(hr,&s,sizeof(int),&r,NULL);
      if(!r){
        int zero = 0;
        WriteFile(hw,&zero,sizeof(int),&r,NULL);
      }else{
        if(s < 0 ||  s > 1024 * 1024 ){
           MessageBox(NULL,"設定ファイル破損エラー","エラー:001",MB_OK);
           CloseHandle(hr);
           CloseHandle(hw);
           free(design);
           return FALSE;
        }
        char * buf = (char *) malloc(s);
        ReadFile(hr,buf,s,&r,NULL);
        WriteFile(hw,&s,sizeof(int),&r,NULL);
        WriteFile(hw,buf,s,&r,NULL);
        free(buf);
      }
    }
  }

  if(hr != INVALID_HANDLE_VALUE){
    int s = 0;
    ReadFile(hr,&s,sizeof(int),&r,NULL);
    if(s < 0 || s > 1024 * 1024 ){
       MessageBox(NULL,"設定ファイル破損エラー","エラー:002",MB_OK);
       CloseHandle(hr);
	   CloseHandle(hw);
       free(design);
       return FALSE;
    }

    char * buf = (char *) malloc(s);
    ReadFile(hr,buf,s,&r,NULL);
    free(buf);
  }

  if(s_design != NULL){
    if(tori_or_yomi == TORI){
      WriteFile(hw,&size,sizeof(int),&r,NULL);
      int s = sizeof(class ToriDesign);
      WriteFile(hw,&s,sizeof(int),&r,NULL);
      WriteFile(hw,design,sizeof(class ToriDesign),&r,NULL);
      s = s_design->simonoku.length();
      WriteFile(hw,&s,sizeof(int),&r,NULL);
      WriteFile(hw,s_design->simonoku.c_str(),s,&r,NULL);
      s = s_design->kimariji.length();
      WriteFile(hw,&s,sizeof(int),&r,NULL);
      WriteFile(hw,s_design->kimariji.c_str(),s,&r,NULL);
      WriteFile(hw,&notnigatefuda,sizeof(int),&r,NULL);
      int count = 0;
      for(it = torifuda.begin(); it != torifuda.end(); it ++){
        if(it -> nigatefuda == FALSE){
          WriteFile(hw,&count,sizeof(int),&r,NULL);
        }
        count ++;
      }
    }else{
      WriteFile(hw,&size,sizeof(int),&r,NULL);
      int s = sizeof(class YomiDesign);
      WriteFile(hw,&s,sizeof(int),&r,NULL);
      WriteFile(hw,design,sizeof(class YomiDesign),&r,NULL);
      s = s_design->waka.length();
      WriteFile(hw,&s,sizeof(int),&r,NULL);
      WriteFile(hw,s_design->waka.c_str(),s,&r,NULL);
    }
  }else{
    int zero = 0;
    WriteFile(hw,&zero,sizeof(int),&r,NULL);
  }
  if(hr != INVALID_HANDLE_VALUE){
    while(1){
      char buf[1024];
      int rr = ReadFile(hr,buf,sizeof(buf),&r,NULL);
      if(!rr || (rr && !r) ){
        break;
      }
      WriteFile(hw,buf,r,&r,NULL);
    }
  }
  CloseHandle(hr);
  CloseHandle(hw);
  DeleteFile(filename);
  if(!MoveFile("kazesomiso_temp.ini",filename)){
    char buf[1024];
    wsprintf(buf,"エラー: 設定ファイル %s に書き込めませんでした。",filename);
    MessageBox(NULL,buf,"エラー",MB_OK);
    free(design);
    return FALSE;
  }

  if(s_design != NULL){
    if(tori_or_yomi == TORI && s_design->tori->bgimage_mode != 0){
      CopyFile(s_design->tori->background_image,design->background_image,FALSE);
    }else if(tori_or_yomi == YOMI && s_design->yomi->bgimage_mode != 0){
      CopyFile(s_design->yomi->background_image,design->background_image,FALSE);
    }else{
      DeleteFile(newfile);
    }
  }else{
    DeleteFile(newfile);
  }

  free(design);
  return TRUE;
}

BOOL LoadFromIniFile(int num, class ToriYomiDesign & design, std::vector<int> * notnigate){
  char * filename;
  if(tori_or_yomi == TORI){
    filename = (char *)TORIINI;
  }else{
    filename = (char *)YOMIINI;
  }

  HANDLE h = CreateFile(filename,GENERIC_READ,0,NULL,OPEN_EXISTING,0,NULL);
  if(h == INVALID_HANDLE_VALUE){
    return FALSE;
  }
  DWORD r;
  char b[8+1];
  ReadFile(h,b,8,&r,NULL);
  if(r != 8 || memcmp(b,magicword,8)){
    char buf[1024];
    wsprintf(buf,"エラー: 設定ファイルのバージョンが違います。 この %s は使えません。",filename);
    MessageBox(NULL,buf,"エラー",MB_OK);
    CloseHandle(h);
    return FALSE;
  }
  
  for(int i = 0; i < num; i++){
    int j = 0;
    ReadFile(h,&j,sizeof(int),&r,NULL);
    SetFilePointer(h,j,NULL,FILE_CURRENT);
  }

  int s = 0;
  int rr = ReadFile(h,&s,sizeof(int),&r,NULL);
  if( (rr && !r) || s == 0 ){
    CloseHandle(h);
    return FALSE;
  }
  ReadFile(h,&s,sizeof(int),&r,NULL);
  if(tori_or_yomi == TORI && s != sizeof(class ToriDesign) ||
     tori_or_yomi == YOMI && s != sizeof(class YomiDesign)
   ){
    char buf[1024];
    wsprintf(buf,"エラー: 設定ファイル %s が壊れています。読み込めません。",filename);
    MessageBox(NULL,buf,"エラー",MB_OK);
    CloseHandle(h);
    return FALSE;
  }
  if(tori_or_yomi == TORI){
    ReadFile(h,design.tori,sizeof(class ToriDesign),&r,NULL);
  }else{
    ReadFile(h,design.yomi,sizeof(class YomiDesign),&r,NULL);
  }
  ReadFile(h,&s,sizeof(int),&r,NULL);
  if(s < 0 ||  s > 1024 * 1024 ){
    MessageBox(NULL,"設定ファイル破損エラー","エラー:003",MB_OK);
    CloseHandle(h);
    return FALSE;
  }

  char * buf = (char *)malloc(s+1);
  ReadFile(h,buf,s,&r,NULL);
  buf[s] = '\0';
  if(tori_or_yomi == TORI){
    design.simonoku = buf;
  }else{
    design.waka = buf;
  }
  free(buf);

  if(tori_or_yomi == YOMI){
    CloseHandle(h);
    return TRUE;
  }

  ReadFile(h,&s,sizeof(int),&r,NULL);
  if(s < 0 || s > 1024 * 1024 ){
    MessageBox(NULL,"設定ファイル破損エラー","エラー:004",MB_OK);
    CloseHandle(h);
    return FALSE;
  }

  buf = (char *)malloc(s+1);
  ReadFile(h,buf,s,&r,NULL);
  buf[s] = '\0';
  design.kimariji = buf;
  free(buf);

  ReadFile(h,&s,sizeof(int),&r,NULL);
  for(int i = 0; i < s; i++){
    int ss = 0;
    ReadFile(h,&ss,sizeof(int),&r,NULL);
    if(ss >= 0 && ss < torifuda.size()){
      notnigate->push_back(ss);
    }
  }
  
  CloseHandle(h);

  return TRUE;
}


BOOL LoadImageToDC(const HDC & hdc,const class ToriYomiDesign &toriyomi_design){

  class BaseDesign * design;
  if(tori_or_yomi == TORI){
    design = toriyomi_design.tori;
  }else{
    design = toriyomi_design.yomi;
  }
  int mode = design->bgimage_mode;

  if(bgimage_structure.hdc != NULL){
    DeleteDC(bgimage_structure.hdc);
    bgimage_structure.hdc = NULL;
    if(!gdiplus_useable){
      DeleteObject(bgimage_structure.hbmp);
      bgimage_structure.hbmp = NULL;
    }else{
      delete bgimage_structure.gbmp;
      bgimage_structure.gbmp = NULL;
    }
  }

  if(mode == 0 || design->background_image[0] == '\0'){
    //表示しない
    return TRUE;
  }
  
  // ファイルタイプの判定
  HANDLE hin = CreateFile(design->background_image,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
  if(hin == INVALID_HANDLE_VALUE){
    return FALSE;
  }
  DWORD r;
  unsigned char b[3];
  ReadFile(hin,b,sizeof(b),&r,NULL);
  CloseHandle(hin);

  HBITMAP hbmp = NULL;

  //画像をhbmpにロード
  if( b[0] == 'B' && b[1] == 'M' ){
    //BMPファイル
    hbmp = (HBITMAP)LoadImage(hInst,design->background_image,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
  }else{
    BOOL r = ToBmp(design->background_image,"kazesomiso_temp_2.bmp");
    if(!r){
      return FALSE;
    }
    hbmp = (HBITMAP)LoadImage(hInst,"kazesomiso_temp_2.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    DeleteFile("kazesomiso_temp_2.bmp");
  }

  BITMAP bmp;
  if(!GetObject(hbmp,sizeof(BITMAP),&bmp)){
    return FALSE;
  }
  int bw = bmp.bmWidth;
  int bh = bmp.bmHeight;

  if(bw <= 0 || bh <= 0){
    return FALSE;
  }

  //透明度の設定
  BITMAPINFO info;
  ZeroMemory(&info,sizeof(BITMAPINFO));
  info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
  info.bmiHeader.biWidth=bw;
  info.bmiHeader.biHeight=bh;
  info.bmiHeader.biPlanes=1;
  info.bmiHeader.biBitCount=24;
  info.bmiHeader.biCompression=BI_RGB;

  LPBYTE lpDibData; 
  HBITMAP hDib = CreateDIBSection(NULL,&info,0,(VOID**)&lpDibData,NULL,0);

  HDC hdc11 = CreateCompatibleDC(hdc);
  HDC hdc21 = CreateCompatibleDC(hdc);

  HBITMAP hold11 = (HBITMAP)SelectObject(hdc11,hbmp);
  HBITMAP hold21 = (HBITMAP)SelectObject(hdc21,hDib);

  BitBlt(hdc21,0,0,bw,bh,hdc11,0,0,SRCCOPY);

  for(int i =0; i < bw; i ++){
    for(int j = 0; j < bh; j++){
      int index = (i + j * bw) * 3;
      int bb = lpDibData[index];
      int gg = lpDibData[index+1];
      int rr = lpDibData[index+2];

      double tt = double(design->bgimage_transparency) / 100.0;
      lpDibData[index] = bb + int((0xFF - bb) * tt);
      lpDibData[index+1] = gg + int((0xFF - gg) * tt);
      lpDibData[index+2] = rr + int((0xFF - rr) * tt);

    }
  }
  BitBlt(hdc11,0,0,bw,bh,hdc21,0,0,SRCCOPY);

  SelectObject(hdc11,hold11);
  SelectObject(hdc21,hold21);

  DeleteObject(hDib);
  DeleteDC(hdc11);
  DeleteDC(hdc21);

  int w = design->width - design->border_left_right_width * 2;
  int h = design->height - design->border_top_bottom_width * 2;

  int bw_dest = bw;
  int bh_dest = bh;

  // 背景画像の拡大縮小はメモリのビットマップ上で行う
  // プリンタの性能が悪ければこちらを試す。
  // if(fudascale > 0.0){
  //   w *= fudascale_x;
  //   h *= fudascale_y;
  //   bw_dest *= fudascale_x;
  //   bh_dest *= fudascale_y;
  // }

  // modeによってどう配置するか決定
  if(mode == 1){
    //並べて表示
    HDC hdc2 = CreateCompatibleDC(hdc);

    HDC hdc1;
    HBITMAP hbmp2,hold1,hold2;
    Gdiplus::Graphics * g1;
    Gdiplus::Bitmap * b2;

    if(!gdiplus_useable){
      hdc1 = CreateCompatibleDC(hdc);
      hbmp2 = CreateCompatibleBitmap(hdc,w,h);
      hold1 = (HBITMAP)SelectObject(hdc1,hbmp);
      hold2 = (HBITMAP)SelectObject(hdc2,hbmp2);
    }else{
      bgimage_structure.gbmp = new Gdiplus::Bitmap(w,h,PixelFormat24bppRGB);
      g1 = new Gdiplus::Graphics(bgimage_structure.gbmp);
      b2 = new Gdiplus::Bitmap(hbmp,Gdiplus::Graphics::GetHalftonePalette());
      g1->SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
    }
    int x = 0, y = 0;
    while(1){
      int ww, hh;
      if(x + bw_dest > w){
        ww = w - x;
      }else{
        ww = bw_dest;
      }
      if(y + bh_dest > h){
        hh = h - y;
      }else{
        hh = bh_dest;
      }
      if(!gdiplus_useable){
        if(bw_dest != bw || bh_dest != bh){
          SetStretchBltMode(hdc2,HALFTONE);
          StretchBlt(hdc2,x,y,ww,hh,hdc1,0,0,bw,bh,SRCCOPY);
        }else{
          BitBlt(hdc2,x,y,ww,hh,hdc1,0,0,SRCCOPY);
        }
      }else{
        Gdiplus::ImageAttributes ia;
        ia.SetWrapMode(Gdiplus::WrapModeTile);
        Gdiplus::Rect r(x,y,bw_dest,bh_dest);
        g1->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
        g1->DrawImage(b2,r,0,0,bw,bh,Gdiplus::UnitPixel,&ia);
      }
      x += bw_dest;
      if( x >= w){
        x = 0;
        y += bh_dest;
      }
      if( y >= h){
        break;
      }
    }

    if(!gdiplus_useable){
      SelectObject(hdc1,hold1);
      SelectObject(hdc2,hold2);
      bgimage_structure.hbmp = hbmp2;
      DeleteDC(hdc1);
    }else{
      delete g1;
      delete b2;
    }
    
    bgimage_structure.hdc = hdc2;
  }else if(mode == 2){

    if(fudascale > 0.0){
      w = bw;
      h = bh;
    }

    //拡大縮小
    HDC hdc2 = CreateCompatibleDC(hdc);
    if(!gdiplus_useable){
      HDC hdc1 = CreateCompatibleDC(hdc);
      HBITMAP hbmp2 = CreateCompatibleBitmap(hdc,w,h);
      HBITMAP hold1 = (HBITMAP)SelectObject(hdc1,hbmp);
      HBITMAP hold2 = (HBITMAP)SelectObject(hdc2,hbmp2);

      SetStretchBltMode(hdc2,HALFTONE);
      StretchBlt(hdc2,0,0,w,h,hdc1,0,0,bw,bh,SRCCOPY);

      SelectObject(hdc1,hold1);
      SelectObject(hdc2,hold2);
      bgimage_structure.hbmp = hbmp2;
      DeleteDC(hdc1);
    }else{
      bgimage_structure.gbmp = new Gdiplus::Bitmap(w,h,PixelFormat24bppRGB);
      
      Gdiplus::Bitmap b2(hbmp,Gdiplus::Graphics::GetHalftonePalette());

      Gdiplus::Graphics g1(bgimage_structure.gbmp);
      g1.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
      g1.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

      Gdiplus::ImageAttributes ia;
      ia.SetWrapMode(Gdiplus::WrapModeTileFlipXY);
      Gdiplus::Rect r(0,0,w,h);
      g1.DrawImage(&b2,r,0,0,bw,bh,Gdiplus::UnitPixel,&ia);
    }

    bgimage_structure.hdc = hdc2;
  }else if(mode >= 3 && mode <= 7 ){
    //3: 中央 4:左上 5:右上 6:左下 7:右下
    HDC hdc2 = CreateCompatibleDC(hdc);

    HDC hdc1;
    HBITMAP hbmp2,hold1,hold2;

    Gdiplus::Graphics * g1;
    Gdiplus::Bitmap * b2;


    if(!gdiplus_useable){
      hdc1 = CreateCompatibleDC(hdc);
      hbmp2 = CreateCompatibleBitmap(hdc,w,h);
      hold1 = (HBITMAP)SelectObject(hdc1,hbmp);
      hold2 = (HBITMAP)SelectObject(hdc2,hbmp2);
      HBRUSH hbr = CreateSolidBrush(design->background_color);
      RECT r;
      r.top = 0; r.left = 0; r.bottom = h, r.right = w;
      FillRect(hdc2,&r,hbr);
      DeleteObject(hbr);
    }else{
      bgimage_structure.gbmp = new Gdiplus::Bitmap(w,h,PixelFormat24bppRGB);
      g1 = new Gdiplus::Graphics(bgimage_structure.gbmp);
      g1->SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);

      b2 = new Gdiplus::Bitmap(hbmp,Gdiplus::Graphics::GetHalftonePalette());

      Gdiplus::Rect rct(0,0,w,h);
      Gdiplus::Color clr;
      clr.SetFromCOLORREF(design->background_color);
      Gdiplus::SolidBrush br(clr);
      g1->FillRectangle(&br,rct);
    }
    int x,y;
    if(mode == 3){
      //中央
      x = (w - bw_dest) / 2;
      y = (h - bh_dest) / 2;
    }else if(mode == 4){
      //左上
      x = 0;
      y = 0;
    }else if(mode == 5){
      //右上
      x = w - bw_dest;
      y = 0;
    }else if(mode == 6){
      //左下
      x = 0;
      y = h - bh_dest;
    }else if(mode == 7){
      //右下
      x = w - bw_dest;
      y = h - bh_dest;
    }
    if(!gdiplus_useable){
      if(bw_dest != bw || bh_dest != bh){
        SetStretchBltMode(hdc2,HALFTONE);
        StretchBlt(hdc2,x,y,bw_dest,bh_dest,hdc1,0,0,bw,bh,SRCCOPY);
      }else{
        BitBlt(hdc2,x,y,bw,bh,hdc1,0,0,SRCCOPY);
      }
      SelectObject(hdc1,hold1);
      SelectObject(hdc2,hold2);
      bgimage_structure.hbmp = hbmp2;
      DeleteDC(hdc1);
    }else{
      g1->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
      Gdiplus::ImageAttributes ia;
      ia.SetWrapMode(Gdiplus::WrapModeTileFlipXY);
      Gdiplus::Rect r(x,y,bw_dest,bh_dest);
      g1->DrawImage(b2,r,0,0,bw,bh,Gdiplus::UnitPixel,&ia);

      delete g1;
      delete b2;
    }
    bgimage_structure.hdc = hdc2;
  }

  DeleteObject(hbmp);
  return TRUE;
}

void SaveFudaToBitmap(int fudanum, LPCTSTR lpszFn,const class ToriYomiDesign & toriyomi_design,double scale) {

  class ToriYomiDesign design;
  if(tori_or_yomi == TORI){
    design.simonoku = torifuda[fudanum].simonoku;
    design.kimariji = torifuda[fudanum].kimariji;
    design.tori = toriyomi_design.tori;
  }else{
    design.waka = yomifuda[fudanum].waka;
    design.yomi = toriyomi_design.yomi;
  }
  design.UpdateSize();

  DWORD dwSize,dwFSize,dwWidth,dwHeight,dwLength;
  HANDLE fh;
  LPBITMAPFILEHEADER lpHead;
  LPBITMAPINFOHEADER lpInfo;
  LPBYTE lpBuf,lpPixel;
  HDC hdc,hdcMem;
  HBITMAP hBMP,hOld;
  dwWidth=design.width * scale;
  dwHeight=design.height * scale;
  if ((dwWidth*3) % 4==0) /* バッファの１ラインの長さを計算 */
  dwLength=dwWidth*3;
  else
  dwLength=dwWidth*3+(4-(dwWidth*3) % 4);
  /* 書き込み用バッファのサイズ計算 */
  dwFSize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+dwLength*dwHeight;
  /* バッファ確保とポインタ設定 */
  lpBuf=(LPBYTE)GlobalAlloc(GPTR,dwFSize);
  lpHead=(LPBITMAPFILEHEADER)lpBuf;
  lpInfo=(LPBITMAPINFOHEADER)(lpBuf+sizeof(BITMAPFILEHEADER));
  lpPixel=lpBuf+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
  
  /* 24ビットBMPファイルのヘッダ作成 */
  lpHead->bfType='M'*256+'B';
  lpHead->bfSize=dwFSize;
  lpHead->bfOffBits=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
  lpInfo->biSize=sizeof(BITMAPINFOHEADER);
  lpInfo->biWidth=dwWidth;
  lpInfo->biHeight=dwHeight;
  lpInfo->biPlanes=1;
  lpInfo->biBitCount=24;
  
  /* デバイスコンテキスト作成 */
  hdc=CreateDC("DISPLAY",NULL,NULL,NULL);

  /* デバイスコンテキスト互換のBITMAP作成 */
  hBMP=CreateCompatibleBitmap(hdc,dwWidth,dwHeight);
  /* BITMAPに領域をコピー */

  hdcMem=CreateCompatibleDC(hdc);
  hOld=(HBITMAP)SelectObject(hdcMem,hBMP);

  fudascale = scale;
  fudascale_x = scale;
  fudascale_y = scale;
  fudaunit = Gdiplus::UnitPixel;
  DrawFuda(design,hdcMem,NULL);
  fudaunit = Gdiplus::UnitDisplay;
  fudascale = -1.0;

  /* デスクトップに進行状況を表示 */

  double aspect_ratio = double(dwWidth) / double(dwHeight);
  
  int scWidth = GetSystemMetrics(SM_CXSCREEN);
  int scHeight = GetSystemMetrics(SM_CYSCREEN);

  double screen_ratio = double(scWidth) / double(scHeight);

  int fudamaxnum = torifuda.size();

  int cols = int(sqrt(double(fudamaxnum) * screen_ratio / aspect_ratio));

  int rows = fudamaxnum / cols + ( (fudamaxnum % cols == 0 ) ? 0 : 1);

  int newHeight = int(double(scHeight) / double(rows));
  int newWidth = int(aspect_ratio * newHeight);

  int col = fudanum % cols;
  int row = fudanum / cols;

  int x = col * newWidth;
  int y = row * newHeight;

  if(!gdiplus_useable){
    SetStretchBltMode(hdc,HALFTONE);
    StretchBlt(hdc,x,y,newWidth,newHeight,hdcMem,0,0,dwWidth,dwHeight,SRCCOPY);
  }else{
    Gdiplus::Bitmap gbmp(hBMP,Gdiplus::Graphics::GetHalftonePalette());
    Gdiplus::Graphics g(hdc);
    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
    g.DrawImage(&gbmp,x,y,newWidth,newHeight);
  }
  SelectObject(hdcMem,hOld);
  GetDIBits(hdc,hBMP,0,dwHeight,lpPixel,(LPBITMAPINFO)lpInfo,DIB_RGB_COLORS);
  DeleteDC(hdc);
  DeleteObject(hBMP);
  DeleteObject(hdcMem);
  
  /* バッファをファイルに書き出す */
  fh=CreateFile(lpszFn,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
  WriteFile(fh,lpBuf,dwFSize,&dwSize,NULL);
  CloseHandle(fh);
  GlobalFree(lpBuf);
}
