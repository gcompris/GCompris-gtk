;;
;;  bulgarian.nsh
;;
;;  Bulgarian language strings for the Windows GCompris NSIS installer.
;;  Windows Code page: 1251
;;
;;  Author: Hristo Todorov <igel@bofh.bg>
;;


; Components Page
!define GCOMPRIS_SECTION_TITLE                  "GCompris"
!define GCOMPRIS_SECTION_DESCRIPTION		"Файлове на ядрото на GCompris и библиотеки"

; GCompris Section Prompts and Texts
!define GCOMPRIS_UNINSTALL_DESC			"GCompris (само премахване)"
!define GCOMPRIS_PROMPT_WIPEOUT			"Вашата стара GCompris директория ще бъде изтрита. Искате ли да продължите?$\r$\rЗабележка: Всички нестандартни добавки които сте инсталирали ще бъдат изтрити.$\rНастройките на GCompris няма да бъдат повлияни."
!define GCOMPRIS_PROMPT_DIR_EXISTS		"Директорията която избрахте съществува. Всичко което е в нея$\rще бъде изтрито. Желаете ли да продължите?"

; Uninstall Section Prompts
!define un.GCOMPRIS_UNINSTALL_ERROR_1         "Деисталатоа не може да намери записи в регистъра за GCompris.$\rВероятно е бил инсталиран от друг потребител."
!define un.GCOMPRIS_UNINSTALL_ERROR_2         "Нямате права да деинсталирате тази програма."
