/**
 * @file app/tasks/cmdline/cmd_uname.c
 *
 * Copyright (C) 2023
 *
 * cmd_uname.c is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @author HinsShum hinsshum@qq.com
 *
 * @encoding utf-8
 */

/*---------- includes ----------*/
#include "inc/cmdline_def.h"
#include "options.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "esp_chip_info.h"

/*---------- macro ----------*/
#define UNAME_HELP_INFO                             "Usage: uname [OPTION]...\n" \
                                                    "Print certain system information. With no OPTION, same as -s.\n\n" \
                                                    "    -a, --all-info             print all information, in the following order\n" \
                                                    "    -s, --kernel-name          print the kernel name\n" \
                                                    "    -n, --nodename             print the network node hostname\n" \
                                                    "    -r, --kernel-release       print the kernel release\n" \
                                                    "    -v, --kernel-version       print the kernel version\n" \
                                                    "    -m, --machine              print the machine hardware name\n" \
                                                    "    -o, --operating-system     print the operating system name\n" \
                                                    "        --help    display this help and exit\n" \

#define UNAME_KERNEL_VERSION                        tskKERNEL_VERSION_NUMBER " " __DATE__ " " __TIME__ " UTC"

/*---------- type define ----------*/
struct uname_args {
    struct arg_lit *all;
    struct arg_lit *kername_name;
    struct arg_lit *nodename;
    struct arg_lit *kernel_release;
    struct arg_lit *kernel_version;
    struct arg_lit *machine;
    struct arg_lit *operating_system;
    struct arg_lit *help;
    struct arg_end *end;
};

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int do_cmd_uname(int argc, char **argv);

/*---------- variable ----------*/
static struct uname_args uname_args;
static const esp_console_cmd_t cmd_uname = {
    .command = "uname",
    .help = "Print certain system information. With no OPTION, same as -s.\n",
    .hint = NULL,
    .func = do_cmd_uname,
    .argtable = &uname_args
};

/*---------- function ----------*/
static char *_get_machine_name(uint32_t machine_type)
{
    char *machine_name = "Unknown";

    switch(machine_type) {
        case CHIP_ESP32:
            machine_name = "ESP32";
            break;
        case CHIP_ESP32S2:
            machine_name = "ESP32-S2";
            break;
        case CHIP_ESP32S3:
            machine_name = "ESP32-S3";
            break;
        case CHIP_ESP32C3:
            machine_name = "ESP32-C3";
            break;
        case CHIP_ESP32H2:
            machine_name = "ESP32-H2";
            break;
        case CHIP_ESP32C2:
            machine_name = "ESP32-C2";
            break;
    }

    return machine_name;
}

static int do_cmd_uname(int argc, char **argv)
{
    int32_t nerrors = arg_parse(argc, argv, (void **)&uname_args);
    int err = 0;
    esp_chip_info_t chip = {0};

    do {
        if(nerrors != 0) {
            arg_print_errors(stderr, uname_args.end, argv[0]);
            err = 1;
            break;
        }
        if(uname_args.help->count) {
            xlog_info(UNAME_HELP_INFO);
            break;
        }
        esp_chip_info(&chip);
        if(uname_args.all->count) {
            xlog_info("FreeRTOS hinsshum-esp32 %s %s %s FreeRTOS\n", esp_get_idf_version(),
                    UNAME_KERNEL_VERSION, _get_machine_name(chip.model));
            break;
        }
        xlog_info(" \b");
        if(uname_args.kername_name->count) {
            xlog_cont("FreeRTOS ");
        }
        if(uname_args.nodename->count) {
            xlog_cont("hinsshum-esp32 ");
        }
        if(uname_args.kernel_release->count) {
            xlog_cont("%s ", esp_get_idf_version());
        }
        if(uname_args.kernel_version->count) {
            xlog_cont("%s ", UNAME_KERNEL_VERSION);
        }
        if(uname_args.machine->count) {
            xlog_cont("%s ", _get_machine_name(chip.model));
        }
        if(uname_args.operating_system->count) {
            xlog_cont("FreeRTOS ");
        }
        xlog_cont("\n");
    } while(0);

    return err;
}

void cmd_uname_init(void)
{
    uname_args.all = arg_lit0("a", "all-info", "print all information");
    uname_args.kername_name = arg_lit0("s", "kernel-name", "print the kernel name");
    uname_args.nodename = arg_lit0("n", "nodename", "print the network node hostname");
    uname_args.kernel_version = arg_lit0("v", "kernel-version", "print the kernel version");
    uname_args.kernel_release = arg_lit0("r", "kernel-release", "print the kernel release");
    uname_args.machine = arg_lit0("m", "machine", "print the machine hardware name");
    uname_args.operating_system = arg_lit0("o", "operating-system", "print the operating system name");
    uname_args.help = arg_lit0(NULL, "help", "display this help and exit");
    uname_args.end = arg_end(1);
    esp_console_cmd_register(&cmd_uname);
}
