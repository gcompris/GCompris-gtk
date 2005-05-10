;;
;;  slovak.nsh
;;
;;  Slovak language strings for the Windows GCompris NSIS installer.
;;  Windows Code page: 1250
;;
;;  Author: dominik@internetkosice.sk
;;  Version 2


; License Page
!define GCOMPRIS_LICENSE_BUTTON			"Ïalej >"
!define GCOMPRIS_LICENSE_BOTTOM_TEXT		"$(^Name) je vydanı pod GPL licenciou. Táto licencia je len pre informaèné úèely. $_CLICK"

; Components Page
!define GCOMPRIS_SECTION_TITLE                  "GCompris"
!define GCOMPRIS_SECTION_DESCRIPTION		"Jadro GCompris-u a nevyhnutné DLL súbory"


; Installer Finish Page
!define GCOMPRIS_FINISH_VISIT_WEB_SITE		"Navštívi webstránku Windows GCompris"

; GCompris Section Prompts and Texts
!define GCOMPRIS_UNINSTALL_DESC			"GCompris (len odstráni)"
!define GCOMPRIS_PROMPT_WIPEOUT			"Váš adresár GCompris bude zmazanı. Chcete pokraèova?$\r$\rPoznámka: Všetky prídavne pluginy, ktoré ste nainštalovali budú tie zmazané.$\rNastavenia uivate¾ského úètu GCompris-u budú ponechané."
!define GCOMPRIS_PROMPT_DIR_EXISTS		"Adresár, ktorı ste zadali, u existuje. Jeho obsah bude zmazanı. Chcete pokraèova?"



; Uninstall Section Prompts
!define un.GCOMPRIS_UNINSTALL_ERROR_1		"Inštalátoru sa nepodarilo nájs poloky v registri pre GCompris.$\rJe moné, e túto aplikáciu nainštaloval inı pouívate¾."
!define un.GCOMPRIS_UNINSTALL_ERROR_2		"Nemáte oprávnenie na odinštaláciu tejto aplikácie."
