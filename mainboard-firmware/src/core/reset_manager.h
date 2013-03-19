/*
 * reset_manager.h
 *
 *
 */

//Prototype for callback functions that manage resets
//function is passed an opaque integer handle that corresponds
//to the function's entry in the global reset manager table
//in case the handler wishes to deregister itself (i.e. it's single
//shot)
typedef void (*reset_handler)(unsigned int);

//The maximum number of handlers that can be registered for each
//reset type
#define MAX_RESETS_PER_TYPE 1
#define INVALID_RESET_HANDLE ((unsigned int)0xFFFF)

typedef enum
{
    kPowerOnReset = 0,
    kSleepReset,
    kDeepSleepReset,
    kSoftwareReset,
    kMCLRReset,
    kAllResets,
    kNumResets

} ResetType;

enum
{
    kNoError = 0,
    kInvalidHandle = -1,
    kNoHandler = -2
};

//Functions
unsigned int    register_reset_handler(ResetType type, reset_handler handler);
int             remove_reset_handler(unsigned int handle);

void            handle_reset();
ResetType       get_reset_type();


//Reset Type Handlers
void handle_poweron_reset(unsigned int type);
void handle_all_resets(unsigned int type);

void heartbeat(void);