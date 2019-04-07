/* simple app to run under valgrind for testing, etc... */
#include <signal.h>
#include <mikmod.h>

static int quit = 0;
static void my_sighandler (int sig)
{
  (void) sig;
#ifdef SIGBREAK
  signal(SIGBREAK, SIG_DFL);
#endif
  signal(SIGINT, SIG_DFL);
  signal(SIGTERM, SIG_DFL);
  quit = 1;
}

int main(int argc, char **argv)
{
  MODULE *module;
  if (argc < 2) {
    fprintf(stderr, "Usage: ./splay file\n");
    return 1;
  }
  /* initialize MikMod threads */
  MikMod_InitThreads ();
  /* register only the null driver */
  MikMod_RegisterDriver(&drv_nos);
  /* register all the module loaders */
  MikMod_RegisterAllLoaders();
  /* init the library */
  md_mode |= DMODE_SOFT_MUSIC | DMODE_NOISEREDUCTION;
  if (MikMod_Init("")) {
    fprintf(stderr, "Could not initialize sound, reason: %s\n", MikMod_strerror(MikMod_errno));
    return 2;
  }
  /* load module */
  module = Player_Load(argv[1], 64, 0);
  if (module) {
    /* start module */
    printf("Playing %s (%d chn)\n", module->songname, (int) module->numchn);
    module->loop = 0; /* disable in-module loops */
    Player_Start(module);
    /* handle Ctrl-C, etc. */
#ifdef SIGBREAK
    signal(SIGBREAK, my_sighandler);
#endif
    signal(SIGINT, my_sighandler);
    signal(SIGTERM, my_sighandler);
    while (!quit && Player_Active()) {
    /* call update without usleep() or something: we only registered null driver */
      MikMod_Update();
    }
    /* restore signals. */
#ifdef SIGBREAK
    signal(SIGBREAK, SIG_DFL);
#endif
    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    Player_Stop();
    Player_Free(module);
  }
  else {
    fprintf(stderr, "Could not load module, reason: %s\n", MikMod_strerror(MikMod_errno));
  }
  MikMod_Exit();
  return 0;
}
