;;
;;  catalan.nsh
;;
;;  Catalan language strings for the Windows GCompris NSIS installer.
;;  Windows Code page: 1252
;;
;;  Author: "Bernat López" <bernatl@adequa.net>
;;  Version 2
;;  


; License Page
!define GCOMPRIS_LICENSE_BUTTON			"Següent >"
!define GCOMPRIS_LICENSE_BOTTOM_TEXT		"$(^Name) és distribuït sota llicència GPLe. Podeu consultar la llicència, només per proposits informatius, aquí. $_CLICK"

; Components Page
!define GCOMPRIS_SECTION_TITLE                  "GCompris"
!define GCOMPRIS_SECTION_DESCRIPTION		"Fitxers i dlls del nucli de GCompris"


; Installer Finish Page
!define GCOMPRIS_FINISH_VISIT_WEB_SITE		"Visita la pàgina web de GCompris per Windows"

; GCompris Section Prompts and Texts
!define GCOMPRIS_UNINSTALL_DESC			"GCompris (només esborrar)"
!define GCOMPRIS_PROMPT_WIPEOUT			"El teu directori antic de GCompris serà esborrat. Vols continuar?$\r$\rNota: Els plugins no estàndards que tinguis instal.lats seran esborrats.$\rLes preferències d'usuari de GCompris no es veruan afectades."
!define GCOMPRIS_PROMPT_DIR_EXISTS		"El directori d'instal.lació que has especificat ja existeix. Tots els continguts$\rseran esborrats. Vols continuar?"



; Uninstall Section Prompts
!define un.GCOMPRIS_UNINSTALL_ERROR_1		"L'instal.lador podria no trobar les entrades del registre de GCompris.$\rProbablement un altre usuari ha instal.lat aquesta aplicació."
!define un.GCOMPRIS_UNINSTALL_ERROR_2		"No tens permís per desinstal.lar aquesta aplicació."
