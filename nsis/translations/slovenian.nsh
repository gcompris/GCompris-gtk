;;
;;  slovenian.nsh
;;
;;  Slovenian language strings for the Windows GCompris NSIS installer.
;;  Windows Code page: 1250
;;
;;  Author: Martin Srebotnjak <miles@filmsi.net>
;;  Version 2
;;

!define INSTALLER_IS_RUNNING			"Namešèanje že poteka."
!define GCOMPRIS_IS_RUNNING				"Trenutno že teèe razlièica GComprisa. Prosimo zaprite GCompris in poskusite znova."

; License Page
!define GCOMPRIS_LICENSE_BUTTON			"Naprej >"
!define GCOMPRIS_LICENSE_BOTTOM_TEXT		"$(^Name) je na voljo pod licenco GPL. Ta licenca je tu na voljo le v informativne namene. $_CLICK"

; Components Page
!define GCOMPRIS_SECTION_TITLE                  "GCompris"
!define GCOMPRIS_SECTION_DESCRIPTION		"Temeljne datoteke GComprisa"


; Installer Finish Page
!define GCOMPRIS_FINISH_VISIT_WEB_SITE		"Obišèite spletno stran Windows GCompris"

; GCompris Section Prompts and Texts
!define GCOMPRIS_UNINSTALL_DESC			"GCompris (samo odstrani)"
!define GCOMPRIS_PROMPT_WIPEOUT			"Vaš star imenik GCompris bo zbrisan. Želite nadaljevati?$\r$\rOpomba: Vsi nestandardni vtièniki, ki ste jih namestili, bodo zbrisani.$\rUporabniške nastavitve za GCompris se bodo ohranile."
!define GCOMPRIS_PROMPT_DIR_EXISTS		"Namestitveni imenik, ki ste ga navedli, že obstaja. Vsa vsebina$\rbo zbrisana. Želite nadaljevati?"



; Uninstall Section Prompts
!define un.GCOMPRIS_UNINSTALL_ERROR_1		"Ne morem najti vnosov v registru za GCompris.$\rNajverjetneje je ta program namestil drug uporabnik."
!define un.GCOMPRIS_UNINSTALL_ERROR_2		"Za odstranitev programa nimate ustreznih pravic."
