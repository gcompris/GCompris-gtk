;;
;;  polish.nsh
;;
;;  Polish language strings for the Windows GCompris NSIS installer.
;;  Windows Code page: 1250
;;
;;  Author: Piotr Dr¹g <piotrdrag@gmail.com>
;;  Version 3
;;

; Startup Checks
!define INSTALLER_IS_RUNNING			"Instalator jest ju¿ uruchomiony."
!define GCOMPRIS_IS_RUNNING			"Wyst¹pienie programu GCompris jest obecnie uruchomione. Proszê zakoñczyæ dzia³anie programu GCompris i spróbowaæ ponownie."

; License Page
!define GCOMPRIS_LICENSE_BUTTON			"Dalej >"
!define GCOMPRIS_LICENSE_BOTTOM_TEXT		"Program $(^Name) zosta³ wydany na warunkach Powszechnej Licencji Publicznej GNU (GPL). Tekst licencji jest podany wy³¹cznie w celach informacyjnych. $_CLICK"

; Components Page
!define GCOMPRIS_SECTION_TITLE			"Komunikator GCompris (wymagany)"
!define GCOMPRIS_SECTION_DESCRIPTION		"G³ówne pliki i biblioteki DLL programu GCompris"


; Installer Finish Page
!define GCOMPRIS_FINISH_VISIT_WEB_SITE		"Otwórz stronê programu GCompris dla systemu Windows"

; GCompris Section Prompts and Texts
!define GCOMPRIS_UNINSTALL_DESC			"GCompris (tylko usuwanie)"
!define GCOMPRIS_PROMPT_WIPEOUT			"Poprzedni katalog programu GCompris zostanie usuniêty. Kontynuowaæ?$\r$\rUwaga: wszystkie niestandardowe wtyczki zostan¹ usuniête.$\rUstawienia u¿ytkownika programu GCompris nie zostan¹ usuniête."
!define GCOMPRIS_PROMPT_DIR_EXISTS		"Podany katalog instalacji ju¿ istnieje. Jego zawartoœæ$\rzostanie usuniêta. Kontynuowaæ?"



; Uninstall Section Prompts
!define un.GCOMPRIS_UNINSTALL_ERROR_1		"Deinstalator nie mo¿e odnaleŸæ wpisów rejestru dla programu GCompris.$\rMo¿liwe, ¿e inny u¿ytkownik zainstalowa³ ten program."
!define un.GCOMPRIS_UNINSTALL_ERROR_2		"Brak uprawnieñ do odinstalowania tego programu."
