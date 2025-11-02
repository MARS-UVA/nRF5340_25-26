#include "talon_fx.h"
#include "util.h"

void talon_fx_send_can_message(talon_fx_t *self, int msg_id, char *message, uint8_t length)
{
    send_can_message(self->can_dev, self->identifier | msg_id, message, length);
}

void talon_fx_set(talon_fx_t *self, double speed)
{
    short valueInt = (short)(speed * 1024);
    if (valueInt < 0)
    {
        valueInt = 0xfff - (-1 * valueInt);
    }

    talon_fx_send_can_message(self, 0x204b540, (char[]){0, 1, 0, 0, 0, 0, valueInt & 255, (valueInt >> 8) & 255}, 8);
}

void talon_fx_set_neutral_mode(talon_fx_t *self, talon_fx_neutral_mode_t mode)
{
    char msg[] = {0x21, 0x6E, 0x08, (mode == TALON_FX_NEUTRAL_COAST ? 0 : 1), 0, 0, 0, 0xAA};
    for (int pair = 0; pair < (mode == TALON_FX_NEUTRAL_COAST ? 2 : 1); pair++)
    {
        talon_fx_send_can_message(self, 0x2047c00, msg, 8);
        k_msleep(1);
    }
    talon_fx_send_can_message(self, 0x2047c00, "\x10\x0c\xc5\x06\x0d\x00\x00\x00", 8);
}

void talon_fx_apply_supply_current_limit(talon_fx_t *self, float current) {
	char x[] = {0x21, 0x70, 0x08, 0, 0, 0, 0, 0xaa};
	float_to_byte_array(current, &x[3]);
	talon_fx_send_can_message(self, 0x2047c00, x, 8);
	talon_fx_send_can_message(self, 0x2047c00, "\x10\x0c\xc5\x06\x0d\x00\x00\x00", 8);
	k_msleep(1);
	talon_fx_send_can_message(self, 0x2047c00, x, 8);
	talon_fx_send_can_message(self, 0x2047c00, "\x10\x0c\xc5\x06\x0d\x00\x00\x00", 8);
	k_msleep(1);
}

void talon_fx_apply_config(talon_fx_t *self, talon_fx_slot0_config_t *config)
{
	double *configs[] = {
			&(config->kP),
			&(config->kI),
			&(config->kD),
			&(config->kS),
			&(config->kV),
			&(config->kA),
			&(config->kG)
	};

	for (int i = 0; i < 7; i++)
	{
		char x[] = { 0x21, 0x53 + i, 0x08, 0, 0, 0, 0, 0xaa };
		float_to_byte_array(*configs[i], &x[3]);
		talon_fx_send_can_message(self, 0x2047c00, x, 8);
		talon_fx_send_can_message(self, 0x2047c00, "\x10\x0c\xc5\x06\x0d\x00\x00\x00", 8);
		k_msleep(1);
	}
}

void talon_fx_set_control(talon_fx_t *self, int velocity, double feedforward)
{
	// Get velocity value (3 bytes)
	if (velocity >= 0) {
		velocity *= 2;
		velocity *= 16;
	}
	else {
		velocity *= 2;
		velocity = 0x40000 - (-16 * velocity);
	}
	// Get feedforward value
	int feedforwardInt = feedforward * 100;
	if (feedforward < 0)
		feedforwardInt = (~(feedforwardInt * -1)) + 1;


	char x[] = {0, 1, velocity & 0xff, (velocity >> 8) & 0xff, velocity >> 16 & 0xff, 0, feedforwardInt & 0xff, (feedforwardInt >> 8) & 0xff};
	talon_fx_send_can_message(self, 0x2043700, x, 8);
	k_msleep(1);
}

void talon_fx_set_closed_loop_ramp_period(talon_fx_t *self, float period)
{
	char x[] = {0x21, 0x84, 0x08, 0x00, 0x00, 0x00, 0x00, 0xaa};
	float_to_byte_array(period, &x[3]);
	talon_fx_send_can_message(self, 0x2047c00, x, 8);
	talon_fx_send_can_message(self, 0x2047c00, "\x10\x0c\xc5\x06\x0d\x00\x00\x00", 8);
	k_msleep(1);
	talon_fx_send_can_message(self, 0x2047c00, x, 8);
	talon_fx_send_can_message(self, 0x2047c00, "\x10\x0c\xc5\x06\x0d\x00\x00\x00", 8);
	k_msleep(1);
}

bool talon_fx_init(talon_fx_t *instance, const struct device *can_dev, int identifier)
{
    if (!instance)
        return false;

    *instance = (talon_fx_t){
        .initialized = true,
        .can_dev = (struct device *)can_dev,
        .identifier = identifier,
        .set = talon_fx_set,
        .set_neutral_mode = talon_fx_set_neutral_mode,
        .apply_supply_current_limit = talon_fx_apply_supply_current_limit,
        .apply_config = talon_fx_apply_config,
        .set_control = talon_fx_set_control,
        .set_closed_loop_ramp_period = talon_fx_set_closed_loop_ramp_period,
    };

    return true;
}
