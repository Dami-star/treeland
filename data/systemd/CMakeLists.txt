macro(install_symlink filepath wantsdir)
    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/link/${wantsdir}/)
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_INSTALL_PREFIX}/lib/systemd/user/${filepath} ${PROJECT_BINARY_DIR}/link/${wantsdir}/${filepath})
    install(FILES ${PROJECT_BINARY_DIR}/link/${wantsdir}/${filepath} DESTINATION lib/systemd/user/${wantsdir}/)
endmacro(install_symlink)

configure_file("dde-session-pre.target.wants/treeland-sd.service.in" "dde-session-pre.target.wants/treeland-sd.service" IMMEDIATE @ONLY)
configure_file("dde-session-pre.target.wants/treeland-xwayland.service.in" "dde-session-pre.target.wants/treeland-xwayland.service" IMMEDIATE @ONLY)
configure_file("dde-session-pre.target.wants/treeland-shortcut.service.in" "dde-session-pre.target.wants/treeland-shortcut.service" IMMEDIATE @ONLY)
configure_file("dde-session-pre.target.wants/treeland.service.in" "dde-session-pre.target.wants/treeland.service" IMMEDIATE @ONLY)

install(FILES ${SERVICES} DESTINATION lib/systemd/user/)

install(DIRECTORY DESTINATION lib/systemd/user/dde-session-pre.target.wants/)
set(DDE_SESSION_PRE_WANTS
    dde-session-pre.target.wants/treeland-sd.socket
    dde-session-pre.target.wants/treeland-xwayland.socket
    ${CMAKE_CURRENT_BINARY_DIR}/dde-session-pre.target.wants/treeland-sd.service
    ${CMAKE_CURRENT_BINARY_DIR}/dde-session-pre.target.wants/treeland-xwayland.service
    ${CMAKE_CURRENT_BINARY_DIR}/dde-session-pre.target.wants/treeland-shortcut.service
    ${CMAKE_CURRENT_BINARY_DIR}/dde-session-pre.target.wants/treeland.service
)

install(FILES ${DDE_SESSION_PRE_WANTS} DESTINATION lib/systemd/user/)
install_symlink(treeland-sd.socket dde-session-pre.target.wants)
install_symlink(treeland-sd.service dde-session-pre.target.wants)
install_symlink(treeland-xwayland.socket dde-session-pre.target.wants)
install_symlink(treeland-xwayland.service dde-session-pre.target.wants)
install_symlink(treeland-shortcut.service dde-session-pre.target.wants)
install_symlink(treeland.service dde-session-pre.target.wants)
