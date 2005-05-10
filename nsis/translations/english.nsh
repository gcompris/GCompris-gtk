;;
;;  english.nsh
;;
;;  Default language strings for the Windows GCompris NSIS installer.
;;  Windows Code page: 1252
;;
;;  Version 3
;;  Note: If translating this file, replace "!insertmacro GCOMPRIS_MACRO_DEFAULT_STRING"
;;  with "!define".

; Make sure to update the GCOMPRIS_MACRO_LANGUAGEFILE_END macro in
; langmacros.nsh when updating this file

; Startup Checks
!insertmacro GCOMPRIS_MACRO_DEFAULT_STRING INSTALLER_IS_RUNNING			"The installer is already running."
!insertmacro GCOMPRIS_MACRO_DEFAULT_STRING GCOMPRIS_IS_RUNNING			"An instance of GCompris is currently running. Exit GCompris and then try again."

; License Page
!insertmacro GCOMPRIS_MACRO_DEFAULT_STRING GCOMPRIS_LICENSE_BUTTON			"Next >"
!insertmacro GCOMPRIS_MACRO_DEFAULT_STRING GCOMPRIS_LICENSE_BOTTOM_TEXT		"$(^Name) is released under the GNU General Public License (GPL). The license is provided here for information purposes only. $_CLICK"

; Components Page
!insertmacro GCOMPRIS_MACRO_DEFAULT_STRING GCOMPRIS_SECTION_TITLE			"GCompris Instant Messaging Client (required)"
!insertmacro GCOMPRIS_MACRO_DEFAULT_STRING GCOMPRIS_SECTION_DESCRIPTION		"Core GCompris files and dlls"


; Installer Finish Page
!insertmacro GCOMPRIS_MACRO_DEFAULT_STRING GCOMPRIS_FINISH_VISIT_WEB_SITE		"Visit the Windows GCompris Web Page"

; GCompris Section Prompts and Texts
!insertmacro GCOMPRIS_MACRO_DEFAULT_STRING GCOMPRIS_UNINSTALL_DESC			"GCompris (remove only)"
!insertmacro GCOMPRIS_MACRO_DEFAULT_STRING GCOMPRIS_PROMPT_WIPEOUT			"Your old GCompris directory is about to be deleted. Would you like to continue?$\r$\rNote: Any non-standard plugins that you may have installed will be deleted.$\rGCompris user settings will not be affected."
!insertmacro GCOMPRIS_MACRO_DEFAULT_STRING GCOMPRIS_PROMPT_DIR_EXISTS		"The installation directory you specified already exists. Any contents$\rwill be deleted. Would you like to continue?"



; Uninstall Section Prompts
!insertmacro GCOMPRIS_MACRO_DEFAULT_STRING un.GCOMPRIS_UNINSTALL_ERROR_1		"The uninstaller could not find registry entries for GCompris.$\rIt is likely that another user installed this application."
!insertmacro GCOMPRIS_MACRO_DEFAULT_STRING un.GCOMPRIS_UNINSTALL_ERROR_2		"You do not have permission to uninstall this application."
