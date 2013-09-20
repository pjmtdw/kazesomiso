#include "kazesomiso.h"

#include "firstfuda.h"

const char * TORIFUDAFILE = "torifuda.txt";
const char * YOMIFUDAFILE = "yomifuda.txt";
const char magicword[8+1] = "KAEDE001";
const char * YOMIINI = "kazesomiso_yomi.ini";
const char * TORIINI = "kazesomiso_tori.ini";
const char * szTitle = "かぜそみそ";
const char * szWindowClass = "kazesomiso_qn7t0ee";

double fudascale = -1.0;
enum Gdiplus::Unit fudaunit = Gdiplus::UnitDisplay;
double fudascale_x,fudascale_y;

enum TORIYOMI tori_or_yomi;

class ToriDesign currenttori;
class YomiDesign currentyomi;

HINSTANCE hInst;
HWND hWndMain;
HWND hDlgWnd = NULL;

std::vector<class torifuda> torifuda;
std::vector<class torifuda *> torifuda_of_fudaotosi;
int fudaotosinum;

std::vector<class yomifuda> yomifuda;

class BackStructure front_structure;
class BackStructure bgimage_structure;

class ToriYomiDesign currentdesign;
class torifuda * currentfuda;
class torifuda firstfuda;

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);

LRESULT CALLBACK  WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK  ToriDesignProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK  YomiDesignProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK  FudaOtosiProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK  SaveAsImageProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK  SettingsProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK  VersionProc(HWND, UINT, WPARAM, LPARAM);

std::string GetMaxMinDisplayColors(int *, int *);


BOOL gdiplus_useable = FALSE;

void ClearNigateFuda();
void CreateFudaOtositorifuda();

int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPTSTR lpCmdLine,int nCmdShow)
{
  HWND hprevwnd = FindWindow(szWindowClass,NULL);
  if ( hprevwnd != NULL){
    MessageBox(NULL,"「かぜそみそ」はすでに起動しています","(・◇・)",MB_OK);
    return 0;
  }

  // GDI+が使えるかチェック
  HMODULE hmd = LoadLibrary("gdiplus.dll");
  if(hmd != NULL){
    gdiplus_useable = TRUE;
    FreeLibrary(hmd); // 遅延ロードで勝手にロードしてくれるはずなのでFreeする
  }else{
    gdiplus_useable = FALSE;
  }

  ULONG_PTR gdiplusToken = NULL;
  if(gdiplus_useable){
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
  }
  srand(GetTickCount());

  SetCurrentDirectoryToExePath();

  if(!LoadTorifuda()){
    return 0;
  }
  if(!LoadYomiFuda()){
    return 0;
  }
  std::string s1,s2;
  std::vector<int> notnigate;

  currentdesign.tori = & currenttori;

  currentdesign.yomi = & currentyomi;

  tori_or_yomi = TORI;
  LoadFromIniFile(0,currentdesign,&notnigate);

  tori_or_yomi = YOMI;
  LoadFromIniFile(0,currentdesign,NULL);

  tori_or_yomi = TORI;
  currentdesign.UpdateSize();

  std::vector<int>::iterator it;
  for(it = notnigate.begin(); it != notnigate.end(); it++){
   torifuda[*it].nigatefuda = FALSE;
  }
  firstfuda.simonoku = firstfudalist[myrandint(sizeof(firstfudalist) / sizeof(*firstfudalist))];
  firstfuda.kimariji = std::string("きまりじ");

  currentfuda = &firstfuda;
  currentdesign.kimariji = currentfuda->kimariji;
  currentdesign.simonoku = currentfuda->simonoku;
  currentdesign.waka = std::string("難波津(なにはづ)に\n咲(さ)くやこの花(はな)\n冬(ふゆ)ごもり\n今(いま)を春(はる)べと\n咲(さ)くやこの花(はな)");

  MSG msg;
  MyRegisterClass(hInstance);
  if (!InitInstance (hInstance, nCmdShow))
  {
    return FALSE;
  }
  HACCEL hAccelTable;
  hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_KAZESOMISO);
  BeginPrinter();
  while (GetMessage(&msg, NULL, 0, 0))
  {
    if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
    {
      if(hDlgWnd == NULL || !IsDialogMessage(hDlgWnd,&msg)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
  }
  EndPrinter();
  if(gdiplusToken != NULL){
    Gdiplus::GdiplusShutdown(gdiplusToken);
  }
  return (int) msg.wParam;
}
ATOM MyRegisterClass(HINSTANCE hInstance)
{
  WNDCLASSEX wcex;
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style      = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc  = (WNDPROC)WndProc;
  wcex.cbClsExtra    = 0;
  wcex.cbWndExtra    = 0;
  wcex.hInstance    = hInstance;
  wcex.hIcon      = LoadIcon(hInstance, (LPCTSTR)IDI_KAZESOMISO);
  wcex.hCursor    = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
  wcex.lpszMenuName  = (LPCTSTR)IDC_KAZESOMISO;
  wcex.lpszClassName  = szWindowClass;
  wcex.hIconSm    = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_KAZESOMISO);
  return RegisterClassEx(&wcex);
}
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
  HWND hWnd;
  hInst = hInstance;
  hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION ,
  CW_USEDEFAULT, 0, currentdesign.tori->width, currentdesign.tori->height, NULL, NULL, hInstance, NULL);
  if (!hWnd)
  {
    return FALSE;
  }
  hWndMain = hWnd;
  HDC hdc = GetDC(hWndMain);
  LoadImageToDC(hdc,currentdesign);
  ReleaseDC(hWndMain,hdc);

  ShowWindow(hWnd, nCmdShow);

  UpdateMainWindow(TRUE);

  FixWindowSizeToFudaSize();
  return TRUE;
}

