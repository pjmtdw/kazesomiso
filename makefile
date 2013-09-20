# Makefile for Microsoft Visual C++ .NET

VPATH = "C:\Program Files\Microsoft Visual Studio .NET"
CL = $(VPATH)"\Vc7\Bin\cl" -GX
RC = $(VPATH)"\Vc7\Bin\rc"
LINK = $(VPATH)"\Vc7\Bin\link"
DELAYLOAD = /DELAYLOAD:gdiplus.dll

# DEBUG = /MLd /Fd"vc70.pdb" /ZI /D"_DEBUG"
# LDEBUG = /DEBUG /PDB:"kazesomiso.pdb"

INC = -I $(VPATH)"\Vc7\Include" -I $(VPATH)"\Vc7\PlatformSDK\Include" \
      -I $(VPATH)"\Vc7\PlatformSDK\Include\prerelease"

LIBPATH = /LIBPATH:$(VPATH)"\Vc7\PlatformSDK\lib" /LIBPATH:$(VPATH)"\Vc7\lib" \
          /LIBPATH:$(VPATH)"\Vc7\PlatformSDK\lib\prerelease"
LIBS = uuid.lib libcp.lib gdi32.lib gdiplus.lib user32.lib \
       shell32.lib comdlg32.lib delayimp.lib ./imgctl.lib

OBJS = kazesomiso.obj convert_image.obj utils.obj drawfuda.obj fileio.obj \
       fudaotosi_proc.obj settings_proc.obj fudadesign_proc.obj saveasimage_proc.obj \
       myprint_proc.obj classes.obj

RESOURCE = kazesomiso.RES

.SUFFIXES: .obj .cpp

kazesomiso.exe: $(OBJS) $(RESOURCE)
	$(LINK) $(LDEBUG) $(DELAYLOAD) $(LIBPATH) $(OBJS) $(LIBS) $(RESOURCE)

.cpp.obj:
	$(CL) $(DEBUG) $(INC) -c $<

kazesomiso.RES: kazesomiso.rc
	$(RC) $(INC) $<

clean:
	rm *.obj *.exe *.RES *.pdb *.idb *.ilk *.ini
