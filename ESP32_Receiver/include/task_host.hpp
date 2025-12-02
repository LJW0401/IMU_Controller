#include <Arduino.h>

namespace task_host
{

typedef struct
{
    bool connected;
    ulong last_update_ms;
} connect_t;

void HostTask(void * pvParameters);
}  // namespace task_host