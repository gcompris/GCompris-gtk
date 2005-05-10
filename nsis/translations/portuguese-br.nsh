;;
;;  portuguese-br.nsh
;;
;;  Portuguese (BR) language strings for the Windows GCompris NSIS installer.
;;  Windows Code page: 1252
;;
;;  Author: Maurício de Lemos Rodrigues Collares Neto <mauricioc@myrealbox.com>, 2003.
;;  Version 2
;;


; License Page
!define GCOMPRIS_LICENSE_BUTTON			"Avançar >"
!define GCOMPRIS_LICENSE_BOTTOM_TEXT		"$(^Name) é distribuído sob a licença GPL. Esta licença é disponibilizada aqui apenas para fins informativos. $_CLICK" 

; Components Page
!define GCOMPRIS_SECTION_TITLE                  "GCompris"
!define GCOMPRIS_SECTION_DESCRIPTION		"Arquivos e bibliotecas principais do GCompris"


; GCompris Section Prompts and Texts
!define GCOMPRIS_UNINSTALL_DESC			"GCompris (apenas remover)"
!define GCOMPRIS_PROMPT_WIPEOUT			"Sua antiga instalação do GCompris está prestes a ser removida. Você gostaria de continuar?$\r$\rNota: Quaisquer plugins não-padrão que você pode ter instalado serão removidos.$\rAs configurações de usuário do GCompris não serão afetadas."
!define GCOMPRIS_PROMPT_DIR_EXISTS		"O diretório de instalação do que você especificou já existe. Qualquer conteúdo$\rserá deletado. Deseja continuar?"



; Installer Finish Page
!define GCOMPRIS_FINISH_VISIT_WEB_SITE		"Visite a página da web do GCompris para Windows"

; Uninstall Section Prompts
!define un.GCOMPRIS_UNINSTALL_ERROR_1		"O desinstalador não pôde encontrar entradas de registro do GCompris.$\rÉ provável que outro usuário tenha instalado esta aplicação."
!define un.GCOMPRIS_UNINSTALL_ERROR_2		"Você não tem permissão para desinstalar essa aplicação."