const char * robinson = "ROBINSON";
const char * timegoesby = "TIMEGOESBY";
char * robinsonp = NULL;
char * timegoesbyp = NULL;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  int wmId, wmEvent;
  PAINTSTRUCT ps;
  HDC hdc;
  switch (message)
  {
  case WM_CREATE:
    {
      robinsonp = (char *)robinson;
      timegoesbyp = (char *)timegoesby;
    }
    break;
  case WM_COMMAND:
    wmId    = LOWORD(wParam);
    wmEvent = HIWORD(wParam);
    switch (wmId)
    {
    case IDM_TORISETTINGS: case IDM_YOMISETTINGS:
      {
        if(wmId == IDM_TORISETTINGS){
          ChangeToriYomi(TORI);
        }else{
          ChangeToriYomi(YOMI);
        }
        DialogBox(hInst,MAKEINTRESOURCE(IDD_SETTINGS),hWnd,(DLGPROC)SettingsProc);

        HDC hdc = GetDC(hWndMain);
        LoadImageToDC(hdc,currentdesign);
        ReleaseDC(hWndMain,hdc);
      }
      break;
    case IDM_SAVEASIMAGE: case IDM_YOMIASIMAGE:
      {
        if(wmId == IDM_SAVEASIMAGE){
          ChangeToriYomi(TORI);
        }else{
          ChangeToriYomi(YOMI);
        }
        
        DialogBox(hInst,MAKEINTRESOURCE(IDD_SAVEIMAGE),hWnd,(DLGPROC)SaveAsImageProc);
      }
      break;
    case IDM_FUDADESIGN: case IDM_YOMIDESIGN:
      {
        if(wmId == IDM_FUDADESIGN){
          ChangeToriYomi(TORI);
          HWND hDlg = CreateDialog(hInst,MAKEINTRESOURCE(IDD_FUDADESIGN),hWnd,(DLGPROC)ToriDesignProc);
          ShowWindow(hDlg,SW_SHOW);
        }else{
          ChangeToriYomi(YOMI);
          HWND hDlg = CreateDialog(hInst,MAKEINTRESOURCE(IDD_YOMIDESIGN),hWnd,(DLGPROC)YomiDesignProc);
          ShowWindow(hDlg,SW_SHOW);
        }
      }
      break;
    case IDM_FUDAOTOSI:
      {
        ChangeToriYomi(TORI);
        CreateFudaOtositorifuda();
        if(torifuda_of_fudaotosi.size() == 0){
          if(IDYES!=MessageBox(hWnd,"貴方はすでに全ての札を覚えています。\r\nまた最初からやり直しますか？\r\n","確認",MB_YESNO)){
            return 0;
          };
          ClearNigateFuda();
          CreateFudaOtositorifuda();
        }
        HWND hDlg = CreateDialog(hInst,MAKEINTRESOURCE(IDD_FUDAOTOSI),hWnd,(DLGPROC)FudaOtosiProc);
        ShowWindow(hDlg,SW_SHOW);

      }
      break;
    case IDM_VERSION:
      DialogBox(hInst,MAKEINTRESOURCE(IDD_VERSION),hWnd,(DLGPROC)VersionProc);
      break;
    case IDM_EXIT:
      DestroyWindow(hWnd);
      break;
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
    }
    break;
    case WM_PAINT:{
      hdc = BeginPaint(hWnd, &ps);

      if(front_structure.hdc != NULL){
        int width, height;
        width = currentdesign.width;
        height = currentdesign.height;

        HBITMAP hold1 = (HBITMAP)SelectObject(front_structure.hdc,front_structure.hbmp);
        HBITMAP hCurrentBMP = CreateCompatibleBitmap(hdc,width,height);
        HBITMAP hold = (HBITMAP)SelectObject(hdc,hCurrentBMP);
        BitBlt(hdc,0,0,width,height,front_structure.hdc,0,0,SRCCOPY);
        SelectObject(hdc,hold);
        SelectObject(front_structure.hdc,hold1);
        DeleteObject(hCurrentBMP);
      }

     EndPaint(hWnd, &ps);
    }
    break;
  case WM_DESTROY:
    tori_or_yomi = TORI;
    SaveToIniFile(0, &currentdesign);
    tori_or_yomi = YOMI;
    SaveToIniFile(0, &currentdesign);
    DeleteFile("bg_image\\tori_current.img");
    DeleteFile("bg_image\\yomi_current.img");
    PostQuitMessage(0);
    break;
  case WM_KEYDOWN:
    {
      // robinson

      if(robinsonp != NULL){
        if(wParam == *robinsonp){
          robinsonp ++;
        }else{
          robinsonp = (char *)robinson;
        }
        if(*robinsonp == '\0'){
          robinsonp = (char *)robinson;

          HMODULE hmd = GetModuleHandle("gdiplus.dll");
          if(hmd == NULL){
            MessageBox(hWnd,"GDI+ には対応していません","(@_@)",MB_OK);
          }else{
            if(!gdiplus_useable){
              gdiplus_useable = 1;
              SetWindowText(hWnd,"GDI+: ON");
            }else{
              gdiplus_useable = 0;
              SetWindowText(hWnd,"GDI+: OFF");
            }
            HDC hdc = GetDC(hWndMain);
            LoadImageToDC(hdc,currentdesign);
            ReleaseDC(hWndMain,hdc);
            UpdateMainWindow(FALSE);
          }
        }
      }

      // timegoesby
      if(timegoesbyp != NULL){
        if(wParam == *timegoesbyp){
          timegoesbyp ++;
        }else{
          timegoesbyp = (char *)timegoesby;
        }
        if(*timegoesbyp == '\0'){
          timegoesbyp = (char *)timegoesby;

          std::string buf;
          int maxbpp, minbpp;
          buf += GetMaxMinDisplayColors(&maxbpp,&minbpp);
          char b[16];
          wsprintf(b,"%d",maxbpp);
          buf += b;
          buf += " ビットに変更 -> [はい]\r\n";
          wsprintf(b,"%d",minbpp);
          buf += b;
          buf += " ビットに変更 -> [いいえ]\r\n";
          int r = MessageBox(hWnd,buf.c_str(),"(^θ^)",MB_YESNOCANCEL);
          if( r == IDYES || r == IDNO){
            DEVMODE dv;
            ZeroMemory(&dv,sizeof(DEVMODE));
            dv.dmSize = sizeof(DEVMODE);
            dv.dmFields = DM_BITSPERPEL;
            if(r == IDYES){
              dv.dmBitsPerPel = maxbpp;
            }else{
              dv.dmBitsPerPel = minbpp;
            }
            ChangeDisplaySettings(&dv,0);
          }
        }
      }


    }
    break;
  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}

