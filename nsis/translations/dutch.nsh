;;
;;  dutch.nsh
;;
;;  Default language strings for the Windows GCompris NSIS installer.
;;  Windows Code page: 1252
;;
;;  Author: Vincent van Adrighem <vincent@dirck.mine.nu>
;;  Version 2
;;
; Startup Checks
!define INSTALLER_IS_RUNNING			"Er is al een installatie actief."
!define GCOMPRIS_IS_RUNNING			"GCompris wordt op dit moment uitgevoerd. Sluit GCompris af en start de installatie opnieuw."


; License Page
!define GCOMPRIS_LICENSE_BUTTON			"Volgende >"
!define GCOMPRIS_LICENSE_BOTTOM_TEXT		"$(^Name) wordt uitgegeven onder de GPL licentie. Deze licentie wordt hier slechts ter informatie aangeboden. $_CLICK"

; Components Page
!define GCOMPRIS_SECTION_TITLE                  "GCompris"
!define GCOMPRIS_SECTION_DESCRIPTION		"GCompris hoofdbestanden en dlls"


; Installer Finish Page
!define GCOMPRIS_FINISH_VISIT_WEB_SITE		"Neem een kijkje op de Windows GCompris webpagina"

; GCompris Section Prompts and Texts
!define GCOMPRIS_UNINSTALL_DESC			"GCompris (alleen verwijderen)"
!define GCOMPRIS_PROMPT_WIPEOUT			"Uw oude GCompris map staat op het punt om verwijderd te worden. Wilt u doorgaan?$\r$\rLet op: Alle door uzelf geïnstalleerde plugins zullen ook verwijderd worden.$\rDe gebruikersinstellingen van GCompris worden niet aangeraakt."
!define GCOMPRIS_PROMPT_DIR_EXISTS		"De gegeven installatiemap bestaat al. Eventuele inhoud zal verwijderd worden. Wilt u doorgaan?"



; Uninstall Section Prompts
!define un.GCOMPRIS_UNINSTALL_ERROR_1         "Het verwijderingsprogramma voor GCompris kon geen register-ingangen voor GCompris vinden.$\rWaarschijnlijk heeft een andere gebruiker het programma geïnstalleerd."
!define un.GCOMPRIS_UNINSTALL_ERROR_2         "U mag dit programma niet verwijderen."
