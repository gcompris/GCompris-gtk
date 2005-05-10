;;
;;  czech.nsh
;;
;;  Czech language strings for the Windows GCompris NSIS installer.
;;  Windows Code page: 1252
;;
;;  Author: Jan Kolar <jan@e-kolar.net>
;;  Version 2
;;


; License Page
!define GCOMPRIS_LICENSE_BUTTON			"Další >"
!define GCOMPRIS_LICENSE_BOTTOM_TEXT		"K použití $(^Name) se vztahuje GPL licence. Licence je zde uvedena pouze pro Vaší informaci. $_CLICK"

; Components Page
!define GCOMPRIS_SECTION_TITLE                  "GCompris"
!define GCOMPRIS_SECTION_DESCRIPTION		"Základní soubory a DLL pro GCompris"


; Installer Finish Page
!define GCOMPRIS_FINISH_VISIT_WEB_SITE		"Navštívit Windows GCompris Web Page"

; GCompris Section Prompts and Texts
!define GCOMPRIS_UNINSTALL_DESC			"GCompris (odinstalovat)"
!define GCOMPRIS_PROMPT_WIPEOUT			"Váš starý adresáø pro GCompris bude vymazán. Chcete pokraèovat?$\r$\rUpozornìní: Jakákoli nestandardní rozšíøení (plugin) , která máte nainstalována budou ztracena.$\rUživatelská nastavení pro GCompris budou zachována."
!define GCOMPRIS_PROMPT_DIR_EXISTS		"Adresáø který byl zadán pro instalaci již existuje. Veškerý obsah$\rbude smazán. Chcete pokraèovat?"



; Uninstall Section Prompts
!define un.GCOMPRIS_UNINSTALL_ERROR_1		"Odinstalèní proces nemùže najít záznamy pro GCompris v registrech.$\rPravdìpodobnì instalaci této aplikace provedl jiný uživatel."
!define un.GCOMPRIS_UNINSTALL_ERROR_2		"Nemáte oprávnìní k odinstalaci této aplikace."
