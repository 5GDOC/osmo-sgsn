/* (C) 2008-2009 by Harald Welte <laforge@gnumonks.org>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <openbsc/gsm_data.h>

void set_ts_e1link(struct gsm_bts_trx_ts *ts, u_int8_t e1_nr,
		   u_int8_t e1_ts, u_int8_t e1_ts_ss)
{
	ts->e1_link.e1_nr = e1_nr;
	ts->e1_link.e1_ts = e1_ts;
	ts->e1_link.e1_ts_ss = e1_ts_ss;
}

static const char *pchan_names[] = {
	[GSM_PCHAN_NONE]	= "NONE",
	[GSM_PCHAN_CCCH]	= "CCCH",
	[GSM_PCHAN_CCCH_SDCCH4]	= "CCCH+SDCCH4",
	[GSM_PCHAN_TCH_F]	= "TCH/F",
	[GSM_PCHAN_TCH_H]	= "TCH/H",
	[GSM_PCHAN_SDCCH8_SACCH8C] = "SDCCH8",
	[GSM_PCHAN_UNKNOWN]	= "UNKNOWN",
};

const char *gsm_pchan_name(enum gsm_phys_chan_config c)
{
	if (c >= ARRAY_SIZE(pchan_names))
		return "INVALID";

	return pchan_names[c];
}

static const char *lchan_names[] = {
	[GSM_LCHAN_NONE]	= "NONE",
	[GSM_LCHAN_SDCCH]	= "SDCCH",
	[GSM_LCHAN_TCH_F]	= "TCH/F",
	[GSM_LCHAN_TCH_H]	= "TCH/H",
	[GSM_LCHAN_UNKNOWN]	= "UNKNOWN",
};

const char *gsm_lchan_name(enum gsm_chan_t c)
{
	if (c >= ARRAY_SIZE(lchan_names))
		return "INVALID";

	return lchan_names[c];
}

static const char *chreq_names[] = {
	[GSM_CHREQ_REASON_EMERG]	= "EMERGENCY",
	[GSM_CHREQ_REASON_PAG]		= "PAGING",
	[GSM_CHREQ_REASON_CALL]		= "CALL",
	[GSM_CHREQ_REASON_LOCATION_UPD]	= "LOCATION_UPDATE",
	[GSM_CHREQ_REASON_OTHER]	= "OTHER",
};

const char *gsm_chreq_name(enum gsm_chreq_reason_t c)
{
	if (c >= ARRAY_SIZE(chreq_names))
		return "INVALID";

	return chreq_names[c];
}

struct gsm_network *gsm_network_init(unsigned int num_bts, enum gsm_bts_type bts_type,
				     u_int16_t country_code, u_int16_t network_code)
{
	int i;
	struct gsm_network *net;

	if (num_bts > GSM_MAX_BTS)
		return NULL;

	net = malloc(sizeof(*net));
	if (!net)
		return NULL;
	memset(net, 0, sizeof(*net));	

	net->country_code = country_code;
	net->network_code = network_code;
	net->num_bts = num_bts;

	for (i = 0; i < num_bts; i++) {
		struct gsm_bts *bts = &net->bts[i];
		int j;
		
		bts->network = net;
		bts->nr = i;
		bts->type = bts_type;
		bts->tsc = HARDCODED_TSC;

		for (j = 0; j < BTS_MAX_TRX; j++) {
			struct gsm_bts_trx *trx = &bts->trx[j];
			int k;

			trx->bts = bts;
			trx->nr = j;

			for (k = 0; k < 8; k++) {
				struct gsm_bts_trx_ts *ts = &trx->ts[k];
				int l;
				
				ts->trx = trx;
				ts->nr = k;
				ts->pchan = GSM_PCHAN_NONE;

				for (l = 0; l < TS_MAX_LCHAN; l++) {
					struct gsm_lchan *lchan;
					lchan = &ts->lchan[l];

					lchan->ts = ts;
					lchan->nr = l;
					lchan->type = GSM_LCHAN_NONE;
				}
			}
		}

		bts->num_trx = 1;	/* FIXME */
#ifdef HAVE_TRX1
		bts->num_trx++;
#endif
		bts->c0 = &bts->trx[0];
		bts->c0->ts[0].pchan = GSM_PCHAN_CCCH_SDCCH4;
	}
	return net;
}

static char ts2str[255];

char *gsm_ts_name(struct gsm_bts_trx_ts *ts)
{
	snprintf(ts2str, sizeof(ts2str), "(bts=%d,trx=%d,ts=%d)",
		 ts->trx->bts->nr, ts->trx->nr, ts->nr);

	return ts2str;
}

static const char *bts_types[] = {
	[GSM_BTS_TYPE_UNKNOWN] = "unknown",
	[GSM_BTS_TYPE_BS11] = "bs11",
	[GSM_BTS_TYPE_NANOBTS_900] = "nanobts900",
	[GSM_BTS_TYPE_NANOBTS_1800] = "nanobts1800",
};

enum gsm_bts_type parse_btstype(char *arg)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(bts_types); i++) {
		if (!strcmp(arg, bts_types[i]))
			return i;
	}	
	return GSM_BTS_TYPE_BS11; /* Default: BS11 */
}

char *btstype2str(enum gsm_bts_type type)
{
	if (type > ARRAY_SIZE(bts_types))
		return "undefined";
	return bts_types[type];
}
