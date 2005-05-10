;;
;;  romanian.nsh
;;
;;  Romanian language strings for the Windows GCompris NSIS installer.
;;  Windows Code page: 1250
;;
;;  Author: Miºu Moldovan <dumol@go.ro>
;;


; Components Page
!define GCOMPRIS_SECTION_TITLE                  "GCompris"
!define GCOMPRIS_SECTION_DESCRIPTION		"Fiºiere GCompris ºi dll-uri"


; GCompris Section Prompts and Texts
!define GCOMPRIS_UNINSTALL_DESC			"GCompris (doar dezinstalare)"
!define GCOMPRIS_PROMPT_WIPEOUT			"Vechiul director GCompris va fi ºters. Doriþi sã continuaþi?$\r$\rNotã: Orice module externe vor fi ºterse.$\rSetãrile utilizatorilor GCompris nu vor fi afectate."
!define GCOMPRIS_PROMPT_DIR_EXISTS		"Directorul ales pentru instalare existã deja.$\rConþinutul sãu va fi ºters. Doriþi sã continuaþi?"



; Uninstall Section Prompts
!define un.GCOMPRIS_UNINSTALL_ERROR_1         "Programul de dezinstalare nu a gãsit intrãri GCompris în regiºtri.$\rProbabil un alt utilizator a instalat aceastã aplicaþie."
!define un.GCOMPRIS_UNINSTALL_ERROR_2         "Nu aveþi drepturile de acces necesare dezinstalãrii acestei aplicaþii."
