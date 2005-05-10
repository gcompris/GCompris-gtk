;;
;;  swedish.nsh
;;
;;  Swedish language strings for the Windows GCompris NSIS installer.
;;  Windows Code page: 1252
;;
;;  Author: Tore Lundqvist <tlt@mima.x.se>, 2003.
;;  Author: Peter Hjalmarsson <xake@telia.com>, 2005.
;;  Version 3
;;

; Make sure to update the GCOMPRIS_MACRO_LANGUAGEFILE_END macro in
; langmacros.nsh when updating this file

; Startup Checks
!define INSTALLER_IS_RUNNING			"Installationsprogrammet körs redan."
!define GCOMPRIS_IS_RUNNING			"En instans av Giam körs redan. Avsluta GCompris och försök igen."

; License Page
!define GCOMPRIS_LICENSE_BUTTON			"Nästa >"
!define GCOMPRIS_LICENSE_BOTTOM_TEXT		"$(^Name) är utgivet under GPL. Licensen finns tillgänglig här för infromationssyften enbart. $_CLICK"

; Components Page
!define GCOMPRIS_SECTION_TITLE                  "GCompris"
!define GCOMPRIS_SECTION_DESCRIPTION		"GCompriss kärnfiler och DLL:er"


; Installer Finish Page
!define GCOMPRIS_FINISH_VISIT_WEB_SITE		"Besök Windows-GCompriss hemsida"

; GCompris Section Prompts and Texts
!define GCOMPRIS_UNINSTALL_DESC			"GCompris (enbart för avinstallation)"
!define GCOMPRIS_PROMPT_WIPEOUT			"Din gamla GCompris-katalog kommer att raderas, vill du fortsätta?$\r$\rOBS! om du har installerat några extra insticksmoduler kommer de raderas.$\rGCompriss användarinställningar kommer inte påverkas."
!define GCOMPRIS_PROMPT_DIR_EXISTS		"Den katalog du vill installera i finns redan. Allt i katalogen$\rkommer att raderas, vill du fortsätta?"



; Uninstall Section Prompts
!define un.GCOMPRIS_UNINSTALL_ERROR_1         "Avinstalleraren kunde inte hitta registervärden för GCompris.$\rAntagligen har en annan användare installerat applikationen."
!define un.GCOMPRIS_UNINSTALL_ERROR_2         "Du har inte rättigheter att avinstallera den här applikationen."
