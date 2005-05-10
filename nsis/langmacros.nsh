;;
;; Windows Gcompris NSIS installer language macros
;;

!macro GCOMPRIS_MACRO_DEFAULT_STRING LABEL VALUE
  !ifndef "${LABEL}"
    !define "${LABEL}" "${VALUE}"
    !ifdef INSERT_DEFAULT
      !warning "${LANG} lang file mising ${LABEL}, using default.."
    !endif
  !endif
!macroend

!macro GCOMPRIS_MACRO_LANGSTRING_INSERT LABEL LANG
  LangString "${LABEL}" "${LANG_${LANG}}" "${${LABEL}}"
  !undef "${LABEL}"
!macroend

!macro GCOMPRIS_MACRO_LANGUAGEFILE_BEGIN LANG
  !define CUR_LANG "${LANG}"
!macroend

!macro GCOMPRIS_MACRO_LANGUAGEFILE_END
  !define INSERT_DEFAULT
  !include "${GCOMPRIS_DEFAULT_LANGFILE}"
  !undef INSERT_DEFAULT

  ; GCOMPRIS Language file Version 3
  ; String labels should match those from the default language file.

  ; Startup checks
  !insertmacro GCOMPRIS_MACRO_LANGSTRING_INSERT INSTALLER_IS_RUNNING			${CUR_LANG}
  !insertmacro GCOMPRIS_MACRO_LANGSTRING_INSERT GCOMPRIS_IS_RUNNING				${CUR_LANG}

  ; License Page
  !insertmacro GCOMPRIS_MACRO_LANGSTRING_INSERT GCOMPRIS_LICENSE_BUTTON			${CUR_LANG}
  !insertmacro GCOMPRIS_MACRO_LANGSTRING_INSERT GCOMPRIS_LICENSE_BOTTOM_TEXT		${CUR_LANG}

  ; Components Page
  !insertmacro GCOMPRIS_MACRO_LANGSTRING_INSERT GCOMPRIS_SECTION_TITLE			${CUR_LANG}
  !insertmacro GCOMPRIS_MACRO_LANGSTRING_INSERT GCOMPRIS_SECTION_DESCRIPTION		${CUR_LANG}

  ; Installer Finish Page
  !insertmacro GCOMPRIS_MACRO_LANGSTRING_INSERT GCOMPRIS_FINISH_VISIT_WEB_SITE		${CUR_LANG}

  ; Gcompris Section Prompts and Texts
  !insertmacro GCOMPRIS_MACRO_LANGSTRING_INSERT GCOMPRIS_UNINSTALL_DESC			${CUR_LANG}
  !insertmacro GCOMPRIS_MACRO_LANGSTRING_INSERT GCOMPRIS_PROMPT_WIPEOUT			${CUR_LANG}
  !insertmacro GCOMPRIS_MACRO_LANGSTRING_INSERT GCOMPRIS_PROMPT_DIR_EXISTS		${CUR_LANG}

  ; Uninstall Section Prompts
  !insertmacro GCOMPRIS_MACRO_LANGSTRING_INSERT un.GCOMPRIS_UNINSTALL_ERROR_1		${CUR_LANG}
  !insertmacro GCOMPRIS_MACRO_LANGSTRING_INSERT un.GCOMPRIS_UNINSTALL_ERROR_2		${CUR_LANG}

  !undef CUR_LANG
!macroend

!macro GCOMPRIS_MACRO_INCLUDE_LANGFILE LANG FILE
  !insertmacro GCOMPRIS_MACRO_LANGUAGEFILE_BEGIN "${LANG}"
  !include "${FILE}"
  !insertmacro GCOMPRIS_MACRO_LANGUAGEFILE_END
!macroend
