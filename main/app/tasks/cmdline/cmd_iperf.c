/**
 * @file app/tasks/cmdline/cmd_iperf.c
 *
 * Copyright (C) 2023
 *
 * cmd_iperf.c is free software: you can redistribute it and/or modify
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
#include "data_proc_def.h"
#include "resources.h"
#include "options.h"
#include "esp_netif.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "iperf.h"

/*---------- macro ----------*/
#define TAG                                         "CMDIperf"

/*---------- type define ----------*/
struct iperf_args {
    struct arg_str *ip;
    struct arg_lit *server;
    struct arg_lit *udp;
    struct arg_lit *version;
    struct arg_int *port;
    struct arg_int *length;
    struct arg_int *interval;
    struct arg_int *time;
    struct arg_lit *abort;
    struct arg_end *end;
};

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int do_cmd_iperf(int argc, char **argv);

/*---------- variable ----------*/
static struct iperf_args iperf_args;
static const esp_console_cmd_t cmd_iperf = {
    .command = "iperf",
    .help = "iperf command",
    .hint = NULL,
    .func = do_cmd_iperf,
    .argtable = &iperf_args
};

/*---------- function ----------*/
static int do_cmd_iperf(int argc, char **argv)
{
    int32_t nerrors = arg_parse(argc, argv, (void **)&iperf_args);
    iperf_cfg_t cfg = {0};
    account_t account = resources_get("account_common");
    struct dp_wifi_sta dp = {0};
    int err = 1;

    do {
        if(nerrors) {
            arg_print_errors(stderr, iperf_args.end, argv[0]);
            break;
        }
        if(!account) {
            break;
        }
        dp.type = DP_WIFI_STA_TYPE_NETWORK;
        account->ops.pull(account, "wifi_sta", &dp, sizeof(dp));
        if(dp.param.network.state == WIFI_STA_DISCONNECTED) {
            break;
        }
        cfg.type = IPERF_IP_TYPE_IPV4;
        if(iperf_args.abort->count) {
            iperf_stop();
            break;
        }
        if((!iperf_args.ip->count && !iperf_args.server->count) ||
                (iperf_args.ip->count && iperf_args.server->count)) {
            xlog_tag_error(TAG, "Should specific client/server mode");
            break;
        }
        if(!iperf_args.ip->count) {
            cfg.flag |= IPERF_FLAG_SERVER;
        } else {
            cfg.destination_ip4 = esp_ip4addr_aton(iperf_args.ip->sval[0]);
            cfg.flag |= IPERF_FLAG_CLIENT;
        }
        cfg.source_ip4 = htonl(dp.param.network.ipv4.ip);
        if(!cfg.source_ip4) {
            break;
        }
        if(!iperf_args.udp->count) {
            cfg.flag |= IPERF_FLAG_TCP;
        } else {
            cfg.flag |= IPERF_FLAG_UDP;
        }
        if(!iperf_args.length->count) {
            cfg.len_send_buf = 0;
        } else {
            cfg.len_send_buf = iperf_args.length->ival[0];
        }
        if(!iperf_args.port->count) {
            cfg.sport = IPERF_DEFAULT_PORT;
            cfg.dport = IPERF_DEFAULT_PORT;
        } else {
            if(cfg.flag & IPERF_FLAG_SERVER) {
                cfg.sport = iperf_args.port->ival[0];
                cfg.dport = IPERF_DEFAULT_PORT;
            } else {
                cfg.sport = IPERF_DEFAULT_PORT;
                cfg.dport = iperf_args.port->ival[0];
            }
        }
        if(!iperf_args.interval->count) {
            cfg.interval = IPERF_DEFAULT_INTERVAL;
        } else {
            cfg.interval = iperf_args.interval->ival[0];
            if(cfg.interval <= 0) {
                cfg.interval = IPERF_DEFAULT_INTERVAL;
            }
        }
        if(!iperf_args.time->count) {
            cfg.time = IPERF_DEFAULT_TIME;
        } else {
            cfg.time = iperf_args.time->ival[0];
            if(cfg.time <= cfg.interval) {
                cfg.time = cfg.interval;
            }
        }
        xlog_tag_info(TAG, "mode=%s-%s sip=%d.%d.%d.%d:%d, dip=%d.%d.%d.%d:%d, interval=%d, time:%d\n",
                cfg.flag & IPERF_FLAG_TCP ? "tcp" : "udp",
                cfg.flag & IPERF_FLAG_SERVER ? "server" : "client",
                cfg.source_ip4 & 0xFF,
                (cfg.source_ip4 >> 8) & 0xFF,
                (cfg.source_ip4 >> 16) & 0xFF,
                (cfg.source_ip4 >> 24) & 0xFF,
                cfg.sport,
                cfg.destination_ip4 & 0xFF,
                (cfg.destination_ip4 >> 8) & 0xFF,
                (cfg.destination_ip4 >> 16) & 0xFF,
                (cfg.destination_ip4 >> 24) & 0xFF,
                cfg.dport,
                cfg.interval,
                cfg.time);
        iperf_start(&cfg);
        err = 0;
    } while(0);

    return err;
}

void cmd_iperf_init(void)
{
    iperf_args.ip = arg_str0("c", "client", "<ip>", "run in client mode, connecting to <host>");
    iperf_args.server = arg_lit0("s", "server", "run in server mode");
    iperf_args.udp = arg_lit0("u", "udp", "use UDP rather than TCP");
    iperf_args.version = arg_lit0("V", "ipv6_domain", "use IPV6 address rather than IPV4");
    iperf_args.port = arg_int0("p", "port", "<port>", "server port to listen on/connect to");
    iperf_args.length = arg_int0("l", "len", "<length", "Set read/write buffer size");
    iperf_args.interval = arg_int0("i", "interval", "<interval>", "seconds between periodic bandwith reports");
    iperf_args.time = arg_int0("t", "time", "<time>", "time in seconds to transmit for (default 10 secs)");
    iperf_args.abort = arg_lit0("a", "abort", "abort running iperf");
    iperf_args.end = arg_end(1);
    esp_console_cmd_register(&cmd_iperf);
}