LRESULT CALLBACK VersionProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
  case WM_INITDIALOG:
    {
      HWND h = GetDlgItem(hDlg,IDC_DEVICEINFO);
      std::string buf;
      buf += "* * * * * * * *\r\n";
      buf += "* ディスプレイ情報\r\n";
      buf += "* * * * * * * *\r\n";
      buf += GetMaxMinDisplayColors(NULL,NULL);
      buf += "---\r\n";
      if(gdiplus_useable){
        HMODULE hmd = GetModuleHandle("gdiplus.dll");
        char b[MAX_PATH] = "";
        GetModuleFileName(hmd,b,sizeof(b)-1);
        buf += "GDI+: 使用可->\"";
        buf += b;
        buf += "\"\r\n";
      }else{
        buf += "GDI+: 使用不可->gdiplus.dllが見つからない\r\n";
      }
      buf += "---\r\n";
      DWORD r;
      const int dcnum1[]     = { RASTERCAPS , CURVECAPS , LINECAPS , POLYGONALCAPS , TEXTCAPS };
      const char * dcname1[] = {"RASTERCAPS","CURVECAPS","LINECAPS","POLYGONALCAPS","TEXTCAPS"};
      const int dcnum2[]     = { 
       NUMBRUSHES , NUMPENS , NUMFONTS , NUMCOLORS ,
       HORZSIZE , VERTSIZE , HORZRES , VERTRES , LOGPIXELSX , LOGPIXELSY ,
       BITSPIXEL , PLANES , ASPECTX , ASPECTY , ASPECTXY , CLIPCAPS , 
       PHYSICALWIDTH , PHYSICALHEIGHT , PHYSICALOFFSETX , PHYSICALOFFSETY , SCALINGFACTORX , SCALINGFACTORY };
      const char * dcname2[] = {
       "NUMBRUSHES","NUMPENS","NUMFONTS","NUMCOLORS",
       "HORZSIZE","VERTSIZE","HORZRES","VERTRES","LOGPIXELSX","LOGPIXELSY",
       "BITSPIXEL","PLANES","ASPECTX","ASPECTY","ASPECTXY","CLIPCAPS",
       "PHYSICALWIDTH","PHYSICALHEIGHT","PHYSICALOFFSETX","PHYSICALOFFSETY","SCALINGFACTORX","SCALINGFACTORY"};
      
      HDC hdc = CreateDC("DISPLAY",NULL,NULL,NULL);
      for(int i = 0; i < sizeof(dcnum1)/sizeof(*dcnum1); i++){
        char b[16];
        r = GetDeviceCaps(hdc,dcnum1[i]);
        wsprintf(b,"%08X",r);
        buf += dcname1[i];
        buf += ": ";
        buf += b;
        buf += "\r\n";
      }
      DeleteDC(hdc);
      buf += "* * * * * * * *\r\n";
      buf += "* プリンタ情報 \r\n";
      buf += "* * * * * * * *\r\n";
      if(hPrintDC == NULL){
        buf += "プリンタDCを取得できません\r\n";
      }else{
        if(hDevNames == NULL){
          buf += "DEVNAMES が NULL です\r\n";
        }else{
          DEVNAMES * pdn = (DEVNAMES *)GlobalLock(hDevNames);
          buf += "デバイスドライバ: ";
          buf += (char *)pdn + pdn->wDriverOffset;
          buf += "\r\n";
          buf += "デバイス名: ";
          buf += (char *)pdn + pdn->wDeviceOffset;
          buf += "\r\n";
          buf += "出力メディア: ";
          buf += (char *)pdn + pdn->wOutputOffset;
          buf += "\r\n";
          GlobalUnlock(hDevNames);
        }
        buf += "---\r\n";
        if(hDevMode == NULL){
          buf += "DEVMODE が NULL です\r\n";
        }else{
          DEVMODE * pdv = (DEVMODE *)GlobalLock(hDevMode);
          char bb[16];
          if(pdv->dmFields & DM_ORIENTATION){wsprintf(bb,"%d",pdv->dmOrientation );buf +=           "ORIENTATION: "; buf += bb; buf+="\r\n";}
          if(pdv->dmFields & DM_PAPERSIZE){wsprintf(bb,"%d",pdv->dmPaperSize );buf +=               "PAPERSIZE: "; buf += bb; buf+="\r\n";}
          if(pdv->dmFields & DM_PAPERLENGTH){wsprintf(bb,"%d",pdv->dmPaperLength );buf +=           "PAPERLENGTH: "; buf += bb; buf+="\r\n";}
          if(pdv->dmFields & DM_PAPERWIDTH){wsprintf(bb,"%d",pdv->dmPaperWidth );buf +=             "PAPERWIDTH: "; buf += bb; buf+="\r\n";}
          if(pdv->dmFields & DM_POSITION){wsprintf(bb,"%d",pdv->dmPosition );buf +=                 "POSITION: "; buf += bb; buf+="\r\n";}
          if(pdv->dmFields & DM_SCALE){wsprintf(bb,"%d",pdv->dmScale );buf +=                       "SCALE: "; buf += bb; buf+="\r\n";}
          if(pdv->dmFields & DM_COPIES){wsprintf(bb,"%d",pdv->dmCopies );buf +=                     "COPIES: "; buf += bb; buf+="\r\n";}
          if(pdv->dmFields & DM_DEFAULTSOURCE){wsprintf(bb,"%d",pdv->dmDefaultSource );buf +=       "DEFAULTSOURCE: "; buf += bb; buf+="\r\n";}
          if(pdv->dmFields & DM_PRINTQUALITY){wsprintf(bb,"%d",pdv->dmPrintQuality );buf +=         "PRINTQUALITY: "; buf += bb; buf+="\r\n";}
          if(pdv->dmFields & DM_COLOR){
            buf += "COLOR: ";
            if(pdv->dmColor == DMCOLOR_COLOR){
              buf += "COLOR";
            }else if(pdv->dmColor == DMCOLOR_MONOCHROME){
              buf += "MONOCHROME";
            }else{
              wsprintf(bb,"%d",pdv->dmColor);
              buf += bb;
            }
            buf+="\r\n";
          }
          if(pdv->dmFields & DM_DUPLEX){wsprintf(bb,"%d",pdv->dmDuplex );buf +=                     "DUPLEX: "; buf += bb; buf+="\r\n";}
          if(pdv->dmFields & DM_YRESOLUTION){wsprintf(bb,"%d",pdv->dmYResolution );buf +=           "YRESOLUTION: "; buf += bb; buf+="\r\n";}
          if(pdv->dmFields & DM_TTOPTION){
            buf += "TTOPTION: ";
            if(pdv->dmTTOption == DMTT_BITMAP){
              buf += "BITMAP";
            }else if(pdv->dmTTOption == DMTT_DOWNLOAD){
              buf += "DOWNLOAD";
            }else if(pdv->dmTTOption == DMTT_DOWNLOAD_OUTLINE){
              buf += "DOWNLOAD_OUTLINE";
            }else if(pdv->dmTTOption == DMTT_SUBDEV){
              buf += "SUBDEV";
            }else{
              wsprintf(bb,"%d",pdv->dmTTOption);
              buf += bb;
            }
            buf+="\r\n";
          }
          if(pdv->dmFields & DM_COLLATE){wsprintf(bb,"%d",pdv->dmCollate );buf +=                   "COLLATE: "; buf += bb; buf+="\r\n";}
          if(pdv->dmFields & DM_FORMNAME){
            buf += "FORMNAME: ";
            buf += (char *)(pdv->dmFormName);
            buf+="\r\n";
          }
          if(pdv->dmFields & DM_LOGPIXELS){wsprintf(bb,"%d",pdv->dmLogPixels );buf +=               "LOGPIXELS: "; buf += bb; buf+="\r\n";}
          if(pdv->dmFields & DM_BITSPERPEL){wsprintf(bb,"%d",pdv->dmBitsPerPel );buf +=             "BITSPERPEL: "; buf += bb; buf+="\r\n";}
          if(pdv->dmFields & DM_PELSWIDTH){wsprintf(bb,"%d",pdv->dmPelsWidth );buf +=               "PELSWIDTH: "; buf += bb; buf+="\r\n";}
          if(pdv->dmFields & DM_PELSHEIGHT){wsprintf(bb,"%d",pdv->dmPelsHeight );buf +=             "PELSHEIGHT: "; buf += bb; buf+="\r\n";}
          if(pdv->dmFields & DM_DISPLAYFLAGS){wsprintf(bb,"%d",pdv->dmDisplayFlags );buf +=         "DISPLAYFLAGS: "; buf += bb; buf+="\r\n";}
          if(pdv->dmFields & DM_NUP){wsprintf(bb,"%d",pdv->dmNup );buf +=                           "NUP: "; buf += bb; buf+="\r\n";}
          if(pdv->dmFields & DM_DISPLAYFREQUENCY){wsprintf(bb,"%d",pdv->dmDisplayFrequency );buf += "DISPLAYFREQUENCY: "; buf += bb; buf+="\r\n";}
          if(pdv->dmFields & DM_ICMMETHOD){wsprintf(bb,"%d",pdv->dmICMMethod );buf +=               "ICMMETHOD: "; buf += bb; buf+="\r\n";}
          if(pdv->dmFields & DM_ICMINTENT){wsprintf(bb,"%d",pdv->dmICMIntent );buf +=               "ICMINTENT: "; buf += bb; buf+="\r\n";}
          if(pdv->dmFields & DM_MEDIATYPE){wsprintf(bb,"%d",pdv->dmMediaType );buf +=               "MEDIATYPE: "; buf += bb; buf+="\r\n";}
          if(pdv->dmFields & DM_DITHERTYPE){wsprintf(bb,"%d",pdv->dmDitherType );buf +=             "DITHERTYPE: "; buf += bb; buf+="\r\n";}
          GlobalUnlock(hDevMode);
        }
        buf += "---\r\n";
        for(int i = 0; i < sizeof(dcnum2)/sizeof(*dcnum2); i++){
          char b[16];
          r = GetDeviceCaps(hPrintDC,dcnum2[i]);
          wsprintf(b,"%d",r);
          buf += dcname2[i];
          buf += ": ";
          buf += b;
          buf += "\r\n";
        }
        for(int i = 0; i < sizeof(dcnum1)/sizeof(*dcnum1); i++){
          char b[16];
          r = GetDeviceCaps(hPrintDC,dcnum1[i]);
          wsprintf(b,"%08X",r);
          buf += dcname1[i];
          buf += ": ";
          buf += b;
          buf += "\r\n";
        }
      }
      buf += "-- 以上 --\r\n";
      SetWindowText(h,buf.c_str());
    }
    return TRUE;
    break;
  case WM_COMMAND:
    {
      int iEvent = HIWORD(wParam);
      int iId = LOWORD(wParam);
      switch(iId){
      case IDOK: case IDCANCEL:
        {
          EndDialog(hDlg,0);
        }
        break;
      }
    }
    return FALSE;
    break;
  }
  return FALSE;
}

