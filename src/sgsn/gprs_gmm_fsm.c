#include <osmocom/core/tdef.h>

#include <osmocom/sgsn/gprs_gmm_fsm.h>
#include <osmocom/sgsn/gprs_mm_state_gb_fsm.h>
#include <osmocom/sgsn/gprs_mm_state_iu_fsm.h>

#include <osmocom/sgsn/debug.h>
#include <osmocom/sgsn/sgsn.h>

#define X(s) (1 << (s))

static const struct osmo_tdef_state_timeout gmm_fsm_timeouts[32] = {
	[ST_GMM_DEREGISTERED] = { },
	[ST_GMM_COMMON_PROC_INIT] = { },
	[ST_GMM_REGISTERED_NORMAL] = { },
	[ST_GMM_REGISTERED_SUSPENDED] = { },
	[ST_GMM_DEREGISTERED_INIT] = { },
};

#define gmm_fsm_state_chg(fi, NEXT_STATE) \
	osmo_tdef_fsm_inst_state_chg(fi, NEXT_STATE, gmm_fsm_timeouts, sgsn->cfg.T_defs, -1)

static void st_gmm_deregistered(struct osmo_fsm_inst *fi, uint32_t event, void *data)
{
	switch(event) {
	case E_GMM_COMMON_PROC_INIT_REQ:
		gmm_fsm_state_chg(fi, ST_GMM_COMMON_PROC_INIT);
		break;
	case E_GMM_ATTACH_SUCCESS:
		gmm_fsm_state_chg(fi, ST_GMM_REGISTERED_NORMAL);
		break;
	}
}

static void st_gmm_common_proc_init(struct osmo_fsm_inst *fi, uint32_t event, void *data)
{
	switch(event) {
	/* TODO: events not used
	case E_GMM_LOWER_LAYER_FAILED:
	case E_GMM_COMMON_PROC_FAILED:
		gmm_fsm_state_chg(fi, ST_GMM_DEREGISTERED);
		break;
	*/
	case E_GMM_COMMON_PROC_SUCCESS:
	case E_GMM_ATTACH_SUCCESS:
		gmm_fsm_state_chg(fi, ST_GMM_REGISTERED_NORMAL);
		break;
	}
}

static void st_gmm_registered_normal(struct osmo_fsm_inst *fi, uint32_t event, void *data)
{
	switch(event) {
	case E_GMM_COMMON_PROC_INIT_REQ:
		gmm_fsm_state_chg(fi, ST_GMM_COMMON_PROC_INIT);
		break;
	/* case E_GMM_NET_INIT_DETACH_REQ:
		gmm_fsm_state_chg(fi, ST_GMM_DEREGISTERED_INIT);
		break; */
	/* case E_GMM_MS_INIT_DETACH_REQ:
		gmm_fsm_state_chg(fi, ST_GMM_DEREGISTERED);
		break; */
	case E_GMM_SUSPEND:
		gmm_fsm_state_chg(fi, ST_GMM_REGISTERED_SUSPENDED);
		break;
	}
}

static void st_gmm_registered_suspended(struct osmo_fsm_inst *fi, uint32_t event, void *data)
{
	switch(event) {
	case E_GMM_RESUME:
		gmm_fsm_state_chg(fi, ST_GMM_REGISTERED_NORMAL);
		break;
	}
}

static void st_gmm_deregistered_init(struct osmo_fsm_inst *fi, uint32_t event, void *data)
{
	switch(event) {
	/* TODO: events not used in osmo-sgsn code
	case E_GMM_DETACH_ACCEPTED:
	case E_GMM_LOWER_LAYER_FAILED:
		gmm_fsm_state_chg(fi, ST_GMM_DEREGISTERED);
		break;
	*/
	}
}

