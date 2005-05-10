;;
;;  hungarian.nsh
;;
;;  Default language strings for the Windows GCompris NSIS installer.
;;  Windows Code page: 1250
;;
;;  Author: Sutto Zoltan <suttozoltan@chello.hu
;;


; Components Page
!define GCOMPRIS_SECTION_TITLE                  "GCompris"
!define GCOMPRIS_SECTION_DESCRIPTION		"GCompris fájlok és dll-ek"


; GCompris Section Prompts and Texts
!define GCOMPRIS_UNINSTALL_DESC			"GCompris (csak eltávolítás)"
!define GCOMPRIS_PROMPT_WIPEOUT			"Az Ön korábbi GCompris könyvtára törlõdni fog. Folytatni szeretné?$\r$\rMegjegyzés: Minden Ön által telepített plugin törlõdni fog.$\rGCompris felhasználói beállításokat ez nem érinti."
!define GCOMPRIS_PROMPT_DIR_EXISTS		"A telepítéskor megadott könyvtár már létezik. Minden állomány törlõdni fog.$\rFolytatni szeretné?"



; Uninstall Section Prompts
!define un.GCOMPRIS_UNINSTALL_ERROR_1         "Az eltávolító nem talált GCompris registry bejegyzéseket.$\rValószínüleg egy másik felhasználó telepítette az alkalmazást."
!define un.GCOMPRIS_UNINSTALL_ERROR_2         "Nincs jogosultsága az alkalmazás eltávolításához."
