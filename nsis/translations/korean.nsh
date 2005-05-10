;;
;;  korean.nsh
;;
;;  Korean language strings for the Windows GCompris NSIS installer.
;;  Windows Code page: 949
;;
;;  Author: Kyung-uk Son <vvs740@chol.com>
;;


; Components Page
!define GCOMPRIS_SECTION_TITLE                  "GCompris"
!define GCOMPRIS_SECTION_DESCRIPTION		"가임의 코어 파일과 dll"


; GCompris Section Prompts and Texts
!define GCOMPRIS_UNINSTALL_DESC			"GCompris (remove only)"
!define GCOMPRIS_PROMPT_WIPEOUT			"가임 디렉토리가 지워질 것입니다. 계속 할까요?$\r$\rNote: 비표준 플러그인은 지워지지 않을 수도 있습니다.$\r가임 사용자 설정에는 영향을 미치지 않습니다."
!define GCOMPRIS_PROMPT_DIR_EXISTS		"입력하신 설치 디렉토리가 이미 있습니다. 안에 들은 내용이 지워질 수도 있습니다. 계속할까요?"



; Uninstall Section Prompts
!define un.GCOMPRIS_UNINSTALL_ERROR_1         "언인스톨러가 가임의 레지스트리 엔트리를 찾을 수 없습니다.$\r이 프로그램을 다른 유저 권한으로 설치한 것 같습니다."
!define un.GCOMPRIS_UNINSTALL_ERROR_2         "이 프로그램을 제거할 수 있는 권한이 없습니다."
