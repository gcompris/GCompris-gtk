;;
;;  norwegian.nsh
;;
;;  Norwegian language strings for the Windows GCompris NSIS installer.
;;  Windows Code page: 1252
;;
;;  Jørgen_Vinne_Iversen <jorgenvi@tihlde.org>
;;  Version 2
;;


; License Page
!define GCOMPRIS_LICENSE_BUTTON			"Neste >"
!define GCOMPRIS_LICENSE_BOTTOM_TEXT		"$(^Name) er utgitt under GPL (GNU General Public License). Lisensen er oppgitt her kun med henblikk på informasjon. $_CLICK"

; Components Page
!define GCOMPRIS_SECTION_TITLE                  "GCompris"
!define GCOMPRIS_SECTION_DESCRIPTION		"GCompriss kjernefiler og dll'er"


; Installer Finish Page
!define GCOMPRIS_FINISH_VISIT_WEB_SITE		"Besøk GCompris for Windows' Nettside"

; GCompris Section Prompts and Texts
!define GCOMPRIS_UNINSTALL_DESC			"GCompris (kun avinstallering)"
!define GCOMPRIS_PROMPT_WIPEOUT			"Din gamle GCompris-katalog holder på å slettes. Ønsker du å fortsette?$\r$\rMerk: Eventuelle ikke-standard plugin'er du har installert vil bli slettet.$\rGCompriss brukerinstillinger vil ikke bli berørt."
!define GCOMPRIS_PROMPT_DIR_EXISTS		"Installasjonskatalogen du har spesifisert finnes fra før. Eventuelt innhold$\rvil bli slettet. Ønsker du å fortsette?"



; Uninstall Section Prompts
!define un.GCOMPRIS_UNINSTALL_ERROR_1		"Avinstalleringsprogrammet kunne ikke finne noen registeroppføring for GCompris.$\rTrolig har en annen bruker installert denne applikasjonen."
!define un.GCOMPRIS_UNINSTALL_ERROR_2		"Du har ikke rettigheter til å avinstallere denne applikasjonen."
