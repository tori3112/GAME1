set(component_srcs "main.c" "alphabeta.c" "board.c" "rest_server.c" "main.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS ".")
