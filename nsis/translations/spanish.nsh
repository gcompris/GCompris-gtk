;;
;;  spanish.nsh
;;
;;  Spanish language strings for the Windows GCompris NSIS installer.
;;  Windows Code page: 1252
;;  Translator: Javier Fernandez-Sanguino Peña
;;
;;  Version 2
;;


; License Page
!define GCOMPRIS_LICENSE_BUTTON			"Siguiente >"
!define GCOMPRIS_LICENSE_BOTTOM_TEXT		"$(^Name) se distribuye bajo la licencia GPL. Esta licencia se incluye aquí sólo con propósito informativo: $_CLICK"

; Components Page
!define GCOMPRIS_SECTION_TITLE                  "GCompris"
!define GCOMPRIS_SECTION_DESCRIPTION		"Ficheros y dlls principales de Core"


; Installer Finish Page
!define GCOMPRIS_FINISH_VISIT_WEB_SITE		"Visite la página Web de GCompris Windows"

; GCompris Section Prompts and Texts
!define GCOMPRIS_UNINSTALL_DESC			"GCompris (sólo eliminar)"
!define GCOMPRIS_PROMPT_WIPEOUT			"Su directorio antigüo de GCompris va a ser borrado. ¿Desea continuar?$\r$\rObservación: cualquier aplique no estándar que pudiera haber instalado será borrado.$\rÉsto no afectará a sus preferencias de usuario en GCompris."
!define GCOMPRIS_PROMPT_DIR_EXISTS		"El directorio de instalación que ha especificado ya existe. Todos los contenidos$\rserá borrados. ¿Desea continuar?"



; Uninstall Section Prompts
!define un.GCOMPRIS_UNINSTALL_ERROR_1         "El desinstalador no pudo encontrar las entradas en el registro de GCompris.$\rEs probable que otro usuario instalara la aplicación."
!define un.GCOMPRIS_UNINSTALL_ERROR_2         "No tiene permisos para desinstalar esta aplicación."
