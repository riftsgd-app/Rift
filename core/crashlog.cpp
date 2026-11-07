#include <signal.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <ucontext.h>
extern "C" int __android_log_print(int prio, const char *tag, const char *fmt, ...);

#if defined(__aarch64__)
#define UC_PC(uc) ((uintptr_t)(uc)->uc_mcontext.pc)
#define UC_LR(uc) ((uintptr_t)(uc)->uc_mcontext.regs[30])
#else
#define UC_PC(uc) ((uintptr_t)(uc)->uc_mcontext.gregs[REG_RIP])
#define UC_LR(uc) ((uintptr_t)0)
#endif

static struct sigaction g_oldSegv;

static void dumpMaps(uintptr_t a, uintptr_t b) {
    FILE *f = fopen("/proc/self/maps", "r");
    if (!f) return;
    char line[512];
    while (fgets(line, sizeof line, f)) {
        unsigned long s = 0, e = 0;
        if (sscanf(line, "%lx-%lx", &s, &e) != 2) continue;
        if ((a >= s && a < e) || (b >= s && b < e)) {
            size_t n = strlen(line);
            if (n && line[n - 1] == '\n') line[n - 1] = 0;
            __android_log_print(6, "Rift", "MAP %s", line);
        }
    }
    fclose(f);
}

static void onSegv(int sig, siginfo_t *si, void *ctx) {
    ucontext_t *uc = (ucontext_t *)ctx;
    uintptr_t pc = UC_PC(uc), lr = UC_LR(uc), fa = (uintptr_t)si->si_addr;
    Dl_info di;
    memset(&di, 0, sizeof di);
    int ok = dladdr((void *)pc, &di);
    __android_log_print(6, "Rift", "CRASH sig=%d code=%d addr=%p pc=%p lr=%p dladdr=%d", sig, si->si_code, (void *)fa, (void *)pc, (void *)lr, ok);
    __android_log_print(6, "Rift", "CRASH lib=%s base=%p off_in_lib=0x%lx sym=%s", di.dli_fname ? di.dli_fname : "?", di.dli_fbase, (unsigned long)(pc - (uintptr_t)di.dli_fbase), di.dli_sname ? di.dli_sname : "?");
    dumpMaps(pc, fa);
    const uint32_t *ip = (const uint32_t *)(pc - 16);
    __android_log_print(6, "Rift", "CODE@pc-16: %08x %08x %08x %08x [%08x] %08x %08x %08x", ip[0], ip[1], ip[2], ip[3], ip[4], ip[5], ip[6], ip[7]);
    const uint32_t *fp = (const uint32_t *)fa;
    __android_log_print(6, "Rift", "BYTES@fault: %08x %08x", fp[0], fp[1]);
    sigaction(SIGSEGV, &g_oldSegv, nullptr);
}

__attribute__((constructor)) static void riftCrashInit() {
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_sigaction = onSegv;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGSEGV, &sa, &g_oldSegv);
    __android_log_print(4, "Rift", "crashlog aktif");
}
