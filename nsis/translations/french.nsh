;;
;;  french.nsh
;;
;;  French language strings for the Windows GCompris NSIS installer.
;;  Windows Code page: 1252
;;
;;  Author: Eric Boumaour <zongo@nekeme.net>, 2003.
;;  Version 2
;;


; License Page
!define GCOMPRIS_LICENSE_BUTTON			"Suivant >"
!define GCOMPRIS_LICENSE_BOTTOM_TEXT		"$(^Name) est disponible sous licence GPL. Le texte de licence suivant est fourni uniquement à titre informatif. $_CLICK" 

; Components Page
!define GCOMPRIS_SECTION_TITLE                  "GCompris"
!define GCOMPRIS_SECTION_DESCRIPTION		"Fichiers et DLLs de base de GCompris"


; Installer Finish Page
!define GCOMPRIS_FINISH_VISIT_WEB_SITE		"Visitez la page web de GCompris Windows" 

; GCompris Section Prompts and Texts
!define GCOMPRIS_UNINSTALL_DESC			"GCompris (supprimer uniquement)"
!define GCOMPRIS_PROMPT_WIPEOUT			"L'ancien répertoire de GCompris va être supprimé. Voulez-vous continuer ?$\r$\rNote : Tous les plugins non standards que vous avez installés seront aussi supprimés.$\rLes configurations des utilisateurs de GCompris ne sont pas touchés."
!define GCOMPRIS_PROMPT_DIR_EXISTS		"Le répertoire d'installation existe déjà. Son contenu sera effacé.$\rVoulez-vous continuer ?"



; Uninstall Section Prompts
!define un.GCOMPRIS_UNINSTALL_ERROR_1         "Les clefs de GCompris n'ont pas été trouvées dans la base de registres.$\rL'application a peut-être été installée par un utilisateur différent."
!define un.GCOMPRIS_UNINSTALL_ERROR_2         "Vous n'avez pas les permissions pour supprimer cette application."

