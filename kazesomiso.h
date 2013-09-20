#ifndef _KAZESOMISO_H_
#define _KAZESOMISO_H_

#include "resource.h"
#include "classes.h"

#ifdef _DEBUG
#include <assert.h>
#endif

#include <windows.h>
#include <Commdlg.h>
#include <shlobj.h>

#include <vector>
#include <string>
#include <algorithm>

#include <math.h>
#include <iostream>
#include <fstream>

// #define STRICT
// using std::min;
// using std::max;
#include <gdiplus.h>

// fileio.cc に記述
extern BOOL LoadTorifuda();
extern BOOL SaveToIniFile(int, const class ToriYomiDesign *);
extern BOOL LoadFromIniFile(int, class ToriYomiDesign &, std::vector<int> *);
extern BOOL LoadImageToDC(const HDC &, const class ToriYomiDesign &);
extern void SaveFudaToBitmap(int,LPCTSTR,const class ToriYomiDesign &,double);
extern BOOL LoadYomiFuda();

// utils.cc に記述
extern void EnableMenuAll(HWND, BOOL);
extern int myrandint(int);
extern void SetCurrentDirectoryToExePath();
extern std::vector<std::string> ParseSimonoku(std::string s);
extern UINT_PTR CALLBACK CCHookProc(HWND, UINT, WPARAM, LPARAM);
extern void CallChooseFont(HWND,LOGFONT &,BOOL);
extern void CallChooseColor(HWND,COLORREF &);
extern void UpdateMainWindow(BOOL);
extern void FixWindowSizeToFudaSize();
extern void ChangeToriYomi(enum TORIYOMI);
extern void MoveWindowToRight(HWND);
extern void DoWhenVscroll(HWND,int,int,int);
extern void ChangeDisplayColorToMax();

// drawfuda.cc に記述

extern const int yomimaxrownum;
extern void DrawFuda(const class ToriYomiDesign &, const HDC &, MYRECT *);
extern std::string RemoveFuriganaAndTrunc(std::string);

// convert_image.cc に記述
extern BOOL ToBmp(const char *, const char *);
extern BOOL Bmp2Gif(const char *, const char *);
extern BOOL Bmp2Jpg(const char *, const char *,int);
extern BOOL Bmp2Png(const char *, const char *);

// myprint_proc.cc に記述
extern void BeginPrinter();
extern void EndPrinter();
extern HDC hPrintDC;
extern HGLOBAL hDevMode;
extern HGLOBAL hDevNames;

// kazesomiso.cc に記述

extern double fudascale;
extern enum Gdiplus::Unit fudaunit;
extern double fudascale_x;
extern double fudascale_y;

extern enum TORIYOMI tori_or_yomi;


extern BOOL gdiplus_useable;
extern struct BackStructure bgimage_structure;
extern const char * TORIFUDAFILE;
extern const char * YOMIFUDAFILE;
extern const char magicword[8+1];
extern const char * YOMIINI;
extern const char * TORIINI;
extern const char * szTitle;
extern const char * szWindowClass;

extern std::vector<class torifuda> torifuda;
extern HWND hDlgWnd;
extern HINSTANCE hInst;
extern HWND hWndMain;
extern class BackStructure front_structure;
extern class ToriYomiDesign currentdesign;

extern class torifuda * currentfuda;

extern std::vector<class torifuda *> torifuda_of_fudaotosi;
extern int fudaotosinum;

extern std::vector<class yomifuda> yomifuda;

extern void ClearNigateFuda();
extern void CreateFudaOtositorifuda();

#endif // _KAZESOMISO_H_
