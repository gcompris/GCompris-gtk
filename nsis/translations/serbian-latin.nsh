;;
;;  serbian-latin.nsh
;;
;;  Serbian (Latin) language strings for the Windows GCompris NSIS installer.
;;  Windows Code page: 1250
;;
;;  Author: Danilo Segan <dsegan@gmx.net>
;;


; Components Page
!define GCOMPRIS_SECTION_TITLE                  "GCompris"
!define GCOMPRIS_SECTION_DESCRIPTION		"Osnovne GCompris datoteke i dinamièke biblioteke"


; GCompris Section Prompts and Texts
!define GCOMPRIS_UNINSTALL_DESC			"GCompris (samo uklanjanje)"
!define GCOMPRIS_PROMPT_WIPEOUT			"Vaš stari GCompris direktorijum æe biti obrisan. Da li želite da nastavite?$\r$\rPrimedba: Svi nestandardni dodaci koje ste instalirali æe biti obrisani.$\rGCompris postavke korisnika neæe biti promenjene."
!define GCOMPRIS_PROMPT_DIR_EXISTS		"Instalacioni direktorijum koji ste naveli veæ postoji. Sav sadržaj$\ræe biti obrisan. Da li želite da nastavite?"



; Uninstall Section Prompts
!define un.GCOMPRIS_UNINSTALL_ERROR_1         "Program za uklanjanje instalacije ne može da pronaðe stavke registra za GCompris.$\rVerovatno je ovu aplikaciju instalirao drugi korisnik."
!define un.GCOMPRIS_UNINSTALL_ERROR_2         "Nemate ovlašæenja za deinstalaciju ove aplikacije."
