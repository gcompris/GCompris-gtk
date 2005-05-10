;;
;;  portuguese.nsh
;;
;;  Portuguese (PT) language strings for the Windows GCompris NSIS installer.
;;  Windows Code page: 1252
;;
;;  Author: Duarte Serrano Gonçalves Henriques <duarte_henriques@myrealbox.com>, 2003.
;;  Version 2
;;


; License Page
!define GCOMPRIS_LICENSE_BUTTON			"Seguinte >"
!define GCOMPRIS_LICENSE_BOTTOM_TEXT		"$(^Name) está disponível sob alicença GPL. O texto da licença é fornecido meramente a título informativo. $_CLICK" 

; Components Page
!define GCOMPRIS_SECTION_TITLE                  "GCompris"
!define GCOMPRIS_SECTION_DESCRIPTION		"Ficheiros e bibliotecas principais do GCompris"


; Installer Finish Page
!define GCOMPRIS_FINISH_VISIT_WEB_SITE		"Visite a página web do GCompris para Windows" 

; GCompris Section Prompts and Texts
!define GCOMPRIS_UNINSTALL_DESC			"GCompris (remover apenas)"
!define GCOMPRIS_PROMPT_WIPEOUT			"A sua antiga instalação do GCompris está prestes a ser removida. Deseja continuar?$\r$\rNota: Quaisquer plugins não-padrão que poderá ter instalado serão removidos.$\rAs configurações de utilizador do GCompris não serão afectadas."
!define GCOMPRIS_PROMPT_DIR_EXISTS		"A directoria de instalação do que especificou já existe. Qualquer conteúdo$\rserá apagado. Deseja continuar?"



; Uninstall Section Prompts
!define un.GCOMPRIS_UNINSTALL_ERROR_1         "O desinstalador não pôde encontrar entradas de registo do GCompris.$\rÉ provável que outro utilizador tenha instalado este programa."
!define un.GCOMPRIS_UNINSTALL_ERROR_2         "Não tem permissão para desinstalar este programa."
