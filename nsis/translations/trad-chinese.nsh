;;
;;  trad-chinese.nsh
;;
;;  Traditional Chineese language strings for the Windows GCompris NSIS installer.
;;  Windows Code page:950 
;;
;;  Author: Paladin R. Liu <paladin@ms1.hinet.net>
;;  Minor updates: Ambrose C. Li <acli@ada.dhs.org>
;;


; License Page
!define GCOMPRIS_LICENSE_BUTTON			"下一步 >"
!define GCOMPRIS_LICENSE_BOTTOM_TEXT		"$(^Name) 係以 GPL 方式授權發佈，在此列出授權書，純綷係作為參考之用。$_CLICK"

; Components Page
!define GCOMPRIS_SECTION_TITLE                  "GCompris"
!define GCOMPRIS_SECTION_DESCRIPTION		"GCompris 核心檔案及動態函式庫"


; GCompris Section Prompts and Texts
!define GCOMPRIS_UNINSTALL_DESC			"GCompris v${GCOMPRIS_VERSION} (只供移除)"
!define GCOMPRIS_PROMPT_WIPEOUT			"您先前安裝於目錄中的舊版 GCompris 將會被移除。您要繼續嗎？$\r$\r請注意：任何您所安裝的非官方維護模組都將被刪除。$\r而 GCompris 的使用者設定將不會受到影響。"
!define GCOMPRIS_PROMPT_DIR_EXISTS		"您所選定的安裝目錄下的所有檔案都將被移除。$\r您要繼續嗎？"



; Uninstall Section Prompts
!define un.GCOMPRIS_UNINSTALL_ERROR_1         "移除程式無法找到 GCompris 的安裝資訊。$\r這應該是有其他的使用者重新安裝了這個程式。"
!define un.GCOMPRIS_UNINSTALL_ERROR_2         "您目前的權限無法移除 GCompris。"
