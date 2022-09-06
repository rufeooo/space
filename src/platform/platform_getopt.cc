
static const char* kEmptyString = "";

EXTERN(int platform_optind = 1);
EXTERN(const char* platform_optarg);

int
platform_getopt(int argc, char* const argv[], const char* optstring)
{
  int ret = -1;
  int idx = platform_optind;
  const char* opt = kEmptyString;

  for (int it = idx; it < argc; ++it) {
    const char* arg = argv[it];
    const char* optiter = optstring;

    if (arg[0] != '-') continue;

    ret = '?';
    for (; *optiter; ++optiter) {
      if (*optiter != arg[1]) continue;

      ret = *optiter;
      int val_param = (optiter[1] == ':') && (it + 1 < argc);
      if (val_param) {
        opt = argv[it + 1];
        it += 1;
      }
      break;
    }

    platform_optind = it + 1;
    goto done;
  }

done:
  platform_optarg = opt;
  return ret;
}
