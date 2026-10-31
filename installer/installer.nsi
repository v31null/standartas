Unicode true
!include "x64.nsh"
!include "LogicLib.nsh"

Name "V31np format for windows"
OutFile "${OUT}\baíbúočman.exe"
InstallDir "$PROGRAMFILES64\V31np format for windows"
RequestExecutionLevel admin
BrandingText " "
VIProductVersion "1.0.0.0"
VIAddVersionKey ProductName "V31np format for windows"
VIAddVersionKey CompanyName "v31n"
VIAddVersionKey FileVersion "1.0.0"
VIAddVersionKey ProductVersion "1.0.0"

Page instfiles
UninstPage instfiles

!define UNINST "Software\Microsoft\Windows\CurrentVersion\Uninstall\v31np"
!define SENDTO_FILE "Baíbíz.v31npsendto"

!macro FreeDll file
  ${If} ${FileExists} "${file}"
    GetTempFileName $9 "$INSTDIR"
    Delete $9
    Rename "${file}" $9
    Delete /REBOOTOK $9
  ${EndIf}
!macroend

!macro Native var
  ${If} ${IsNativeARM64}
    StrCpy ${var} "arm64"
  ${ElseIf} ${IsNativeAMD64}
    StrCpy ${var} "x64"
  ${Else}
    StrCpy ${var} "x86"
  ${EndIf}
!macroend

!macro Trim var
  StrCpy $6 ${var} "" -1
  ${If} $6 == "\"
    StrCpy ${var} ${var} -1
  ${EndIf}
!macroend

!macro Register flag
  !insertmacro Native $5
  ${If} $5 == "x86"
    ExecWait '"$WINDIR\System32\regsvr32.exe" /s ${flag} "$INSTDIR\x86\v31npshell.dll"' $0
  ${Else}
    ExecWait '"$WINDIR\Sysnative\regsvr32.exe" /s ${flag} "$INSTDIR\$5\v31npshell.dll"' $0
    ExecWait '"$WINDIR\SysWOW64\regsvr32.exe" /s ${flag} "$INSTDIR\x86\v31npshell.dll"' $1
  ${EndIf}
!macroend

!macro Check7z dll var
  StrCpy ${var} "1"
  GetDLLVersion "${dll}" $R0 $R1
  IntOp $R2 $R0 >> 16
  IntOp $R3 $R0 & 0xFFFF
  ${If} $R2 != 26
  ${OrIf} $R3 != 5
    MessageBox MB_OKCANCEL "7-Zip not 26.05" IDOK +2
    StrCpy ${var} "0"
  ${EndIf}
!macroend

Function .onInit
  ${If} ${RunningX64}
    SetRegView 64
  ${EndIf}
FunctionEnd

Function un.onInit
  ${If} ${RunningX64}
    SetRegView 64
  ${EndIf}
FunctionEnd

Section
  SetOutPath "$INSTDIR"
  File "${HERE}\nzuu.ico"
  !insertmacro FreeDll "$INSTDIR\x86\v31npshell.dll"
  !insertmacro FreeDll "$INSTDIR\x64\v31npshell.dll"
  !insertmacro FreeDll "$INSTDIR\arm64\v31npshell.dll"
  SetOutPath "$INSTDIR\x86"
  File "${BUILD}\x86\v31npshell.dll"
  File "${BUILD}\x86\v31np.dll"
  SetOutPath "$INSTDIR\x64"
  File "${BUILD}\x64\v31npshell.dll"
  File "${BUILD}\x64\v31np.dll"
  SetOutPath "$INSTDIR\arm64"
  File "${BUILD}\arm64\v31npshell.dll"
  File "${BUILD}\arm64\v31np.dll"
  SetOutPath "$INSTDIR"
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  !insertmacro Register ""
  StrCmp $0 "0" +2
  Abort

  !insertmacro Native $5
  ReadRegStr $2 HKLM "Software\7-Zip" "Path64"
  ${If} $2 == ""
    ReadRegStr $2 HKLM "Software\7-Zip" "Path"
  ${EndIf}
  !insertmacro Trim $2
  ${If} $2 != ""
  ${AndIf} ${FileExists} "$2\7z.dll"
    !insertmacro Check7z "$2\7z.dll" $7
    ${If} $7 == "1"
      CreateDirectory "$2\Formats"
      !insertmacro FreeDll "$2\Formats\v31np.dll"
      CopyFiles /SILENT "$INSTDIR\$5\v31np.dll" "$2\Formats"
      WriteRegStr HKLM "${UNINST}" "SevenZip" "$2\Formats\v31np.dll"
    ${EndIf}
  ${EndIf}
  ${If} ${RunningX64}
    SetRegView 32
    ReadRegStr $3 HKLM "Software\7-Zip" "Path"
    SetRegView 64
    !insertmacro Trim $3
    ${If} $3 != ""
    ${AndIf} $3 != $2
    ${AndIf} ${FileExists} "$3\7z.dll"
      !insertmacro Check7z "$3\7z.dll" $7
      ${If} $7 == "1"
        CreateDirectory "$3\Formats"
        !insertmacro FreeDll "$3\Formats\v31np.dll"
        CopyFiles /SILENT "$INSTDIR\x86\v31np.dll" "$3\Formats"
        WriteRegStr HKLM "${UNINST}" "SevenZip32" "$3\Formats\v31np.dll"
      ${EndIf}
    ${EndIf}
  ${EndIf}

  SetShellVarContext current
  FileOpen $4 "$SENDTO\${SENDTO_FILE}" w
  FileClose $4
  SetShellVarContext all

  WriteRegStr HKLM "${UNINST}" "DisplayName" "V31np format for windows"
  WriteRegStr HKLM "${UNINST}" "Publisher" "v31n"
  WriteRegStr HKLM "${UNINST}" "DisplayVersion" "1.0.0"
  WriteRegStr HKLM "${UNINST}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegStr HKLM "${UNINST}" "DisplayIcon" "$INSTDIR\nzuu.ico"
  WriteRegDWORD HKLM "${UNINST}" "NoModify" 1
  WriteRegDWORD HKLM "${UNINST}" "NoRepair" 1
SectionEnd

Section "Uninstall"
  !insertmacro Register "/u"

  ReadRegStr $2 HKLM "${UNINST}" "SevenZip"
  ${If} $2 != ""
    Delete /REBOOTOK "$2"
  ${EndIf}
  ReadRegStr $3 HKLM "${UNINST}" "SevenZip32"
  ${If} $3 != ""
    Delete /REBOOTOK "$3"
  ${EndIf}

  SetShellVarContext current
  Delete "$SENDTO\${SENDTO_FILE}"
  SetShellVarContext all

  !insertmacro FreeDll "$INSTDIR\x86\v31npshell.dll"
  !insertmacro FreeDll "$INSTDIR\x64\v31npshell.dll"
  !insertmacro FreeDll "$INSTDIR\arm64\v31npshell.dll"
  Delete "$INSTDIR\x86\v31np.dll"
  Delete "$INSTDIR\x64\v31np.dll"
  Delete "$INSTDIR\arm64\v31np.dll"
  RMDir /REBOOTOK "$INSTDIR\x86"
  RMDir /REBOOTOK "$INSTDIR\x64"
  RMDir /REBOOTOK "$INSTDIR\arm64"
  Delete "$INSTDIR\empty.v31np"
  Delete "$INSTDIR\nzuu.ico"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir /REBOOTOK "$INSTDIR"

  DeleteRegKey HKLM "${UNINST}"
SectionEnd
