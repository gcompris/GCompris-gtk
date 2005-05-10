;;
;;  vietnamese.nsh
;;
;;  Vietnamese language strings for the Windows GCompris NSIS installer.
;;  Windows Code page: 1258
;;
;;  Version 2
;;  Note: The NSIS Installer does not yet have Vietnamese translations. Until
;;  it does, these GCompris translations can not be used. 
;;


; License Page
!define GCOMPRIS_LICENSE_BUTTON			"Tiếp theo >"
!define GCOMPRIS_LICENSE_BOTTOM_TEXT		"$(^Name) được phát hành theo giấy  phép GPL. Giấy phép thấy ở đây chỉ là để cung cấp thông tin mà thôi. $_CLICK"

; Components Page
!define GCOMPRIS_SECTION_TITLE                  "GCompris"
!define GCOMPRIS_SECTION_DESCRIPTION		"Các tập tin GCompris chính và dlls"


; Installer Finish Page
!define GCOMPRIS_FINISH_VISIT_WEB_SITE		"Hãy xem trang chủ Windows GCompris"

; GCompris Section Prompts and Texts
!define GCOMPRIS_UNINSTALL_DESC			"GCompris (chỉ bỏ cài đặt)"
!define GCOMPRIS_PROMPT_WIPEOUT			"Thư mục GCompris cũ sẽ bị xóa. Bạn muốn tiếp tục không?$\r$\rNote: Mọi plugin không chuẩn mà bạn đã cài sẽ bị xóa.$\rCác thiết lập người dùng GCompris sẽ không còn tác dụng."
!define GCOMPRIS_PROMPT_DIR_EXISTS		"Thư mục cài đặt mà bạn định ra đã tồn tại rồi. Mọi nội dung$\rsẽ bị xóa. Bạn muốn tiếp tục không?"



; Uninstall Section Prompts
!define un.GCOMPRIS_UNINSTALL_ERROR_1		"Trình gỡ cài đặt không tìm được các  registry entry cho GCompris.$\rCó thể là chương trình được người dùng khác cài đặt."
!define un.GCOMPRIS_UNINSTALL_ERROR_2		"Bạn không có quyền hạn để gỡ bỏ chương trình này."
