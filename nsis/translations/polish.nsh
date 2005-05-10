;;
;;  polish.nsh
;;
;;  Polish language strings for the Windows GCompris NSIS installer.
;;  Windows Code page: 1250
;;
;;  Author: Jan Eldenmalm <jan.eldenmalm@amazingports.com>
;;  Version 2
;;


; License Page
!define GCOMPRIS_LICENSE_BUTTON			"Dalej >"
!define GCOMPRIS_LICENSE_BOTTOM_TEXT		"$(^Name) jest wydzielone w licencji GPL. Udziela siê licencji wy³¹cznie do celów informacyjnych. $_CLICK"

; Components Page
!define GCOMPRIS_SECTION_TITLE                  "GCompris"
!define GCOMPRIS_SECTION_DESCRIPTION		"Zbiory Core GCompris oraz dll"


; Installer Finish Page
!define GCOMPRIS_FINISH_VISIT_WEB_SITE		"WejdŸ na stronê GCompris Web Page"

; GCompris Section Prompts and Texts
!define GCOMPRIS_UNINSTALL_DESC			"GCompris (usuñ program)"
!define GCOMPRIS_PROMPT_WIPEOUT			"Stary katalog GCompris zosta³ usuniêty. Czy chcesz kontunuowaæ?$\r$\rNote: Wszystkie stare - niestandardowe plugin-y zosta³y usuniête.$\r Ustawienia u¿utkownika GCompris bêd¹ wy³¹czone."
!define GCOMPRIS_PROMPT_DIR_EXISTS		"Wybrany katalog instalacyjny ju¿ istnieje. Jego zawartoœæ $\r zostanie skasowana. Czy chcesz kontunuowaæ?"



; Uninstall Section Prompts
!define un.GCOMPRIS_UNINSTALL_ERROR_1		"Deinstalator nie mo¿e znaleŸæ rejestrów dla GCompris.$\r Wskazuje to na to, ¿e instalacjê przeprowadzi³ inny u¿ytkownik."
!define un.GCOMPRIS_UNINSTALL_ERROR_2		"Nie masz uprawnieñ do deinstalacji tej aplikacji."
