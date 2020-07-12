
idf_component_register(
    SRCS "mqtt_mgr.c" "coap_mgr.c"
    INCLUDE_DIRS "."
    REQUIRES console mqtt coap
) 

