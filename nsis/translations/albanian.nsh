;;
;;  albanian.nsh
;;
;;  Albanian language strings for the Windows GCompris NSIS installer.
;;  Windows Code page: 1252
;;
;;  Version 2
;;  Author: Besnik Bleta <besnik@spymac.com>
;;


; License Page
!define GCOMPRIS_LICENSE_BUTTON			"Më tej >"
!define GCOMPRIS_LICENSE_BOTTOM_TEXT		"$(^Name) qarkullon nën licensën GPL. Licensa këtu sillet vetëm për qëllime njoftimi. $_CLICK"

; Components Page
!define GCOMPRIS_SECTION_TITLE                  "GCompris"
!define GCOMPRIS_SECTION_DESCRIPTION		"Kartela dhe dll bazë të GCompris-it"


; Installer Finish Page
!define GCOMPRIS_FINISH_VISIT_WEB_SITE		"Vizitoni Faqen Web të GCompris-it për Windows"

; GCompris Section Prompts and Texts
!define GCOMPRIS_UNINSTALL_DESC			"GCompris (vetëm hiq)"
!define GCOMPRIS_PROMPT_WIPEOUT			"Është gati për t'u fshirë drejtoria juaj e vjetër GCompris. Doni të vazhdohet?$\r$\rShënim: Do të fshihet çfarëdo shtojceë jo standarde që mund të keni instaluar.$\rNuk do të preken rregullime GCompris përdoruesash."
!define GCOMPRIS_PROMPT_DIR_EXISTS		"Drejtoria e instalimit që treguat ekziston tashmë. Çfarëdo përmbajtje$\rdo të fshihet. Do të donit të vazhdohet?"



; Uninstall Section Prompts
!define un.GCOMPRIS_UNINSTALL_ERROR_1		"Çinstaluesi nuk gjeti dot zëra regjistri për GCompris-in.$\rKa mundësi që këtë zbatim ta ketë instaluar një tjetër përdorues."
!define un.GCOMPRIS_UNINSTALL_ERROR_2		"Nuk keni leje të çinstaloni këtë zbatim."
