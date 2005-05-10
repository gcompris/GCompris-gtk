;;
;;  hebrew.nsh
;;
;;  Hebrew language strings for the Windows GCompris NSIS installer.
;;  Windows Code page: 1255
;;
;;  Author: Eugene Shcherbina <eugene@websterworlds.com>
;;  Version 2
;;


; License Page
!define GCOMPRIS_LICENSE_BUTTON			"הבא >"
!define GCOMPRIS_LICENSE_BOTTOM_TEXT		"$(^Name) .הרישיון נמצא כאן בשביל מידע בלבד .GPL משוחרר תחת רישיון $_CLICK"

; Components Page
!define GCOMPRIS_SECTION_TITLE                  "GCompris"
!define GCOMPRIS_SECTION_DESCRIPTION		".בסיססים DLL-ו GCOMPRIS קבצי"


; Installer Finish Page
!define GCOMPRIS_FINISH_VISIT_WEB_SITE		".GCOMPRISבקרו באתר של"

; GCompris Section Prompts and Texts
!define GCOMPRIS_UNINSTALL_DESC			"GCOMPRIS (מחיקה בלבד)"
!define GCOMPRIS_PROMPT_WIPEOUT			"?הישנה הולכת להימחק. להמשיך GCOMPRIS תיקיית$\r$\rNote: הערה: תוספות לא רגילות ימחקו.$\r.הגדרות המשתמש לא ישתנו"
!define GCOMPRIS_PROMPT_DIR_EXISTS		"תיקיית ההתקנה כבר קיימת. כל משבתוכה $\r?ימחק. להמשיך"



; Uninstall Section Prompts
!define un.GCOMPRIS_UNINSTALL_ERROR_2		".אין לך זכות למחוק תוכנה זאת"
