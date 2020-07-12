
idf_component_register(
    SRCS "config_mgr.c" "config_cmds.c"
    INCLUDE_DIRS "."
    REQUIRES console nvs_flash
) 

