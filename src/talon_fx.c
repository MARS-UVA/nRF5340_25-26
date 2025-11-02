#include "talon_fx.h"

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
        // .apply_supply_current_limit = talon_fx_apply_supply_current_limit,
        // .apply_config = talon_fx_apply_config,
        // .set_control = talon_fx_set_control,
        // .set_closed_loop_ramp_period = talon_fx_set_closed_loop_ramp_period,
    };

    return true;
}
