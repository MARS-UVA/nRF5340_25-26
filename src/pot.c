#include <zephyr/drivers/adc.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(potentiometer, LOG_LEVEL_DBG);

static const struct adc_dt_spec adc_channel = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));

// https://academy.nordicsemi.com/courses/nrf-connect-sdk-intermediate/lessons/lesson-6-analog-to-digital-converter-adc/topic/exercise-1-interfacing-with-adc-using-zephyr-api/

int demo()
{
    if (!adc_is_ready_dt(&adc_channel))
    {
        LOG_ERR("ADC controller device %s not ready", adc_channel.dev->name);
        return 0;
    }

    int err = adc_channel_setup_dt(&adc_channel);
    if (err < 0)
    {
        LOG_ERR("Could not setup channel #%d (%d)", 0, err);
        return 0;
    }

    int16_t buf;
    struct adc_sequence sequence = {
        .buffer = &buf,
        /* buffer size in bytes, not number of samples */
        .buffer_size = sizeof(buf),
        // Optional
        //.calibrate = true,
    };

    err = adc_sequence_init_dt(&adc_channel, &sequence);
    if (err < 0)
    {
        LOG_ERR("Could not initialize sequence");
        return 0;
    }

    err = adc_read(adc_channel.dev, &sequence);
    if (err < 0)
    {
        LOG_ERR("Could not read (%d)", err);
    }

    // err = adc_raw_to_millivolts_dt(&adc_channel, &val_mv);
    // /* conversion to mV may not be supported, skip if not */
    // if (err < 0)
    // {
    //     LOG_WRN(" (value in mV not available)\n");
    // }
    // else
    // {
    //     LOG_INF(" = %d mV", val_mv);
    // }

    return 0;
}