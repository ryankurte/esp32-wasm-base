
#include "config_mgr.h"

#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"

#define TAG "CFG_CMD"

static struct {
    struct arg_str *key;
    struct arg_str *ns;
    struct arg_str *val;
    struct arg_end *end;
} cfg_set_args;

static int cfg_set_cmd(int argc, char** argv) {
    // Parse arguments
    int nerrors = arg_parse(argc, argv, (void **) &cfg_set_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, cfg_set_args.end, argv[0]);
        return 1;
    }

    if (!cfg_set_args.key || (cfg_set_args.key->count != 1)) {
        ESP_LOGE(TAG, "Error config key arg required");
        return 2;
    }

    if (!cfg_set_args.val || (cfg_set_args.val->count != 1)) {
        ESP_LOGE(TAG, "Error config val arg required");
        return 3;
    }

    int res = CONFIG_MGR_set(cfg_set_args.key->sval[0], cfg_set_args.val->sval[0]);
    if (res != ESP_OK) {
        return 4;
    }

    printf("Set config key: '%s' val: '%s'\n", cfg_set_args.key->sval[0], cfg_set_args.val->sval[0]);

    return 0;
}

static struct {
    struct arg_str *key;
    struct arg_str *ns;
    struct arg_end *end;
} cfg_get_args;

static int cfg_get_cmd(int argc, char **argv) {
    // Parse arguments
    int nerrors = arg_parse(argc, argv, (void **) &cfg_get_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, cfg_get_args.end, argv[0]);
        return 1;
    }

    char buff[128];

    int res = CONFIG_MGR_get(cfg_get_args.key->sval[0], buff, sizeof(buff));
    if (res != ESP_OK) {
        return 2;
    }

    printf("key: '%s': val: '%s'\n", cfg_get_args.key->sval[0], buff);

    return 0;
}

static struct {
    struct arg_str *key;
    struct arg_str *ns;
    struct arg_end *end;
} cfg_clear_args;

static int cfg_clear_cmd(int argc, char **argv) {
    // Parse arguments
    int nerrors = arg_parse(argc, argv, (void **) &cfg_clear_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, cfg_clear_args.end, argv[0]);
        return 1;
    }

    int res = CONFIG_MGR_clear(cfg_clear_args.key->sval[0]);
    if (res != ESP_OK) {
        return 2;
    }

    printf("Cleared config key: '%s'\n", cfg_clear_args.key->sval[0]);

    return 0;
}

static struct {
    struct arg_str *ns;
    struct arg_end *end;
} cfg_list_args;

static int cfg_list_cmd(int argc, char **argv) {
    // Parse arguments
    int nerrors = arg_parse(argc, argv, (void **) &cfg_list_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, cfg_list_args.end, argv[0]);
        return 1;
    }

    CONFIG_MGR_list();
    
    return 0;
}


void CONFIG_MGR_register_commands() {
    cfg_set_args.key = arg_str1(NULL, NULL, "<key>", "Configuration key");
    cfg_set_args.ns = arg_str0(NULL, "namespace", "<namespace>", "Configuration namespace");
    cfg_set_args.val = arg_str1(NULL, NULL, "<key>", "Configuration value");
    cfg_set_args.end = arg_end(3);

    const esp_console_cmd_t cfg_set = {
        .command = "cfg-set",
        .help = "Set a configuration value",
        .hint = NULL,
        .func = &cfg_set_cmd,
        .argtable = &cfg_set_args,
    };

    cfg_get_args.key = arg_str1(NULL, NULL, "<key>", "Configuration key");
    cfg_get_args.ns = arg_str0(NULL, "namespace", "<namespace>", "Configuration namespace");
    cfg_get_args.end = arg_end(2);

    const esp_console_cmd_t cfg_get = {
        .command = "cfg-get",
        .help = "Get a configuration value",
        .hint = NULL,
        .func = &cfg_get_cmd,
        .argtable = &cfg_get_args,
    };

    cfg_clear_args.key = arg_str1(NULL, NULL, "<key>", "Configuration key");
    cfg_clear_args.ns = arg_str0(NULL, "namespace", "<namespace>", "Configuration namespace");
    cfg_clear_args.end = arg_end(2);

    const esp_console_cmd_t cfg_clear = {
        .command = "cfg-clear",
        .help = "Clear a configuration value",
        .hint = NULL,
        .func = &cfg_clear_cmd,
        .argtable = &cfg_clear_args,
    };

    cfg_list_args.ns = arg_str0(NULL, "namespace", "<namespace>", "Configuration namespace");
    cfg_list_args.end = arg_end(1);

    const esp_console_cmd_t cfg_list = {
        .command = "cfg-list",
        .help = "List all configuration values",
        .hint = NULL,
        .func = &cfg_list_cmd,
        .argtable = &cfg_list_args,
    };

    ESP_ERROR_CHECK( esp_console_cmd_register(&cfg_set));
    ESP_ERROR_CHECK( esp_console_cmd_register(&cfg_get));
    ESP_ERROR_CHECK( esp_console_cmd_register(&cfg_clear));
    ESP_ERROR_CHECK( esp_console_cmd_register(&cfg_list));
}

