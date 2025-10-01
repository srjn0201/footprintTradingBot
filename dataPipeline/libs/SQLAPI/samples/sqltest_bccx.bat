@echo off

rem Register Environment variables for Borland C++ (modern bcc64x compiler)
set BCCDIR="C:\Program Files (x86)\Embarcadero\Studio\23.0"
PATH=%BCCDIR%\Bin64;%BCCDIR%\Bin

rem ***********************************
rem Special build configuration defines
rem ***********************************
rem CFG=(relese|debug)
rem CFG_LINK=(dynamic|static)
rem SA_RUNTIME=(dynamic|static)
rem SA_UNICODE		build UNICODE version
rem OBJS		specify test program object file (sqltest.obj by default)
rem SA_64BIT		build 64-bit version
rem ***********************************
set SA_OPTIONS=SA_64BIT=1 SA_UNICODE=1

echo ------------------------
echo --- Building release ---
echo ------------------------
make /f Makefile.bccx %SA_OPTIONS% CFG=release CFG_LINK=dynamic safe_clean all
rem make /f Makefile.bccx %SA_OPTIONS% CFG=release CFG_LINK=static safe_clean all

echo ----------------------
echo --- Building debug ---
echo ----------------------
rem make /f Makefile.bccx %SA_OPTIONS% CFG=debug CFG_LINK=dynamic safe_clean all
rem make /f Makefile.bccx %SA_OPTIONS% CFG=debug CFG_LINK=static safe_clean all

