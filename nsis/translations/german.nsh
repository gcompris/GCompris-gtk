;;
;;  german.nsh
;;
;;  German language strings for the Windows GCompris NSIS installer.
;;  Windows Code page: 1252
;;
;;  Author: Bjoern Voigt <bjoern@cs.tu-berlin.de>, 2005.
;;  Version 3
;;
 
; Startup checks
!define INSTALLER_IS_RUNNING			"Der Installer läuft schon."
!define GCOMPRIS_IS_RUNNING				"Eine Instanz von GCompris läuft momentan schon. Beenden Sie GCompris und versuchen Sie es nochmal."
 
; License Page
!define GCOMPRIS_LICENSE_BUTTON			"Weiter >"
!define GCOMPRIS_LICENSE_BOTTOM_TEXT		"$(^Name) wird unter der GNU General Public License (GPL) veröffentlicht. Die Lizenz dient hier nur der Information. $_CLICK"
 
; Components Page
!define GCOMPRIS_SECTION_TITLE                  "GCompris"
!define GCOMPRIS_SECTION_DESCRIPTION		"GCompris Basis-Dateien und -DLLs"
 
 
; Installer Finish Page
!define GCOMPRIS_FINISH_VISIT_WEB_SITE		"Besuchen Sie die Windows GCompris-Webseite"
 
; GCompris Section Prompts and Texts
!define GCOMPRIS_UNINSTALL_DESC			"GCompris (nur entfernen)"
!define GCOMPRIS_PROMPT_WIPEOUT			"Ihr altes GCompris-Verzeichnis soll gelöscht werden. Möchten Sie fortfahren?$\r$\rHinweis: Alle nicht-Standard Plugins, die Sie evtl. installiert haben werden$\rgelöscht. GCompris-Benutzereinstellungen sind nicht betroffen."
!define GCOMPRIS_PROMPT_DIR_EXISTS		"Das Installationsverzeichnis, das Sie angegeben haben, existiert schon. Der Verzeichnisinhalt$\rwird gelöscht. Möchten Sie fortfahren?"
 
 
 
; Uninstall Section Prompts
!define un.GCOMPRIS_UNINSTALL_ERROR_1		"Der Deinstaller konnte keine Registrierungsschlüssel für GCompris finden.$\rEs ist wahrscheinlich, dass ein anderer Benutzer diese Anwendunng installiert hat."
!define un.GCOMPRIS_UNINSTALL_ERROR_2		"Sie haben keine Berechtigung, diese Anwendung zu deinstallieren."