struct _myrand{
  int operator () (int m) {
    return myrandint(m);
  }
} myrand;

void CreateFudaOtositorifuda(){
  torifuda_of_fudaotosi.clear();
  std::vector<class torifuda>::iterator it;
  for(it = torifuda.begin(); it != torifuda.end(); it++){
    if(it->nigatefuda){
      torifuda_of_fudaotosi.push_back(&(*it));
    }
  }
  if(torifuda_of_fudaotosi.size() > 0){
    std::random_shuffle(torifuda_of_fudaotosi.begin(),torifuda_of_fudaotosi.end(),myrand);
  }
}
void ClearNigateFuda(){
  std::vector<class torifuda>::iterator it;
  for(it = torifuda.begin(); it != torifuda.end(); it++){
    it->nigatefuda = TRUE;
  }
}

std::string GetMaxMinDisplayColors(int * max, int * min){
  std::string buf;
  HDC hdc = CreateDC("DISPLAY",NULL,NULL,NULL);
  int curbpp = GetDeviceCaps(hdc,BITSPIXEL);
  DeleteDC(hdc);
  buf += "現在の画面の色数は [ ";
  char b[16];
  wsprintf(b,"%d",curbpp);
  buf += b;
  buf += " ビット ] です\r\n対応色数: ";
  int minbpp =  9999;
  int maxbpp = -9999;
  int count = 0;
  for(int bpp = 8; bpp <= 256; bpp += 8){
    DEVMODE dv;
    ZeroMemory(&dv,sizeof(DEVMODE));
    dv.dmSize = sizeof(DEVMODE);
    dv.dmFields = DM_BITSPERPEL;
    dv.dmBitsPerPel = bpp;
    if(ChangeDisplaySettings(&dv,CDS_TEST) == DISP_CHANGE_SUCCESSFUL){
      if(count != 0 && count % 3 == 0){
        buf += "          ";
      }
      wsprintf(b,"%d",bpp);
      buf += b;
      buf += " ビット, ";
      if(bpp < minbpp){
        minbpp = bpp;
      }
      if(bpp > maxbpp){
        maxbpp = bpp;
      }
      if(++count % 3 == 0){
        buf += "\r\n";
      }
    }
  }
  if(count % 3 != 0){
    buf += "\r\n";
  }
  if(max != NULL){
    *max = maxbpp;
  }
  if(min != NULL){
    *min = minbpp;
  }
  return buf;
}
