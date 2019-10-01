if (UNIX)
  set(unix_list_srcs
      "unix_list.cc")

  add_executable(unix_list ${unix_list_srcs})
  set_target_properties(unix_list PROPERTIES FOLDER Examples)
endif (UNIX)
