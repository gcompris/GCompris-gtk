;;
;;  simp-chinese.nsh
;;
;;  Simplified Chinese language strings for the Windows GCompris NSIS installer.
;;  Windows Code page: 936
;;
;;  Author: Funda Wang" <fundawang@linux.net.cn>
;;  Version 2
;;


; License Page
!define GCOMPRIS_LICENSE_BUTTON			"下一步 >"
!define GCOMPRIS_LICENSE_BOTTOM_TEXT		"$(^Name) 以 GPL 许可发布。在此提供此许可仅为参考。$_CLICK"

; Components Page
!define GCOMPRIS_SECTION_TITLE                  "GCompris"
!define GCOMPRIS_SECTION_DESCRIPTION		"GCompris 核心文件和 DLLs"


; Finish Page
!define GCOMPRIS_FINISH_VISIT_WEB_SITE		"浏览 Windows GCompris 网页"

; GCompris Section Prompts and Texts
!define GCOMPRIS_UNINSTALL_DESC			"GCompris (只能删除)"
!define GCOMPRIS_PROMPT_WIPEOUT			"即将删除您的旧 GCompris 目录。您想要继续吗?$\r$\r注意: 您所安装的任何非标准的插件都将被删除。$\r但是不会影响 GCompris 用户设置。"
!define GCOMPRIS_PROMPT_DIR_EXISTS		"您指定的安装目录已经存在。$\r所有内容都将被删除。您想要继续吗?"



; Uninstall Section Prompts
!define un.GCOMPRIS_UNINSTALL_ERROR_1         "卸载程序找不到 GCompris 的注册表项目。$\r可能是另外的用户安装了此程序。"
!define un.GCOMPRIS_UNINSTALL_ERROR_2         "您没有权限卸载此程序。"
