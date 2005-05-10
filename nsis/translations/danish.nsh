;;
;;  danish.nsh
;;
;;  Danish language strings for the Windows GCompris NSIS installer.
;;  Windows Code page: 1252
;;
;;  Author: Ewan Andreasen <wiredloose@myrealbox.com>
;;  Version 2
;;


; License Page
!define GCOMPRIS_LICENSE_BUTTON			"Næste >"
!define GCOMPRIS_LICENSE_BOTTOM_TEXT		"$(^Name) er frigivet under GPL licensen. Licensen er kun medtaget her til generel orientering. $_CLICK"

; Components Page
!define GCOMPRIS_SECTION_TITLE                  "GCompris"
!define GCOMPRIS_SECTION_DESCRIPTION		"Basale GCompris filer og biblioteker"


; Installer Finish Page
!define GCOMPRIS_FINISH_VISIT_WEB_SITE		"Besøg Windows GCompris's hjemmeside"

; GCompris Section Prompts and Texts
!define GCOMPRIS_UNINSTALL_DESC			"GCompris (fjern)"
!define GCOMPRIS_PROMPT_WIPEOUT			"Din gamle GCompris folder vil blive slettet. Ønsker du at fortsætte? $\r$\rNB: Alle ikke-standard plugins du måtte have installeret vil blive slettet.$\rGCompris brugerindstillinger vil ikke blive påvirket af dette."
!define GCOMPRIS_PROMPT_DIR_EXISTS		"Den ønskede installationsfolder eksisterer allerede. Ethvert indhold$\ri folderen vil blive slettet. Ønsker du at fortsætte?"



; Uninstall Section Prompts
!define un.GCOMPRIS_UNINSTALL_ERROR_1		"Afinstallationen kunne ikke finde GCompris i registreringsdatabasen.$\rMuligvis har en anden bruger installeret programmet."
!define un.GCOMPRIS_UNINSTALL_ERROR_2		"Du har ikke tilladelse til at afinstallere dette program."

