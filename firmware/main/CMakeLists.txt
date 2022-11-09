idf_component_register(SRCS "main.c" "bt.c" "log.c" "rtc.c"
                    INCLUDE_DIRS ".")
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