static struct osmo_fsm_state gmm_fsm_states[] = {
	[ST_GMM_DEREGISTERED] = {
		.in_event_mask =
			X(E_GMM_COMMON_PROC_INIT_REQ) |
			X(E_GMM_ATTACH_SUCCESS),
		.out_state_mask = X(ST_GMM_COMMON_PROC_INIT),
		.name = "Deregistered",
		.action = st_gmm_deregistered,
	},
	[ST_GMM_COMMON_PROC_INIT] = {
		.in_event_mask =
			/* X(E_GMM_LOWER_LAYER_FAILED) | */
			/* X(E_GMM_COMMON_PROC_FAILED) | */
			X(E_GMM_COMMON_PROC_SUCCESS) |
			X(E_GMM_ATTACH_SUCCESS),
		.out_state_mask =
			X(ST_GMM_DEREGISTERED) |
			X(ST_GMM_REGISTERED_NORMAL),
		.name = "CommonProcedureInitiated",
		.action = st_gmm_common_proc_init,
	},
	[ST_GMM_REGISTERED_NORMAL] = {
		.in_event_mask =
			X(E_GMM_COMMON_PROC_INIT_REQ) |
			/* X(E_GMM_NET_INIT_DETACH_REQ) | */
			/* X(E_GMM_MS_INIT_DETACH_REQ) | */
			X(E_GMM_SUSPEND),
		.out_state_mask =
			X(ST_GMM_DEREGISTERED) |
			X(ST_GMM_COMMON_PROC_INIT) |
			X(ST_GMM_DEREGISTERED_INIT) |
			X(ST_GMM_REGISTERED_SUSPENDED),
		.name = "Registered.NORMAL",
		.action = st_gmm_registered_normal,
	},
	[ST_GMM_REGISTERED_SUSPENDED] = {
		.in_event_mask = X(E_GMM_RESUME),
		.out_state_mask =
			X(ST_GMM_DEREGISTERED) |
			X(ST_GMM_REGISTERED_NORMAL),
		.name = "Registered.SUSPENDED",
		.action = st_gmm_registered_suspended,
	},
	[ST_GMM_DEREGISTERED_INIT] = {
		.in_event_mask = 0
			/* X(E_GMM_DETACH_ACCEPTED) | */
			/* X(E_GMM_LOWER_LAYER_FAILED) */,
		.out_state_mask = X(ST_GMM_DEREGISTERED),
		.name = "DeregisteredInitiated",
		.action = st_gmm_deregistered_init,
	},
};

const struct value_string gmm_fsm_event_names[] = {
	OSMO_VALUE_STRING(E_GMM_COMMON_PROC_INIT_REQ),
	/* OSMO_VALUE_STRING(E_GMM_COMMON_PROC_FAILED), */
	/*  OSMO_VALUE_STRING(E_GMM_LOWER_LAYER_FAILED),  */
	OSMO_VALUE_STRING(E_GMM_COMMON_PROC_SUCCESS),
	OSMO_VALUE_STRING(E_GMM_ATTACH_SUCCESS),
	/*  OSMO_VALUE_STRING(E_GMM_NET_INIT_DETACH_REQ), */
	/*  OSMO_VALUE_STRING(E_GMM_MS_INIT_DETACH_REQ), */
	/* OSMO_VALUE_STRING(E_GMM_DETACH_ACCEPTED), */
	OSMO_VALUE_STRING(E_GMM_SUSPEND),
	OSMO_VALUE_STRING(E_GMM_CLEANUP),
	OSMO_VALUE_STRING(E_GMM_RAT_CHANGE),
	{ 0, NULL }
};

void gmm_fsm_allstate_action(struct osmo_fsm_inst *fi, uint32_t event, void *data) {
	struct sgsn_mm_ctx *mmctx = fi->priv;
	struct gmm_rat_change_data *rat_chg = (struct gmm_rat_change_data *)data;

	switch (event) {
	case E_GMM_RAT_CHANGE:

		switch (fi->state) {
		case ST_GMM_COMMON_PROC_INIT:
			gmm_fsm_state_chg(fi, ST_GMM_DEREGISTERED);
		default:
			if (mmctx->ran_type == MM_CTX_T_GERAN_Gb)
				osmo_fsm_inst_dispatch(mmctx->gb.mm_state_fsm, E_MM_IMPLICIT_DETACH, NULL);
			else if (mmctx->ran_type == MM_CTX_T_UTRAN_Iu) {
				osmo_fsm_inst_dispatch(mmctx->iu.mm_state_fsm, E_PMM_IMPLICIT_DETACH, NULL);
				mmctx->gb.llme = rat_chg->llme;
			}

			mmctx->ran_type = rat_chg->new_ran_type;
			break;
		}

	case E_GMM_CLEANUP:
		switch (fi->state) {
		case ST_GMM_DEREGISTERED:
			break;
		default:
			gmm_fsm_state_chg(fi, ST_GMM_DEREGISTERED);
			break;
		}
	}
}

int gmm_fsm_timer_cb(struct osmo_fsm_inst *fi)
{
	return 0;
}

struct osmo_fsm gmm_fsm = {
	.name = "GMM",
	.states = gmm_fsm_states,
	.num_states = ARRAY_SIZE(gmm_fsm_states),
	.event_names = gmm_fsm_event_names,
	.allstate_event_mask = X(E_GMM_CLEANUP) | X(E_GMM_RAT_CHANGE),
	.allstate_action = gmm_fsm_allstate_action,
	.log_subsys = DMM,
	.timer_cb = gmm_fsm_timer_cb,
};

static __attribute__((constructor)) void gmm_fsm_init(void)
{
	OSMO_ASSERT(osmo_fsm_register(&gmm_fsm) == 0);
}
