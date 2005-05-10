;;
;;  italian.nsh
;;
;;  Italian language strings for the Windows GCompris NSIS installer.
;;  Windows Code page: 1252
;;
;;  Author: Claudio Satriano <satriano@na.infn.it>, 2003.
;;  Version 2
;;


; License Page
!define GCOMPRIS_LICENSE_BUTTON			"Avanti >"
!define GCOMPRIS_LICENSE_BOTTOM_TEXT		"$(^Name) è distribuito sotto licenza GPL. La licenza è mostrata qui solamente a scopo informativo. $_CLICK" 

; Components Page
!define GCOMPRIS_SECTION_TITLE                  "GCompris"
!define GCOMPRIS_SECTION_DESCRIPTION		"File principali di GCompris e dll"


; Installer Finish Page
!define GCOMPRIS_FINISH_VISIT_WEB_SITE		"Visita la pagina web di GCompris per Windows" 

; GCompris Section Prompts and Texts
!define GCOMPRIS_UNINSTALL_DESC			"GCompris (solo rimozione)"
!define GCOMPRIS_PROMPT_WIPEOUT			"La tua vecchia directory di GCompris sta per essere cancellata. Vuoi andare avanti?$\r$\rNota: Tutti i plugin non standard che hai installato saranno cancellati.$\rLe impostazioni di GCompris non saranno rimosse."
!define GCOMPRIS_PROMPT_DIR_EXISTS		"La directory di installazione specificata esiste già. Tutto il contenuto$\rverrà cancellato. Vuoi continuare?"



; Uninstall Section Prompts
!define un.GCOMPRIS_UNINSTALL_ERROR_1         "Il programma di rimozione non è in grado di trovare le voci di registro per GCompris.$\rProbabilmente un altro utente ha installato questa applicazione."
!define un.GCOMPRIS_UNINSTALL_ERROR_2         "Non hai il permesso per rimuovere questa applicazione."
