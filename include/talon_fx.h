#ifndef INC_TALONFX_H_
#define INC_TALONFX_H_

#include "can.h"

typedef enum
{
    TALON_FX_NEUTRAL_COAST = 0,
    TALON_FX_NEUTRAL_BRAKE = 1,
} talon_fx_neutral_mode_t;

typedef struct
{
    double kP;
    double kI;
    double kD;
    double kS;
    double kV;
    double kA;
    double kG;
} talon_fx_slot0_config_t;

typedef struct talon_fx
{
    bool initialized;
    struct device *can_dev;
    int identifier;

    // Function pointers for control actions
    void (*set)(struct talon_fx *self, double value);
    void (*set_neutral_mode)(struct talon_fx *self, talon_fx_neutral_mode_t mode);
    void (*apply_supply_current_limit)(struct talon_fx *self, float limit);
    void (*apply_config)(struct talon_fx *self, const talon_fx_slot0_config_t *config);
    void (*set_control)(struct talon_fx *self, int control_mode, double value);
    void (*set_closed_loop_ramp_period)(struct talon_fx *self, float period_s);
} talon_fx_t;

/**
 * @brief Initializes a TalonFX structure.
 * @return Initialized TalonFX instance.
 */
bool talon_fx_init(talon_fx_t *instance, const struct device *can_dev, int identifier);

#endif /* INC_TALONFX_H_ */
