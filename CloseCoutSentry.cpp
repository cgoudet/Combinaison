#include "CloseCoutSentry.h"

#include <cstdio>
#include <unistd.h>

bool CloseCoutSentry::open_ = true;
int  CloseCoutSentry::fdOut_ = 0;
int  CloseCoutSentry::fdErr_ = 0;

CloseCoutSentry::CloseCoutSentry(bool silent) :
    silent_(silent)
{
    if (silent_) {
        if (open_) {
            open_ = false;
            if (fdOut_ == 0 && fdErr_ == 0) {
                fdOut_ = dup(1);
                fdErr_ = dup(2);
            }
            FILE* fr = freopen("/dev/null", "w", stdout);
            fr = freopen("/dev/null", "w", stderr);
        } else {
            silent_ = false;
        }
    }
}

CloseCoutSentry::~CloseCoutSentry()
{
    clear();
}

void CloseCoutSentry::clear()
{
    if (silent_) {
        char buf[50];
        sprintf(buf, "/dev/fd/%d", fdOut_); FILE* fr = freopen(buf, "w", stdout);
        sprintf(buf, "/dev/fd/%d", fdErr_); fr = freopen(buf, "w", stderr);
        open_   = true;
        silent_ = false;
    }
}
