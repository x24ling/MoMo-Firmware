#include "fsu_reset_handler.h"
#include "reset_manager.h"

void register_reset_handlers()
{
    register_reset_handler( kAllResetsBefore, handle_all_resets_before );
    register_reset_handler( kAllResetsAfter, handle_all_resets_after );
    register_reset_handler( kPowerOnReset, handle_poweron_reset );
    register_reset_handler( kMCLRReset, handle_mclr_reset );
}

#include "rtcc.h"
#include "uart.h"
#include "task_manager.h"
#include "scheduler.h"
#include "debug.h"
#include "oscillator.h"
#include "pme.h"

static bool mclr_triggered;
void handle_all_resets_before(unsigned int type)
{
    //Add code here that should be called before all other reset code
    disable_unneeded_peripherals();
    configure_interrupts();
    oscillator_init();
    taskloop_init();
    scheduler_init();

    mclr_triggered = false;
}

void handle_all_resets_after(unsigned int type)
{
    /*
     * Add code that should be called after all other reset code here
     */

    if ( !mclr_triggered ) {
        //taskloop_set_sleep( 1 );
    }
}

void handle_poweron_reset(unsigned int type)
{
    //Power-on reset resets the rtcc, so configure and enable it.
    configure_rtcc();
    enable_rtcc();
}

void handle_mclr_reset(unsigned int type)
{
    mclr_triggered = true;
    debug_init();
}