#include "ft_ping.h"

 extern bool g_signal;

void signalHandler(int signum)
{
    (void)signum;
    g_signal = true;
}
